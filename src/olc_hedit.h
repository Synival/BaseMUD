/***************************************************************************
 *  File: olc_hedit.h                                                      *
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

#ifndef __ROM_OLC_HEDIT_H
#define __ROM_OLC_HEDIT_H

#include "merc.h"

#define HEDIT(fun)          bool fun(CHAR_DATA *ch, char *argument)
#define EDIT_HELP(ch, help) (help = (HELP_DATA *) ch->desc->pEdit)

/* Commands (help). */
HEDIT (hedit_show);
HEDIT (hedit_level);
HEDIT (hedit_keyword);
HEDIT (hedit_new);
HEDIT (hedit_text);
HEDIT (hedit_delete);
HEDIT (hedit_list);

#endif
