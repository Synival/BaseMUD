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

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)    (~(flag1)&((flag1)|(flag2)))

/* Helper functions. */
flag_t flag_convert (char letter);
void assign_area_vnum (int vnum, AREA_T *area);
bool check_pet_affected (int vnum, AFFECT_T *paf);
void db_dump_world (const char *filename);

/* World init functions. */
void boot_db (void);
void init_time_weather (void);
void init_gsns (void);
void db_link_areas (void);
void db_import_json (void);
void init_areas (void);

/* Reading functions. */
char fread_letter (FILE *fp);
int fread_number (FILE *fp);
flag_t fread_flag (FILE *fp);
char *fread_string (FILE *fp);
char *fread_string_eol (FILE *fp);
void fread_to_eol (FILE *fp);
char *fread_word (FILE *fp);
void fread_dice (FILE *fp, DICE_T *out);
bool fread_social_str (FILE *fp, char **str);

/* Loading functions. */
void load_area (FILE *fp);
void load_area_olc (FILE *fp);
void load_resets (FILE *fp);
void load_rooms (FILE *fp);
void load_shops (FILE *fp);
void load_specials (FILE *fp);
void load_socials (FILE *fp);
void load_mobiles (FILE *fp);
void load_objects (FILE *fp);
void load_helps (FILE *fp, char *fname);
void load_mobprogs (FILE *fp);

/* Post-loading functions. */
void db_finalize_mob (MOB_INDEX_T *mob);
void db_finalize_obj (OBJ_INDEX_T *obj);
void db_register_new_room (ROOM_INDEX_T *room);
void db_register_new_mob (MOB_INDEX_T *mob);
void db_register_new_obj (OBJ_INDEX_T *obj);
void fix_exit_doors (ROOM_INDEX_T *room_from, int dir_from,
                     ROOM_INDEX_T *room_to,   int dir_to);
void fix_resets (void);
void fix_exits (void);
void fix_mobprogs (void);
void db_export_json (bool write_indiv, const char *everything);

/* Reset / destruction functions. */
void reset_room (ROOM_INDEX_T *pRoom);
void reset_area (AREA_T *pArea);
void clear_char (CHAR_T *ch);

/* Getter functions. */
char *get_extra_descr (const char *name, EXTRA_DESCR_T *ed);
MOB_INDEX_T *get_mob_index (int vnum);
OBJ_INDEX_T *get_obj_index (int vnum);
ROOM_INDEX_T *get_room_index (int vnum);
ROOM_INDEX_T *get_random_room (CHAR_T *ch);
MPROG_CODE_T *get_mprog_index (int vnum);
int help_area_count_pages (HELP_AREA_T *had);

/* Temporary objects used during loading. */
ANUM_T *anum_new (void);
void anum_free (ANUM_T *anum);

#endif
