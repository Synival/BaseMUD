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

#ifndef __ROM_FREAD_H
#define __ROM_FREAD_H

#include "merc.h"

/* Writing functions. */
char fread_letter (FILE *fp);
int fread_number (FILE *fp);
flag_t fread_flag (FILE *fp);
flag_t fread_flag_convert (char letter);
EXT_FLAGS_T fread_ext_flag (FILE *fp, const EXT_FLAG_DEF_T *table);
char *fread_string_replace (FILE *fp, char **value);
char *fread_string_dup (FILE *fp);
char *fread_string_static (FILE *fp);
char *fread_string (FILE *fp, char *buf, size_t size);
char *fread_string_eol_replace (FILE *fp, char **value);
char *fread_string_eol_dup (FILE *fp);
char *fread_string_eol_static (FILE *fp);
char *fread_string_eol (FILE *fp, char *buf, size_t size);
void fread_to_eol (FILE *fp);
char *fread_word_replace (FILE *fp, char **value);
char *fread_word_dup (FILE *fp);
char *fread_word_static (FILE *fp);
char *fread_word (FILE *fp, char *buf, size_t size);
void fread_dice (FILE *fp, DICE_T *out);

#endif
