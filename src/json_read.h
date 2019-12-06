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

#ifndef __ROM_JSON_READ_H
#define __ROM_JSON_READ_H

#include "merc.h"

/* Read functions. */
JSON_T *json_read_directory_recursive (const char *path,
    int (*load_func) (JSON_T *), int *load_result);
void json_read_directory_real (JSON_T *obj, const char *path, bool recurse,
    int (*load_func) (JSON_T *), int *load_result);
JSON_T *json_read_file (const char *filename);
JSON_T *json_read_object (const char **pos, const char *name);
JSON_T *json_read_array (const char **pos, const char *name);
JSON_T *json_read_any_type (const char **pos, const char *name);
JSON_T *json_read_string (const char **pos, const char *name);
char *json_read_string_content (const char **pos, char *buf, size_t size);
JSON_T *json_read_number (const char **pos, const char *name);
JSON_T *json_read_object (const char **pos, const char *name);
JSON_T *json_read_array (const char **pos, const char *name);
JSON_T *json_read_special (const char **pos, const char *name);

#endif
