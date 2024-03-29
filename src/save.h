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

#ifndef __ROM_SAVE_H
#define __ROM_SAVE_H

#include "merc.h"

#include <stdio.h>

/* Function prototypes. */
void save_char_obj (CHAR_T *ch);
bool load_char_obj (DESCRIPTOR_T *d, char *name);
void load_old_colour (CHAR_T *ch, FILE *fp, char *name);

/* read functions. */
void fread_char (CHAR_T *ch, FILE *fp);
void fread_pet (CHAR_T *ch, FILE *fp);
void fread_obj (CHAR_T *ch, FILE *fp);

/* write functions. */
void fwrite_char (CHAR_T *ch, FILE *fp);
void fwrite_pet (CHAR_T *pet, FILE *fp);
void fwrite_obj (CHAR_T *ch, OBJ_T *obj, FILE *fp, int nest);

#endif
