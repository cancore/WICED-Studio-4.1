/*
 * Copyright 2017, Cypress Semiconductor Corporation or a subsidiary of 
 * Cypress Semiconductor Corporation. All Rights Reserved.
 * 
 * This software, associated documentation and materials ("Software"),
 * is owned by Cypress Semiconductor Corporation
 * or one of its subsidiaries ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products. Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdbool.h>
#include <stdio.h>
#include <debug.h>
#include <assert.h>
#include <fcntl.h>

#include <pthread.h>
#include <mqueue.h>

#include <nuttx/kmalloc.h>
#include <nuttx/wqueue.h>
#include <nuttx/fs/fs.h>
#include <nuttx/fs/ioctl.h>
#include <nuttx/audio/audio.h>

#include <wiced.h>
#include "wiced_platform.h"
#include <wiced_audio.h>
#include <platform.h>

#define PERIOD_SIZE                 (1 * 1024)
#define NUM_PERIODS                 4
#define BUFFER_SIZE                 WICED_AUDIO_BUFFER_ARRAY_DIM_SIZEOF(NUM_PERIODS, PERIOD_SIZE)
#define SAMPLE_FREQUENCY_IN_HZ      (44100)

#ifdef CONFIG_AUDIO_MULTI_SESSION
#define NUM_SESSIONS                (2) /* TX and RX */
#else
#define NUM_SESSIONS                (1) /* TX or RX */
#endif

static int      bcm4390x_audio_getcaps(FAR struct audio_lowerhalf_s *dev, int type,
                                       FAR struct audio_caps_s *caps);
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int      bcm4390x_audio_configure(FAR struct audio_lowerhalf_s *dev,
                                         FAR void *session,
                                         FAR const struct audio_caps_s *caps);
#else
static int      bcm4390x_audio_configure(FAR struct audio_lowerhalf_s *dev,
                                         FAR const struct audio_caps_s *caps);
#endif
static int      bcm4390x_audio_shutdown(FAR struct audio_lowerhalf_s *dev);
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int      bcm4390x_audio_start(FAR struct audio_lowerhalf_s *dev,
                                     FAR void *session);
#else
static int      bcm4390x_audio_start(FAR struct audio_lowerhalf_s *dev);
#endif

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int      bcm4390x_audio_stop(FAR struct audio_lowerhalf_s *dev,
                                    FAR void *session);
#else
static int      bcm4390x_audio_stop(FAR struct audio_lowerhalf_s *dev);
#endif
#endif
#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int      bcm4390x_audio_pause(FAR struct audio_lowerhalf_s *dev,
                                     FAR void* session);
static int      bcm4390x_audio_resume(FAR struct audio_lowerhalf_s *dev,
                                      FAR void* session);
#else
static int      bcm4390x_audio_pause(FAR struct audio_lowerhalf_s *dev);
static int      bcm4390x_audio_resume(FAR struct audio_lowerhalf_s *dev);
#endif
#endif
static int      bcm4390x_audio_allocbuffer(FAR struct audio_lowerhalf_s *dev,
                                           FAR struct audio_buf_desc_s *apb);
static int      bcm4390x_audio_freebuffer(FAR struct audio_lowerhalf_s *dev,
                                          FAR struct audio_buf_desc_s *apb);
static int      bcm4390x_audio_enqueuebuffer(FAR struct audio_lowerhalf_s *dev,
                                             FAR struct ap_buffer_s *apb);
static int      bcm4390x_audio_cancelbuffer(FAR struct audio_lowerhalf_s *dev,
                  FAR struct ap_buffer_s *apb);
static int      bcm4390x_audio_ioctl(FAR struct audio_lowerhalf_s *dev, int cmd,
                  unsigned long arg);
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int      bcm4390x_audio_reserve(FAR struct audio_lowerhalf_s *dev,
                                       FAR void **session);
static int      bcm4390x_audio_release(FAR struct audio_lowerhalf_s *dev,
                                       FAR void **session);
#else
static int      bcm4390x_audio_reserve(FAR struct audio_lowerhalf_s *dev);
static int      bcm4390x_audio_release(FAR struct audio_lowerhalf_s *dev);
#endif
static void    *bcm4390x_audio_workerthread(pthread_addr_t pvarg);

struct audio_lowerhalf_s* bcm4390x_audio_lowerhalf_init(void);
void bcm4390x_audio_lowerhalf_deinit(struct audio_lowerhalf_s *dev);

struct bcm4390x_audio_dev_s;

typedef struct
{
  struct bcm4390x_audio_dev_s *priv;        /* Pointer back to main device */

  mqd_t                   mq;               /* Message queue for receiving messages */
  char                    mqname[16];       /* Our message queue name */
  pthread_t               threadid;         /* ID of our thread */
  uint32_t                bitrate;          /* Actual programmed bit rate */
  uint16_t                samprate;         /* Configured samprate (samples/sec) */
#ifndef CONFIG_AUDIO_EXCLUDE_VOLUME
#ifndef CONFIG_AUDIO_EXCLUDE_BALANCE
  uint16_t                balance;          /* Current balance level (b16) */
#endif  /* CONFIG_AUDIO_EXCLUDE_BALANCE */
  uint8_t                 volume;           /* Current volume level {0..63} */
#endif  /* CONFIG_AUDIO_EXCLUDE_VOLUME */
  uint8_t                 nchannels;        /* Number of channels (1 or 2) */
  uint8_t                 bpsamp;           /* Bits per sample (8 or 16) */
  bool                    running;          /* True: Worker thread is running */
  bool                    paused;           /* True: Playing is paused */
  bool                    mute;             /* True: Output is muted */
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  bool                    terminating;      /* True: Stop requested */
#endif
  bool                    reserved;         /* True: Session is reserved */
  bool                    configured;       /* True: Session is configured */
  wiced_audio_session_ref wiced;
  struct ap_buffer_s      apb[NUM_PERIODS];
  uint32_t                apb_index;
  uint8_t                *buf;              /* Dynamically allocated buffer */
} bcm4390x_session_t;

struct bcm4390x_audio_dev_s
{
  struct audio_lowerhalf_s dev;                 /* bcm4390x audio lower half (this device) */
  /* Ensure that audio_lowerhaf_s is always the first field of this structure */

  /* Our specific driver data goes here */

  bool                     initialized;         /* True: Platform audio initialized */
  sem_t                    sem;
  bcm4390x_session_t       sess[NUM_SESSIONS];  /* Sessions */
};

static const struct audio_ops_s g_audioops =
{
  .getcaps          = bcm4390x_audio_getcaps,       /* getcaps        */
  .configure        = bcm4390x_audio_configure,     /* configure      */
  .shutdown         = bcm4390x_audio_shutdown,      /* shutdown       */
  .start            = bcm4390x_audio_start,         /* start          */
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  .stop             = bcm4390x_audio_stop,          /* stop           */
#endif
#ifndef CONFIG_AUDIO_EXCLUDE_PAUSE_RESUME
  .pause            = bcm4390x_audio_pause,         /* pause          */
  .resume           = bcm4390x_audio_resume,        /* resume         */
#endif
  .allocbuffer      = bcm4390x_audio_allocbuffer,   /* allocbuffer    */
  .freebuffer       = bcm4390x_audio_freebuffer,    /* freebuffer     */
  .enqueuebuffer    = bcm4390x_audio_enqueuebuffer, /* enqueue_buffer */
  .cancelbuffer     = bcm4390x_audio_cancelbuffer,  /* cancel_buffer  */
  .ioctl            = bcm4390x_audio_ioctl,         /* ioctl          */
  .read             = NULL,                         /* read           */
  .write            = NULL,                         /* write          */
  .reserve          = bcm4390x_audio_reserve,       /* reserve        */
  .release          = bcm4390x_audio_release        /* release        */
};

typedef struct
{
    /* Audio ring buffer. */
    wiced_audio_buffer_header_t *buffer;
    /* Start 0-index into the period. */
    uint16_t head;
    /* Bytes pending playback/capture transfer..
     * The inverse represents the bytes available to write (playback)
     * or read (captured).
    */
    uint16_t count;
    /* Total number of audio bytes in the buffer. */
    uint16_t length;
} audio_buffer_t;

struct wiced_audio_session_t
{
    int                             i2s_id;
    int                             i2s_direction;
    wiced_bool_t                    i2s_running;
    wiced_semaphore_t               available_periods;
    uint16_t                        num_periods_requested;
    uint8_t                         frame_size;
    audio_buffer_t                  audio_buffer;
    uint16_t                        period_size;
    wiced_audio_device_interface_t* audio_dev;
    wiced_mutex_t                   session_lock;
    wiced_bool_t                    underrun_occurred;
    wiced_bool_t                    is_initialised;
};

static wiced_audio_config_t wiced_audio_default_config =
{
    .sample_rate        = SAMPLE_FREQUENCY_IN_HZ,
    .channels           = 2,
    .bits_per_sample    = 16,
    .frame_size         = 4,
};

int bcm4390x_audio_initialize(void)
{
  struct audio_lowerhalf_s *bcm4390x_audio_lowerhalf;
  static bool initialized = false;
  char devname[20];
  int ret;

  if(!initialized)
  {
    bcm4390x_audio_lowerhalf = bcm4390x_audio_lowerhalf_init();
    if (NULL == bcm4390x_audio_lowerhalf)
    {
      auddbg("ERROR: bcm4390x_audio_lowerhalf_init() failed");
      ret = -ENODEV;
      goto audio_lowerhalf_init_failed;
    }

    /* Register with /dev/ interface */
    snprintf(devname, 20, "bcm4390x_audio");
    ret = audio_register(devname, bcm4390x_audio_lowerhalf);
    if (ret < 0)
      {
        auddbg("ERROR: Failed to register /dev/%s device: %d\n", devname, ret);
        ret = ERROR;
        goto audio_register_failed;
      }

    /* Now we are initialized */
    initialized = true;
  }

  return OK;

audio_register_failed:
audio_lowerhalf_init_failed:
  DEBUGASSERT(FALSE);
  return ret;
}

struct audio_lowerhalf_s* bcm4390x_audio_lowerhalf_init(void)
{
  struct bcm4390x_audio_dev_s *priv = NULL;

  priv = (struct bcm4390x_audio_dev_s*)kmm_zalloc(sizeof(struct bcm4390x_audio_dev_s));
  if (priv == NULL)
  {
      return NULL;
  }
  else
  {
    priv->dev.ops = &g_audioops;
    sem_init(&priv->sem, 0, 1);
    return &priv->dev;
  }
}

void bcm4390x_audio_lowerhalf_deinit(struct audio_lowerhalf_s *dev)
{
  struct bcm4390x_audio_dev_s *priv = (struct bcm4390x_audio_dev_s *)dev;
  if (priv != NULL)
  {
    kmm_free(priv);
    priv = NULL;
  }
}

static void bcm4390x_sem_wait(sem_t *sem)
{
  int ret;

  do
    {
      ret = sem_wait(sem);
      DEBUGASSERT(ret == 0 || errno == EINTR);
    }
  while (ret < 0);
}

/****************************************************************************
 * Name: bcm4390x_audio_getcaps
 *
 * Description:
 *   Get the audio device capabilities
 *
 ****************************************************************************/

static int bcm4390x_audio_getcaps(FAR struct audio_lowerhalf_s *dev, int type,
                          FAR struct audio_caps_s *caps)
{
  /* Validate the structure */

  DEBUGASSERT(caps && (caps->ac_len == sizeof(struct audio_caps_s)));

  /* Fill in the caller's structure based on requested info */

  caps->ac_format.hw  = 0;
  caps->ac_controls.w = 0;

  switch (caps->ac_type)
  {
      /* Caller is querying for the types of units we support */

      case AUDIO_TYPE_QUERY:

        /* Provide our overall capabilities.  The interfacing software
         * must then call us back for specific info for each capability.
         */

        caps->ac_channels = 2;       /* Stereo output */

        switch (caps->ac_subtype)
        {
            case AUDIO_TYPE_QUERY:
              /* We don't decode any formats!  Only something above us in
               * the audio stream can perform decoding on our behalf.
               */

              /* The types of audio units we implement */

              caps->ac_controls.b[0] = AUDIO_TYPE_OUTPUT | AUDIO_TYPE_FEATURE |
                                     AUDIO_TYPE_PROCESSING;

              break;

            case AUDIO_FMT_MIDI:
              /* We only support Format 0 */

              caps->ac_controls.b[0] = AUDIO_SUBFMT_END;
              break;

            default:
              caps->ac_controls.b[0] = AUDIO_SUBFMT_END;
              break;
        }

        break;

      /* Provide capabilities of our OUTPUT unit */

      case AUDIO_TYPE_OUTPUT:

        caps->ac_channels = 2;

        switch (caps->ac_subtype)
        {
            case AUDIO_TYPE_QUERY:

              /* Report the Sample rates we support */

              caps->ac_controls.b[0] = AUDIO_SAMP_RATE_8K | AUDIO_SAMP_RATE_11K |
                                       AUDIO_SAMP_RATE_16K | AUDIO_SAMP_RATE_22K |
                                       AUDIO_SAMP_RATE_32K | AUDIO_SAMP_RATE_44K |
                                       AUDIO_SAMP_RATE_48K;
              break;

            case AUDIO_FMT_MP3:
            case AUDIO_FMT_WMA:
            case AUDIO_FMT_PCM:
              break;

            default:
              break;
        }

        break;

      /* Provide capabilities of our INPUT unit */

      case AUDIO_TYPE_INPUT:

        caps->ac_channels = 2;

        switch (caps->ac_subtype)
        {
            case AUDIO_TYPE_QUERY:

              /* Report the Sample rates we support */

              caps->ac_controls.b[0] = AUDIO_SAMP_RATE_8K | AUDIO_SAMP_RATE_11K |
                                       AUDIO_SAMP_RATE_16K | AUDIO_SAMP_RATE_22K |
                                       AUDIO_SAMP_RATE_32K | AUDIO_SAMP_RATE_44K |
                                       AUDIO_SAMP_RATE_48K;
              break;

            case AUDIO_FMT_MP3:
            case AUDIO_FMT_WMA:
            case AUDIO_FMT_PCM:
              break;

            default:
              break;
        }

        break;

      /* Provide capabilities of our FEATURE units */

      case AUDIO_TYPE_FEATURE:

        /* If the sub-type is UNDEF, then report the Feature Units we support */

        if (caps->ac_subtype == AUDIO_FU_UNDEF)
        {
            /* Fill in the ac_controls section with the Feature Units we have */

            caps->ac_controls.b[0] = AUDIO_FU_VOLUME | AUDIO_FU_BASS | AUDIO_FU_TREBLE;
            caps->ac_controls.b[1] = AUDIO_FU_BALANCE >> 8;
        }
        else
        {
            /* TODO:  Do we need to provide specific info for the Feature Units,
             * such as volume setting ranges, etc.?
             */
        }

        break;

      /* Provide capabilities of our PROCESSING unit */

      case AUDIO_TYPE_PROCESSING:

        switch (caps->ac_subtype)
        {
              default:
              break;
        }

        break;

      /* All others we don't support */

      default:

        /* Zero out the fields to indicate no support */

        caps->ac_subtype = 0;
        caps->ac_channels = 0;

        break;
  }
  /* Return the length of the audio_caps_s struct for validation of
   * proper Audio device type.
   */

  return caps->ac_len;
}

static int bcm4390x_audio_session_configure(FAR bcm4390x_session_t *sess,
                                            FAR platform_audio_device_id_t dev_id,
                                            FAR wiced_audio_config_t *config)
{
  int ret;

  if (sess->configured)
  {
    auddbg("ERROR: already configured\n");
    return -EPERM;
  }

  if (wiced_audio_init(dev_id, &sess->wiced, PERIOD_SIZE) != WICED_SUCCESS)
  {
    auddbg("ERROR: wiced_audio_init() failed");
    ret = -ENODEV;
    goto out;
  }

  if (sess->buf == NULL)
  {
    /* Allocate Audio Buffer */
    sess->buf = malloc(BUFFER_SIZE);
    if (sess->buf == NULL)
    {
      auddbg("ERROR: malloc failed");
      ret = -ENOMEM;
      goto err_deinit;
    }
  }

  if (wiced_audio_create_buffer(sess->wiced, BUFFER_SIZE, sess->buf,
                                NULL) != WICED_SUCCESS)
  {
    auddbg("ERROR: wiced_audio_create_buffer failed");
    ret = -ENOMEM;
    goto err_free;
  }

  /* Configure Session */
  if (wiced_audio_configure(sess->wiced, config) != WICED_SUCCESS)
  {
    auddbg("ERROR: wiced_audio_configure failed");
    ret = -ERROR;
    goto err_free;
  }

  sess->configured = true;

  return OK;

err_free:
  free(sess->buf);
  sess->buf = NULL;
err_deinit:
  wiced_audio_deinit(sess->wiced);
out:
  return ret;
}

#ifdef CONFIG_AUDIO_MULTI_SESSION
static int bcm4390x_audio_configure(FAR struct audio_lowerhalf_s *dev,
                                    FAR void *session,
                                    FAR const struct audio_caps_s *caps)
#else
static int bcm4390x_audio_configure(FAR struct audio_lowerhalf_s *dev,
                                    FAR const struct audio_caps_s *caps)
#endif
{
  FAR struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *)dev;
  int ret = OK;
  bcm4390x_session_t *sess = &priv->sess[0];

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (session == NULL)
  {
    return -EINVAL;
  }

  sess = session;
#endif

  DEBUGASSERT(priv && sess && caps);

  /* Process the configure operation */

  switch (caps->ac_type)
    {
    case AUDIO_TYPE_FEATURE:
      //audvdbg("  AUDIO_TYPE_FEATURE\n");

      /* Process based on Feature Unit */

      switch (caps->ac_format.hw)
        {
        case AUDIO_FU_VOLUME:
          {
            /* Set the volume */
            double min_volume, max_volume, new_volume;

            wiced_audio_get_volume_range(sess->wiced, &min_volume, &max_volume);

            /* Convert the linear volume (0-1000) into dB */
            new_volume = (((double)caps->ac_controls.hw[0] / 1000.0) *
                          (max_volume - min_volume)) + min_volume;

            wiced_audio_set_volume(sess->wiced, new_volume);
          }
          break;

        default:
          auddbg("    Unrecognized feature unit\n");
          ret = -ENOTTY;
          break;
        }
        break;

    case AUDIO_TYPE_OUTPUT:
      {
        if (sess->configured)
        {
          /* Cannot configure more than once */
          ret = -EPERM;
          break;
        }

        wiced_audio_config_t config;
        platform_audio_device_id_t dev_id;

        config.sample_rate     = caps->ac_controls.hw[0];
        config.channels        = caps->ac_channels;
        config.bits_per_sample = caps->ac_controls.b[2];
        config.frame_size      = (config.channels * config.bits_per_sample)/8;

        dev_id = caps->ac_format.hw;

        /* Save the current stream configuration */
        sess->samprate  = caps->ac_controls.hw[0];
        sess->nchannels = caps->ac_channels;
        sess->bpsamp    = caps->ac_controls.b[2];

        ret = bcm4390x_audio_session_configure(sess, dev_id, &config);
      }
      break;

    case AUDIO_TYPE_PROCESSING:
      break;
    }

  return ret;
}

/****************************************************************************
 * Name: bcm4390x_audio_start
 *
 * Description:
 *   Start the configured operation (audio streaming, volume enabled, etc.).
 *
 ****************************************************************************/

#ifdef CONFIG_AUDIO_MULTI_SESSION
static int bcm4390x_audio_start(FAR struct audio_lowerhalf_s *dev,
                                FAR void *session)
#else
static int bcm4390x_audio_start(FAR struct audio_lowerhalf_s *dev)
#endif
{
  FAR struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *)dev;
  bcm4390x_session_t *sess = &priv->sess[0];
  struct sched_param sparam;
  struct mq_attr attr;
  pthread_attr_t tattr;
  FAR void *value;
  int ret;

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (session == NULL)
  {
    return -EINVAL;
  }

  sess = session;
#endif

  if (!sess->reserved)
  {
    return -EPERM;
  }

  if (!sess->configured)
  {
    if (bcm4390x_audio_session_configure(sess, PLATFORM_DEFAULT_AUDIO_OUTPUT,
                                         &wiced_audio_default_config) != OK)
    {
      return -EINVAL;
    }
  }

  /* Create a message queue for the worker thread */
  snprintf(sess->mqname, sizeof(sess->mqname), "/tmp/%X", sess);

  attr.mq_maxmsg  = 16;
  attr.mq_msgsize = sizeof(struct audio_msg_s);
  attr.mq_curmsgs = 0;
  attr.mq_flags   = 0;

  sess->mq = mq_open(sess->mqname, O_RDWR | O_CREAT, 0644, &attr);
  if (sess->mq == (mqd_t)-1)
    {
      /* Error creating message queue! */

      auddbg("ERROR: Couldn't allocate message queue\n");
      return -ENOMEM;
    }

  /* Join any old worker thread we had created to prevent a memory leak */

  if (sess->threadid != 0)
    {
      audvdbg("Joining old thread\n");
      pthread_join(sess->threadid, &value);
    }

  /* Start our thread for sending data to the device */

  pthread_attr_init(&tattr);
  sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
  (void)pthread_attr_setschedparam(&tattr, &sparam);
  (void)pthread_attr_setstacksize(&tattr, 1536);

  ret = pthread_create(&sess->threadid, &tattr, bcm4390x_audio_workerthread,
                       (pthread_addr_t)sess);
  if (ret != OK)
    {
      auddbg("ERROR: pthread_create failed: %d\n", ret);
    }
  else
    {
      pthread_setname_np(sess->threadid, "bcm4390x_audio");
    }

  return ret;
}

/****************************************************************************
 * Name: bcm4390x_audio_stop
 *
 * Description: Stop the configured operation (audio streaming, volume
 *              disabled, etc.).
 *
 ****************************************************************************/

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
#ifdef CONFIG_AUDIO_MULTI_SESSION
static int bcm4390x_audio_stop(FAR struct audio_lowerhalf_s *dev,
                               FAR void *session)
#else
static int bcm4390x_audio_stop(FAR struct audio_lowerhalf_s *dev)
#endif
{
  struct bcm4390x_audio_dev_s *priv = (struct bcm4390x_audio_dev_s *)dev;
  struct audio_msg_s term_msg;
  void *value;
  bcm4390x_session_t *sess = &priv->sess[0];

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (session == NULL)
  {
    return -EINVAL;
  }

  sess = session;
#endif

  /* Send a message to stop all audio streaming */

  term_msg.msgId = AUDIO_MSG_STOP;
  term_msg.u.data = 0;
  mq_send(sess->mq, (FAR const char *)&term_msg, sizeof(term_msg), 1);

  /* Join the worker thread */

  pthread_join(sess->threadid, &value);
  sess->threadid = 0;

  /* Enter into a reduced power usage mode */
  /* REVISIT: */

  return OK;
}
#endif

static int bcm4390x_audio_shutdown(FAR struct audio_lowerhalf_s *dev)
{
  /* To be implemented */
  return OK;
}

static int bcm4390x_audio_allocbuffer(FAR struct audio_lowerhalf_s *dev,
                                      FAR struct audio_buf_desc_s *apb)
{
  int count;
  struct bcm4390x_audio_dev_s *priv = (struct bcm4390x_audio_dev_s *)dev;
  wiced_audio_buffer_header_t *buffer_ptr = NULL;
  struct ap_buffer_s *apb_ptr;
  bcm4390x_session_t *sess = &priv->sess[0];

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (apb->session == NULL)
  {
    return -EINVAL;
  }

  sess = apb->session;
#endif

  if (!sess->configured)
  {
    if (bcm4390x_audio_session_configure(sess, PLATFORM_DEFAULT_AUDIO_OUTPUT,
                                         &wiced_audio_default_config) != OK)
    {
      auddbg("ERROR: bcm4390x_audio_session_configure() failed\n");
      return -EINVAL;
    }
  }

  if (sess->apb_index > (sizeof(sess->apb) / sizeof(struct ap_buffer_s)))
  {
    return -ENOMEM;
  }

  apb_ptr = &(sess->apb[sess->apb_index]);

  buffer_ptr = sess->wiced->audio_buffer.buffer;
  count = sess->apb_index;

  while( count != 0)
  {
    buffer_ptr = buffer_ptr->next;
    count--;
  }

  apb_ptr->samp = buffer_ptr->data_start;
  apb_ptr->nmaxbytes = buffer_ptr->data_end - buffer_ptr->data_start;

  *apb->u.ppBuffer = apb_ptr;
  sess->apb_index++;

  return sizeof(struct audio_buf_desc_s);
}

static int bcm4390x_audio_freebuffer(FAR struct audio_lowerhalf_s *dev,
                                     FAR struct audio_buf_desc_s *apb)
{
  struct bcm4390x_audio_dev_s *priv = (struct bcm4390x_audio_dev_s *)dev;
  bcm4390x_session_t *sess = &priv->sess[0];

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (apb->session == NULL)
  {
    return -EINVAL;
  }

  sess = apb->session;
#endif

  sess->apb_index--;

  return OK;
}


/****************************************************************************
 * Name: bcm4390x_audio__enqueuebuffer
 *
 * Description: Enqueue an Audio Pipeline Buffer for playback/ processing.
 *
 ****************************************************************************/

static int bcm4390x_audio_enqueuebuffer(FAR struct audio_lowerhalf_s *dev,
                                        FAR struct ap_buffer_s *apb)
{

  int ret = OK;
  wiced_result_t result;
  FAR struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *)dev;
  struct audio_msg_s  term_msg;
  bcm4390x_session_t *sess = &priv->sess[0];

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (apb->session = NULL)
  {
    return -EINVAL;
  }

  sess = apb->session;
#endif

  result = wiced_audio_release_buffer(sess->wiced, PERIOD_SIZE);
  if (result != WICED_SUCCESS)
  {
    auddbg("wiced_audio_release_buffer_failed\n");
    DEBUGASSERT(FALSE);
    return ERROR;
  }

  /* Send a message to the worker thread indicating that a new buffer has been
   * enqueued.  If mq is -1, then the playing has not yet started.  In that
   * case we are just "priming the pump" and we don't need to send any message.
   */

  ret = OK;
  if (sess->mq != (mqd_t)-1)
    {
      term_msg.msgId  = AUDIO_MSG_ENQUEUE;
      term_msg.u.data = 0;

      ret = mq_send(sess->mq, (FAR const char *)&term_msg, sizeof(term_msg),
                    1);
      if (ret < 0)
        {
          int errcode = errno;
          DEBUGASSERT(errcode > 0);

          auddbg("ERROR: mq_send failed: %d\n", errcode);
          UNUSED(errcode);
        }
    }

  return ret;
}

/****************************************************************************
 * Name: bcm4390x_audio_cancelbuffer
 *
 * Description: Called when an enqueued buffer is being cancelled.
 *
 ****************************************************************************/

static int bcm4390x_audio_cancelbuffer(FAR struct audio_lowerhalf_s *dev,
                               FAR struct ap_buffer_s *apb)
{
  return OK;
}

static int bcm4390x_audio_ioctl(FAR struct audio_lowerhalf_s *dev, int cmd,
                        unsigned long arg)
{
  struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *) dev;

  switch (cmd)
  {
#ifdef CONFIG_AUDIO_DRIVER_SPECIFIC_BUFFERS
    case AUDIOIOC_GETBUFFERINFO:
    {
      struct ap_buffer_info_s *bufinfo;
      bufinfo              = (struct ap_buffer_info_s *) arg;
      bufinfo->buffer_size = PERIOD_SIZE;
      bufinfo->nbuffers    = NUM_PERIODS;

      break;
    }
#endif
    case AUDIOIOC_GETLATENCY:
    {
      struct audio_caps_desc_s *caps_desc = (struct audio_caps_desc_s *) arg;
      bcm4390x_session_t *sess = &priv->sess[0];

      if (caps_desc == NULL)
      {
        return -EINVAL;
      }

#ifdef CONFIG_AUDIO_MULTI_SESSION
      if (caps_desc->session == NULL)
      {
        return -EINVAL;
      }

      sess = caps_desc->session;
#endif

      if (sess->wiced == NULL)
      {
        return -EPIPE;
      }

      if (wiced_audio_get_latency(sess->wiced, &caps_desc->caps.ac_controls.w) != WICED_SUCCESS)
      {
        return -EAGAIN;
      }

      break;
    }
    case AUDIOIOC_GETBUFFERWEIGHT:
    {
      struct audio_caps_desc_s *caps_desc = (struct audio_caps_desc_s *) arg;
      bcm4390x_session_t *sess = &priv->sess[0];

      if (caps_desc == NULL)
      {
        return -EINVAL;
      }

#ifdef CONFIG_AUDIO_MULTI_SESSION
      if (caps_desc->session == NULL)
      {
        return -EINVAL;
      }

      sess = caps_desc->session;
#endif

      if (sess->wiced == NULL)
      {
        return -EPIPE;
      }

      if (wiced_audio_get_current_buffer_weight(sess->wiced, &caps_desc->caps.ac_controls.w) != WICED_SUCCESS)
      {
        return -EAGAIN;
      }

      break;
    }
    default:
      break;
  }

  return OK;
}

#ifdef CONFIG_AUDIO_MULTI_SESSION
static int bcm4390x_audio_reserve(FAR struct audio_lowerhalf_s *dev,
                                  FAR void **session)
#else
static int bcm4390x_audio_reserve(FAR struct audio_lowerhalf_s *dev)
#endif
{
  struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *) dev;
  int i;
  int ret = OK;

  bcm4390x_sem_wait(&priv->sem);

  if (!priv->initialized)
  {
    if (platform_init_audio() != WICED_SUCCESS)
    {
      return -ENODEV;
    }

    priv->initialized = true;
  }

  for (i = 0; i < NUM_SESSIONS; i++)
  {
    if (!priv->sess[i].reserved)
    {
      priv->sess[i].priv = priv;
      priv->sess[i].reserved = true;
      priv->sess[i].configured = false;
      priv->sess[i].mq = (mqd_t)-1;
      priv->sess[i].running = false;
      priv->sess[i].paused = false;
      priv->sess[i].mute = false;
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
      priv->sess[i].terminating = false;
#endif
#ifdef CONFIG_AUDIO_MULTI_SESSION
      *session = &priv->sess[i];
#endif
      break;
    }
  }

  if (i == NUM_SESSIONS)
  {
    /* No more sessions available */
    ret = -ENODEV;
  }

  sem_post(&priv->sem);

  return ret;
}

#ifdef CONFIG_AUDIO_MULTI_SESSION
static int bcm4390x_audio_release(FAR struct audio_lowerhalf_s *dev,
                                  FAR void **session)
#else
static int bcm4390x_audio_release(FAR struct audio_lowerhalf_s *dev)
#endif
{
  struct bcm4390x_audio_dev_s *priv = (FAR struct bcm4390x_audio_dev_s *) dev;
  bcm4390x_session_t *sess = &priv->sess[0];
  int ret = OK;

#ifdef CONFIG_AUDIO_MULTI_SESSION
  if (session == NULL || *session == NULL)
  {
    return -EINVAL;
  }

  sess = (bcm4390x_session_t *)*session;
  *session = NULL;
#endif

  bcm4390x_sem_wait(&priv->sem);
  sess->reserved = false;

  if (sess->configured)
  {
    if ( wiced_audio_deinit( sess->wiced ) != WICED_SUCCESS )
    {
      auddbg("ERROR: wiced_audio_deinit() failed");
    }
  }

  sess->wiced = NULL;

  sem_post(&priv->sem);

  return ret;

}

static int bcm4390x_audio_wait_for_send_complete(bcm4390x_session_t *sess)
{
  struct bcm4390x_audio_dev_s *priv = sess->priv;
  wiced_result_t result;
  uint16_t avail = PERIOD_SIZE;
  uint8_t *buf;
  struct ap_buffer_s *apb = &(sess->apb[0]);

  result = wiced_audio_wait_buffer(sess->wiced, PERIOD_SIZE, 1000);
  if (result != WICED_SUCCESS)
  {
    auddbg("ERROR: Timed out waiting for audio buffer\n");
    DEBUGASSERT(FALSE);
    return ERROR;
  }

  result = wiced_audio_get_buffer(sess->wiced, &buf, &avail);
  DEBUGASSERT(avail == PERIOD_SIZE);
  if (result != WICED_SUCCESS)
  {
    auddbg("ERROR: wiced_audio_get_buffer() returned error\n");
    DEBUGASSERT(FALSE);
    return ERROR;
  }

  apb->nmaxbytes = avail;
  apb->samp = buf;

  priv->dev.upper(priv->dev.priv, AUDIO_CALLBACK_DEQUEUE, apb, OK);

  return OK;
}

static void *bcm4390x_audio_workerthread(pthread_addr_t pvarg)
{
  bcm4390x_session_t *sess = (bcm4390x_session_t *) pvarg;
  FAR struct bcm4390x_audio_dev_s *priv = sess->priv;
  struct audio_msg_s msg;
  int msglen;
  int prio;

  //audvdbg("Entry\n");

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  sess->terminating = false;
#endif

  if (WICED_SUCCESS != wiced_audio_start(sess->wiced))
  {
      auddbg("ERROR: wiced_audio_start() failed\n");
      return NULL;

  }

  sess->running = true;

  /* Loop as long as we are supposed to be running and as long as we have
   * buffers in-flight.
   */
  while (sess->running)
    {

      bcm4390x_audio_wait_for_send_complete(sess);

      /* Check if we have been asked to terminate.  We have to check if we
       * still have buffers in-flight.  If we do, then we can't stop until
       * birds come back to roost.
       */
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
      if (sess->terminating)
        {
          /* We are IDLE.  Break out of the loop and exit. */

          break;
        }
#endif

      /* Wait for messages from our message queue */
      msglen = mq_receive(sess->mq, (FAR char *)&msg, sizeof(msg), &prio);

      /* Handle the case when we return with no message */
      if (msglen < sizeof(struct audio_msg_s))
        {
          auddbg("ERROR: Message too small: %d\n", msglen);
          continue;
        }

      /* Process the message */
      switch (msg.msgId)
        {
          case AUDIO_MSG_DATA_REQUEST:
            //audvdbg("AUDIO_MSG_DATA_REQUEST\n");
            break;

          /* Stop the playback */
#ifndef CONFIG_AUDIO_EXCLUDE_STOP
          case AUDIO_MSG_STOP:
            /* Indicate that we are terminating */

            audvdbg("AUDIO_MSG_STOP: Terminating\n");
            sess->terminating = true;
            break;
#endif

          case AUDIO_MSG_ENQUEUE:
            break;

          case AUDIO_MSG_COMPLETE:
            break;

          default:
            audvdbg("ERROR: Ignoring message ID %d\n", msg.msgId);
            break;
        }

    }

#ifndef CONFIG_AUDIO_EXCLUDE_STOP
  wiced_audio_stop(sess->wiced);
#endif

  mq_close(sess->mq);
  mq_unlink(sess->mqname);
  sess->mq = (mqd_t)-1;

  /* Send an AUDIO_MSG_COMPLETE message to the client */
  priv->dev.upper(priv->dev.priv, AUDIO_CALLBACK_COMPLETE, NULL, OK);

  audvdbg("Exit\n");
  return NULL;
}
