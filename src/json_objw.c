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

#include "json_objw.h"

#include "json.h"
#include "lookup.h"
#include "utils.h"

#include <stdio.h>
#include <string.h>

JSON_PROP_FUN_OF (obj_room, json_objw_room, const ROOM_INDEX_T *);
JSON_T *json_objw_room (const char *name, const ROOM_INDEX_T *room) {
    JSON_T *new, *sub;
    EXTRA_DESCR_T *ed;
    RESET_T *r;
    char obj_name[256];
    int i;

    if (room == NULL)
        return json_new_null (name);
    snprintf (obj_name, sizeof (obj_name), "room #%d", room->vnum);
    new = json_new_object (name, JSON_OBJ_ROOM);

    json_prop_string  (new, "area",        room->area->name);
    json_prop_integer (new, "anum",        room->anum);
    json_prop_string  (new, "name",        JSTR (room->name));
    json_prop_string_without_last_newline (new, "description", obj_name,
        room->description);
    json_prop_string  (new, "sector_type", sector_get_name(room->sector_type));

    if (json_not_blank (room->owner))
        json_prop_string  (new, "owner",       JSTR (room->owner));
    if (room->room_flags != 0)
        json_prop_string  (new, "room_flags",  JBITS (room_bit_name (room->room_flags)));
    if (room->heal_rate != 100)
        json_prop_integer (new, "heal_rate",   room->heal_rate);
    if (room->mana_rate != 100)
        json_prop_integer (new, "mana_rate",   room->mana_rate);
    if (room->clan != 0)
        json_prop_string  (new, "clan",        JSTR (clan_get_name (room->clan)));
    if (room->portal && json_not_blank (room->portal->name))
        json_prop_string (new, "portal", room->portal->name);

    for (i = 0; i < DIR_MAX; i++)
        if (room->exit[i] != NULL)
            break;
    if (i < DIR_MAX) {
        int j;
        sub = json_prop_array (new, "doors");
        for (i = 0; i < DIR_MAX; i++) {
            for (j = 0; j < DIR_MAX; j++) {
                if (room->exit[j] && room->exit[j]->orig_door == i)
                    json_prop_obj_exit (sub, NULL, room, room->exit[j]);
            }
        }
    }

    if (room->extra_descr_first) {
        sub = json_prop_array (new, "extra_description");
        for (ed = room->extra_descr_first; ed != NULL; ed = ed->on_next)
            json_prop_obj_extra_descr (sub, NULL, ed);
    }

    if (room->reset_first) {
        sub = json_prop_array (new, "resets");
        for (r = room->reset_first; r != NULL; r = r->room_next)
            json_prop_obj_reset (sub, NULL, r);
    }

    return new;
}

JSON_PROP_FUN_OF (obj_extra_descr, json_objw_extra_descr, const EXTRA_DESCR_T *);
JSON_T *json_objw_extra_descr (const char *name, const EXTRA_DESCR_T *ed) {
    JSON_T *new;
    char obj_name[256];

    if (ed == NULL)
        return json_new_null (name);
    snprintf (obj_name, sizeof (obj_name), "extra_descr");
    new = json_new_object (name, JSON_OBJ_EXTRA_DESCR);

    json_prop_string (new, "keyword",     JSTR (ed->keyword));
    json_prop_string_without_last_newline (new, "description", obj_name,
        ed->description);

    return new;
}

JSON_T *json_prop_obj_exit (JSON_T *parent, const char *name,
    const ROOM_INDEX_T *from, const EXIT_T *ex)
{
    JSON_T *new = json_objw_exit (name, from, ex);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_objw_exit (const char *name, const ROOM_INDEX_T *from,
    const EXIT_T *ex)
{
    JSON_T *new;
    char obj_name[256];

    if (ex == NULL)
        return json_new_null (name);
    snprintf (obj_name, sizeof (obj_name), "room #%d, exit #%d",
        from->vnum, ex->orig_door);
    new = json_new_object (name, JSON_OBJ_EXIT);

    json_prop_string (new, "dir", door_get_name (ex->orig_door));
    if (ex->to_room != NULL && ex->to_room->area == from->area && !ex->portal)
        json_prop_integer (new, "to", ex->to_room->anum);

    if (json_not_blank (ex->keyword))
        json_prop_string (new, "keyword", JSTR (ex->keyword));
    if (json_not_blank (ex->description))
        json_prop_string_without_last_newline (new, "description", obj_name,
            ex->description);

    if (ex->rs_flags != 0)
        json_prop_string (new, "exit_flags", JBITSF (exit_flags, ex->rs_flags));
    if (ex->rs_flags & EX_ISDOOR) {
        if (ex->key == 0)
            json_prop_string (new, "key", "nokey");
        else if (ex->key >= KEY_VALID)
            json_prop_obj_anum (new, "key", from->area, ex->key);
    }
    if (ex->portal && json_not_blank (ex->portal->name))
        json_prop_string (new, "portal", ex->portal->name);

    return new;
}

JSON_PROP_FUN_OF (obj_shop, json_objw_shop, const SHOP_T *);
JSON_T *json_objw_shop (const char *name, const SHOP_T *shop) {
    JSON_T *new, *sub;
    int i;

    if (shop == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SHOP);

    sub = json_prop_array (new, "trades");
    for (i = 0; i < MAX_TRADE; i++) {
        const char *name = shop->buy_type[i] <= 0 ? NULL :
            item_get_name (shop->buy_type[i]);
        if (name == NULL)
            continue;
        if (strcmp (name, "unknown") == 0) {
            bugf ("json_objw_stop(): Unknown item type '%d'.",
                shop->buy_type[i]);
            continue;
        }
        json_prop_string (sub, NULL, name);
    }

    json_prop_integer (new, "profit_buy",  shop->profit_buy);
    json_prop_integer (new, "profit_sell", shop->profit_sell);
    json_prop_integer (new, "open_hour",   shop->open_hour);
    json_prop_integer (new, "close_hour",  shop->close_hour);

    return new;
}

JSON_PROP_FUN_OF (obj_mobile, json_objw_mobile, const MOB_INDEX_T *);
JSON_T *json_objw_mobile (const char *name, const MOB_INDEX_T *mob) {
    JSON_T *new, *sub;
    const char *spec_fun;
    char obj_name[256];
    int i;

    if (mob == NULL)
        return json_new_null (name);
    snprintf (obj_name, sizeof (obj_name), "mob #%d", mob->vnum);
    new = json_new_object (name, JSON_OBJ_MOBILE);

    json_prop_string  (new, "area",        mob->area->name);
    json_prop_integer (new, "anum",        mob->anum);
    json_prop_string  (new, "name",        JSTR (mob->name));
    json_prop_string  (new, "short_descr", JSTR (mob->short_descr));
    json_prop_string_without_last_newline (new, "long_descr", obj_name,
        mob->long_descr);
    json_prop_string_without_last_newline (new, "description", obj_name,
        mob->description);
    json_prop_string  (new, "race",        JSTR (race_get_name (mob->race)));

    if (mob->material != MATERIAL_GENERIC)
        json_prop_string  (new, "material", JSTR (material_get_name (mob->material)));

    json_prop_integer (new, "alignment",   mob->alignment);
    if (mob->group != 0)
        json_prop_integer (new, "group", mob->group);
    json_prop_integer (new, "level",       mob->level);
    json_prop_integer (new, "hitroll",     mob->hitroll);
    json_prop_dice    (new, "hit_dice",    &(mob->hit));
    json_prop_dice    (new, "mana_dice",   &(mob->mana));
    json_prop_dice    (new, "damage_dice", &(mob->damage));

    if (mob->attack_type > 0)
        json_prop_string (new, "attack_type", JSTR (attack_get_name (mob->attack_type)));

    sub = json_prop_object (new, "ac", JSON_OBJ_ANY);
    for (i = 0; i < AC_MAX; i++)
        json_prop_integer (sub, ac_types[i].name, mob->ac[ac_types[i].type]);

    if (mob->start_pos != POS_STANDING)
        json_prop_string (new, "start_pos",
            JSTR (position_get_name (mob->start_pos)));
    if (mob->default_pos != POS_STANDING)
        json_prop_string  (new, "default_pos",
            JSTR (position_get_name (mob->default_pos)));
    if (mob->sex != SEX_EITHER)
        json_prop_string  (new, "sex",
            JSTR (sex_get_name (mob->sex)));

    json_prop_integer  (new, "wealth", mob->wealth);
    json_prop_string   (new, "size",   JSTR (size_get_name (mob->size)));
    if (mob->shop)
        json_prop_obj_shop (new, "shop", mob->shop);

    if (EXT_IS_NONZERO (mob->ext_mob_plus))
        json_prop_string (new, "mob_flags",   JBITSXF (mob_flags, EXT_WITHOUT (mob->ext_mob_plus, MOB_IS_NPC)));
    if (mob->affected_by_plus != 0)
        json_prop_string (new, "affected_by", JBITSF (affect_flags, mob->affected_by_plus));
    if (mob->off_flags_plus != 0)
        json_prop_string (new, "offense",     JBITSF (off_flags, mob->off_flags_plus));
    if (mob->imm_flags_plus != 0)
        json_prop_string (new, "immune",      JBITSF (res_flags, mob->imm_flags_plus));
    if (mob->res_flags_plus != 0)
        json_prop_string (new, "resist",      JBITSF (res_flags, mob->res_flags_plus));
    if (mob->vuln_flags_plus != 0)
        json_prop_string (new, "vulnerable",  JBITSF (res_flags, mob->vuln_flags_plus));
    if (mob->form_plus != 0)
        json_prop_string (new, "form",        JBITSF (form_flags, mob->form_plus));
    if (mob->parts_plus != 0)
        json_prop_string (new, "parts",       JBITSF (part_flags, mob->parts_plus));

    if (EXT_IS_NONZERO (mob->ext_mob_minus))
        json_prop_string (new, "mob_flags_minus",   JBITSXF (mob_flags, mob->ext_mob_minus));
    if (mob->affected_by_minus != 0)
        json_prop_string (new, "affected_by_minus", JBITSF (affect_flags, mob->affected_by_minus));
    if (mob->off_flags_minus != 0)
        json_prop_string (new, "offense_minus",     JBITSF (off_flags, mob->off_flags_minus));
    if (mob->imm_flags_minus != 0)
        json_prop_string (new, "immune_minus",      JBITSF (res_flags, mob->imm_flags_minus));
    if (mob->res_flags_minus != 0)
        json_prop_string (new, "resist_minus",      JBITSF (res_flags, mob->res_flags_minus));
    if (mob->vuln_flags_minus != 0)
        json_prop_string (new, "vulnerable_minus",  JBITSF (res_flags, mob->vuln_flags_minus));
    if (mob->form_minus != 0)
        json_prop_string (new, "form_minus",        JBITSF (form_flags, mob->form_minus));
    if (mob->parts_minus != 0)
        json_prop_string (new, "parts_minus",       JBITSF (part_flags, mob->parts_minus));

    if (mob->mprog_first)
        bugf ("*** Ignoring '%s' (#%d) mprogs ***", mob->short_descr, mob->vnum);

    spec_fun = spec_function_name (mob->spec_fun);
    if (spec_fun != NULL && spec_fun[0] != '\0')
        json_prop_string (new, "spec_fun", JSTR (spec_fun));

    return new;
}

JSON_PROP_FUN_OF (obj_object, json_objw_object, const OBJ_INDEX_T *);
JSON_T *json_objw_object (const char *name, const OBJ_INDEX_T *obj) {
    JSON_T *new, *sub;
    const OBJ_MAP_T *map;
    EXTRA_DESCR_T *ed;
    AFFECT_T *aff;

    if (obj == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_string  (new, "area",        obj->area->name);
    json_prop_integer (new, "anum",        obj->anum);
    json_prop_string  (new, "name",        obj->name);
    json_prop_string  (new, "short_descr", JSTR (obj->short_descr));
    json_prop_string  (new, "description", JSTR (obj->description));
    if (obj->material != MATERIAL_GENERIC)
        json_prop_string (new, "material", JSTR (material_get_name (obj->material)));
    json_prop_string  (new, "item_type",   JBITS (item_get_name (obj->item_type)));

    sub = json_prop_object (new, "values", JSON_OBJ_ANY);
    map = obj_map_get (obj->item_type);
    if (map == NULL) {
        bugf ("json_objw_object: No map for object '%s' (%d).",
            item_get_name (obj->item_type), obj->item_type);
    }
    else {
        const OBJ_MAP_VALUE_T *value;
        JSON_T *vjson;
        flag_t vobj;
        int i;

        for (i = 0; i < OBJ_VALUE_MAX; i++) {
            if ((value = obj_map_value_get (map, i)) == NULL)
                continue;

            vjson = NULL;
            vobj = obj->v.value[i];

            if (vobj == value->default_value)
                continue;

            switch (value->type) {
                case MAP_INTEGER:
                    vjson = json_new_integer (value->name, vobj);
                    break;
                case MAP_BOOLEAN:
                    vjson = json_new_boolean (value->name, vobj);
                    break;
                case MAP_IGNORE: {
                    char vname[16];
                    snprintf (vname, sizeof (vname), "value%d", i);
                    vjson = json_new_integer (vname, vobj);
                    break;
                }

                case MAP_LOOKUP: {
                    const char *vstr = map_lookup_get_string (
                        value->sub_type, vobj);
                    if (vstr == NULL)
                        vjson = json_new_null (value->name);
                    else
                        vjson = json_new_string (value->name,
                            (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                case MAP_FLAGS: {
                    char vstr[256];
                    if (!map_flags_get_string (value->sub_type, vobj, vstr, sizeof(vstr)))
                        vjson = json_new_null (value->name);
                    else
                        vjson = json_new_string (value->name,
                            (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                default:
                    bugf ("json_objw_object: Cannot convert map type '%d' "
                          "to JSON value", value->type);
                    continue;
            }

            if (vjson)
                json_attach_under (vjson, sub);
        }
    }

    json_prop_integer (new, "level",  obj->level);
    json_prop_integer (new, "weight", obj->weight);
    json_prop_integer (new, "cost",   obj->cost);
    if (obj->condition != 100)
        json_prop_integer (new, "condition", obj->condition);

    if (obj->extra_flags != 0)
        json_prop_string  (new, "extra_flags", JBITS (extra_bit_name (obj->extra_flags)));
    if (obj->wear_flags != 0)
        json_prop_string  (new, "wear_flags",  JBITS (wear_flag_name (obj->wear_flags)));

    if (obj->extra_descr_first) {
        sub = json_prop_array (new, "extra_description");
        for (ed = obj->extra_descr_first; ed; ed = ed->on_next)
            json_prop_obj_extra_descr (sub, NULL, ed);
    }

    if (obj->affect_first) {
        sub = json_prop_array (new, "affects");
        for (aff = obj->affect_first; aff; aff = aff->on_next)
            json_prop_obj_affect (sub, NULL, aff);
    }

    return new;
}

JSON_PROP_FUN_OF (obj_affect, json_objw_affect, const AFFECT_T *);
JSON_T *json_objw_affect (const char *name, const AFFECT_T *aff) {
    JSON_T *new;
    const char *bv;
    const AFFECT_BIT_T *bits;

    if (aff == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_integer (new, "level",    aff->level);

    if (aff->apply != TYPE_NONE && aff->apply != APPLY_NONE) {
        json_prop_string  (new, "apply",    type_get_name (affect_apply_types,
            aff->apply));
        json_prop_integer (new, "modifier", aff->modifier);
    }

    if (aff->bits != FLAG_NONE) {
        json_prop_string  (new, "bit_type", affect_bit_get_name (aff->bit_type));

        bv = NULL;
        bits = affect_bit_get (aff->bit_type);
        if (bits == NULL)
            bugf ("json_objw_affect: Unhandled bit_type '%d'", aff->bit_type);
        else {
            bv = JBITSF (bits->flags, aff->bits);
            if (bv != NULL && bv[0] == '\0')
                bv = NULL;
        }
        json_prop_string (new, "bits", bv);
    }

    return new;
}

JSON_T *json_prop_obj_anum (JSON_T *parent, const char *name,
    AREA_T *area_from, int vnum)
{
    JSON_T *new = json_objw_anum (name, area_from, vnum);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_objw_anum (const char *name, AREA_T *area_from, int vnum) {
    if (vnum >= area_from->min_vnum && vnum <= area_from->max_vnum) {
        return json_new_integer (name, vnum - area_from->min_vnum);
    }
    else {
        const AREA_T *area;
        JSON_T *new;

        if ((area = area_get_by_inner_vnum (vnum)) == NULL) {
            bugf ("json_objw_anum(): No area for vnum '%d'", vnum);
            return json_new_null (name);
        }

        new = json_new_object (name, JSON_OBJ_ANY);
        json_prop_string  (new, "area", JSTR (area->name));
        json_prop_integer (new, "anum", vnum - area->min_vnum);
        return new;
    }
}

JSON_PROP_FUN_OF (obj_reset, json_objw_reset, const RESET_T *);
JSON_T *json_objw_reset (const char *name, const RESET_T *reset) {
    JSON_T *new, *sub;
    const RESET_VALUE_T *v;
    char *command, letter_buf[2];

    if (reset == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_RESET);

    switch (reset->command) {
        case 'M': command = "mobile";    break;
        case 'O': command = "object";    break;
        case 'G': command = "give";      break;
        case 'E': command = "equip";     break;
        case 'P': command = "put";       break;
        case 'R': command = "randomize"; break;

        default:
            letter_buf[0] = reset->command;
            letter_buf[1] = '\0';
            command = letter_buf;
            break;
    }
    json_prop_string (new, "command", command);

    sub = json_prop_object (new, "values", JSON_OBJ_ANY);
    v = &(reset->v);
    switch (reset->command) {
        case 'M':
            json_prop_obj_anum (sub, "mob", reset->area, v->mob.mob_vnum);
            json_prop_integer  (sub, "global_limit", v->mob.global_limit);
            json_prop_integer  (sub, "room_limit",   v->mob.room_limit);
            break;

        case 'O':
            json_prop_obj_anum (sub, "obj", reset->area, v->obj.obj_vnum);
            json_prop_integer  (sub, "global_limit", v->obj.global_limit);
            json_prop_integer  (sub, "room_limit",   v->obj.room_limit);
            break;

        case 'G':
            json_prop_obj_anum (sub, "obj", reset->area, v->give.obj_vnum);
            json_prop_integer  (sub, "global_limit", v->give.global_limit);
            break;

        case 'E':
            json_prop_obj_anum (sub, "obj", reset->area, v->equip.obj_vnum);
            json_prop_string   (sub, "wear_loc",
                wear_loc_get_name (v->equip.wear_loc));
            json_prop_integer  (sub, "global_limit", v->equip.global_limit);
            break;

        case 'P':
            json_prop_obj_anum (sub, "obj",  reset->area, v->put.obj_vnum);
            json_prop_obj_anum (sub, "into", reset->area, v->put.into_vnum);
            json_prop_integer  (sub, "global_limit", v->put.global_limit);
            json_prop_integer  (sub, "put_count",    v->put.put_count);
            break;

        case 'R':
            json_prop_integer (sub, "dir_count", v->randomize.dir_count);
            break;

        default:
            bugf ("json_objw_reset: Unhandled command '%c'", reset->command);
            json_prop_integer (sub, "value0", reset->v.value[0]);
            json_prop_integer (sub, "value1", reset->v.value[1]);
            json_prop_integer (sub, "value2", reset->v.value[2]);
            json_prop_integer (sub, "value3", reset->v.value[3]);
            json_prop_integer (sub, "value4", reset->v.value[4]);
    }

    return new;
}

JSON_PROP_FUN_OF (obj_area, json_objw_area, const AREA_T *);
JSON_T *json_objw_area (const char *name, const AREA_T *area) {
    JSON_T *new;
    if (area == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_AREA);

    json_prop_string  (new, "name",       area->name);
    json_prop_string  (new, "filename",   area->filename);
    json_prop_string  (new, "title",      area->title);
    json_prop_string  (new, "credits",    area->credits);
    json_prop_integer (new, "min_vnum",   area->min_vnum);
    json_prop_integer (new, "max_vnum",   area->max_vnum);
    json_prop_string  (new, "builders",   area->builders);
    json_prop_integer (new, "security",   area->security);

    if (area->low_range != 0)
        json_prop_integer (new, "low_range",  area->low_range);
    if (area->high_range != 0)
        json_prop_integer (new, "high_range", area->high_range);

    return new;
}

JSON_PROP_FUN_OF (obj_social, json_objw_social, const SOCIAL_T *);
JSON_T *json_objw_social (const char *name, const SOCIAL_T *soc) {
    JSON_T *new;
    if (soc == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SOCIAL);

    json_prop_string (new, "name",           soc->name);
    if (soc->char_no_arg)
        json_prop_string (new, "char_no_arg",    soc->char_no_arg);
    if (soc->others_no_arg)
        json_prop_string (new, "others_no_arg",  soc->others_no_arg);
    if (soc->char_found)
        json_prop_string (new, "char_found",     soc->char_found);
    if (soc->others_found)
        json_prop_string (new, "others_found",   soc->others_found);
    if (soc->vict_found)
        json_prop_string (new, "vict_found",     soc->vict_found);
    if (soc->char_not_found)
        json_prop_string (new, "char_not_found", soc->char_not_found);
    if (soc->char_auto)
        json_prop_string (new, "char_auto",      soc->char_auto);
    if (soc->others_auto)
        json_prop_string (new, "others_auto",    soc->others_auto);
    if (soc->min_pos != POS_RESTING)
        json_prop_string (new, "min_pos",
            JSTR (position_get_name (soc->min_pos)));

    return new;
}

JSON_PROP_FUN_OF (obj_table, json_objw_table, const TABLE_T *);
JSON_T *json_objw_table (const char *name, const TABLE_T *table) {
    JSON_T *new, *sub;
    char *type_str;

    if (table == NULL || table->json_write_func == NULL)
        return json_new_null (name);

    type_str = NULL;
    switch (table->type) {
        case TABLE_FLAGS:     type_str = "flags";     break;
        case TABLE_EXT_FLAGS: type_str = "ext_flags"; break;
        case TABLE_TYPES:     type_str = "types";     break;
        case TABLE_UNIQUE:    type_str = "table";     break;
        default:
            bugf ("json_objw_table: Error: table '%s' is of unhandled "
                "type %d", table->name, table->type);
            return NULL;
    }

    if (table->obj_name == NULL) {
        new = json_new_object (name, JSON_OBJ_TABLE);
        json_prop_string (new, "name",        table->name);
        json_prop_string (new, "description", table->description);
        sub = json_prop_array (new, "values");
    }
    else {
        new = json_new_array (name, NULL);
        sub = NULL;
    }

    {
        const void *obj = table->table;
        JSON_T *json;
        flag_t expected_flag = 0x01;
        type_t expected_type = -999;

        do {
            if ((json = table->json_write_func (obj, table->obj_name)) == NULL)
                break;
            if (table->obj_name)
                sub = json_prop_object (new, NULL, JSON_OBJ_ANY);

            if (table->type == TABLE_FLAGS) {
                const FLAG_T *flag = obj;
                if (flag->bit != expected_flag) {
                    bugf ("json_objw_table: Warning: %s table '%s' row "
                        "'%s' should be %ld, but it's %ld", type_str,
                        table->name, flag->name, expected_flag, flag->bit);
                }
                expected_flag <<= 1;
            }
            else if (table->type == TABLE_EXT_FLAGS) {
                const EXT_FLAG_DEF_T *ext_flag = obj;
                if (expected_type == -999)
                    expected_type = ext_flag->bit;
                else if (ext_flag->bit != expected_type) {
                    bugf ("json_objw_table: Warning: %s table '%s' row "
                        "'%s' should be %ld, but it's %ld", type_str,
                        table->name, ext_flag->name, expected_type,
                        ext_flag->bit);
                }
                ++expected_type;
            }
            else if (table->type == TABLE_TYPES) {
                const TYPE_T *type = obj;
                if (expected_type == -999)
                    expected_type = type->type;
                else if (type->type != expected_type) {
                    bugf ("json_objw_table: Warning: %s table '%s' row "
                        "'%s' should be %ld, but it's %ld", type_str,
                        table->name, type->name, expected_type, type->type);
                }
                ++expected_type;
            }

            json_attach_under (json, sub);
            obj += table->type_size;
        } while (1);
    }
    return new;
}

JSON_PROP_FUN_OF (obj_portal_exit, json_objw_portal_exit, const PORTAL_EXIT_T *);
JSON_T *json_objw_portal_exit (const char *name, const PORTAL_EXIT_T *pex) {
    JSON_T *new;
    if (pex == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL_EXIT);

    return new;
}

JSON_PROP_FUN_OF (obj_portal, json_objw_portal, const PORTAL_T *);
JSON_T *json_objw_portal (const char *name, const PORTAL_T *portal) {
    JSON_T *new;
    if (portal == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL);

    json_prop_boolean (new, "two-way", portal->two_way);
    json_prop_string  (new, "from",    portal->name_from);
    json_prop_string  (new, "to",      portal->name_to);

    return new;
}

JSON_PROP_FUN_OF (obj_help_area, json_objw_help_area, const HELP_AREA_T *);
JSON_T *json_objw_help_area (const char *name, const HELP_AREA_T *had) {
    JSON_T *new, *sub;
    HELP_T *help;
    if (had == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP_AREA);

    if (had->area)
        json_prop_string (new, "area", JSTR (had->area->name));
    else
        json_prop_string (new, "area", NULL);

    json_prop_string (new, "name",     JSTR (had->name));
    json_prop_string (new, "filename", JSTR (had->filename));

    sub = json_prop_array (new, "pages");
    for (help = had->help_first; help; help = help->had_next)
        json_prop_obj_help (sub, NULL, help);

    return new;
}

JSON_PROP_FUN_OF (obj_help, json_objw_help, const HELP_T *);
JSON_T *json_objw_help (const char *name, const HELP_T *help) {
    JSON_T *new;

    if (help == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP);

    json_prop_string  (new, "keyword", JSTR (help->keyword));
    json_prop_string  (new, "text",    JSTR (help->text));

    if (help->level >= 0) {
        if (help->level != 0)
            json_prop_integer (new, "level", help->level);
    }
    else {
        if (help->level != -1)
            json_prop_integer (new, "level", -1 - help->level);
        json_prop_boolean (new, "hide_keywords", TRUE);
    }

    return new;
}
