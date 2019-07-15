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

#ifndef __ROM_ACT_SHOP_H
#define __ROM_ACT_SHOP_H

#include "merc.h"

/* Sub-routines and filters. */
bool do_filter_get_keeper (CHAR_DATA *ch, CHAR_DATA **out_keeper);
void do_buy_pet (CHAR_DATA *ch, char *argument);
void do_buy_item (CHAR_DATA *ch, char *argument);
void do_list_pets (CHAR_DATA *ch, char *argument);
void do_list_items (CHAR_DATA *ch, char *argument);

/* Commands. */
DECLARE_DO_FUN (do_buy);
DECLARE_DO_FUN (do_list);
DECLARE_DO_FUN (do_sell);
DECLARE_DO_FUN (do_value);
DECLARE_DO_FUN (do_heal);

#endif
