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

#ifndef __ROM_JSON_OBJ_H
#define __ROM_JSON_OBJ_H

#include "merc.h"

#include "json.h"

/* useful macros. */
#define JBITS(val) \
    json_not_none(val)
#define JSTR(val) \
    json_not_blank(val)

/* useful function prototypes. */
const char *json_not_none (const char *value);
const char *json_not_blank (const char *value);

/* node creation. */
JSON_T *json_new_obj_room (const char *name, const ROOM_INDEX_DATA *room);
JSON_T *json_new_obj_extra_descr (const char *name, const EXTRA_DESCR_DATA *ed);
JSON_T *json_new_obj_exit (const char *name, const ROOM_INDEX_DATA *from, int dir, const EXIT_DATA *ex);
JSON_T *json_new_obj_shop (const char *name, const SHOP_DATA *shop);
JSON_T *json_new_obj_mobile (const char *name, const MOB_INDEX_DATA *mob);
JSON_T *json_new_obj_object (const char *name, const OBJ_INDEX_DATA *obj);
JSON_T *json_new_obj_affect (const char *name, const AFFECT_DATA *aff);
JSON_T *json_new_obj_anum (const char *name, AREA_DATA *area_from, int vnum);
JSON_T *json_new_obj_reset (const char *name, const RESET_DATA *reset);
JSON_T *json_new_obj_shop (const char *name, const SHOP_DATA *shop);
JSON_T *json_new_obj_area (const char *name, const AREA_DATA *area);
JSON_T *json_new_obj_social (const char *name, const SOCIAL_TYPE *soc);
JSON_T *json_new_obj_table (const char *name, const TABLE_TYPE *table);
JSON_T *json_new_obj_portal_exit (const char *name, const PORTAL_EXIT_TYPE *pex);
JSON_T *json_new_obj_portal (const char *name, const PORTAL_TYPE *portal);
JSON_T *json_new_obj_help_area (const char *name, const HELP_AREA *had);
JSON_T *json_new_obj_help (const char *name, const HELP_DATA *help);

/* shorthand-functions for node creation as properties. */
JSON_T *json_prop_obj_room (JSON_T *parent, const char *name, const ROOM_INDEX_DATA *room);
JSON_T *json_prop_obj_extra_descr (JSON_T *parent, const char *name, const EXTRA_DESCR_DATA *ed);
JSON_T *json_prop_obj_exit (JSON_T *parent, const char *name, const ROOM_INDEX_DATA *from, int dir, const EXIT_DATA *ex);
JSON_T *json_prop_obj_shop (JSON_T *parent, const char *name, const SHOP_DATA *shop);
JSON_T *json_prop_obj_mobile (JSON_T *parent, const char *name, const MOB_INDEX_DATA *mob);
JSON_T *json_prop_obj_object (JSON_T *parent, const char *name, const OBJ_INDEX_DATA *obj);
JSON_T *json_prop_obj_affect (JSON_T *parent, const char *name, const AFFECT_DATA *aff);
JSON_T *json_prop_obj_anum (JSON_T *parent, const char *name, AREA_DATA *area_from, int vnum);
JSON_T *json_prop_obj_reset (JSON_T *parent, const char *name, const RESET_DATA *reset);
JSON_T *json_prop_obj_shop (JSON_T *parent, const char *name, const SHOP_DATA *shop);
JSON_T *json_prop_obj_area (JSON_T *parent, const char *name, const AREA_DATA *area);
JSON_T *json_prop_obj_social (JSON_T *parent, const char *name, const SOCIAL_TYPE *soc);
JSON_T *json_prop_obj_table (JSON_T *parent, const char *name, const TABLE_TYPE *table);
JSON_T *json_prop_obj_portal_exit (JSON_T *parent, const char *name, const PORTAL_EXIT_TYPE *pex);
JSON_T *json_prop_obj_portal (JSON_T *parent, const char *name, const PORTAL_TYPE *portal);
JSON_T *json_prop_obj_help_area (JSON_T *parent, const char *name, const HELP_AREA *had);
JSON_T *json_prop_obj_help (JSON_T *parent, const char *name, const HELP_DATA *help);

#endif
