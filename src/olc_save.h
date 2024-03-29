/***************************************************************************
 *  File: olc_save.h                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#ifndef __ROM_OLC_SAVE_H
#define __ROM_OLC_SAVE_H

#include "merc.h"

#include <stdio.h>

/* Function prototypes. */
char *olc_save_filename_static (const char *filename);
char *fix_string (const char *str);
void save_area_list (void);
void save_mobprogs (FILE *fp, AREA_T *area);
void save_mobile (FILE *fp, MOB_INDEX_T *mob_index);
void save_mobiles (FILE *fp, AREA_T *area);
void save_object (FILE *fp, OBJ_INDEX_T *obj_index);
void save_objects (FILE *fp, AREA_T *area);
void save_room (FILE *fp, ROOM_INDEX_T *room_index);
void save_exit (FILE *fp, EXIT_T *ex);
void save_rooms (FILE *fp, AREA_T *area);
void save_specials (FILE *fp, AREA_T *area);
void save_door_resets (FILE *fp, AREA_T *area);
void save_resets (FILE *fp, AREA_T *area);
void save_shops (FILE *fp, AREA_T *area);
void save_helps (FILE *fp, HELP_AREA_T *ha);
int save_other_helps (CHAR_T *ch);
void save_area (AREA_T *area);

#endif
