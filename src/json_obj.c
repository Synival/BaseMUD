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

#include <string.h>

#include "lookup.h"
#include "utils.h"

#include "json_obj.h"

const char *json_not_none (const char *value) {
    return (value == NULL || !strcmp (value, "none"))
        ? NULL : value;
}

const char *json_not_blank (const char *value) {
    return (value == NULL || value[0] == '\0')
        ? NULL : value;
}

JSON_PROP_FUNC (obj_room, const ROOM_INDEX_DATA *);
JSON_T *json_new_obj_room (const char *name, const ROOM_INDEX_DATA *room) {
    JSON_T *new, *sub;
    EXTRA_DESCR_DATA *ed;
    RESET_DATA *r;
    int i;

    if (room == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_ROOM);

    json_prop_string  (new, "area",        room->area->name);
    json_prop_integer (new, "anum",        room->anum);
    json_prop_string  (new, "owner",       JSTR (room->owner));
    json_prop_string  (new, "name",        JSTR (room->name));
    json_prop_string  (new, "description", JSTR (room->description));
    json_prop_string  (new, "room_flags",  JBITS (room_bit_name (room->room_flags)));
    json_prop_string  (new, "sector_type", sector_get_name(room->sector_type));
    json_prop_integer (new, "heal_rate",   room->heal_rate);
    json_prop_integer (new, "mana_rate",   room->mana_rate);
    json_prop_string  (new, "clan",        JSTR (room->clan_str));
    json_prop_string  (new, "portal",      room->portal ? room->portal->name : NULL);

    sub = json_prop_array (new, "doors");
    for (i = 0; i < DIR_MAX; i++)
        if (room->exit[i] != NULL)
            json_prop_obj_exit (sub, NULL, room, i, room->exit[i]);

    sub = json_prop_array (new, "extra_description");
    for (ed = room->extra_descr; ed != NULL; ed = ed->next)
        json_prop_obj_extra_descr (sub, NULL, ed);

    sub = json_prop_array (new, "resets");
    for (r = room->reset_first; r != NULL; r = r->next)
        json_prop_obj_reset (sub, NULL, r);

    return new;
}

JSON_PROP_FUNC (obj_extra_descr, const EXTRA_DESCR_DATA *);
JSON_T *json_new_obj_extra_descr (const char *name, const EXTRA_DESCR_DATA *ed) {
    JSON_T *new;
    if (ed == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_EXTRA_DESCR);

    json_prop_string (new, "keyword",     ed->keyword);
    json_prop_string (new, "description", ed->description);

    return new;
}

JSON_T *json_prop_obj_exit (JSON_T *parent, const char *name,
    const ROOM_INDEX_DATA *from, int dir, const EXIT_DATA *ex)
{
    JSON_T *new = json_new_obj_exit (name, from, dir, ex);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_new_obj_exit (const char *name, const ROOM_INDEX_DATA *from, int dir,
    const EXIT_DATA *ex)
{
    JSON_T *new;
    if (ex == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_EXIT);

    json_prop_string  (new, "dir", door_get_name (dir));
    if (ex->to_room == NULL || ex->to_room->area != from->area)
        json_prop_null (new, "to");
    else
        json_prop_integer (new, "to", ex->to_room->anum);

    json_prop_string  (new, "keyword",     JSTR (ex->keyword));
    json_prop_string  (new, "description", JSTR (ex->description));
    json_prop_string  (new, "exit_flags",  JBITS(
        flag_string (exit_flags, ex->rs_flags)));
    json_prop_integer (new, "key",         ex->key);
    json_prop_string  (new, "portal",      ex->portal ? ex->portal->name : NULL);

    return new;
}

JSON_PROP_FUNC (obj_shop, const SHOP_DATA *);
JSON_T *json_new_obj_shop (const char *name, const SHOP_DATA *shop) {
    JSON_T *new, *sub;
    int i;
    if (shop == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SHOP);

    json_prop_integer (new, "keeper", shop->keeper);
    sub = json_prop_array (new, "trades");
    for (i = 0; i < MAX_TRADE; i++) {
        const char *name = shop->buy_type[i] <= 0 ? NULL :
            item_get_name (shop->buy_type[i]);
        if (name == NULL)
            continue;
        json_prop_string (sub, NULL, name);
    }

    json_prop_integer (new, "profit_buy",  shop->profit_buy);
    json_prop_integer (new, "profit_sell", shop->profit_sell);
    json_prop_integer (new, "open_hour",   shop->open_hour);
    json_prop_integer (new, "close_hour",  shop->close_hour);

    return new;
}

JSON_PROP_FUNC (obj_mobile, const MOB_INDEX_DATA *);
JSON_T *json_new_obj_mobile (const char *name, const MOB_INDEX_DATA *mob) {
    JSON_T *new, *sub;
    int i;

    if (mob == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_MOBILE);

    json_prop_string  (new, "area",        mob->area->name);
    json_prop_integer (new, "anum",        mob->anum);
    json_prop_string  (new, "name",        JSTR (mob->name));
    json_prop_string  (new, "short_descr", JSTR (mob->short_descr));
    json_prop_string  (new, "long_descr",  JSTR (mob->long_descr));
    json_prop_string  (new, "description", JSTR (mob->description));
    json_prop_string  (new, "race",        JSTR (mob->race_str));
    json_prop_string  (new, "material",    JSTR (mob->material_str));
    json_prop_string  (new, "mob_flags",   JBITS(
        flag_string (mob_flags, mob->mob_orig & ~MOB_IS_NPC)));
    json_prop_string  (new, "affected_by", JBITS(
        flag_string (affect_flags, mob->affected_by_orig)));
    json_prop_integer (new, "alignment",   mob->alignment);
    json_prop_integer (new, "group",       mob->group);
    json_prop_integer (new, "level",       mob->level);
    json_prop_integer (new, "hitroll",     mob->hitroll);
    json_prop_dice    (new, "hit_dice",    mob->hit);
    json_prop_dice    (new, "mana_dice",   mob->mana);
    json_prop_dice    (new, "damage_dice", mob->damage);
    json_prop_string  (new, "dam_type",    mob->dam_type_str);

    sub = json_prop_object (new, "ac", JSON_OBJ_ANY);
    for (i = 0; i < AC_MAX; i++)
        json_prop_integer (sub, ac_types[i].name, mob->ac[ac_types[i].bit]);

    json_prop_string  (new, "offense", JBITS(
        flag_string (off_flags, mob->off_flags_orig)));
    json_prop_string  (new, "immune", JBITS(
        flag_string (res_flags, mob->imm_flags_orig)));
    json_prop_string  (new, "resist", JBITS(
        flag_string (res_flags, mob->res_flags_orig)));
    json_prop_string  (new, "vulnerable", JBITS(
        flag_string (res_flags, mob->vuln_flags_orig)));

    json_prop_string  (new, "start_pos",   JSTR (mob->start_pos_str));
    json_prop_string  (new, "default_pos", JSTR (mob->default_pos_str));
    json_prop_string  (new, "sex",         JSTR (mob->sex_str));
    json_prop_integer (new, "wealth",      mob->wealth);
    json_prop_string  (new, "form",        flag_string (form_flags, mob->form_orig));
    json_prop_string  (new, "parts",       flag_string (part_flags, mob->parts_orig));
    json_prop_string  (new, "size",        JSTR (mob->size_str));
    json_prop_obj_shop(new, "shop",        mob->pShop);

    if (mob->mprogs)
        printf ("*** Ignoring '%s' (#%d) mprogs ***\n", mob->short_descr, mob->vnum);

    return new;
}

JSON_PROP_FUNC (obj_object, const OBJ_INDEX_DATA *);
JSON_T *json_new_obj_object (const char *name, const OBJ_INDEX_DATA *obj) {
    JSON_T *new, *sub;
    const OBJ_MAP *map;
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *aff;

    if (obj == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_string  (new, "area",        obj->area->name);
    json_prop_integer (new, "anum",        obj->anum);
    json_prop_boolean (new, "new_format",  obj->new_format);
    json_prop_string  (new, "name",        obj->name);
    json_prop_string  (new, "short_descr", JSTR (obj->short_descr));
    json_prop_string  (new, "description", JSTR (obj->description));
    json_prop_string  (new, "material",    JSTR (obj->material_str));
    json_prop_string  (new, "item_type",   JBITS(item_get_name (obj->item_type)));
    json_prop_string  (new, "extra_flags", JBITS(extra_bit_name (obj->extra_flags)));
    json_prop_string  (new, "wear_flags",  JBITS(wear_flag_name (obj->wear_flags)));

    sub = json_prop_object (new, "values", JSON_OBJ_ANY);
    map = obj_map_get (obj->item_type);
    if (map == NULL) {
        bugf ("json_new_obj_object: No map for object '%s' (%d).",
            item_get_name (obj->item_type), obj->item_type);
    }
    else {
        const OBJ_MAP_VALUE *value;
        JSON_T *vjson;
        flag_t vobj;
        int i;

        for (i = 0; i < OBJ_VALUE_MAX; i++) {
            if ((value = obj_map_value_get (map, i)) == NULL)
                continue;

            vjson = NULL;
            vobj = obj->v.value[i];

            switch (value->type) {
                case MAP_INTEGER:
                    vjson = json_new_integer (value->name, vobj);
                    break;
                case MAP_BOOLEAN:
                    vjson = json_new_boolean (value->name, vobj);
                    break;
                case MAP_IGNORE:
                    vjson = NULL;
                    break;

                case MAP_LOOKUP: {
                    const char *vstr = map_lookup_get_string (
                        value->sub_type, vobj);
                    if (vstr == NULL)
                        continue;
                    vjson = json_new_string (value->name,
                        (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                case MAP_FLAGS: {
                    char vstr[256];
                    if (!map_flags_get_string (value->sub_type, vobj, vstr, sizeof(vstr)))
                        continue;
                    vjson = json_new_string (value->name,
                        (vstr[0] == '\0') ? NULL : vstr);
                    break;
                }

                default:
                    bugf ("json_new_obj_object: Cannot convert map type '%d' "
                          "to JSON value", value->type);
                    continue;
            }

            if (vjson)
                json_attach_under (vjson, sub);
        }
    }

    json_prop_integer (new, "level",     obj->level);
    json_prop_integer (new, "weight",    obj->weight);
    json_prop_integer (new, "cost",      obj->cost);
    json_prop_integer (new, "condition", obj->condition);

    sub = json_prop_array (new, "extra_description");
    for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
        json_prop_obj_extra_descr (sub, NULL, ed);

    sub = json_prop_array (new, "affects");
    for (aff = obj->affected; aff != NULL; aff = aff->next)
        json_prop_obj_affect (sub, NULL, aff);

    return new;
}

JSON_PROP_FUNC (obj_affect, const AFFECT_DATA *);
JSON_T *json_new_obj_affect (const char *name, const AFFECT_DATA *aff) {
    JSON_T *new;
    const char *bv;
    const AFFECT_BIT_TYPE *bits;

    if (aff == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJECT);

    json_prop_string  (new, "apply",    flag_string (affect_apply_types, aff->apply));
    json_prop_integer (new, "level",    aff->level);
    json_prop_integer (new, "modifier", aff->modifier);
    json_prop_string  (new, "bit_type", affect_bit_get_name (aff->bit_type));

    #define BV_BITS(flags) \
        bv = JBITS (flag_string (flags, aff->bits))

    bv = NULL;
    bits = affect_bit_get (aff->bit_type);
    if (bits == NULL)
        bugf ("json_new_obj_affect: Unhandled bit_type '%d'", aff->bit_type);
    else {
        bv = JBITS (flag_string (bits->flags, aff->bits));
        if (bv != NULL && bv[0] == '\0')
            bv = NULL;
    }

    json_prop_string (new, "bits", bv);
    return new;
}

JSON_T *json_prop_obj_anum (JSON_T *parent, const char *name,
    AREA_DATA *area_from, int vnum)
{
    JSON_T *new = json_new_obj_anum (name, area_from, vnum);
    json_attach_under (new, parent);
    return new;
}

JSON_T *json_new_obj_anum (const char *name, AREA_DATA *area_from, int vnum) {
    if (vnum >= area_from->min_vnum && vnum <= area_from->max_vnum)
        return json_new_integer (name, vnum - area_from->min_vnum);
    else {
        const AREA_DATA *area;
        JSON_T *new;

        if ((area = area_get_by_inner_vnum (vnum)) == NULL)
            return json_new_null (name);

        new = json_new_object (name, JSON_OBJ_ANY);
        json_prop_string (new, "area", JSTR (area->name));
        json_prop_integer (new, "anum", vnum - area->min_vnum);
        return new;
    }
}

JSON_PROP_FUNC (obj_reset, const RESET_DATA *);
JSON_T *json_new_obj_reset (const char *name, const RESET_DATA *reset) {
    JSON_T *new, *sub;
    const RESET_VALUE_TYPE *v;
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
            json_prop_string   (sub, "wear_loc", flag_string (wear_loc_types,
                v->equip.wear_loc));
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
            bugf ("json_new_obj_reset: Unhandled command '%c'", reset->command);
            json_prop_integer (sub, "value0", reset->v.value[0]);
            json_prop_integer (sub, "value1", reset->v.value[1]);
            json_prop_integer (sub, "value2", reset->v.value[2]);
            json_prop_integer (sub, "value3", reset->v.value[3]);
            json_prop_integer (sub, "value4", reset->v.value[4]);
    }

    return new;
}

JSON_PROP_FUNC (obj_area, const AREA_DATA *);
JSON_T *json_new_obj_area (const char *name, const AREA_DATA *area) {
    JSON_T *new;
    if (area == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_AREA);

    json_prop_string  (new, "name",       area->name);
    json_prop_string  (new, "filename",   area->filename);
    json_prop_string  (new, "title",      area->title);
    json_prop_string  (new, "credits",    area->credits);
    json_prop_integer (new, "low_range",  area->low_range);
    json_prop_integer (new, "high_range", area->high_range);
    json_prop_integer (new, "min_vnum",   area->min_vnum);
    json_prop_integer (new, "max_vnum",   area->max_vnum);
    json_prop_string  (new, "builders",   area->builders);
    json_prop_integer (new, "security",   area->security);

    return new;
}

JSON_PROP_FUNC (obj_social, const SOCIAL_TYPE *);
JSON_T *json_new_obj_social (const char *name, const SOCIAL_TYPE *soc) {
    JSON_T *new;
    if (soc == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_SOCIAL);

    json_prop_string (new, "name",           soc->name);
    json_prop_string (new, "char_no_arg",    soc->char_no_arg);
    json_prop_string (new, "others_no_arg",  soc->others_no_arg);
    json_prop_string (new, "char_found",     soc->char_found);
    json_prop_string (new, "others_found",   soc->others_found);
    json_prop_string (new, "vict_found",     soc->vict_found);
    json_prop_string (new, "char_not_found", soc->char_not_found);
    json_prop_string (new, "char_auto",      soc->char_auto);
    json_prop_string (new, "others_auto",    soc->others_auto);

    return new;
}

JSON_PROP_FUNC (obj_table, const TABLE_TYPE *);
JSON_T *json_new_obj_table (const char *name, const TABLE_TYPE *table) {
    JSON_T *new, *sub;
    char *type_str;

    if (table == NULL || table->json_write_func == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_TABLE);

    type_str = NULL;
    if (table->flags & TABLE_FLAG_TYPE)
        type_str = (table->flags & TABLE_BITS) ? "flags" : "types";
    else
        type_str = "table";

    json_prop_string (new, "name",        table->name);
    json_prop_string (new, "description", table->description);
    json_prop_string (new, "type",        type_str);

    sub = json_prop_array (new, "values");

    {
        const void *obj = table->table;
        const FLAG_TYPE *flag;
        JSON_T *json;
        flag_t expected = (table->flags & TABLE_BITS) ? 1 : 0;
        bool had_none = FALSE;

        do {
            if ((json = table->json_write_func (obj)) == NULL)
                break;
            if (table->flags & TABLE_FLAG_TYPE) {
                flag = obj;
                if (had_none) {
                    bugf ("json_new_obj_table: Warning: %s table '%s' row "
                        "'%s' should not be after -1 bit", type_str,
                        table->name, flag->name);
                }
                else if (flag->bit == -1 && !IS_SET(table->flags, TABLE_BITS))
                    had_none = TRUE;
                else if (flag->bit != expected) {
                    bugf ("json_new_obj_table: Warning: %s table '%s' row "
                        "'%s' should be %ld, but it's %ld", type_str,
                        table->name, flag->name, expected, flag->bit);
                }

                if (table->flags & TABLE_BITS)
                    expected <<= 1;
                else
                    ++expected;
            }

            json_attach_under (json, sub);
            obj += table->type_size;
        } while (1);
    }
    return new;
}

JSON_PROP_FUNC (obj_portal_exit, const PORTAL_EXIT_TYPE *);
JSON_T *json_new_obj_portal_exit (const char *name,
    const PORTAL_EXIT_TYPE *pex)
{
    JSON_T *new;
    if (pex == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL_EXIT);

    return new;
}

JSON_PROP_FUNC (obj_portal, const PORTAL_TYPE *);
JSON_T *json_new_obj_portal (const char *name, const PORTAL_TYPE *portal) {
    JSON_T *new;
    if (portal == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_PORTAL);

    json_prop_boolean (new, "two-way", portal->opposite != NULL);
    json_prop_string  (new, "from",    portal->name_from);
    json_prop_string  (new, "to",      portal->name_to);

    return new;
}

JSON_PROP_FUNC (obj_help_area, const HELP_AREA *);
JSON_T *json_new_obj_help_area (const char *name, const HELP_AREA *had) {
    JSON_T *new, *sub;
    HELP_DATA *help;
    if (had == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP_AREA);

    json_prop_string (new, "name",     JSTR (had->name));
    json_prop_string (new, "filename", JSTR (had->filename));

    sub = json_prop_array (new, "pages");
    for (help = had->first; help; help = help->next_area)
        json_prop_obj_help (sub, NULL, help);

    return new;
}

JSON_PROP_FUNC (obj_help, const HELP_DATA *);
JSON_T *json_new_obj_help (const char *name, const HELP_DATA *help) {
    JSON_T *new;
    if (help == NULL)
        return json_new_null (name);
    new = json_new_object (name, JSON_OBJ_HELP);

    json_prop_string (new, "keyword", JSTR (help->keyword));
    json_prop_string (new, "text",    JSTR (help->text));

    return new;
}
