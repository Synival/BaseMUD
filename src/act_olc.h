/***************************************************************************
 *  File: act_olc.h                                                        *
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

#ifndef __ROM_ACT_OLC_H
#define __ROM_ACT_OLC_H

#include "merc.h"

/* Sub-routines and filters. */
void do_resets_display (CHAR_DATA * ch);

/* Commands. */
DECLARE_DO_FUN (do_olc);
DECLARE_DO_FUN (do_aedit);
DECLARE_DO_FUN (do_hedit);
DECLARE_DO_FUN (do_medit);
DECLARE_DO_FUN (do_mpedit);
DECLARE_DO_FUN (do_oedit);
DECLARE_DO_FUN (do_redit);
DECLARE_DO_FUN (do_resets);
DECLARE_DO_FUN (do_alist);
DECLARE_DO_FUN (do_asave);

#endif
