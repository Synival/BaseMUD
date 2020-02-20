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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "utils.h"
#include "recycle.h"
#include "db.h"
#include "lookup.h"
#include "portals.h"
#include "globals.h"
#include "memory.h"
#include "json_objr.h"
#include "help.h"
#include "json_read.h"
#include "rooms.h"
#include "mobiles.h"
#include "objs.h"

#include "json_import.h"

int json_import_objects (JSON_T *json) {
    const TABLE_T *table;
    JSON_T *sub;
    int count = 0;

    /* skip invalid import types. */
    if (json == NULL)
        return 0;
    if (json->type != JSON_OBJECT && json->type != JSON_ARRAY)
        return 0;

    /* if this is a named member, it might be eligable for importing. */
    if (json->type == JSON_OBJECT && json->parent &&
        json->parent->type == JSON_OBJECT && json->name != NULL)
    {
        /* skip certain objects. */
        if (strcmp (json->name, "flags") == 0)
            return 0;
        if (strcmp (json->name, "types") == 0)
            return 0;

        /* import! */
        /* TODO: this should be a table as well! */
        if (strcmp (json->name, "room") == 0)
            return json_objr_room (json) ? 1 : 0;
        if (strcmp (json->name, "mobile") == 0)
            return json_objr_mobile (json) ? 1 : 0;
        if (strcmp (json->name, "object") == 0)
            return json_objr_object (json) ? 1 : 0;
        if (strcmp (json->name, "area") == 0)
            return json_objr_area (json) ? 1 : 0;
        if (strcmp (json->name, "social") == 0)
            return json_objr_social (json) ? 1 : 0;
        if (strcmp (json->name, "portal") == 0)
            return json_objr_portal (json) ? 1 : 0;
        if (strcmp (json->name, "help") == 0) {
            HELP_AREA_T *had = json_objr_help_area (json);
            if (had == NULL)
                return 0;
            return 1 + help_area_count_pages (had);
        }
        if ((table = master_table_get_by_obj_name (json->name)) != NULL) {
            if (table->json_read_func)
                return (table->json_read_func (json, json->name)) ? 1 : 0;
            else
                return 0;
        }

        json_logf (json, "json_import_objects(): Unknown object type '%s'.",
            json->name);
        return 0;
    }

    /* recurse through arrays or objects. */
    for (sub = json->first_child; sub != NULL; sub = sub->next)
        if (sub->type == JSON_OBJECT || sub->type == JSON_ARRAY)
            count += json_import_objects (sub);

    /* return the total number of objects recursively imported. */
    return count;
}

bool json_import_expect (const char *type, const JSON_T *json, ...) {
    char *prop;
    va_list vargs;
    struct json_eprop *eprops = NULL, *eprops_back = NULL;
    struct json_eprop *eprop;
    bool success;
    JSON_T *jprop;

    /* get all of our supplied arguments. */
    va_start (vargs, json);
    eprops = NULL;
    eprops_back = NULL;

    success = TRUE;
    while (1) {
        if ((prop = va_arg (vargs, char *)) == NULL)
            break;

        eprop = calloc (1, sizeof (struct json_eprop));
        if (prop[0] == '*') {
            eprop->required = FALSE;
            eprop->name = strdup (prop + 1);
        }
        else {
            eprop->required = TRUE;
            eprop->name = strdup (prop);
        }

        LIST2_BACK (eprop, prev, next, eprops, eprops_back);
    }
    va_end (vargs);

    /* TODO: this algorithm could be much more optimized. */
    /* go through our expected properties and flag ones that are missing. */
    for (eprop = eprops; eprop != NULL; eprop = eprop->next) {
        for (jprop = json->first_child; jprop != NULL; jprop = jprop->next)
            if (strcmp (jprop->name, eprop->name) == 0)
                break;
        if (jprop == NULL && eprop->required) {
            json_logf (json, "json_import_expect(): Didn't find required "
                "property '%s' in object '%s'.\n", eprop->name, type);
            success = FALSE;
        }
    }

    for (jprop = json->first_child; jprop != NULL; jprop = jprop->next) {
        for (eprop = eprops; eprop != NULL; eprop = eprop->next)
            if (strcmp (jprop->name, eprop->name) == 0)
                break;
        if (eprop == NULL) {
            json_logf (json, "json_import_expect(): Found unexpected property "
                "'%s' in object '%s'.\n", jprop->name, type);
            success = FALSE;
        }
    }

    /* free allocated data. */
    while (eprops) {
        eprop = eprops;
        LIST2_REMOVE (eprop, prev, next, eprops, eprops_back);

        free (eprop->name);
        free (eprop);
    }

    return success;
}

char *json_string_append_newline (char *buf, size_t size) {
    int len;

    if (buf == NULL)
        return NULL;
    if ((len = strlen (buf)) + 1 >= size)
        return buf;

    buf[len + 0] = '\n';
    buf[len + 1] = '\r';
    buf[len + 2] = '\0';
    return buf;
}

void json_import_all (void) {
    JSON_T *json;
    int imported;

    /* read all files instead simple JSON objects. */
    imported = 0;
    log_f ("Loading configuration from '%s'...", JSON_CONFIG_DIR);
    if ((json = json_read_directory_recursive (JSON_CONFIG_DIR,
            json_import_objects, &imported)) != NULL)
        json_free (json);

#ifdef BASEMUD_IMPORT_JSON
    log_f ("Loading help from '%s'...", JSON_HELP_DIR);
    if ((json = json_read_directory_recursive (JSON_HELP_DIR,
            json_import_objects, &imported)) != NULL)
        json_free (json);

    log_f ("Loading areas from '%s'...", JSON_AREAS_DIR);
    if ((json = json_read_directory_recursive (JSON_AREAS_DIR,
            json_import_objects, &imported)) != NULL)
        json_free (json);
#else
    log_string ("Ignoring JSON area and help files.");
    log_string ("Define BASEMUD_IMPORT_JSON in 'basemud.h' to enable.");
#endif

    /* reconstruct the world based on the JSON read. */
    log_f ("%d object(s) loaded successfully.", imported);

    /* link any objects with 'area_str' to their respective objects. */
    log_f ("Linking everything together...");
    json_import_link_areas ();
    portal_link_exits ();
}

AREA_T *json_import_link_areas_get_area (char **name) {
    AREA_T *area;

    if (*name == NULL)
        return NULL;

    area = area_get_by_name_exact (*name);
    if (area == NULL)
        bugf ("json_import_link_areas_get_area(): area '%s' not found.", *name);
    str_free (&(*name));

    return area;
}

void json_import_link_areas (void) {
    HELP_AREA_T *had;
    AREA_T *area;
    ROOM_INDEX_T *room, *to_room;
    MOB_INDEX_T *mob;
    OBJ_INDEX_T *obj;
    EXIT_T *exit;
    ANUM_T *anum, *anum_next;
    RESET_T *reset;
    int i;

    /* For each 'anum' value, determine the 'vnum'. */
    for (anum = anum_first; anum != NULL; anum = anum_next) {
        anum_next = anum->global_next;
        do {
            if (anum->vnum_ref == NULL)
                continue;
            if ((area = json_import_link_areas_get_area (&(anum->area_str))) == NULL)
                continue;
            *(anum->vnum_ref) = area->min_vnum + anum->anum;
        } while (0);
        anum_free (anum);
    }

    /* Link rooms and their resets to areas and register them. */
    for (room = room_index_get_first(); room != NULL;
         room = room_index_get_next(room))
    {
        if ((area = json_import_link_areas_get_area (&(room->area_str))) == NULL)
            continue;
        room_to_area (room, area);

        room->vnum = room->anum + area->min_vnum;
        for (reset = room->reset_first; reset != NULL; reset = reset->room_next)
            reset->room_vnum = room->vnum;
        db_register_new_room (room);
    }

    /* Now that rooms have vnums, link exits. */
    for (room = room_index_get_first(); room != NULL;
         room = room_index_get_next(room))
    {
        for (i = 0; i < DIR_MAX; i++) {
            if ((exit = room->exit[i]) == NULL)
                continue;
            exit->area_vnum = -1;
            if (exit->room_anum < 0)
                continue;

            exit->vnum = exit->room_anum + room->area->min_vnum;
            if ((to_room = room_get_index (exit->vnum)) == NULL) {
                bugf ("json_import_link_areas(): Room '%s' (%d) exit '%d' cannot find "
                    "room with vnum %d.\n", room->name, room->vnum, i,
                    exit->vnum);
                continue;
            }
            exit->to_room = to_room;
            exit->area_vnum = to_room->area->vnum;
        }
    }

    /* Link mobs to areas and register them. */
    for (mob = mob_index_get_first(); mob != NULL;
         mob = mob_index_get_next(mob))
    {
        if ((area = json_import_link_areas_get_area (&(mob->area_str))) == NULL)
            continue;
        mob_index_to_area (mob, area);

        mob->vnum = mob->anum + area->min_vnum;
        if (mob->shop)
            mob->shop->keeper = mob->vnum;

        db_register_new_mob (mob);
    }

    /* Link objs to areas and register them. */
    for (obj = obj_index_get_first(); obj != NULL;
         obj = obj_index_get_next(obj))
    {
        if ((area = json_import_link_areas_get_area (&(obj->area_str))) == NULL)
            continue;
        obj_index_to_area (obj, area);

        obj->vnum = obj->anum + area->min_vnum;
        db_register_new_obj (obj);
    }

    /* Link help groups to areas. */
    for (had = had_get_first(); had != NULL; had = had_get_next(had)) {
        if ((area = json_import_link_areas_get_area (&(had->area_str))) == NULL)
            continue;
        help_area_to_area (had, area);
    }
}
