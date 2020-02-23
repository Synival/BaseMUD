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

#ifndef __ROM_JSON_EXPORT_H
#define __ROM_JSON_EXPORT_H

#include "merc.h"

/* Flags and modes for functions. */
#define JSON_EXPORT_MODE_SAVE           0
#define JSON_EXPORT_MODE_SAVE_AND_KEEP  1
#define JSON_EXPORT_MODE_ONLY_LOAD      2

#define JSON_EXPORT_OPTION_WRITE_INDIV  0x01
#define JSON_EXPORT_OPTION_UNLOAD       0x02

typedef void *   (*JSON_EXPORT_REC_NEXT_FUNC)  (const void *);
typedef JSON_T * (*JSON_EXPORT_REC_WRITE_FUNC) (const char *, const void *);
typedef bool     (*JSON_EXPORT_REC_CHECK_FUNC) (const void *);

/* Function prototypes. */
void json_export_all (bool write_indiv, const char *everything);
bool json_export_interpret_mode (int mode, flag_t *options_out);
void json_export_area (const AREA_T *area, int mode);
void json_export_recycleable (const char *objname, const char *filename,
    void *first,
    JSON_EXPORT_REC_NEXT_FUNC next_func,
    JSON_EXPORT_REC_WRITE_FUNC write_func,
    JSON_EXPORT_REC_CHECK_FUNC check_func,
    int mode);
void json_export_table (const TABLE_T *table, int mode);
void json_export_help_area (const HELP_AREA_T *had, int mode);

#endif
