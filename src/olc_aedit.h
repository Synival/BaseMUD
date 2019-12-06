/***************************************************************************
 *  File: olc_aedit.h                                                      *
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

#ifndef __ROM_OLC_AEDIT_H
#define __ROM_OLC_AEDIT_H

#include "merc.h"

#define AEDIT(fun)          bool fun(CHAR_T *ch, char *argument)
#define EDIT_AREA(ch, area) (area = (AREA_T *) ch->desc->pEdit)

/* Sub-routines and filters. */
bool aedit_check_range (int lower, int upper);

/* Commands (areas). */
AEDIT (aedit_show);
AEDIT (aedit_reset);
AEDIT (aedit_create);
AEDIT (aedit_title);
AEDIT (aedit_credits);
AEDIT (aedit_file);
AEDIT (aedit_age);
/* AEDIT (aedit_recall); */
AEDIT (aedit_security);
AEDIT (aedit_builder);
AEDIT (aedit_vnum);
AEDIT (aedit_lvnum);
AEDIT (aedit_uvnum);

#endif
