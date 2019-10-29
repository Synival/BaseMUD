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
 **************************************************************************/

/***************************************************************************
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 **************************************************************************/

#ifndef __ROM_DB_H
#define __ROM_DB_H

#include "merc.h"

/* Globals. */
extern HELP_DATA *help_first, *help_last;
extern SHOP_DATA *shop_first, *shop_last;
extern AREA_DATA *area_first, *area_last;
extern BAN_DATA  *ban_first,  *ban_last;
extern HELP_AREA *had_first,  *had_last;

extern MPROG_CODE *mprog_list;

extern int newmobs;
extern int newobjs;

extern char bug_buf[2 * MAX_INPUT_LENGTH];
extern CHAR_DATA *char_list;
extern char *help_greeting;
extern char log_buf[2 * MAX_INPUT_LENGTH];
extern KILL_DATA kill_table[MAX_LEVEL];
extern OBJ_DATA *object_list;
extern TIME_INFO_DATA time_info;
extern WEATHER_DATA weather_info;

/* Locals. */
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern char *string_hash[MAX_KEY_HASH];
extern AREA_DATA *current_area;

/* Semi-locals.  */
extern bool fBootDb;
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)    (~(flag1)&((flag1)|(flag2)))

/* Magic number for memory allocation */
#define MAGIC_NUM 52571214

/* Helper functions. */
flag_t flag_convert (char letter);
void assign_area_vnum (int vnum);
void fix_bogus_obj (OBJ_INDEX_DATA * obj);
void room_take_reset (ROOM_INDEX_DATA * pR, RESET_DATA * pReset);
bool check_pet_affected (int vnum, AFFECT_DATA *paf);
char *memory_dump (char *eol);

/* World init functions. */
void boot_db (void);
void init_string_space (void);
void init_time_weather (void);
void init_gsns (void);
void init_areas (void);

/* Reading functions. */
char fread_letter (FILE * fp);
int fread_number (FILE * fp);
flag_t fread_flag (FILE * fp);
char *fread_string (FILE * fp);
char *fread_string_eol (FILE * fp);
void fread_to_eol (FILE * fp);
char *fread_word (FILE * fp);
void fread_dice (FILE *fp, sh_int *out);
bool fread_social_str (FILE *fp, char **str);

/* Loading functions. */
void load_area (FILE * fp);
void load_area_olc (FILE * fp);
void load_resets (FILE * fp);
void load_rooms (FILE * fp);
void load_shops (FILE * fp);
void load_specials (FILE * fp);
void load_socials (FILE * fp);
void load_mobiles (FILE * fp);
void load_objects (FILE * fp);
void load_helps (FILE * fp, char *fname);
void load_mobprogs (FILE * fp);

/* Post-loading functions. */
void fix_exit_doors (ROOM_INDEX_DATA *room_from, int dir_from,
                     ROOM_INDEX_DATA *room_to,   int dir_to);
void fix_exits (void);
void fix_mobprogs (void);
void db_export_json (void);

/* Reset / destruction functions. */
void reset_room (ROOM_INDEX_DATA * pRoom);
void reset_area (AREA_DATA * pArea);
void clear_char (CHAR_DATA * ch);

/* Getter functions. */
char *get_extra_descr (const char *name, EXTRA_DESCR_DATA * ed);
MOB_INDEX_DATA *get_mob_index (int vnum);
OBJ_INDEX_DATA *get_obj_index (int vnum);
ROOM_INDEX_DATA *get_room_index (int vnum);
ROOM_INDEX_DATA *get_random_room (CHAR_DATA * ch);
MPROG_CODE *get_mprog_index (int vnum);

#endif
