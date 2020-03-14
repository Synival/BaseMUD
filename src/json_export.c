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

#include "areas.h"
#include "recycle.h"
#include "utils.h"
#include "json_objw.h"
#include "json_write.h"
#include "lookup.h"

#include "json_export.h"

void json_export_all (bool write_indiv, const char *everything) {
    int mode;

    /* Make sure we write at least SOMETHING. */
    if (!write_indiv && !everything)
        return;

    /* Write all areas. Determine the mode based on what we're exporting. */
    mode = write_indiv
        ? (everything ? JSON_EXPORT_MODE_SAVE : JSON_EXPORT_MODE_SAVE_AND_KEEP)
        : JSON_EXPORT_MODE_ONLY_LOAD;

    /* Write everything! */
    json_export_areas (mode);
    json_export_socials (mode);
    json_export_portals (mode);
    json_export_tables (mode);
    json_export_help_areas (mode);

    /* Write one giant file with everything. */
    if (everything) {
        log_f("Exporting JSON: %s", everything);
        json_fwrite (json_root(), everything);
    }

    /* Free all allocated JSON. */
    json_free (json_root());
}

void json_export_areas (int mode) {
    const AREA_T *area;
    for (area = area_get_first(); area; area = area_get_next (area))
        json_export_area (area, mode);
}

void json_export_socials (int mode) {
    json_export_recycleable ("social", "config/socials.json",
        social_get_first(), (JSON_EXPORT_REC_NEXT_FUNC)  social_get_next,
        (JSON_EXPORT_REC_WRITE_FUNC) json_objw_social, mode);
}

void json_export_portals (int mode) {
    json_export_recycleable ("portal", "config/portals.json",
        portal_get_first(), (JSON_EXPORT_REC_NEXT_FUNC)  portal_get_next,
        (JSON_EXPORT_REC_WRITE_FUNC) json_objw_portal, mode);
}

void json_export_tables (int mode) {
    const TABLE_T *table;
    for (table = master_get_first(); table; table = master_get_next(table))
        if (json_can_export_table (table))
            json_export_table (table, mode);
}

void json_export_help_areas (int mode) {
    const HELP_AREA_T *had;
    for (had = had_get_first(); had; had = had_get_next (had))
        json_export_help_area (had, mode);
}

bool json_export_interpret_mode (int mode, flag_t *options_out) {
    switch (mode) {
        case JSON_EXPORT_MODE_SAVE:
            *options_out = JSON_EXPORT_OPTION_WRITE_INDIV |
                           JSON_EXPORT_OPTION_UNLOAD;
            return TRUE;

        case JSON_EXPORT_MODE_SAVE_AND_KEEP:
            *options_out = JSON_EXPORT_OPTION_WRITE_INDIV;
            return TRUE;

        case JSON_EXPORT_MODE_ONLY_LOAD:
            *options_out = 0;
            return TRUE;

        default:
            *options_out = 0;
            return FALSE;
    }
}

void json_export_area (const AREA_T *area, int mode) {
    JSON_T *jgrp, *jarea, *json;
    char buf[256], fbuf[256];
    flag_t options;

    if (!json_export_interpret_mode (mode, &options)) {
        bugf ("json_export_area(): Unknown mode %d", mode);
        return;
    }

    snprintf (fbuf, sizeof(fbuf), "%s/", area->name);

    snprintf (buf, sizeof(buf), "areas/%s", area->name);
    jarea = json_root_area (buf);
    jgrp = json_prop_array (jarea, NULL);

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV)
        log_f("Exporting JSON: %s%s*", JSON_AREAS_DIR, fbuf);

    json = json_wrap_obj (json_objw_area (NULL, area), "area");
    json_attach_under (json, jgrp);

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV) {
        snprintf (buf, sizeof(buf), "%s%sarea.json", JSON_AREAS_DIR, fbuf);
        json_mkdir_to (buf);
        json_fwrite (jgrp, buf);
    }

    /* NOTE: This is extremely nasty, but refactoring it into a function
     * or having copy-pasted code is even nastier. This is the least-worst
     * solution, IMO. -- Synival */
    #define ADD_AREA_JSON(oname, fname, alist, otype, objw_func) \
        do { \
            const otype *obj; \
            jgrp = json_prop_array (jarea, NULL); \
            \
            for (obj = area->alist; obj; obj = obj->area_next) { \
                if (obj->vnum < area->min_vnum || obj->vnum > area->max_vnum) \
                    bugf ("Warning: " #otype " #%d should be >= %d and <= %d", \
                        obj->vnum, area->min_vnum, area->max_vnum);\
                json = json_wrap_obj (objw_func (NULL, obj), oname); \
                json_attach_under (json, jgrp); \
            } \
            if (jgrp->first_child && (options & JSON_EXPORT_OPTION_WRITE_INDIV)) { \
                snprintf (buf, sizeof (buf), "%s%s" fname, JSON_AREAS_DIR, fbuf); \
                json_mkdir_to (buf); \
                json_fwrite (jgrp, buf); \
            } \
        } while (0)

    ADD_AREA_JSON ("room",   "rooms.json",   room_first, ROOM_INDEX_T, json_objw_room);
    ADD_AREA_JSON ("object", "objects.json", obj_first,  OBJ_INDEX_T,  json_objw_object);
    ADD_AREA_JSON ("mobile", "mobiles.json", mob_first,  MOB_INDEX_T,  json_objw_mobile);

    /* Unload all parsed JSON if the mode specifies it. */
    if (options & JSON_EXPORT_OPTION_UNLOAD)
        json_free (jarea);
}

void json_export_recycleable (const char *objname, const char *filename,
    void *first,
    JSON_EXPORT_REC_NEXT_FUNC next_func,
    JSON_EXPORT_REC_WRITE_FUNC write_func,
    int mode)
{
    JSON_T *jarea, *json;
    flag_t options;
    char fbuf[256];
    const void *obj;

    if (!json_export_interpret_mode (mode, &options)) {
        bugf ("json_export_recycleable(): Unknown mode %d", mode);
        return;
    }

    jarea = json_root_area (filename);
    snprintf (fbuf, sizeof (fbuf), "%s%s", JSON_DIR, filename);

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV)
        log_f ("Exporting JSON: %s", fbuf);
       
    for (obj = first; obj; obj = next_func (obj)) {
        json = json_wrap_obj (write_func (NULL, obj), objname);
        json_attach_under (json, jarea);
    }
    if (jarea->first_child && (options & JSON_EXPORT_OPTION_WRITE_INDIV)) {
        json_mkdir_to (fbuf);
        json_fwrite (jarea, fbuf);
    }

    /* Unload all parsed JSON if the mode specifies it. */
    if (options & JSON_EXPORT_OPTION_UNLOAD)
        json_free (jarea);
}

bool json_can_export_table (const TABLE_T *table) {
    switch (table->type) {
        case TABLE_UNIQUE:
        case TABLE_FLAGS:
        case TABLE_EXT_FLAGS:
        case TABLE_TYPES:
            return TRUE;
        default:
            return FALSE;
    }
}

void json_export_table (const TABLE_T *table, int mode) {
    JSON_T *json;
    flag_t options;
    char buf[256];
    const char *oname;

    if (!json_export_interpret_mode (mode, &options)) {
        bugf ("json_export_table(): Unknown mode %d", mode);
        return;
    }

    BAIL_IF_BUGF (!json_can_export_table (table),
        "json_export_table(): Cannot export table '%s'", table->name);

    switch (table->type) {
        case TABLE_UNIQUE:
            if (table->json_write_func == NULL)
                return;
            oname = "table";
            break;

        case TABLE_FLAGS:     oname = "flags";     break;
        case TABLE_EXT_FLAGS: oname = "ext_flags"; break;
        case TABLE_TYPES:     oname = "types";     break;

        default:
            bugf ("json_export_table(): Unhandled table type '%d'"
                  "for '%s'", table->type, table->name);
            return;
    }

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV)
        log_f("Exporting JSON: %s%s/%s.json", JSON_DIR,
            table->json_path, table->name);

    json = json_objw_table (NULL, table);
    if (json->type != JSON_ARRAY)
        json = json_wrap_obj (json, oname);
    json_attach_under (json, json_root());

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV) {
        snprintf (buf, sizeof (buf), "%s%s/%s.json",
            JSON_DIR, table->json_path, table->name);
        json_mkdir_to (buf);
        json_fwrite (json, buf);
    }

    /* Unload all parsed JSON if the mode specifies it. */
    if (options & JSON_EXPORT_OPTION_UNLOAD)
        json_free (json);
}

void json_export_help_area (const HELP_AREA_T *had, int mode) {
    JSON_T *jarea, *json;
    char buf[256], fbuf[256];
    flag_t options;

    if (!json_export_interpret_mode (mode, &options)) {
        bugf ("json_export_help_area(): Unknown mode %d", mode);
        return;
    }

    snprintf (buf, sizeof(buf), "help/%s", had->name);
    jarea = json_root_area (buf);
    snprintf (fbuf, sizeof (fbuf), "%s%s.json", JSON_HELP_DIR, had->name);

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV)
        log_f("Exporting JSON: %s", fbuf);

    json = json_wrap_obj (json_objw_help_area (NULL, had), "help");
    json_attach_under (json, jarea);

    if (options & JSON_EXPORT_OPTION_WRITE_INDIV) {
        json_mkdir_to (fbuf);
        json_fwrite (jarea, fbuf);
    }

    /* Unload all parsed JSON if the mode specifies it. */
    if (options & JSON_EXPORT_OPTION_UNLOAD)
        json_free (jarea);
}
