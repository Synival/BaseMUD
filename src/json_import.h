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

#ifndef __ROM_JSON_IMPORT_H
#define __ROM_JSON_IMPORT_H

#include "merc.h"

/* general import functions. */
int json_import_objects (JSON_T *json);
void json_import_expect (const char *type, const JSON_T *json, ...);

/* creating world objects from JSON objects. */
ROOM_INDEX_T *json_import_obj_room (const JSON_T *json);
EXTRA_DESCR_T *json_import_obj_extra_descr (const JSON_T *json);
EXIT_T *json_import_obj_exit (const JSON_T *json, ROOM_INDEX_T *room,
    int *dir_out);
RESET_T *json_import_obj_reset (const JSON_T *json, ROOM_INDEX_T *room);
void json_import_obj_reset_values (const JSON_T *json, RESET_VALUE_T *v,
    char command, ROOM_INDEX_T *room);
SHOP_T *json_import_obj_shop (const JSON_T *json, const char *backup_area);
MOB_INDEX_T *json_import_obj_mobile (const JSON_T *json);
OBJ_INDEX_T *json_import_obj_object (const JSON_T *json);
void json_import_obj_object_values (const JSON_T *json, OBJ_INDEX_T *obj);
AREA_T *json_import_obj_area (const JSON_T *json);
SOCIAL_T *json_import_obj_social (const JSON_T *json);
PORTAL_T *json_import_obj_portal (const JSON_T *json);
HELP_AREA_T *json_import_obj_help_area (const JSON_T *json);
HELP_T *json_import_obj_help (const JSON_T *json);
AFFECT_T *json_import_obj_affect (JSON_T *json);
ANUM_T *json_import_anum (const JSON_T *json, int type, sh_int *vnum_ptr,
    const char *backup_area);

#endif
