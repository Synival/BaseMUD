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

#ifndef __ROM_WIZ_IM_H
#define __ROM_WIZ_IM_H

#include "merc.h"

/* Sub-routines and filters. */
/* (none) */

/* Commands. */
DECLARE_DO_FUN (do_wizhelp);
DECLARE_DO_FUN (do_holylight);
DECLARE_DO_FUN (do_incognito);
DECLARE_DO_FUN (do_invis);
DECLARE_DO_FUN (do_memory);
DECLARE_DO_FUN (do_mwhere);
DECLARE_DO_FUN (do_owhere);
DECLARE_DO_FUN (do_stat);
DECLARE_DO_FUN (do_rstat);
DECLARE_DO_FUN (do_ostat);
DECLARE_DO_FUN (do_mstat);
DECLARE_DO_FUN (do_wiznet);
DECLARE_DO_FUN (do_immtalk);
DECLARE_DO_FUN (do_imotd);
DECLARE_DO_FUN (do_smote);
DECLARE_DO_FUN (do_prefi);
DECLARE_DO_FUN (do_prefix);
DECLARE_DO_FUN (do_mpdump);
DECLARE_DO_FUN (do_mpstat);

#endif
