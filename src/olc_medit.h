/***************************************************************************
 *  File: olc_medit.h                                                      *
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

#ifndef __ROM_OLC_MEDIT_H
#define __ROM_OLC_MEDIT_H

#include "merc.h"

#define MEDIT(fun)        bool fun(CHAR_DATA *ch, char *argument)
#define EDIT_MOB(Ch, Mob) (Mob = (MOB_INDEX_DATA *) Ch->desc->pEdit)

/* Commands (mobiles). */
MEDIT (medit_show);
MEDIT (medit_create);
MEDIT (medit_spec);
MEDIT (medit_damtype);
MEDIT (medit_align);
MEDIT (medit_level);
MEDIT (medit_desc);
MEDIT (medit_long);
MEDIT (medit_short);
MEDIT (medit_name);
MEDIT (medit_shop);
MEDIT (medit_sex);
MEDIT (medit_act);
MEDIT (medit_affect);
MEDIT (medit_ac);
MEDIT (medit_form);
MEDIT (medit_part);
MEDIT (medit_imm);
MEDIT (medit_res);
MEDIT (medit_vuln);
MEDIT (medit_material);
MEDIT (medit_off);
MEDIT (medit_size);
MEDIT (medit_hitdice);
MEDIT (medit_manadice);
MEDIT (medit_damdice);
MEDIT (medit_race);
MEDIT (medit_position);
MEDIT (medit_gold);
MEDIT (medit_hitroll);
MEDIT (medit_group);
MEDIT (medit_addmprog);
MEDIT (medit_delmprog);

#endif
