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
#include <nuttx/rtc.h>

#include <stdio.h>

#include "wiced_platform.h"

#include "platform_config.h"

#ifdef CONFIG_RTC_HIRES
#error "CONFIG_RTC_HIRES should NOT be set for 4390x RTC driver"
#endif

#ifdef CONFIG_RTC_ALARM
#error "CONFIG_RTC_ALARM should NOT be set for 4390x RTC driver"
#endif

#define BCM4390X_RTC_CURRENT_CENTURY (20)
#define NUM_SECONDS_IN_DAY           (60 * 60 * 24)
#define NUM_DAYS_IN_WEEK             (7)

/* Thursday (0-Sunday ... 6-Saturday) */
#define TM_EPOCH_WEEKDAY             (4)
#define TM_BASE_YEAR                 (1900)

volatile bool g_rtc_enabled = false;

static int bcm4390x_rtc_getdatetime( FAR struct tm* tp )
{
    int result;
    wiced_rtc_time_t rtc_time;

    if ( g_rtc_enabled != true )
    {
        result = -EIO;
    }
    else
    {
        if ( wiced_platform_get_rtc_time( &rtc_time ) != WICED_SUCCESS )
        {
            result = -EIO;
        }
        else
        {
            tp->tm_sec = rtc_time.sec;
            tp->tm_min = rtc_time.min;
            tp->tm_hour = rtc_time.hr;
            tp->tm_mday = rtc_time.date;
            tp->tm_mon = rtc_time.month - 1;
            tp->tm_year = ((BCM4390X_RTC_CURRENT_CENTURY * 100) + rtc_time.year) - TM_BASE_YEAR;
#ifdef CONFIG_LIBC_LOCALTIME
            tp->tm_isdst = -1;
#endif

            result = OK;
        }
    }

    return result;
}

static int bcm4390x_rtc_setdatetime( FAR const struct timespec* tp )
{
    int result;
    int tm_wday;
    FAR struct tm time;
    wiced_rtc_time_t rtc_time;

    if ( g_rtc_enabled != true )
    {
        result = -EIO;
    }
    else
    {
        if ( tp->tv_sec < 0 )
        {
            result = -EIO;
        }
        else
        {
            tm_wday = (TM_EPOCH_WEEKDAY + ((tp->tv_sec / NUM_SECONDS_IN_DAY) % NUM_DAYS_IN_WEEK)) % NUM_DAYS_IN_WEEK;

            (void)gmtime_r(&tp->tv_sec, &time);

            if ( time.tm_year < ((BCM4390X_RTC_CURRENT_CENTURY * 100) - TM_BASE_YEAR) )
            {
                result = -EIO;
            }
            else
            {
                rtc_time.sec = time.tm_sec;
                rtc_time.min = time.tm_min;
                rtc_time.hr = time.tm_hour;
                rtc_time.date = time.tm_mday;
                rtc_time.weekday = tm_wday + 1;
                rtc_time.month = time.tm_mon + 1;
                rtc_time.year = (time.tm_year + TM_BASE_YEAR) - (BCM4390X_RTC_CURRENT_CENTURY * 100);

                if ( wiced_platform_set_rtc_time( &rtc_time ) != WICED_SUCCESS )
                {
                    result = -EIO;
                }
                else
                {
                    result = OK;
                }
            }
        }
    }

    return result;
}

int up_rtcinitialize(void)
{
    int result;

    if ( g_rtc_enabled == true )
    {
        result = OK;
    }
    else
    {
        /* Initialize the 4390x RTC block */
        if ( platform_rtc_init() == PLATFORM_SUCCESS )
        {
            g_rtc_enabled = true;
            result = OK;
        }
        else
        {
            g_rtc_enabled = false;
            result = -EIO;
        }
    }

    return result;
}

#ifndef CONFIG_RTC_HIRES
time_t up_rtc_time(void)
{
    FAR time_t time_sec;
    FAR struct tm time;

    if ( bcm4390x_rtc_getdatetime( &time ) != OK )
    {
        time_sec = 0;
    }
    else
    {
        /* Extract RTC time in seconds */
        time_sec = mktime( &time );
    }

    return time_sec;
}
#endif

#ifdef CONFIG_RTC_DATETIME
int up_rtc_getdatetime(FAR struct tm *tp)
{
    return bcm4390x_rtc_getdatetime( tp );
}
#endif

int up_rtc_settime(FAR const struct timespec *tp)
{
    return bcm4390x_rtc_setdatetime( tp );
}
