/***************************************************************************
 *  File: string.h                                                         *
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

#ifndef __ROM_STRING_H
#define __ROM_STRING_H

#include "merc.h"

/* Function prototypes. */
void string_edit (CHAR_T *ch, char **string_edit);
void string_append (CHAR_T *ch, char **string_edit);
char *string_replace (char *orig, char *old, char *new);
void string_add (CHAR_T *ch, char *argument);
char *format_string (char *oldstring /*, bool space */ );
char *first_arg (char *argument, char *arg_first, bool mod_case);
char *string_unpad (char *argument);
char *string_proper (char *argument);
char *string_linedel (char *string, int line);
char *string_lineadd (char *string, char *newstr, int line);
char *merc_getline (char *str, char *buf);
char *numlines (char *string);

#endif
