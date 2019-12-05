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

#ifndef __ROM_UTILS_H
#define __ROM_UTILS_H

#include "merc.h"

/* String utilities. */
char *capitalize (const char *str);
void smash_tilde (char *str);
void smash_dollar (char *str);
bool str_cmp (const char *astr, const char *bstr);
bool str_prefix (const char *astr, const char *bstr);
bool str_infix (const char *astr, const char *bstr);
bool str_suffix (const char *astr, const char *bstr);
const char *if_null_str (const char *str, const char *ifnull);
char *trim_extension (char *input);
bool is_name (const char *str, char *namelist);
bool is_exact_name (const char *str, char *namelist);
bool is_full_name (const char *str, char *namelist);

/* Number utilities. */
int interpolate (int level, int value_00, int value_32);

/* Random number utilities. */
void init_mm (void);
long number_mm (void);
int number_fuzzy (int number);
int number_range (int from, int to);
int number_percent (void);
int number_door (void);
int number_bits (int width);
int dice (int number, int size);

/* File utilities. */
void append_file (CHAR_T *ch, char *file, char *str);

/* Logging utilities. */
void bug (const char *str, int param);
void bugf (const char *fmt, ...);
void log_string (const char *str);
void log_f (const char *fmt, ...);
void tail_chain (void);

#endif
