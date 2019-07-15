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

#ifndef __ROM_SPECIAL_H
#define __ROM_SPECIAL_H

#include "merc.h"

/* The following special functions are available for mobiles. */
DECLARE_SPEC_FUN (spec_breath_any);
DECLARE_SPEC_FUN (spec_breath_acid);
DECLARE_SPEC_FUN (spec_breath_fire);
DECLARE_SPEC_FUN (spec_breath_frost);
DECLARE_SPEC_FUN (spec_breath_gas);
DECLARE_SPEC_FUN (spec_breath_lightning);
DECLARE_SPEC_FUN (spec_cast_adept);
DECLARE_SPEC_FUN (spec_cast_cleric);
DECLARE_SPEC_FUN (spec_cast_judge);
DECLARE_SPEC_FUN (spec_cast_mage);
DECLARE_SPEC_FUN (spec_cast_undead);
DECLARE_SPEC_FUN (spec_executioner);
DECLARE_SPEC_FUN (spec_fido);
DECLARE_SPEC_FUN (spec_guard);
DECLARE_SPEC_FUN (spec_janitor);
DECLARE_SPEC_FUN (spec_mayor);
DECLARE_SPEC_FUN (spec_poison);
DECLARE_SPEC_FUN (spec_thief);
DECLARE_SPEC_FUN (spec_nasty);
DECLARE_SPEC_FUN (spec_troll_member);
DECLARE_SPEC_FUN (spec_ogre_member);
DECLARE_SPEC_FUN (spec_patrolman);

#endif
