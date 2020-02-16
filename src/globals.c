/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
 *  ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
 *  ROM has been brought to you by the ROM consortium                      *
 *      Russ Taylor (rtaylor@hypercube.org)                                *
 *      Gabrielle Taylor (gtaylor@hypercube.org)                           *
 *      Brian Moore (zump@rom.org)                                         *
 *  By using this code, you have agreed to follow the terms of the         *
 *  ROM license, in the file Rom24/doc/rom.license                         *
 ***************************************************************************/

#include "globals.h"

HELP_T       *help_first,   *help_last;
SHOP_T       *shop_first,   *shop_last;
AREA_T       *area_first,   *area_last;
BAN_T        *ban_first,    *ban_last;
HELP_AREA_T  *had_first,    *had_last;
ANUM_T       *anum_first,   *anum_last;
MPROG_CODE_T *mpcode_first, *mpcode_last;
OBJ_T        *object_first, *object_last;
CHAR_T       *char_first,   *char_last;

int newmob_count = 0;
int newobj_count = 0;
int mobile_count = 0;

char *help_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];
KILL_T kill_table[MAX_LEVEL];
TIME_INFO_T time_info;
WEATHER_T weather_info;

MOB_INDEX_T  *mob_index_hash[MAX_KEY_HASH],
             *mob_index_hash_back[MAX_KEY_HASH];
OBJ_INDEX_T  *obj_index_hash[MAX_KEY_HASH],
             *obj_index_hash_back[MAX_KEY_HASH];
ROOM_INDEX_T *room_index_hash[MAX_KEY_HASH],
             *room_index_hash_back[MAX_KEY_HASH];

bool  in_boot_db;
bool  in_game_loop;
FILE *current_area_file;
char  current_area_filename[MAX_INPUT_LENGTH];

/* Socket and TCP/IP stuff. */
#if defined(macintosh) || defined(MSDOS)
    const char echo_off_str[] = { '\0' };
    const char echo_on_str[]  = { '\0' };
    const char go_ahead_str[] = { '\0' };
#endif

#if defined(unix)
    #include "telnet.h"
    const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
    const char echo_on_str[]  = { IAC, WONT, TELOPT_ECHO, '\0' };
    const char go_ahead_str[] = { IAC, GA, '\0' };
#endif

/* Global variables. */
DESCRIPTOR_T *descriptor_first, *descriptor_last; /* All open descriptors */
DESCRIPTOR_T *d_next;          /* Next descriptor in loop  */
FILE *reserve_file;            /* Reserved file handle     */
bool god;                      /* All new chars are gods!  */
bool merc_down = TRUE;         /* Shutdown                 */
bool wizlock;                  /* Game is wizlocked        */
bool newlock;                  /* Game is newlocked        */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time;           /* time of this pulse */
bool trigger_mobs = TRUE;      /* act() switch */

/* Needs to be global because of do_copyover */
int port, control;

/* Put global mud config values here. Look at qmconfig command for clues.     */
/*   -- JR 09/23/2000                                                         */
/* Set values for all but IP address in ../area/qmconfig.rc file.             */
/*   -- JR 05/10/2001                                                         */
int mud_ansiprompt, mud_ansicolor, mud_telnetga;

/* Set this to the IP address you want to listen on (127.0.0.1 is good for    */
/* paranoid types who don't want the 'net at large peeking at their MUD)      */
char *mud_ipaddress = "0.0.0.0";

int find_last_count = 0;
int find_next_count = 0;

/* Log-all switch. */
bool log_all_commands = FALSE;

int top_vnum_room = 0;
int top_vnum_mob  = 0;
int top_vnum_obj  = 0;

/* Music stuff. */
flag_t music_line = -1;
flag_t music_song = -1;
flag_t music_queue[MAX_SONG_GLOBAL];

/* Memory stuff. */
char str_empty[1];
size_t new_buf_size = BASE_BUF;
