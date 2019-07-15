/***************************************************************************
 *  File: olc_mpedit.h                                                     *
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

#ifndef __ROM_OLC_MPEDIT_H
#define __ROM_OLC_MPEDIT_H

#include "merc.h"

#define MPEDIT(fun)           bool fun(CHAR_DATA *ch, char *argument)
#define EDIT_MPCODE(Ch, Code) (Code = (MPROG_CODE*) Ch->desc->pEdit)

/* Commands (mobile programs). */
MPEDIT (mpedit_create);
MPEDIT (mpedit_show);
MPEDIT (mpedit_code);
MPEDIT (mpedit_list);

#endif
