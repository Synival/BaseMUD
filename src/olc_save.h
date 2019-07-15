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

#define DIF(a,b) (~((~a)|(b)))

/* Function prototypes. */
char *fix_string (const char *str);
void save_area_list ();
char *fwrite_flag (long flags, char buf[]);
void save_mobprogs (FILE * fp, AREA_DATA * pArea);
void save_mobile (FILE * fp, MOB_INDEX_DATA * pMobIndex);
void save_mobiles (FILE * fp, AREA_DATA * pArea);
void save_object (FILE * fp, OBJ_INDEX_DATA * pObjIndex);
void save_objects (FILE * fp, AREA_DATA * pArea);
void save_room (FILE * fp, ROOM_INDEX_DATA * pRoomIndex);
void save_rooms (FILE * fp, AREA_DATA * pArea);
void save_specials (FILE * fp, AREA_DATA * pArea);
void save_door_resets (FILE * fp, AREA_DATA * pArea);
void save_resets (FILE * fp, AREA_DATA * pArea);
void save_shops (FILE * fp, AREA_DATA * pArea);
void save_helps (FILE * fp, HELP_AREA * ha);
void save_other_helps (CHAR_DATA * ch);
void save_area (AREA_DATA * pArea);
char *fix_string (const char *str);

#endif
