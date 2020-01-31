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

#include <string.h>

#include "json_import.h"
#include "recycle.h"
#include "memory.h"
#include "lookup.h"
#include "portals.h"
#include "db.h"
#include "globals.h"

#include "json_objr.h"

ROOM_INDEX_T *json_objr_room (const JSON_T *json) {
    JSON_T *array, *sub;
    ROOM_INDEX_T *room;
    RESET_T *reset;
    EXIT_T *exit;
    EXTRA_DESCR_T *ed;
    char buf[MAX_STRING_LENGTH];
    int dir;

    if (!json_import_expect ("room", json,
        "area", "anum", "name", "description", "sector_type",

        "*owner",      "*room_flags", "*heal_rate", "*mana_rate",
        "*clan",       "*portal",     "*doors",     "*extra_description",
        "*resets",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    room = room_index_new ();

    READ_PROP_STRP (room->area_str,    "area");
    READ_PROP_INT  (room->anum,        "anum");
    READ_PROP_STRP (room->name,        "name");
    READ_PROP_STRP_NL (room->description, "description");

    READ_PROP_STR (buf, "sector_type");
    room->sector_type = sector_lookup (buf);

    READ_PROP_STRP (room->owner,       "owner");
    READ_PROP_FLAGS(room->room_flags,  "room_flags", room_flags);
    if (json_get (json, "heal_rate"))
        READ_PROP_INT (room->heal_rate, "heal_rate");
    if (json_get (json, "mana_rate"))
        READ_PROP_INT (room->mana_rate, "mana_rate");

    READ_PROP_STR (buf, "clan");
    if (buf[0] != '\0')
        room->clan = lookup_func_backup (clan_lookup_exact,
            buf, "Unknown clan '%s'", 0);

    READ_PROP_STR (buf, "portal");
    if (buf[0] != '\0')
        room->portal = portal_exit_create (buf, room, DIR_NONE);

    array = json_get (json, "doors");
    if (array != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if ((exit = json_objr_exit (sub, room, &dir)) == NULL)
                continue;
            if (dir < 0 || dir >= DIR_MAX) {
                exit_free (exit);
                continue;
            }
            room->exit[dir] = exit;
        }
    }

    array = json_get (json, "resets");
    if (array != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if ((reset = json_objr_reset (sub, room)) == NULL)
                continue;
            LISTB_BACK (reset, next, room->reset_first, room->reset_last);
        }
    }

    array = json_get (json, "extra_description");
    if (array != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if ((ed = json_objr_extra_descr (sub)) == NULL)
                continue;
            LIST_BACK (ed, next, room->extra_descr, EXTRA_DESCR_T);
        }
    }

    NO_NULL_STR (room->area_str);
    NO_NULL_STR (room->owner);
    NO_NULL_STR (room->name);
    NO_NULL_STR (room->description);

    return room;
}

EXTRA_DESCR_T *json_objr_extra_descr (const JSON_T *json) {
    EXTRA_DESCR_T *ed;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("extra_description", json,
        "keyword", "description", NULL
    ))
        return NULL;

    ed = extra_descr_new ();

    READ_PROP_STRP (ed->keyword,     "keyword");
    READ_PROP_STRP_NL (ed->description, "description");

    NO_NULL_STR (ed->keyword);
    NO_NULL_STR (ed->description);

    return ed;
}

EXIT_T *json_objr_exit (const JSON_T *json, ROOM_INDEX_T *room,
    int *dir_out)
{
    EXIT_T *exit;
    JSON_T *sub;
    char buf[MAX_STRING_LENGTH];

    if (dir_out)
        *dir_out = -1;
    if (!json_import_expect ("exit", json,
        "dir",

        "*keyword", "*description", "*exit_flags", "*key", "*portal", "*to",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates in room! */
    exit = exit_new ();

    READ_PROP_STR (buf, "dir");
    *dir_out = door_lookup (buf);
    exit->orig_door = *dir_out;

    sub = json_get (json, "to");
    if (sub != NULL && sub->type != JSON_NULL)
        READ_PROP_INT (exit->room_anum, "to");
    else
        exit->room_anum = -1;

    READ_PROP_STRP    (exit->keyword,     "keyword");
    READ_PROP_STRP_NL (exit->description, "description");
    READ_PROP_FLAGS   (exit->exit_flags,  "exit_flags", exit_flags);

    if ((sub = json_get (json, "key")) != NULL) {
        if (sub->type == JSON_STRING && strcmp ((char *) sub->value, "nokey") == 0)
            exit->key = KEY_NOKEY;
        else
            json_objr_anum (sub, ANUM_OBJ, &(exit->key), room->area_str);
    }

    READ_PROP_STR (buf, "portal");
    if (buf[0] != '\0') {
        exit->portal = portal_exit_create (buf, room, *dir_out);
        exit->room_anum = -1;
    }

    NO_NULL_STR (exit->keyword);
    NO_NULL_STR (exit->description);

    exit->rs_flags = exit->exit_flags;
    return exit;
}

RESET_T *json_objr_reset (const JSON_T *json, ROOM_INDEX_T *room)
{
    RESET_T *reset;
    JSON_T *values;
    char buf[MAX_STRING_LENGTH];
    char command;

    if (!json_import_expect ("reset", json,
        "command", "values", NULL
    ))
        return NULL;

    reset = reset_data_new ();

    READ_PROP_STR (buf, "command");
         if (strcmp (buf, "mobile")    == 0) command = 'M';
    else if (strcmp (buf, "object")    == 0) command = 'O';
    else if (strcmp (buf, "give")      == 0) command = 'G';
    else if (strcmp (buf, "equip")     == 0) command = 'E';
    else if (strcmp (buf, "put")       == 0) command = 'P';
    else if (strcmp (buf, "randomize") == 0) command = 'R';
    else {
        json_logf (json, "json_objr_reset(): Unknown command '%s'.\n",
            buf);
        command = buf[0];
    }

    reset->command = command;
    values = json_get (json, "values");
    json_objr_reset_values (values, &(reset->v), command, room);

    return reset;
}

void json_objr_reset_values (const JSON_T *json, RESET_VALUE_T *v,
    char command, ROOM_INDEX_T *room)
{
    char buf[MAX_STRING_LENGTH];
    sh_int *room_vnum = NULL;
    const char *backup_area = room->area_str;

    switch (command) {
        case 'M':
            if (!json_import_expect ("reset.mobile", json,
                "mob", "global_limit", "room_limit", NULL
            ))
                return;

            json_objr_anum (json_get (json, "mob"),
                ANUM_MOB, &(v->mob.mob_vnum), backup_area);
            READ_PROP_INT (v->mob.global_limit, "global_limit");
            READ_PROP_INT (v->mob.room_limit,   "room_limit");

            room_vnum = &(v->mob.room_vnum);
            break;

        case 'O':
            if (!json_import_expect ("reset.object", json,
                "obj", "global_limit", "room_limit", NULL
            ))
                return;

            json_objr_anum (json_get (json, "obj"),
                ANUM_OBJ, &(v->obj.obj_vnum), backup_area);
            READ_PROP_INT (v->obj.global_limit, "global_limit");
            READ_PROP_INT (v->obj.room_limit,   "room_limit");

            room_vnum = &(v->obj.room_vnum);
            break;

        case 'G':
            if (!json_import_expect ("reset.give", json,
                "obj", "global_limit", NULL
            ))
                return;

            v->value[0] = 1;
            json_objr_anum (json_get (json, "obj"),
                ANUM_OBJ, &(v->give.obj_vnum), backup_area);
            READ_PROP_INT (v->give.global_limit, "global_limit");
            break;

        case 'E':
            if (!json_import_expect ("reset.equip", json,
                "obj", "wear_loc", "global_limit", NULL
            ))
                return;

            v->value[0] = 1;
            json_objr_anum (json_get (json, "obj"),
                ANUM_OBJ, &(v->equip.obj_vnum), backup_area);
            READ_PROP_STR (buf, "wear_loc");
            v->equip.wear_loc = wear_loc_lookup (buf);
            READ_PROP_INT (v->equip.global_limit, "global_limit");
            break;

        case 'P':
            if (!json_import_expect ("reset.put", json,
                "obj", "into", "global_limit", "put_count", NULL
            ))
                return;

            v->value[0] = 1;
            json_objr_anum (json_get (json, "obj"),
                ANUM_OBJ, &(v->put.obj_vnum), backup_area);
            json_objr_anum (json_get (json, "into"),
                ANUM_OBJ, &(v->put.into_vnum), backup_area);
            READ_PROP_INT (v->put.global_limit, "global_limit");
            READ_PROP_INT (v->put.put_count,    "put_count");
            break;

        case 'R':
            if (!json_import_expect ("reset.randomize", json,
                "dir_count", NULL
            ))
                return;

            READ_PROP_INT (v->randomize.dir_count, "dir_count");
            room_vnum = &(v->randomize.room_vnum);
            break;

        default:
            if (!json_import_expect ("reset.general", json,
                "value0", "value1", "value2", "value3", "value4", NULL
            ))
                return;

            READ_PROP_INT (v->value[0], "value0");
            READ_PROP_INT (v->value[1], "value1");
            READ_PROP_INT (v->value[2], "value2");
            READ_PROP_INT (v->value[3], "value3");
            READ_PROP_INT (v->value[4], "value4");
            break;
    }

    if (room_vnum != NULL) {
        ANUM_T *anum = anum_new ();
        anum->type     = ANUM_ROOM;
        anum->area_str = str_dup (backup_area);
        anum->anum     = room->anum;
        anum->vnum_ref = room_vnum;
    }
}

SHOP_T *json_objr_shop (const JSON_T *json, const char *backup_area) {
    SHOP_T *shop;
    JSON_T *array, *sub;
    char buf[MAX_STRING_LENGTH];
    int i;

    if (!json_import_expect ("shop", json,
        "trades", "profit_buy", "profit_sell", "open_hour", "close_hour",
        NULL
    ))
        return NULL;

    shop = shop_new ();

    array = json_get (json, "trades");
    if (array != NULL) {
        for (sub = array->first_child, i = 0; sub != NULL && i < MAX_TRADE;
             sub = sub->next, i++)
        {
            json_value_as_string (sub, buf, sizeof (buf));
            type_t type = type_lookup_exact (item_types, buf);
            if (type < 0 || type >= ITEM_MAX || type == TYPE_NONE) {
                json_logf (json, "json_objr_shop(): Invalid item type "
                    "'%s'.\n", buf);
                continue;
            }
            shop->buy_type[i] = type;
        }
    }

    READ_PROP_INT (shop->profit_buy,  "profit_buy");
    READ_PROP_INT (shop->profit_sell, "profit_sell");
    READ_PROP_INT (shop->open_hour,   "open_hour");
    READ_PROP_INT (shop->close_hour,  "close_hour");

    LISTB_BACK (shop, next, shop_first, shop_last);
    return shop;
}

MOB_INDEX_T *json_objr_mobile (const JSON_T *json) {
    MOB_INDEX_T *mob;
    JSON_T *array, *sub;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("mobile", json,
        "area",       "anum",        "name",        "short_descr",
        "long_descr", "description", "race",
        "alignment",  "level",       "hitroll",
        "hit_dice",   "mana_dice",   "damage_dice",
        "ac",         "wealth",      "size",

        "*start_pos", "*default_pos",     "*sex",         "*material",
        "*mob_flags", "*mob_flags_minus", "*affected_by", "*affected_by_minus",
        "*offense",   "*offense_minus",   "*immune",      "*immune_minus",
        "*resist",    "*resist_minus",    "*vulnerable",  "*vulnerable_minus",
        "*form",      "*form_minus",      "*parts",       "*parts_minus",
        "*shop",      "*spec_fun",        "*group",       "*attack_type",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    mob = mob_index_new ();

    READ_PROP_STRP (mob->area_str,     "area");
    READ_PROP_INT  (mob->anum,         "anum");
    READ_PROP_STRP (mob->name,         "name");
    READ_PROP_STRP (mob->short_descr,  "short_descr");
    READ_PROP_STRP_NL (mob->long_descr,  "long_descr");
    READ_PROP_STRP_NL (mob->description, "description");

    READ_PROP_STR (buf, "race");
    mob->race = lookup_func_backup (race_lookup_exact, buf,
        "Unknown race '%s'", 0);

    READ_PROP_STR (buf, "material");
    if (buf[0] != '\0')
        mob->material = lookup_func_backup (material_lookup_exact, buf,
            "Unknown material '%s'", MATERIAL_GENERIC);

    READ_PROP_INT  (mob->alignment,    "alignment");
    READ_PROP_INT  (mob->group,        "group");
    READ_PROP_INT  (mob->level,        "level");
    READ_PROP_INT  (mob->hitroll,      "hitroll");

    mob->hit    = json_value_as_dice (json_get (json, "hit_dice"));
    mob->mana   = json_value_as_dice (json_get (json, "mana_dice"));
    mob->damage = json_value_as_dice (json_get (json, "damage_dice"));

    READ_PROP_STR (buf, "attack_type");
    if (buf[0] != '\0')
        mob->attack_type = lookup_func_backup (attack_lookup_exact,
            buf, "Unknown attack '%s'", 0);

    array = json_get (json, "ac");
    for (sub = array->first_child; sub != NULL; sub = sub->next) {
        const TYPE_T *type = type_get_by_name (ac_types, sub->name);
        if (type == NULL) {
            json_logf (json, "json_objr_mobile(): Invalid ac type "
                "'%s'.\n", sub->name);
            continue;
        }
        mob->ac[type->type] = json_value_as_int (sub);
    }

    READ_PROP_STR (buf, "start_pos");
    if (buf[0] != '\0')
        mob->start_pos = lookup_func_backup (position_lookup_exact,
            buf, "Unknown start position '%s'", POS_STANDING);

    READ_PROP_STR (buf, "default_pos");
    if (buf[0] != '\0')
        mob->default_pos = lookup_func_backup (position_lookup_exact,
            buf, "Unknown default position '%s'", POS_STANDING);

    READ_PROP_STR (buf, "sex");
    if (buf[0] != '\0')
        mob->sex = lookup_func_backup (sex_lookup_exact,
            buf, "Unknown sex '%s'", SEX_EITHER);

    READ_PROP_INT  (mob->wealth,          "wealth");

    READ_PROP_EXT_FLAGS (mob->ext_mob_plus, "mob_flags",   mob_flags);
    READ_PROP_FLAGS (mob->affected_by_plus, "affected_by", affect_flags);
    READ_PROP_FLAGS (mob->off_flags_plus,   "offense",     off_flags);
    READ_PROP_FLAGS (mob->imm_flags_plus,   "immune",      res_flags);
    READ_PROP_FLAGS (mob->res_flags_plus,   "resist",      res_flags);
    READ_PROP_FLAGS (mob->vuln_flags_plus,  "vulnerable",  res_flags);
    READ_PROP_FLAGS (mob->form_plus,        "form",        form_flags);
    READ_PROP_FLAGS (mob->parts_plus,       "parts",       part_flags);

    READ_PROP_EXT_FLAGS (mob->ext_mob_minus, "mob_flags_minus",   mob_flags);
    READ_PROP_FLAGS (mob->affected_by_minus, "affected_by_minus", affect_flags);
    READ_PROP_FLAGS (mob->off_flags_minus,   "offense_minus",     off_flags);
    READ_PROP_FLAGS (mob->imm_flags_minus,   "immune_minus",      res_flags);
    READ_PROP_FLAGS (mob->res_flags_minus,   "resist_minus",      res_flags);
    READ_PROP_FLAGS (mob->vuln_flags_minus,  "vulnerable_minus",  res_flags);
    READ_PROP_FLAGS (mob->form_minus,        "form_minus",        form_flags);
    READ_PROP_FLAGS (mob->parts_minus,       "parts_minus",       part_flags);

    READ_PROP_STR (buf, "size");
    mob->size = lookup_func_backup (size_lookup_exact, buf,
        "Unknown size '%s'", SIZE_MEDIUM);

    sub = json_get (json, "shop");
    if (sub != NULL && sub->type != JSON_NULL)
        mob->shop = json_objr_shop (sub, mob->area_str);

    READ_PROP_STR (buf, "spec_fun");
    if (buf[0] != '\0') {
        mob->spec_fun = spec_lookup_function (buf);
        if (mob->spec_fun == NULL)
            json_logf (json, "json_objr_mobile(): Unknown special "
                "function '%s'", buf);
    }

    NO_NULL_STR (mob->name);
    NO_NULL_STR (mob->short_descr);
    NO_NULL_STR (mob->long_descr);
    NO_NULL_STR (mob->description);

    /* Post-processing on loaded mobs. */
    db_finalize_mob (mob);

    newmob_count++;
    return mob;
}

OBJ_INDEX_T *json_objr_object (const JSON_T *json) {
    OBJ_INDEX_T *obj;
    JSON_T *array, *sub;
    EXTRA_DESCR_T *ed;
    AFFECT_T *aff;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("object", json,
        "area",        "anum",      "name",      "short_descr",
        "description", "item_type", "values",    "level",
        "weight",      "cost",

        "*material", "*extra_flags", "*wear_flags", "*extra_description",
        "*affects",  "*condition",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    obj = obj_index_new();
    obj->new_format = TRUE;
    obj->condition = 100;

    READ_PROP_STRP (obj->area_str,     "area");
    READ_PROP_INT  (obj->anum,         "anum");
    READ_PROP_STRP (obj->name,         "name");
    READ_PROP_STRP (obj->short_descr,  "short_descr");
    READ_PROP_STRP (obj->description,  "description");

    READ_PROP_STR (buf, "material");
    if (buf[0] != '\0')
        obj->material = lookup_func_backup (material_lookup_exact,
            buf, "Unknown material '%s'", MATERIAL_GENERIC);

    READ_PROP_STR (buf, "item_type");
    obj->item_type = lookup_func_backup (item_lookup_exact,
        buf, "Unknown item type '%s'", ITEM_NONE);

    READ_PROP_FLAGS (obj->extra_flags, "extra_flags", extra_flags);
    READ_PROP_FLAGS (obj->wear_flags,  "wear_flags",  wear_flags);

    array = json_get (json, "values");
    if (array != NULL)
        json_objr_object_values (array, obj);

    READ_PROP_INT (obj->level,     "level");
    READ_PROP_INT (obj->weight,    "weight");
    READ_PROP_INT (obj->cost,      "cost");
    if (json_get (json, "condition"))
        READ_PROP_INT (obj->condition, "condition");

    array = json_get (json, "extra_description");
    if (array != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if ((ed = json_objr_extra_descr (sub)) == NULL)
                continue;
            LIST_BACK (ed, next, obj->extra_descr, EXTRA_DESCR_T);
        }
    }

    array = json_get (json, "affects");
    if (array != NULL) {
        for (sub = array->first_child; sub != NULL; sub = sub->next) {
            if ((aff = json_objr_affect (sub)) == NULL)
                continue;
            LIST_BACK (aff, next, obj->affected, AFFECT_T);
        }
    }

    NO_NULL_STR (obj->name);
    NO_NULL_STR (obj->short_descr);
    NO_NULL_STR (obj->description);

    /* Post-processing on loaded objs. */
    db_finalize_obj (obj);

    newobj_count++;
    return obj;
}

void json_objr_object_values (const JSON_T *json, OBJ_INDEX_T *obj) {
    const OBJ_MAP_T *map;
    const OBJ_MAP_VALUE_T *value;
    const JSON_T *sub;
    flag_t *vobj;
    int i;
    char buf[MAX_STRING_LENGTH];

    map = obj_map_get (obj->item_type);

    if (map == NULL) {
        json_logf (json, "json_objr_object(): No map for object "
            "'%s' (%d).", item_get_name (obj->item_type), obj->item_type);
        return;
    }

    for (i = 0; i < OBJ_VALUE_MAX; i++) {
        vobj = &(obj->v.value[i]);
        if ((value = obj_map_value_get (map, i)) == NULL)
            continue;
        *vobj = value->default_value;

        if (value->type == MAP_IGNORE || value->name == NULL) {
            snprintf (buf, sizeof (buf), "value%d", i);
            if ((sub = json_get (json, buf)) != NULL)
                *vobj = json_value_as_int (sub);
            continue;
        }

        sub = json_get (json, value->name);
        if (sub == NULL)
            continue;

        switch (value->type) {
            case MAP_INTEGER:
                READ_PROP_INT (*vobj, value->name);
                break;
            case MAP_BOOLEAN:
                READ_PROP_BOOL (*vobj, value->name);
                break;

            case MAP_LOOKUP: {
                buf[0] = '\0';
                READ_PROP_STR (buf, value->name);
                if (buf[0] != '\0')
                    *vobj = map_lookup_get_type (value->sub_type, buf);
                break;
            }

            case MAP_FLAGS: {
                buf[0] = '\0';
                READ_PROP_STR (buf, value->name);
                if (buf[0] != '\0')
                    *vobj = map_flags_get_value (value->sub_type, buf);
                break;
            }

            default:
                json_logf (json, "json_new_obj_object(): Cannot convert map "
                    "type '%d' to JSON value", value->type);
                continue;
        }
    }
}

AREA_T *json_objr_area (const JSON_T *json) {
    AREA_T *area;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("area", json,
        "name",      "filename", "title",    "credits",
        "min_vnum",  "max_vnum", "builders", "security",

        "*low_range", "*high_range",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    area = area_new ();

    READ_PROP_STRP (area->name,       "name");
    READ_PROP_STRP (area->filename,   "filename");
    READ_PROP_STRP (area->title,      "title");
    READ_PROP_STRP (area->credits,    "credits");
    READ_PROP_INT  (area->low_range,  "low_range");
    READ_PROP_INT  (area->high_range, "high_range");
    READ_PROP_INT  (area->min_vnum,   "min_vnum");
    READ_PROP_INT  (area->max_vnum,   "max_vnum");
    READ_PROP_STRP (area->builders,   "builders");
    READ_PROP_INT  (area->security,   "security");

    area->age     = 15;
    area->nplayer = 0;
    area->empty   = FALSE;
    area->area_flags = 0;

    NO_NULL_STR (area->name);
    NO_NULL_STR (area->filename);
    NO_NULL_STR (area->title);
    NO_NULL_STR (area->credits);
    NO_NULL_STR (area->builders);

    LISTB_BACK (area, next, area_first, area_last);
    return area;
}

SOCIAL_T *json_objr_social (const JSON_T *json) {
    SOCIAL_T *social;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("social", json,
        "name",

        "*char_no_arg", "*others_no_arg",  "*char_found", "*others_found",
        "*vict_found",  "*char_not_found", "*char_auto",  "*others_auto",
        "*min_pos",

        NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    social = social_new ();

    READ_PROP_STRP (social->name,           "name");
    READ_PROP_STRP (social->char_no_arg,    "char_no_arg");
    READ_PROP_STRP (social->others_no_arg,  "others_no_arg");
    READ_PROP_STRP (social->char_found,     "char_found");
    READ_PROP_STRP (social->others_found,   "others_found");
    READ_PROP_STRP (social->vict_found,     "vict_found");
    READ_PROP_STRP (social->char_not_found, "char_not_found");
    READ_PROP_STRP (social->char_auto,      "char_auto");
    READ_PROP_STRP (social->others_auto,    "others_auto");

    READ_PROP_STR (buf, "min_pos");
    if (buf[0] != '\0')
        social->min_pos = lookup_func_backup (position_lookup_exact,
            buf, "Unknown start position '%s'", POS_RESTING);

    return social;
}

PORTAL_T *json_objr_portal (const JSON_T *json) {
    PORTAL_T *portal;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("portal", json,
        "two-way", "from", "to", NULL
    ))
        return NULL;

    portal = portal_new ();

    READ_PROP_BOOL (portal->two_way,   "two-way");
    READ_PROP_STRP (portal->name_from, "from");
    READ_PROP_STRP (portal->name_to,   "to");

    return portal;
}

HELP_AREA_T *json_objr_help_area (const JSON_T *json) {
    JSON_T *pages, *sub;
    HELP_AREA_T *area;
    HELP_T *help;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("help_area", json,
        "area", "name", "filename", "pages", NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    area = had_new ();

    READ_PROP_STRP (area->area_str, "area");
    READ_PROP_STRP (area->name,     "name");
    READ_PROP_STRP (area->filename, "filename");

    pages = json_get (json, "pages");
    if (pages == NULL || pages->type != JSON_ARRAY) {
        json_logf (json, "json_objr_help_area(): No array 'pages'.\n");
        had_free (area);
        return NULL;
    }

    for (sub = pages->first_child; sub != NULL; sub = sub->next) {
        if ((help = json_objr_help (sub)) == NULL)
            continue;
        LISTB_BACK (help, next_area, area->first, area->last);
    }

    LISTB_BACK (area, next, had_first, had_last);
    return area;
}

HELP_T *json_objr_help (const JSON_T *json) {
    HELP_T *help;
    char buf[MAX_STRING_LENGTH];
    bool hide_keywords;

    if (!json_import_expect ("help", json,
        "keyword", "text", "*level", "*hide_keywords", NULL
    ))
        return NULL;

    /* TODO: check for duplicates! */
    help = help_new ();

    READ_PROP_INT  (help->level,   "level");
    READ_PROP_STRP (help->keyword, "keyword");
    READ_PROP_STRP (help->text,    "text");
    READ_PROP_BOOL (hide_keywords, "hide_keywords");

    if (hide_keywords != 0 && help->level >= 0)
        help->level = -1 - help->level;
    else if (hide_keywords == 0 && help->level < 0)
        help->level = -1 - help->level;

    LISTB_BACK (help, next, help_first, help_last);
    return help;
}

AFFECT_T *json_objr_affect (JSON_T *json) {
    AFFECT_T *affect;
    const AFFECT_BIT_T *bits;
    char buf[MAX_STRING_LENGTH];

    if (!json_import_expect ("affect", json,
        "level", "*apply", "*modifier", "*bit_type", "*bits", NULL
    ))
        return NULL;

    affect = affect_new ();

    READ_PROP_INT  (affect->level,    "level");

    if (json_get (json, "apply") != NULL) {
        READ_PROP_TYPE (affect->apply,    "apply", affect_apply_types);
        READ_PROP_INT  (affect->modifier, "modifier");
    }

    if (json_get (json, "bits") != NULL) {
        READ_PROP_STR (buf, "bit_type");
        if ((bits = affect_bit_get_by_name (buf)) == NULL) {
            json_logf (json, "json_objr_affect(): Invalid bit_type '%s'.\n",
                buf);
        }
        else {
            affect->bit_type = bits->type;
            READ_PROP_FLAGS (affect->bits, "bits", bits->flags);
        }
    }

    return affect;
}

ANUM_T *json_objr_anum (const JSON_T *json, int type, sh_int *vnum_ptr,
    const char *backup_area)
{
    ANUM_T *anum;
    char buf[MAX_STRING_LENGTH];

    anum = anum_new ();

    if (json->type == JSON_OBJECT) {
        if (!json_import_expect ("anum", json,
            "area", "anum", NULL
        ))
            return NULL;
        READ_PROP_STRP (anum->area_str, "area");
        READ_PROP_INT  (anum->anum,     "anum");
    }
    else {
        anum->area_str = str_dup (backup_area);
        anum->anum = json_value_as_int (json);
    }

    anum->type = type;
    anum->vnum_ref = vnum_ptr;

    return anum;
}
