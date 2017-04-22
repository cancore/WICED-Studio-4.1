#ifndef AVRC_H
#define AVRC_H

/*********************************************************************/
/* Application defined sample data for songs */
/*********************************************************************/

/* Media Attribute types
*/
#define APP_AVRC_MEDIA_ATTR_ID_TITLE                 0x01
#define APP_AVRC_MEDIA_ATTR_ID_ARTIST                0x02
#define APP_AVRC_MEDIA_ATTR_ID_ALBUM                 0x03
#define APP_AVRC_MEDIA_ATTR_ID_TRACK_NUM             0x04
#define APP_AVRC_MEDIA_ATTR_ID_NUM_TRACKS            0x05
#define APP_AVRC_MEDIA_ATTR_ID_GENRE                 0x06
#define APP_AVRC_MEDIA_ATTR_ID_PLAYING_TIME          0x07        /* in miliseconds */
#define APP_AVRC_MAX_NUM_MEDIA_ATTR_ID               7


/* media name definition struct */
typedef struct
{
    /* The media name, name length and character set id. */
    uint8_t              str_len;     /* String len, 255 max */
    uint8_t              *p_str;     /* String value */
} tAPP_AVRC_NAME;

/* Media attribute */
typedef struct
{
    uint8_t             attr_id;        /* Media type   AVRC_MEDIA_ATTR_ID_TITLE, AVRC_MEDIA_ATTR_ID_ARTIST, AVRC_MEDIA_ATTR_ID_ALBUM,
                                                        AVRC_MEDIA_ATTR_ID_TRACK_NUM, AVRC_MEDIA_ATTR_ID_NUM_TRACKS,
                                                        AVRC_MEDIA_ATTR_ID_GENRE, AVRC_MEDIA_ATTR_ID_PLAYING_TIME */
    tAPP_AVRC_NAME      name;           /* Media name  */
} tAPP_AVRC_ATTR_ENTRY;

/* Media (song) info */
typedef struct
{
    uint8_t                   attr_count;     /* The number of attributes in p_attr_list */
    tAPP_AVRC_ATTR_ENTRY      p_attr_list[7]; /* Attribute entry list. */
} tAPP_AVRC_ITEM_MEDIA;


#define APP_AVRC_NUMSONGS                           3
#define APP_AVRC_ATTR_SONG_LEN                      255000
#define APP_AVRC_ATTR_SONG_POS                      120000

/*********************************************************************/
/* Application defined sample data for PLayer setting (repeat, shuffle settings) */
/*********************************************************************/

/* Define the Player Application Settings IDs */
#define APP_AVRC_MAX_APP_ATTR_SIZE                  16
#define APP_AVRC_PLAYER_SETTING_REPEAT              0x02
#define APP_AVRC_PLAYER_SETTING_SHUFFLE             0x03

/* Define the possible values of the Player Application Settings */
#define APP_AVRC_PLAYER_VAL_OFF                     0x01
#define APP_AVRC_PLAYER_VAL_ON                      0x02
#define APP_AVRC_PLAYER_VAL_SINGLE_REPEAT           0x02
#define APP_AVRC_PLAYER_VAL_ALL_REPEAT              0x03
#define APP_AVRC_PLAYER_VAL_GROUP_REPEAT            0x04
#define APP_AVRC_PLAYER_VAL_ALL_SHUFFLE             0x02
#define APP_AVRC_PLAYER_VAL_GROUP_SHUFFLE           0x03
#define APP_AVRC_PLAYER_VAL_ALL_SCAN                0x02
#define APP_AVRC_PLAYER_VAL_GROUP_SCAN              0x03


/* Player Attribute value definition struct */
typedef struct {
    uint8_t                 attr_id; /* attribute type (repeat or shuffle) */
    uint8_t                 num_val; /* number of values possible */
    uint8_t                 vals[APP_AVRC_MAX_APP_ATTR_SIZE]; /* array of values */
    uint8_t                 curr_value; /*current value set by user */
} tAPP_AVRC_META_ATTRIB;

///* Sample Player Attribute value for 'Repeat'*/
//tAPP_AVRC_META_ATTRIB     repeat =
//    {
//    APP_AVRC_PLAYER_SETTING_REPEAT, // attr_id
//    3, // num_val
//    { // vals
//        APP_AVRC_PLAYER_VAL_OFF,
//        APP_AVRC_PLAYER_VAL_SINGLE_REPEAT,
//        APP_AVRC_PLAYER_VAL_ALL_REPEAT,
//    },
//    APP_AVRC_PLAYER_VAL_OFF, // curr_value
//};

///* Sample Player Attribute value for 'Shuffle'*/
//tAPP_AVRC_META_ATTRIB     shuffle =
//    {
//    APP_AVRC_PLAYER_SETTING_SHUFFLE, // attr_id
//    3, // num_val
//    { // vals
//        APP_AVRC_PLAYER_VAL_OFF,
//        APP_AVRC_PLAYER_VAL_ALL_SHUFFLE,
//        APP_AVRC_PLAYER_VAL_GROUP_SHUFFLE,
//    },
//    APP_AVRC_PLAYER_VAL_OFF, // curr_value
//};

/* sample appr value list */
//#define APP_AVRC_ATTR_COUNT 2

//tAPP_AVRC_META_ATTRIB * app_avrc_attr_value_list[] =
//{
//    &repeat,
//    &shuffle
//};

/*********************************************************************/
/* Application defined sample data for play state */
/*********************************************************************/

/* Current play state definitions */
#define APP_AVRC_PLAYSTATE_STOPPED                  0x00    /* Stopped */
#define APP_AVRC_PLAYSTATE_PLAYING                  0x01    /* Playing */
#define APP_AVRC_PLAYSTATE_PAUSED                   0x02    /* Paused  */


/*********************************************************************/

#endif // AVRC_H


