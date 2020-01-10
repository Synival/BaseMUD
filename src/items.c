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

#include "utils.h"
#include "comm.h"
#include "interp.h"
#include "objs.h"
#include "lookup.h"
#include "chars.h"
#include "affects.h"
#include "magic.h"
#include "skills.h"
#include "db.h"
#include "recycle.h"
#include "rooms.h"
#include "mob_prog.h"
#include "groups.h"
#include "globals.h"
#include "music.h"

#include "act_info.h"
#include "act_move.h"
#include "act_group.h"

#include "items.h"

bool item_is_closed (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
            return IS_SET (obj->v.container.flags, CONT_CLOSED)
                ? TRUE : FALSE;

        case ITEM_PORTAL:
            return IS_SET (obj->v.portal.exit_flags, EX_CLOSED)
                ? TRUE : FALSE;

        default:
            return FALSE;
    }
}

bool item_is_edible (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_FOOD:
        case ITEM_PILL:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_drinkable (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_comparable (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_ARMOR:
        case ITEM_WEAPON:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_container (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_lit (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            return (obj->v.light.duration != 0) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_is_boat (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_BOAT:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_visible_when_blind (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_POTION:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_armor (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_ARMOR:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_weapon (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WEAPON:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_warp_stone (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WARP_STONE:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_is_playing (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_JUKEBOX:
            return (obj->v.jukebox.song >= 0) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_has_liquid (OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_DRINK_CON:
            return (obj->v.drink_con.capacity > 0 &&
                    obj->v.drink_con.filled <= 0)
                ? FALSE : TRUE;

        case ITEM_FOUNTAIN:
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_has_worth_as_room_reset (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_TREASURE:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_has_charges (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_STAFF:
            return (obj->v.staff.charges > 0) ? TRUE : FALSE;
        case ITEM_WAND:
            return (obj->v.wand.charges > 0) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_can_put_objs (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_sacrifice (OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CORPSE_PC:
            return (obj->contains) ? FALSE : TRUE;
        default:
            return TRUE;
    }
}

bool item_can_quaff (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_POTION:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_recite (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_SCROLL:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_brandish (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_STAFF:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_zap (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WAND:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_sell (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_TRASH:
            return FALSE;
        default:
            return TRUE;
    }
}

bool item_can_wield (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WEAPON:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_fill (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_DRINK_CON:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_play (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_JUKEBOX:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_can_position_at (const OBJ_T *obj, int pos) {
    switch (obj->item_type) {
        case ITEM_FURNITURE: {
            const FURNITURE_BITS_T *furn;
            flag_t bits;
            if ((furn = furniture_get (POS_STANDING)) == NULL)
                return FALSE;
            bits = furn->bit_at | furn->bit_on | furn->bit_in;
            return ((obj->v.furniture.flags & bits) != 0) ? TRUE : FALSE;
        }
        default:
            return FALSE;
    }
}

bool item_can_fit_obj_in (const OBJ_T *container, const OBJ_T *obj) {
    switch (container->item_type) {
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            return TRUE;

        case ITEM_CONTAINER: {
            int weight = obj_get_weight (obj);
            if (weight + obj_get_true_weight (container) >
                    (container->v.container.capacity * 10))
                return FALSE;
            if (weight > (container->v.container.max_weight * 10))
                return FALSE;
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_can_wear_flag (const OBJ_T *obj, flag_t wear_flag) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            return (wear_flag == ITEM_WEAR_LIGHT) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_can_enter_as (const OBJ_T *obj, const CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_PORTAL:
            if (IS_TRUSTED (ch, ANGEL))
                return TRUE;
            if (IS_SET (obj->v.portal.exit_flags, EX_CLOSED))
                return FALSE;
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_can_loot_as (const OBJ_T *obj, const CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
            return TRUE;
        case ITEM_CORPSE_PC:
            return char_can_loot (ch, obj) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_should_save_for_level (const OBJ_T *obj, int ch_level) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
            return TRUE;
        case ITEM_KEY:
            return FALSE;
        case ITEM_MAP:
            return (obj->v.map.persist) ? TRUE : FALSE;
        default:
            return (ch_level >= obj->level - 2) ? TRUE : FALSE;
    }
}

bool item_should_spill_contents_when_poofed (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CORPSE_PC:
            return TRUE;
        default:
            return FALSE;
    }
}

int item_get_compare_value (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_ARMOR:
            return obj->v.armor.vs_pierce +
                   obj->v.armor.vs_bash +
                   obj->v.armor.vs_slash;

        case ITEM_WEAPON:
            if (obj->index_data->new_format)
                return (obj->v.weapon.dice_size + 1) * obj->v.weapon.dice_num;
            else
                return (obj->v.weapon.dice_size + 0) * obj->v.weapon.dice_num;

        default:
            return 0;
    }
}

int item_get_door_flags (const OBJ_T *obj, flag_t *flags, int *key) {
    switch (obj->item_type) {
        case ITEM_PORTAL:
            if (flags) *flags = obj->v.portal.exit_flags;
            if (key)   *key   = obj->v.portal.key;
            return ITEM_DOOR_EXIT;

        case ITEM_CONTAINER:
            if (flags) *flags = obj->v.container.flags;
            if (key)   *key   = obj->v.container.key;
            return ITEM_DOOR_CONTAINER;

        default:
            return ITEM_DOOR_NONE;
    }
}

const char **item_get_put_messages (const OBJ_T *obj) {
    const static char *on_messages[2] =
        { "You put $p on $P.", "$n puts $p on $P." };
    const static char *in_messages[2] =
        { "You put $p in $P.", "$n puts $p in $P." };

    if (obj->item_type == ITEM_CONTAINER &&
            IS_SET (obj->v.container.flags, CONT_PUT_ON))
        return on_messages;
    else
        return in_messages;
}

bool item_get_drink_values (OBJ_T *obj, flag_t **capacity, flag_t **filled,
    flag_t **liquid, flag_t **poisoned, int *drink_amount)
{
    const LIQ_T *liq;

    switch (obj->item_type) {
        case ITEM_FOUNTAIN:
            *capacity = &(obj->v.fountain.capacity);
            *filled   = &(obj->v.fountain.filled);
            *liquid   = &(obj->v.fountain.liquid);
            *poisoned = &(obj->v.fountain.poisoned);
            break;

        case ITEM_DRINK_CON:
            *capacity = &(obj->v.drink_con.capacity);
            *filled   = &(obj->v.drink_con.filled);
            *liquid   = &(obj->v.drink_con.liquid);
            *poisoned = &(obj->v.drink_con.poisoned);
            break;

        default:
            return FALSE;
    }

    if ((liq = liq_get (**liquid)) == NULL) {
        bug ("item_get_drink_values: bad liquid number %d.", **liquid);
        **liquid = 0;
        liq = liq_get (0);
    }

    if (*capacity == NULL || **capacity <= 0)
        *drink_amount = liq->serving_size * 3;
    else
        *drink_amount = UMIN (liq->serving_size, **filled);

    return TRUE;
}

const LIQ_T *item_get_liquid (const OBJ_T *obj) {
    const LIQ_T *liq;

    switch (obj->item_type) {
        case ITEM_DRINK_CON:
            if ((liq = liq_get (obj->v.drink_con.liquid)) == NULL)
                liq = liq_get (0);
            return liq;

        case ITEM_FOUNTAIN:
            if ((liq = liq_get (obj->v.fountain.liquid)) == NULL)
                liq = liq_get (0);
            return liq;

        default:
            return NULL;
    }
}

int item_get_sacrifice_silver (const OBJ_T *obj) {
    int silver = UMAX (1, obj->level * 3);
    switch (obj->item_type) {
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            return silver;
        default:
            return UMIN (silver, obj->cost);
    }
}

int item_get_modified_cost (const OBJ_T *obj, int cost) {
    switch (obj->item_type) {
        case ITEM_STAFF:
            return (obj->v.staff.recharge == 0)
                ? (cost / 4)
                : (cost * obj->v.staff.charges / obj->v.staff.recharge);

        case ITEM_WAND:
            return (obj->v.wand.recharge == 0)
                ? (cost / 4)
                : (cost * obj->v.wand.charges / obj->v.wand.recharge);

        default:
            return cost;
    }
}

const char *item_get_corrode_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
            return "$p fumes and dissolves.";
        case ITEM_ARMOR:
            return "$p is pitted and etched.";
        case ITEM_CLOTHING:
            return "$p is corroded into scrap.";
        case ITEM_STAFF:
        case ITEM_WAND:
            return "$p corrodes and breaks.";
        case ITEM_SCROLL:
            return "$p is burned into waste.";
        default:
            return NULL;
    }
}

int item_get_corrode_chance_modifier (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_STAFF:
        case ITEM_WAND:
            return -10;
        case ITEM_SCROLL:
            return 10;
        default:
            return 0;
    }
}

const char *item_get_freeze_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_POTION:
        case ITEM_DRINK_CON:
            return "$p freezes and shatters!";
        default:
            return NULL;
    }
}

int item_get_freeze_chance_modifier (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_POTION:
            return 25;
        case ITEM_DRINK_CON:
            return 5;
        default:
            return 0;
    }
}

const char *item_get_burn_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
            return "$p ignites and burns!";
        case ITEM_POTION:
            return "$p bubbles and boils!";
        case ITEM_SCROLL:
            return "$p crackles and burns!";
        case ITEM_STAFF:
            return "$p smokes and chars!";
        case ITEM_WAND:
            return "$p sparks and sputters!";
        case ITEM_FOOD:
            return "$p blackens and crisps!";
        case ITEM_PILL:
            return "$p melts and drips!";
        default:
            return NULL;
    }
}

int item_get_burn_chance_modifier (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_POTION:
            return 25;
        case ITEM_SCROLL:
            return 50;
        case ITEM_STAFF:
            return 10;
        default:
            return 0;
    }
}

const char *item_get_poison_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_FOOD:
            return "";
        case ITEM_DRINK_CON:
            if (obj->v.drink_con.capacity == obj->v.drink_con.filled)
                return NULL;
            return "";
        default:
            return NULL;
    }
}

int item_get_poison_chance_modifier (const OBJ_T *obj) {
    return 0;
}

const char *item_get_shock_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WAND:
        case ITEM_STAFF:
            return "$p overloads and explodes!";
        case ITEM_JEWELRY:
            return "$p is fused into a worthless lump.";
        default:
            return NULL;
    }
}

int item_get_shock_chance_modifier (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_WAND:
        case ITEM_STAFF:
            return 10;
        case ITEM_JEWELRY:
            return -10;
        default:
            return 0;
    }
}

bool item_get_corpse_timer_range (const OBJ_T *obj, int *timer_min,
    int *timer_max)
{
    switch (obj->item_type) {
        case ITEM_POTION:
            *timer_min = 500;
            *timer_max = 1000;
            return TRUE;

        case ITEM_SCROLL:
            *timer_min = 1000;
            *timer_max = 2500;
            return TRUE;

        default:
            return FALSE;
    }
}

int item_get_ac (const OBJ_T *obj, int ac_type) {
    switch (obj->item_type) {
        case ITEM_ARMOR:
            switch (ac_type) {
                case AC_PIERCE: return obj->v.armor.vs_pierce; break;
                case AC_BASH:   return obj->v.armor.vs_bash;   break;
                case AC_SLASH:  return obj->v.armor.vs_slash;  break;
                case AC_EXOTIC: return obj->v.armor.vs_magic;  break;
                default:        return 0;
            }

        default:
            return 0;
    }
}

int item_get_carry_number (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_MONEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
            return 0;
        default:
            return 1;
    }
}

int item_get_furn_preposition_type (const OBJ_T *obj, int position) {
    const FURNITURE_BITS_T *bits;
    if (obj == NULL)
        return POS_PREP_NO_OBJECT;
    if (obj->item_type != ITEM_FURNITURE)
        return POS_PREP_NOT_FURNITURE;
    if ((bits = furniture_get (position)) == NULL)
        return POS_PREP_BAD_POSITION;

         if (obj->v.furniture.flags & bits->bit_at) return POS_PREP_AT;
    else if (obj->v.furniture.flags & bits->bit_on) return POS_PREP_ON;
    else if (obj->v.furniture.flags & bits->bit_in) return POS_PREP_IN;
    else                                            return POS_PREP_BY;
}

int item_get_weight_mult (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
            return obj->v.container.weight_mult;
        default:
            return 100;
    }
}

const char *item_get_poof_message (const OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_FOUNTAIN:
            return "$p dries up.";

        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            return "$p decays into dust.";

        case ITEM_FOOD:
            return "$p decomposes.";

        case ITEM_POTION:
            return "$p has evaporated from disuse.";

        case ITEM_PORTAL:
            return "$p fades out of existence.";

        case ITEM_CONTAINER:
            if (obj_can_wear_flag (obj, ITEM_WEAR_FLOAT)) {
                if (obj->contains)
                    return "$p flickers and vanishes, spilling its contents on the floor.";
                else
                    return "$p flickers and vanishes.";
            }
            return "$p crumbles into dust.";

        default:
            return "$p crumbles into dust.";
    }
}

flag_t *item_get_poison_flag (OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_FOUNTAIN:
            return &(obj->v.fountain.poisoned);
        case ITEM_DRINK_CON:
            return &(obj->v.drink_con.poisoned);
        case ITEM_FOOD:
            return &(obj->v.food.poisoned);
        default:
            return NULL;
    }
}

bool item_get_recharge_values (OBJ_T *obj, flag_t *level,
    flag_t **recharge_ptr, flag_t **charges_ptr)
{
    switch (obj->item_type) {
        case ITEM_WAND:
            *level        = obj->v.wand.level;
            *recharge_ptr = &(obj->v.wand.recharge);
            *charges_ptr  = &(obj->v.wand.charges);
            return TRUE;

        case ITEM_STAFF:
            *level        = obj->v.staff.level;
            *recharge_ptr = &(obj->v.staff.recharge);
            *charges_ptr  = &(obj->v.staff.charges);
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_init (OBJ_T *obj, const OBJ_INDEX_T *obj_index, int level) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            if (obj->v.light.duration == 999)
                obj->v.light.duration = -1;
            return TRUE;

        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_PORTAL:
            if (!obj_index->new_format)
                obj->cost /= 5;
            return TRUE;

        case ITEM_TREASURE:
        case ITEM_WARP_STONE:
        case ITEM_ROOM_KEY:
        case ITEM_GEM:
        case ITEM_JEWELRY:
            return TRUE;

        case ITEM_JUKEBOX: {
            int i;
            for (i = 0; i < OBJ_VALUE_MAX; i++)
                obj->v.value[i] = -1;
            return TRUE;
        }

        case ITEM_SCROLL:
            if (level != -1 && !obj_index->new_format)
                obj->v.scroll.level = number_fuzzy (obj->v.scroll.level);
            return TRUE;

        case ITEM_WAND:
            if (level != -1 && !obj_index->new_format) {
                obj->v.wand.level    = number_fuzzy (obj->v.wand.level);
                obj->v.wand.recharge = number_fuzzy (obj->v.wand.recharge);
                obj->v.wand.charges  = obj->v.wand.recharge;
            }
            if (!obj_index->new_format)
                obj->cost *= 2;
            return TRUE;

        case ITEM_STAFF:
            if (level != -1 && !obj_index->new_format) {
                obj->v.staff.level    = number_fuzzy (obj->v.staff.level);
                obj->v.staff.recharge = number_fuzzy (obj->v.staff.recharge);
                obj->v.staff.charges  = obj->v.staff.recharge;
            }
            if (!obj_index->new_format)
                obj->cost *= 2;
            return TRUE;

        case ITEM_WEAPON:
            if (level != -1 && !obj_index->new_format) {
                obj->v.weapon.dice_num  = number_fuzzy (number_fuzzy (
                    1 * level / 4 + 2));
                obj->v.weapon.dice_size = number_fuzzy (number_fuzzy (
                    3 * level / 4 + 6));
            }
            return TRUE;

        case ITEM_ARMOR:
            if (level != -1 && !obj_index->new_format) {
                obj->v.armor.vs_pierce = number_fuzzy (level / 5 + 3);
                obj->v.armor.vs_bash   = number_fuzzy (level / 5 + 3);
                obj->v.armor.vs_slash  = number_fuzzy (level / 5 + 3);
            }
            return TRUE;

        case ITEM_POTION:
            if (level != -1 && !obj_index->new_format)
                obj->v.potion.level = number_fuzzy (number_fuzzy (
                    obj->v.potion.level));
            return TRUE;

        case ITEM_PILL:
            if (level != -1 && !obj_index->new_format)
                obj->v.pill.level = number_fuzzy (number_fuzzy (
                    obj->v.pill.level));
            return TRUE;

        case ITEM_MONEY:
            if (!obj_index->new_format)
                obj->v.money.silver = obj->cost;
            return TRUE;

        default:
            bugf ("item_init: vnum %d bad type %d.", obj_index->vnum,
                obj->item_type);
            return FALSE;
    }
}

bool item_look_in (const OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_DRINK_CON:
            if (obj->v.drink_con.filled <= 0)
                send_to_char ("It is empty.\n\r", ch);
            else if (obj->v.drink_con.filled >= obj->v.drink_con.capacity) {
                printf_to_char (ch,
                    "It's completely filled with a %s liquid.\n\r",
                    liq_table[obj->v.drink_con.liquid].color);
            }
            else {
                int percent;
                char *fullness;

                percent = (obj->v.drink_con.filled * 100)
                    / obj->v.drink_con.capacity;

                     if (percent >= 66) fullness = "more than half-filled";
                else if (percent >= 33) fullness = "about half-filled";
                else                    fullness = "less than half-filled";

                printf_to_char (ch, "It's %s with a %s liquid.\n\r",
                    fullness, liq_table[obj->v.drink_con.liquid].color);
            }
            return TRUE;

        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            if (IS_SET (obj->v.container.flags, CONT_CLOSED))
                send_to_char ("It is closed.\n\r", ch);
            else {
                act ("$p holds:", ch, obj, NULL, TO_CHAR);
                obj_list_show_to_char (obj->contains, ch, TRUE, TRUE);
            }
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_examine (const OBJ_T *obj, CHAR_T *ch) {
    char buf[256];

    switch (obj->item_type) {
        case ITEM_JUKEBOX:
            music_list_jukebox_songs (obj, ch, "");
            return TRUE;

        case ITEM_MONEY:
            if (obj->v.money.silver == 0) {
                if (obj->v.money.gold == 0)
                    snprintf (buf, sizeof (buf), "Odd...there's no coins in "
                        "the pile.\n\r");
                else if (obj->v.money.gold == 1)
                    snprintf (buf, sizeof (buf), "Wow. One gold coin.\n\r");
                else
                    snprintf (buf, sizeof (buf), "There are %ld gold coins "
                        "in the pile.\n\r", obj->v.money.gold);
            }
            else if (obj->v.money.gold == 0) {
                if (obj->v.money.silver == 1)
                    snprintf (buf, sizeof (buf), "Wow. One silver coin.\n\r");
                else
                    snprintf (buf, sizeof (buf), "There are %ld silver coins "
                        "in the pile.\n\r", obj->v.money.silver);
            }
            else {
                snprintf (buf, sizeof (buf), "There are %ld gold and %ld "
                    "silver coins in the pile.\n\r", obj->v.money.gold,
                    obj->v.money.silver);
            }
            send_to_char (buf, ch);
            return TRUE;

        case ITEM_DRINK_CON:
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            item_look_in (obj, ch);
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_eat_effect (OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_FOOD:
            char_change_conditions (ch, 0, obj->v.food.hunger, 0,
                obj->v.food.fullness);
            if (obj->v.food.poisoned != 0) {
                AFFECT_T af;

                /* The food was poisoned! */
                send_to_char ("You choke and gag.\n\r", ch);
                act ("$n chokes and gags.", ch, NULL, NULL, TO_NOTCHAR);

                affect_init (&af, AFF_TO_AFFECTS, SN(POISON), number_fuzzy (
                    obj->v.food.hunger), 2 * obj->v.food.hunger, APPLY_NONE, 0,
                    AFF_POISON);
                affect_join (ch, &af);
            }
            obj_extract (obj);
            return TRUE;

        case ITEM_PILL: {
            int i, level = obj->v.pill.level;
            for (i = 0; i < PILL_SKILL_MAX; i++)
                obj_cast_spell (obj->v.pill.skill[i], level, ch, ch, NULL);
            obj_extract (obj);
            return TRUE;
        }

        default:
            obj_extract (obj);
            return TRUE;
    }
}

bool item_drink_effect (OBJ_T *obj, CHAR_T *ch) {
    const LIQ_T *liq;
    const sh_int *cond;
    flag_t *capacity, *filled, *liquid, *poisoned;
    int amount;

    if (!item_get_drink_values (obj, &capacity, &filled,
            &liquid, &poisoned, &amount))
        return FALSE;

    /* TODO: these numbers - along with serving size - are so arbitrary... */
    liq = liq_get (*liquid);
    cond = liq->cond;
    char_change_conditions (ch,
        amount * cond[COND_DRUNK]  / 36, amount * cond[COND_FULL]   / 4,
        amount * cond[COND_THIRST] / 10, amount * cond[COND_HUNGER] / 2);

    if (*poisoned != 0) {
        AFFECT_T af;

        /* The drink was poisoned! */
        send_to_char ("You choke and gag.\n\r", ch);
        act ("$n chokes and gags.", ch, NULL, NULL, TO_NOTCHAR);

        affect_init (&af, AFF_TO_AFFECTS, SN(POISON), number_fuzzy (amount),
            3 * amount, APPLY_NONE, 0, AFF_POISON);
        affect_join (ch, &af);
    }

    if (capacity != NULL && filled != NULL && *capacity > 0)
        *filled = UMAX (*filled - amount, 0);
    return TRUE;
}

bool item_quaff_effect (OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_POTION: {
            int i, level = obj->v.potion.level;
            for (i = 0; i < POTION_SKILL_MAX; i++)
                obj_cast_spell (obj->v.potion.skill[i], level, ch, ch, NULL);
            obj_extract (obj);
            return TRUE;
        }

        default:
            obj_extract (obj);
            return TRUE;
    }
}

bool item_recite_effect (OBJ_T *obj, CHAR_T *ch, CHAR_T *victim,
    OBJ_T *obj_target)
{
    switch (obj->item_type) {
        case ITEM_SCROLL: {
            int i, level = obj->v.scroll.level;
            for (i = 0; i < SCROLL_SKILL_MAX; i++)
                obj_cast_spell (obj->v.scroll.skill[i], level, ch,
                    victim, obj_target);
            obj_extract (obj);
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_brandish_effect (OBJ_T *obj, CHAR_T *ch, bool try_improve) {
    switch (obj->item_type) {
        case ITEM_STAFF: {
            CHAR_T *vch, *vch_next;
            int sn = obj->v.staff.skill;

            RETURN_IF_BUG (sn < 0 || sn >= SKILL_MAX ||
                    skill_table[sn].spell_fun == 0,
                "do_brandish: bad sn %d.", sn, TRUE);

            for (vch = ch->in_room->people; vch; vch = vch_next) {
                vch_next = vch->next_in_room;
                switch (skill_table[sn].target) {
                    case SKILL_TARGET_IGNORE:
                        if (vch != ch)
                            continue;
                        break;

                    case SKILL_TARGET_CHAR_OFFENSIVE:
                        if (IS_NPC (ch) ? IS_NPC (vch) : !IS_NPC (vch))
                            continue;
                        break;

                    case SKILL_TARGET_CHAR_DEFENSIVE:
                        if (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch))
                            continue;
                        break;

                    case SKILL_TARGET_CHAR_SELF:
                        if (vch != ch)
                            continue;
                        break;

                    default:
                        bug ("item_staff_effect: bad target for sn %d.", sn);
                        return TRUE;
                }
                obj_cast_spell (obj->v.staff.skill, obj->v.staff.level,
                    ch, vch, NULL);
                if (try_improve)
                    char_try_skill_improve (ch, SN(STAVES), TRUE, 2);
            }
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_zap_effect (OBJ_T *obj, CHAR_T *ch, CHAR_T *victim,
    OBJ_T *obj_target)
{
    switch (obj->item_type) {
        case ITEM_WAND:
            obj_cast_spell (obj->v.wand.skill, obj->v.wand.level,
                ch, victim, obj_target);
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_corrode_effect (OBJ_T *obj, int level) {
    switch (obj->item_type) {
        /* etch it */
        case ITEM_ARMOR: {
            AFFECT_T *paf;
            bool af_found = FALSE;
            int i;

            obj_enchant (obj);
            for (paf = obj->affected; paf != NULL; paf = paf->next) {
                if (paf->apply == APPLY_AC) {
                    af_found = TRUE;
                    paf->type = -1;
                    paf->modifier += 1;
                    paf->level = UMAX (paf->level, level);
                    break;
                }
            }

            /* needs a new affect */
            if (!af_found) {
                paf = affect_new ();
                affect_init (paf, AFF_TO_AFFECTS, -1, level, -1, APPLY_AC, 1, 0);
                LIST_FRONT (paf, next, obj->affected);
            }
            if (obj->carried_by != NULL && obj->wear_loc != WEAR_LOC_NONE)
                for (i = 0; i < 4; i++)
                    obj->carried_by->armor[i] += 1;
            SET_BIT (obj->extra_flags, ITEM_CORRODED);
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_freeze_effect (OBJ_T *obj, int level) {
    /* NOTE: If you want items to do something when they freeze,
     *       add the behavior here! (see item_corrode_effect) */
    return FALSE;
}

bool item_burn_effect (OBJ_T *obj, int level) {
    /* NOTE: If you want items to do something when they burn,
     *       add the behavior here! (see item_corrode_effect) */
    return FALSE;
}

bool item_poison_effect (OBJ_T *obj, int level) {
    switch (obj->item_type) {
        case ITEM_FOOD:
            obj->v.food.poisoned = TRUE;
            return TRUE;

        case ITEM_DRINK_CON:
            obj->v.drink_con.poisoned = TRUE;
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_shock_effect (OBJ_T *obj, int level) {
    /* NOTE: If you want items to do something when they're shocked,
     *       add the behavior here! (see item_corrode_effect) */
    return FALSE;
}

bool item_set_exit_flag (OBJ_T *obj, flag_t exit_flag) {
    switch (obj->item_type) {
        case ITEM_PORTAL:
            SET_BIT (obj->v.portal.exit_flags, exit_flag);
            return TRUE;

        case ITEM_CONTAINER: {
            flag_t container_flag;
            if ((container_flag = item_exit_flag_to_cont (exit_flag)) == 0)
                return FALSE;
            SET_BIT (obj->v.container.flags, container_flag);
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_remove_exit_flag (OBJ_T *obj, flag_t exit_flag) {
    switch (obj->item_type) {
        case ITEM_PORTAL:
            REMOVE_BIT (obj->v.portal.exit_flags, exit_flag);
            return TRUE;

        case ITEM_CONTAINER: {
            flag_t container_flag;
            if ((container_flag = item_exit_flag_to_cont (exit_flag)) == 0)
                return FALSE;
            REMOVE_BIT (obj->v.container.flags, container_flag);
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_write_save_data (const OBJ_T *obj, FILE *fp) {
    switch (obj->item_type) {
        case ITEM_POTION: {
            int i;
            for (i = 0; i < POTION_SKILL_MAX; i++) {
                if (obj->v.potion.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.potion.skill[i]].name);
            }
            return TRUE;
        }

        case ITEM_SCROLL: {
            int i;
            for (i = 0; i < SCROLL_SKILL_MAX; i++) {
                if (obj->v.scroll.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.scroll.skill[i]].name);
            }
            return TRUE;
        }

        case ITEM_PILL: {
            int i;
            for (i = 0; i < PILL_SKILL_MAX; i++) {
                if (obj->v.pill.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.pill.skill[i]].name);
            }
            return TRUE;
        }

        case ITEM_STAFF:
            if (obj->v.staff.skill > 0)
                fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->v.staff.skill].name);
            return TRUE;

        case ITEM_WAND:
            if (obj->v.wand.skill > 0)
                fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->v.wand.skill].name);
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_envenom_effect (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell)
{
    switch (obj->item_type) {
        case ITEM_FOOD:
        case ITEM_DRINK_CON:
            item_envenom_effect_food (obj, ch, skill, sn, as_spell);
            return TRUE;

        case ITEM_WEAPON:
            item_envenom_effect_weapon (obj, ch, skill, sn, as_spell);
            return TRUE;

        default:
            return FALSE;
    }
}

void item_envenom_effect_food (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell)
{
    flag_t *pflag;

    BAIL_IF_ACT (IS_OBJ_STAT (obj, ITEM_BLESS) ||
                 IS_OBJ_STAT (obj, ITEM_BURN_PROOF),
        (as_spell ? "Your spell fails to corrupt $p."
                  : "You fail to poison $p."), ch, obj, NULL);

    switch (obj->item_type) {
        case ITEM_FOOD:
            pflag = &(obj->v.food.poisoned);
            break;
        case ITEM_DRINK_CON:
            pflag = &(obj->v.drink_con.poisoned);
            break;
        default:
            pflag = NULL;
    }

    if (*pflag != 0) {
        act ("$p is already poisoned.", ch, obj, NULL, TO_CHAR);
        *pflag = TRUE;
        return;
    }

    if (as_spell) {
        act ("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
        *pflag = TRUE;
    }
    else {
        if (number_percent () >= skill) {
            act ("You fail to poison $p.", ch, obj, NULL, TO_CHAR);
            if (*pflag == 0)
                char_try_skill_improve (ch, sn, FALSE, 4);
            return;
        }
        act2 ("You treat $p with deadly poison.",
              "$n treats $p with deadly poison.", ch, obj, NULL, 0, POS_RESTING);
        char_try_skill_improve (ch, sn, TRUE, 4);
    }
}

void item_envenom_effect_weapon (OBJ_T *obj, CHAR_T *ch, int skill, int sn,
    bool as_spell)
{
    AFFECT_T af;
    int percent;

    if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)  ||
        IS_WEAPON_STAT (obj, WEAPON_FROST)    ||
        IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC) ||
        IS_WEAPON_STAT (obj, WEAPON_SHARP)    ||
        IS_WEAPON_STAT (obj, WEAPON_VORPAL)   ||
        IS_WEAPON_STAT (obj, WEAPON_SHOCKING) ||
        IS_OBJ_STAT (obj, ITEM_BLESS)         ||
        IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
    {
        act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
        return;
    }

    if (!as_spell) {
        BAIL_IF (obj->v.weapon.attack_type < 0 ||
                 attack_table[obj->v.weapon.attack_type].dam_type == DAM_BASH,
            "You can only envenom edged weapons.\n\r", ch);
    }
    BAIL_IF_ACT (IS_WEAPON_STAT (obj, WEAPON_POISON),
        "$p is already envenomed.", ch, obj, NULL);

    if (as_spell) {
        affect_init (&af, AFF_TO_WEAPON, sn, skill / 2, skill / 8, 0, 0, WEAPON_POISON);
        affect_to_obj (obj, &af);
        act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
    }
    else {
        percent = number_percent ();
        if (skill < 100 && percent >= skill) {
            act ("You fail to envenom $p.", ch, obj, NULL, TO_CHAR);
            char_try_skill_improve (ch, sn, FALSE, 3);
            return;
        }

        affect_init (&af, AFF_TO_WEAPON, sn, ch->level * percent / 100, ch->level / 2 * percent / 100, 0, 0, WEAPON_POISON);
        affect_to_obj (obj, &af);

        act2 ("You coat $p with venom.",
              "$n coats $p with deadly venom.", ch, obj, NULL, 0, POS_RESTING);
        char_try_skill_improve (ch, sn, TRUE, 3);
    }

}

bool item_light_fade (OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            if (obj->v.light.duration <= 0)
                return FALSE;
            if (obj->carried_by) {
                CHAR_T *ch = obj->carried_by;
                if (--obj->v.light.duration == 0 && ch->in_room != NULL) {
                    --ch->in_room->light;
                    act2 ("$p flickers and goes out.", "$p goes out.",
                        ch, obj, NULL, 0, POS_RESTING);
                    obj_extract (obj);
                }
                else if (obj->v.light.duration <= 5 && ch->in_room != NULL)
                    act ("$p flickers.", ch, obj, NULL, TO_CHAR);
            }
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_enter_effect (OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_PORTAL: {
            const ROOM_INDEX_T *old_room;
            ROOM_INDEX_T *location;
            CHAR_T *fch, *fch_next;
            const char *msg;

            RETURN_IF (ch->position != POS_STANDING,
                "You'd better stand up first.\n\r", ch, TRUE);

            RETURN_IF (!IS_TRUSTED (ch, ANGEL) &&
                     !IS_SET (obj->v.portal.gate_flags, GATE_NOCURSE) &&
                     (IS_AFFECTED (ch, AFF_CURSE) ||
                      IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL)),
                "Something prevents you from leaving...\n\r", ch, TRUE);

            /* Determine the target room. */
            if (IS_SET (obj->v.portal.gate_flags, GATE_RANDOM) ||
                    obj->v.portal.to_vnum == -1)
            {
                location = get_random_room (ch);
                obj->v.portal.to_vnum = location->vnum; /* for record keeping :) */
            }
            else if (IS_SET (obj->v.portal.gate_flags, GATE_BUGGY) &&
                    (number_percent () < 5))
                location = get_random_room (ch);
            else
                location = get_room_index (obj->v.portal.to_vnum);

            /* Check if the target room if valid. */
            RETURN_IF_ACT (location == NULL || location == ch->in_room ||
                    !char_can_see_room (ch, location) ||
                    (room_is_private (location) && !IS_TRUSTED (ch, IMPLEMENTOR)),
                "$p doesn't seem to go anywhere.", ch, obj, NULL, TRUE);
            RETURN_IF (IS_NPC (ch) && IS_SET (ch->mob, MOB_AGGRESSIVE) &&
                     IS_SET (location->room_flags, ROOM_LAW),
                "Something prevents you from leaving...\n\r", ch, TRUE);

            /* We're leaving! Outgoing message. */
            msg = IS_SET (obj->v.portal.gate_flags, GATE_NORMAL_EXIT)
                ? "You enter $p."
                : "You walk through $p and find yourself somewhere else...";
            act2 (msg, "$n steps into $p.", ch, obj, NULL, 0, POS_RESTING);

            /* Leave, take the portal along if specified. */
            old_room = ch->in_room;
            char_from_room (ch);
            char_to_room (ch, location);
            if (IS_SET (obj->v.portal.gate_flags, GATE_GOWITH)) {
                obj_take_from_room (obj);
                obj_give_to_room (obj, location);
            }

            /* Arrival messages. */
            msg = IS_SET (obj->v.portal.gate_flags, GATE_NORMAL_EXIT)
                ? "$n has arrived."
                : "$n has arrived through $p.";
            act (msg, ch, obj, NULL, TO_NOTCHAR);
            do_function (ch, &do_look, "auto");

            /* Charges. Zero charges = infinite uses. */
            if (obj->v.portal.charges > 0) {
                obj->v.portal.charges--;
                if (obj->v.portal.charges == 0)
                    obj->v.portal.charges = -1;
            }

            /* Perform follows. */
            if (old_room != location) {
                for (fch = old_room->people; fch != NULL; fch = fch_next) {
                    fch_next = fch->next_in_room;

                    /* no following through dead portals */
                    if (obj == NULL || obj->v.portal.charges == -1)
                        continue;

                    if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM) &&
                        fch->position < POS_STANDING)
                    {
                        do_function (fch, &do_stand, "");
                    }

                    if (fch->master == ch && fch->position == POS_STANDING) {
                        if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                            && (IS_NPC (fch) && IS_SET (fch->mob, MOB_AGGRESSIVE)))
                        {
                            act ("You can't bring $N into the city.",
                                 ch, NULL, fch, TO_CHAR);
                            act ("You aren't allowed in the city.",
                                 fch, NULL, NULL, TO_CHAR);
                            continue;
                        }

                        act ("You follow $N.", fch, NULL, ch, TO_CHAR);

                        /* TODO: make certain the person can follow. */
                        item_enter_effect (obj, fch);
                    }
                }
            }

            /* If the portal is defunct, destroy it now. */
            if (obj != NULL && obj->v.portal.charges == -1) {
                act ("$p fades out of existence.", ch, obj, NULL, TO_CHAR);
                if (ch->in_room == old_room)
                    act ("$p fades out of existence.", ch, obj, NULL, TO_NOTCHAR);
                else if (old_room->people != NULL)
                    act ("$p fades out of existence.", old_room->people, obj, NULL, TO_ALL);
                obj_extract (obj);
            }

            /* If someone is following the char, these triggers get activated
             * for the followers before the char, but it's safer this way... */
            if (IS_NPC (ch) && HAS_TRIGGER (ch, TRIG_ENTRY))
                mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_ENTRY);
            if (!IS_NPC (ch))
                mp_greet_trigger (ch);
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_take_effect (OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_MONEY:
            obj_take (obj);
            ch->silver += obj->v.money.silver;
            ch->gold   += obj->v.money.gold;

            if (IS_SET (ch->plr, PLR_AUTOSPLIT)) {
                CHAR_T *gch;
                int members;
                char buf[100];

                /* Count the number of members to split. */
                members = 0;
                for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
                    if (!IS_AFFECTED (gch, AFF_CHARM) && is_same_group (gch, ch))
                        members++;

                /* Only split money if there's enough money to split. */
                if (members > 1 && (obj->v.money.silver > 1 ||
                                    obj->v.money.gold   > 0)) {
                    sprintf (buf, "%ld %ld", obj->v.money.silver, obj->v.money.gold);
                    do_function (ch, &do_split, buf);
                }
            }

            obj_extract (obj);
            return TRUE;

        default:
            obj_take (obj);
            obj_give_to_char (obj, ch);
            return TRUE;
    }
}

bool item_consume_charge_as (OBJ_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_STAFF:
            if (--obj->v.staff.charges <= 0) {
                act2 ("Your $p blazes bright and is gone.",
                      "$n's $p blazes bright and is gone.",
                      ch, obj, NULL, 0, POS_RESTING);
                obj_extract (obj);
            }
            return TRUE;

        case ITEM_WAND:
            if (--obj->v.wand.charges <= 0) {
                act2 ("Your $p explodes into fragments.",
                      "$n's $p explodes into fragments.",
                      ch, obj, NULL, 0, POS_RESTING);
                obj_extract (obj);
            }
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_play_effect (OBJ_T *obj, CHAR_T *ch, char *argument) {
    switch (obj->item_type) {
        case ITEM_JUKEBOX: {
            char *str, arg[MAX_INPUT_LENGTH];
            int song;
            bool global = FALSE;

            str = one_argument (argument, arg);
            if (!str_cmp (arg, "list")) {
                music_list_jukebox_songs (obj, ch, argument);
                return TRUE;
            }

            if (!str_cmp (arg, "loud")) {
                argument = str;
                global = TRUE;
            }

            RETURN_IF (argument[0] == '\0',
                "Play what?\n\r", ch, TRUE);
            RETURN_IF ((global && music_queue[MAX_SONG_GLOBAL - 1] > -1) ||
                      (!global && obj->v.jukebox.queue[JUKEBOX_QUEUE_MAX -1 ] > -1),
                "The jukebox is full up right now.\n\r", ch, TRUE);

            song = song_lookup (argument);
            RETURN_IF (song < 0 || song >= MAX_SONGS || song_table[song].name == NULL,
                "That song isn't available.\n\r", ch, TRUE);

            send_to_char ("Coming right up.\n\r", ch);
            if (global) {
                music_queue_values (
                    &music_line, &music_song, music_queue,
                    MAX_SONG_GLOBAL, song
                );
            }
            else {
                music_queue_values (
                    &(obj->v.jukebox.line), &(obj->v.jukebox.song),
                      obj->v.jukebox.queue, MAX_SONG_GLOBAL, song
                );
            }
            return TRUE;
        }

        default:
            return FALSE;
    }
}

bool item_play_continue (OBJ_T *obj) {
    switch (obj->item_type) {
        case ITEM_JUKEBOX:
            music_update_object (obj);
            return TRUE;

        default:
            return FALSE;
    }
}

bool item_index_is_container (const OBJ_INDEX_T *obj) {
    switch (obj->item_type) {
        case ITEM_CONTAINER:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
            return TRUE;
        default:
            return FALSE;
    }
}

bool item_index_can_wear_flag (const OBJ_INDEX_T *obj, flag_t wear_flag) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            return (wear_flag == ITEM_WEAR_LIGHT) ? TRUE : FALSE;
        default:
            return FALSE;
    }
}

bool item_index_fix_old (OBJ_INDEX_T *obj_index) {
    switch (obj_index->item_type) {
        case ITEM_WEAPON:
            if (str_in_namelist ("two",        obj_index->name) ||
                str_in_namelist ("two-handed", obj_index->name) ||
                str_in_namelist ("claymore",   obj_index->name))
            {
                SET_BIT (obj_index->v.weapon.flags, WEAPON_TWO_HANDS);
            }
            return TRUE;

        /* fix armors */
        case ITEM_ARMOR:
            obj_index->v.armor.vs_pierce = obj_index->v.value[0];
            obj_index->v.armor.vs_bash   = obj_index->v.value[0];
            obj_index->v.armor.vs_slash  = obj_index->v.value[0];
            return TRUE;

        /* Translate spell "slot numbers" to internal "skill numbers." */
        case ITEM_PILL: {
            int i;
            for (i = 0; i < PILL_SKILL_MAX; i++)
                obj_index->v.pill.skill[i] = skill_get_index_by_slot (
                    obj_index->v.value[i + 1]);
            return TRUE;
        }

        case ITEM_POTION: {
            int i;
            for (i = 0; i < POTION_SKILL_MAX; i++)
                obj_index->v.potion.skill[i] = skill_get_index_by_slot (
                    obj_index->v.value[i + 1]);
            return TRUE;
        }

        case ITEM_SCROLL: {
            int i;
            for (i = 0; i < SCROLL_SKILL_MAX; i++)
                obj_index->v.scroll.skill[i] = skill_get_index_by_slot (
                    obj_index->v.scroll.skill[i + 1]);
            return TRUE;
        }

        case ITEM_STAFF:
            obj_index->v.staff.skill = skill_get_index_by_slot (
                obj_index->v.value[3]);
            return TRUE;

        case ITEM_WAND:
            obj_index->v.wand.skill = skill_get_index_by_slot (
                obj_index->v.value[3]);
            return TRUE;

        default:
            return FALSE;
    }
}

/* (from OLC) */
/*****************************************************************************
 (Original comments)
 Name:       convert_object
 Purpose:    Converts an old_format obj to new_format
 Called by:  convert_objects (db_old.c).
 Note:       Dug out of create_obj (db.c)
 Author:     Hugin
 ****************************************************************************/
bool item_index_convert_old (OBJ_INDEX_T *obj_index) {
    int level;
    int number, type; /* for dice-conversion */

    if (!obj_index || obj_index->new_format)
        return FALSE;

    level = obj_index->level;

    obj_index->level = UMAX (0, obj_index->level); /* just to be sure */
    obj_index->cost = 10 * level;

    obj_index->new_format = TRUE;
    ++newobj_count;

    switch (obj_index->item_type) {
        case ITEM_WAND:
            obj_index->v.wand.charges = obj_index->v.value[1];
            return TRUE;

        case ITEM_STAFF:
            obj_index->v.staff.charges = obj_index->v.value[1];
            return TRUE;

        case ITEM_WEAPON:
            /*
             * The conversion below is based on the values generated
             * in one_hit() (fight.c).  Since I don't want a lvl 50
             * weapon to do 15d3 damage, the min value will be below
             * the one in one_hit, and to make up for it, I've made
             * the max value higher.
             * (I don't want 15d2 because this will hardly ever roll
             * 15 or 30, it will only roll damage close to 23.
             * I can't do 4d8+11, because one_hit there is no dice-
             * bounus value to set...)
             *
             * The conversion below gives:

             level:   dice      min      max      mean
              1:     1d8      1( 2)    8( 7)     5( 5)
              2:     2d5      2( 3)   10( 8)     6( 6)
              3:     2d5      2( 3)   10( 8)     6( 6)
              5:     2d6      2( 3)   12(10)     7( 7)
             10:     4d5      4( 5)   20(14)    12(10)
             20:     5d5      5( 7)   25(21)    15(14)
             30:     5d7      5(10)   35(29)    20(20)
             50:     5d11     5(15)   55(44)    30(30)

             */

            number = UMIN (level / 4 + 1, 5);
            type = (level + 7) / number;

            obj_index->v.weapon.dice_num  = number;
            obj_index->v.weapon.dice_size = type;
            return TRUE;

        case ITEM_ARMOR:
            obj_index->v.armor.vs_pierce = level / 5 + 3;
            obj_index->v.armor.vs_bash   = obj_index->v.value[0];
            obj_index->v.armor.vs_slash  = obj_index->v.value[0];
            return TRUE;

        case ITEM_MONEY:
            obj_index->v.money.silver = obj_index->cost;
            return TRUE;

        case ITEM_POTION:
        case ITEM_PILL:
        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
            return TRUE;

        default:
            bug ("item_index_convert_old: vnum %d bad type.", obj_index->item_type);
            return TRUE;
    }
}

bool item_index_finalize (OBJ_INDEX_T *obj) {
    switch (obj->item_type) {
#ifdef BASEMUD_FIX_FURNITURE
        case ITEM_FURNITURE: {
            int min_occupants = 0;
            int min_hit       = 100;
            int min_mana      = 100;

            if (str_in_namelist ("tent", obj->name) ||
                str_in_namelist ("cabin", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, REST_IN);
                SET_BIT (obj->v.furniture.flags, SIT_IN);
                SET_BIT (obj->v.furniture.flags, SLEEP_IN);
                SET_BIT (obj->v.furniture.flags, STAND_IN);
                min_occupants = 1;
                min_hit = 250;
            }
            if (str_in_namelist ("bed", obj->name)) {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_IN);
                min_occupants = 1;
                min_hit = 200;
            }
            if (str_in_namelist ("sofa", obj->name) ||
                str_in_namelist ("couch", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 150;
            }
            if (str_in_namelist ("bench", obj->name)) {
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }
            if (str_in_namelist ("chair", obj->name) ||
                str_in_namelist ("stool", obj->name))
            {
                SET_BIT (obj->v.furniture.flags, STAND_ON);
                SET_BIT (obj->v.furniture.flags, REST_ON);
                SET_BIT (obj->v.furniture.flags, SIT_ON);
                SET_BIT (obj->v.furniture.flags, SLEEP_ON);
                min_occupants = 1;
                min_hit = 125;
            }

            if (obj->v.furniture.max_people < min_occupants)
                obj->v.furniture.max_people = min_occupants;
            if (obj->v.furniture.heal_rate < min_hit)
                obj->v.furniture.heal_rate = min_hit;
            if (obj->v.furniture.mana_rate < min_mana)
                obj->v.furniture.mana_rate = min_mana;
            return TRUE;
        }
#endif

        default:
            return FALSE;
    }
}

int item_index_get_old_convert_shop_level (const OBJ_INDEX_T *obj_index) {
    switch (obj_index->item_type) {
        case ITEM_PILL:
        case ITEM_POTION:
            return UMAX (5, obj_index->level);

        case ITEM_SCROLL:
        case ITEM_ARMOR:
        case ITEM_WEAPON:
            return UMAX (10, obj_index->level);

        case ITEM_WAND:
        case ITEM_TREASURE:
            return UMAX (15, obj_index->level);

        case ITEM_STAFF:
            return UMAX (20, obj_index->level);

        default:
            return UMAX (0, obj_index->level);
    }
}

int item_index_get_old_reset_shop_level (const OBJ_INDEX_T *obj_index) {
    switch (obj_index->item_type) {
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL: {
            const flag_t *skills = NULL;
            int olevel = 53, skill_max, i, j;

            switch (obj_index->item_type) {
                case ITEM_PILL:
                    skills = obj_index->v.pill.skill;
                    skill_max = PILL_SKILL_MAX;
                    break;
                case ITEM_POTION:
                    skills = obj_index->v.potion.skill;
                    skill_max = POTION_SKILL_MAX;
                    break;
                case ITEM_SCROLL:
                    skills = obj_index->v.scroll.skill;
                    skill_max = SCROLL_SKILL_MAX;
                    break;
                default:
                    return 0;
            }

            for (i = 0; i < skill_max; i++) {
                int sk = skills[i];
                if (sk <= 0 || sk >= SKILL_MAX)
                    continue;
                for (j = 0; class_get (j) != NULL; j++)
                    olevel = UMIN (olevel, skill_table[sk].classes[j].level);
            }
            return UMAX (0, (olevel * 3 / 4) - 2);
        }

        case ITEM_WAND:     return number_range (10, 20);
        case ITEM_STAFF:    return number_range (15, 25);
        case ITEM_ARMOR:    return number_range (5, 15);

        /* ROM patch weapon, treasure */
        case ITEM_WEAPON:   return number_range (5, 15);
        case ITEM_TREASURE: return number_range (10, 20);

#if 0 /* envy version */
        case ITEM_WEAPON:
            if (reset->command == 'G')
                olevel = number_range (5, 15);
            else
                olevel = number_fuzzy (level);
#endif /* envy version */

        default:
            return 0;
    }
}

bool item_index_read_values_from_file (OBJ_INDEX_T *obj_index, FILE *fp) {
    switch (obj_index->item_type) {
        case ITEM_WEAPON:
            obj_index->v.weapon.weapon_type = weapon_lookup_exact (fread_word (fp));
            obj_index->v.weapon.dice_num    = fread_number (fp);
            obj_index->v.weapon.dice_size   = fread_number (fp);
            obj_index->v.weapon.attack_type = attack_lookup_exact (fread_word (fp));
            obj_index->v.weapon.flags       = fread_flag (fp);
            return TRUE;

        case ITEM_CONTAINER:
            obj_index->v.container.capacity    = fread_number (fp);
            obj_index->v.container.flags       = fread_flag (fp);
            obj_index->v.container.key         = fread_number (fp);
            obj_index->v.container.max_weight  = fread_number (fp);
            obj_index->v.container.weight_mult = fread_number (fp);
            return TRUE;

        case ITEM_DRINK_CON:
            obj_index->v.drink_con.capacity = fread_number (fp);
            obj_index->v.drink_con.filled   = fread_number (fp);
            obj_index->v.drink_con.liquid   = lookup_func_backup (liq_lookup_exact,
                fread_word (fp), "Unknown liquid type '%s'", 0);
            obj_index->v.drink_con.poisoned = fread_number (fp);
            obj_index->v.drink_con._value5  = fread_number (fp);
            return TRUE;

        case ITEM_FOUNTAIN:
            obj_index->v.fountain.capacity = fread_number (fp);
            obj_index->v.fountain.filled   = fread_number (fp);
            obj_index->v.fountain.liquid   = lookup_func_backup (liq_lookup_exact,
                fread_word (fp), "Unknown liquid type '%s'", 0);
            obj_index->v.fountain.poisoned = fread_number (fp);
            obj_index->v.fountain._value5  = fread_number (fp);
            return TRUE;

        case ITEM_WAND:
            obj_index->v.wand.level    = fread_number (fp);
            obj_index->v.wand.recharge = fread_number (fp);
            obj_index->v.wand.charges  = fread_number (fp);
            obj_index->v.wand.skill    = skill_lookup_exact (fread_word (fp));
            obj_index->v.wand._value5  = fread_number (fp);
            return TRUE;

        case ITEM_STAFF:
            obj_index->v.staff.level    = fread_number (fp);
            obj_index->v.staff.recharge = fread_number (fp);
            obj_index->v.staff.charges  = fread_number (fp);
            obj_index->v.staff.skill    = skill_lookup_exact (fread_word (fp));
            obj_index->v.staff._value5  = fread_number (fp);
            return TRUE;

        case ITEM_POTION:
            obj_index->v.potion.level  = fread_number (fp);
            obj_index->v.potion.skill[0] = skill_lookup_exact (fread_word (fp));
            obj_index->v.potion.skill[1] = skill_lookup_exact (fread_word (fp));
            obj_index->v.potion.skill[2] = skill_lookup_exact (fread_word (fp));
            obj_index->v.potion.skill[3] = skill_lookup_exact (fread_word (fp));
            return TRUE;

        case ITEM_PILL:
            obj_index->v.pill.level  = fread_number (fp);
            obj_index->v.pill.skill[0] = skill_lookup_exact (fread_word (fp));
            obj_index->v.pill.skill[1] = skill_lookup_exact (fread_word (fp));
            obj_index->v.pill.skill[2] = skill_lookup_exact (fread_word (fp));
            obj_index->v.pill.skill[3] = skill_lookup_exact (fread_word (fp));
            return TRUE;

        case ITEM_SCROLL:
            obj_index->v.scroll.level  = fread_number (fp);
            obj_index->v.scroll.skill[0] = skill_lookup_exact (fread_word (fp));
            obj_index->v.scroll.skill[1] = skill_lookup_exact (fread_word (fp));
            obj_index->v.scroll.skill[2] = skill_lookup_exact (fread_word (fp));
            obj_index->v.scroll.skill[3] = skill_lookup_exact (fread_word (fp));
            return TRUE;

        default:
            obj_index->v.value[0] = fread_flag (fp);
            obj_index->v.value[1] = fread_flag (fp);
            obj_index->v.value[2] = fread_flag (fp);
            obj_index->v.value[3] = fread_flag (fp);
            obj_index->v.value[4] = fread_flag (fp);
            return TRUE;
    }
}
/* TODO: use type-based values, not v.value[] */
bool item_index_write_values_to_file (OBJ_INDEX_T *obj_index, FILE *fp) {
    char buf[MAX_STRING_LENGTH];

    /* Using fwrite_flag to write most values gives a strange
     * looking area file, consider making a case for each
     * item type later. */
    switch (obj_index->item_type) {
        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            fprintf (fp, "%ld %ld '%s' %ld %ld\n",
                     obj_index->v.value[0],
                     obj_index->v.value[1],
                     liq_table[obj_index->v.value[2]].name,
                     obj_index->v.value[3],
                     obj_index->v.value[4]);
            return TRUE;

        case ITEM_CONTAINER:
            fprintf (fp, "%ld %s %ld %ld %ld\n",
                     obj_index->v.value[0],
                     fwrite_flag (obj_index->v.value[1], buf),
                     obj_index->v.value[2],
                     obj_index->v.value[3],
                     obj_index->v.value[4]);
            return TRUE;

        case ITEM_WEAPON:
            fprintf (fp, "%s %ld %ld %s %s\n",
                     weapon_get_name (obj_index->v.value[0]),
                     obj_index->v.value[1],
                     obj_index->v.value[2],
                     attack_table[obj_index->v.value[3]].name,
                     fwrite_flag (obj_index->v.value[4], buf));
            return TRUE;

        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
            /* no negative numbers */
            fprintf (fp, "%ld '%s' '%s' '%s' '%s'\n",
                     obj_index->v.value[0]  >  0 ? obj_index->v.value[0] : 0,
                     obj_index->v.value[1] != -1 ? skill_table[obj_index->v.value[1]].name : "",
                     obj_index->v.value[2] != -1 ? skill_table[obj_index->v.value[2]].name : "",
                     obj_index->v.value[3] != -1 ? skill_table[obj_index->v.value[3]].name : "",
                     obj_index->v.value[4] != -1 ? skill_table[obj_index->v.value[4]].name : "");
            return TRUE;

        case ITEM_STAFF:
        case ITEM_WAND:
            fprintf (fp, "%ld %ld %ld '%s' %ld\n",
                     obj_index->v.value[0],
                     obj_index->v.value[1],
                     obj_index->v.value[2],
                     obj_index->v.value[3] != -1
                        ? skill_table[obj_index->v.value[3]].name : "",
                     obj_index->v.value[4]);
            return TRUE;

        default:
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[0], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[1], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[2], buf));
            fprintf (fp, "%s ",  fwrite_flag (obj_index->v.value[3], buf));
            fprintf (fp, "%s\n", fwrite_flag (obj_index->v.value[4], buf));
            return TRUE;
    }
}
/* Object Editor Functions. */
bool item_index_show_values (const OBJ_INDEX_T *obj, CHAR_T *ch) {
    switch (obj->item_type) {
        case ITEM_LIGHT:
            if (obj->v.light.duration == -1 ||
                obj->v.light.duration == 999)
            {
                send_to_char ("[v2] Light:  Infinite[-1]\n\r", ch);
            }
            else
                printf_to_char (ch, "[v2] Light:  [%ld]\n\r",
                    obj->v.light.duration);
            return TRUE;

        case ITEM_WAND:
            printf_to_char (ch,
                "[v0] Level:         [%ld]\n\r"
                "[v1] Charges Total: [%ld]\n\r"
                "[v2] Charges Left:  [%ld]\n\r"
                "[v3] Spell:         %s\n\r",
                 obj->v.wand.level,
                 obj->v.wand.recharge,
                 obj->v.wand.charges,
                 obj->v.wand.skill > 0
                    ? skill_table[obj->v.wand.skill].name : "none");
            return TRUE;

        case ITEM_STAFF:
            printf_to_char (ch,
                "[v0] Level:         [%ld]\n\r"
                "[v1] Charges Total: [%ld]\n\r"
                "[v2] Charges Left:  [%ld]\n\r"
                "[v3] Spell:         %s\n\r",
                 obj->v.staff.level,
                 obj->v.staff.recharge,
                 obj->v.staff.charges,
                 obj->v.staff.skill > 0
                    ? skill_table[obj->v.staff.skill].name : "none");
            return TRUE;

        case ITEM_PORTAL:
            printf_to_char (ch,
                "[v0] Charges:        [%ld]\n\r"
                "[v1] Exit Flags:     %s\n\r"
                "[v2] Portal Flags:   %s\n\r"
                "[v3] Goes to (vnum): [%ld]\n\r",
                obj->v.portal.charges,
                flags_to_string (exit_flags, obj->v.portal.exit_flags),
                flags_to_string (gate_flags, obj->v.portal.gate_flags),
                obj->v.portal.to_vnum);
            return TRUE;

        case ITEM_FURNITURE:
            printf_to_char (ch,
                "[v0] Max People:      [%ld]\n\r"
                "[v1] Max Weight:      [%ld]\n\r"
                "[v2] Furniture Flags: %s\n\r"
                "[v3] Heal bonus:      [%ld]\n\r"
                "[v4] Mana bonus:      [%ld]\n\r",
                obj->v.furniture.max_people,
                obj->v.furniture.max_weight,
                flags_to_string (furniture_flags, obj->v.furniture.flags),
                obj->v.furniture.heal_rate,
                obj->v.furniture.mana_rate);
            return TRUE;

        case ITEM_SCROLL: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.scroll.level);
            for (i = 0; i < SCROLL_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.scroll.skill[i] > 0
                        ? skill_table[obj->v.scroll.skill[i]].name : "none");
            }
            return TRUE;
        }

        case ITEM_POTION: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.potion.level);
            for (i = 0; i < POTION_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.potion.skill[i] > 0
                        ? skill_table[obj->v.potion.skill[i]].name : "none");
            }
            return TRUE;
        }

        case ITEM_PILL: {
            int i;
            printf_to_char (ch, "[v0] Level: [%ld]\n\r", obj->v.pill.level);
            for (i = 0; i < PILL_SKILL_MAX; i++) {
                printf_to_char (ch, "[v%d] Spell: %s\n\r", i + 1,
                    obj->v.pill.skill[i] > 0
                        ? skill_table[obj->v.pill.skill[i]].name : "none");
            }
            return TRUE;
        }

        /* ARMOR for ROM */
        case ITEM_ARMOR:
            printf_to_char (ch,
                "[v0] Ac Pierce: [%ld]\n\r"
                "[v1] Ac Bash:   [%ld]\n\r"
                "[v2] Ac Slash:  [%ld]\n\r"
                "[v3] Ac Magic:  [%ld]\n\r",
                obj->v.armor.vs_pierce,
                obj->v.armor.vs_bash,
                obj->v.armor.vs_slash,
                obj->v.armor.vs_magic);
            return TRUE;

        /* WEAPON changed in ROM: */
        /* I had to split the output here, I have no idea why, but it helped -- Hugin */
        /* It somehow fixed a bug in showing scroll/pill/potions too ?! */

        /* ^^^ flag_string() uses a static char[], which must be copied to at
         *     least one separate buffer. -- Synival */

        case ITEM_WEAPON: {
            char wtype[MAX_STRING_LENGTH];
            char wflags[MAX_STRING_LENGTH];
            strcpy (wtype,  type_get_name (weapon_types, obj->v.weapon.weapon_type));
            strcpy (wflags, flags_to_string (weapon_flags, obj->v.weapon.flags));

            printf_to_char (ch,
                "[v0] Weapon Class:   %s\n\r"
                "[v1] Number of Dice: [%ld]\n\r"
                "[v2] Type of Dice:   [%ld]\n\r"
                "[v3] Type:           %s\n\r"
                "[v4] Special Type:   %s\n\r",
                wtype,
                obj->v.weapon.dice_num,
                obj->v.weapon.dice_size,
                attack_table[obj->v.weapon.attack_type].name,
                wflags);
            return TRUE;
        }

        case ITEM_CONTAINER: {
            OBJ_INDEX_T *key = get_obj_index (obj->v.container.key);
            printf_to_char (ch,
                "[v0] Weight:     [%ld kg]\n\r"
                "[v1] Flags:      [%s]\n\r"
                "[v2] Key:        %s [%ld]\n\r"
                "[v3] Capacity    [%ld]\n\r"
                "[v4] Weight Mult [%ld]\n\r",
                obj->v.container.capacity,
                flags_to_string (container_flags, obj->v.container.flags),
                key ? key->short_descr : "none",
                obj->v.container.key,
                obj->v.container.max_weight,
                obj->v.container.weight_mult);
            return TRUE;
        }

        case ITEM_DRINK_CON:
            printf_to_char (ch,
                "[v0] Liquid Total: [%ld]\n\r"
                "[v1] Liquid Left:  [%ld]\n\r"
                "[v2] Liquid:       %s\n\r"
                "[v3] Poisoned:     %s\n\r",
                obj->v.drink_con.capacity,
                obj->v.drink_con.filled,
                liq_table[obj->v.drink_con.liquid].name,
                obj->v.drink_con.poisoned != 0 ? "Yes" : "No");
            return TRUE;

        case ITEM_FOUNTAIN:
            printf_to_char (ch,
                "[v0] Liquid Total: [%ld]\n\r"
                "[v1] Liquid Left:  [%ld]\n\r"
                "[v2] Liquid:        %s\n\r"
                "[v3] Poisoned:      %s\n\r",
                obj->v.fountain.capacity,
                obj->v.fountain.filled,
                liq_table[obj->v.fountain.liquid].name,
                obj->v.fountain.poisoned != 0 ? "Yes" : "No");
            return TRUE;

        case ITEM_FOOD:
            printf_to_char (ch,
                "[v0] Food hours: [%ld]\n\r"
                "[v1] Full hours: [%ld]\n\r"
                "[v3] Poisoned:   %s\n\r",
                obj->v.food.hunger,
                obj->v.food.fullness,
                obj->v.food.poisoned != 0 ? "Yes" : "No");
            return TRUE;

        case ITEM_MONEY:
            printf_to_char (ch,
                "[v0] Silver: [%ld]\n\r",
                "[v1] Gold:   [%ld]\n\r",
                obj->v.money.silver,
                obj->v.money.gold);
            return TRUE;

        default:
            return FALSE;
    }
}

flag_t item_exit_flag_to_cont (flag_t exit_flag) {
    switch (exit_flag) {
        case EX_ISDOOR:    return CONT_CLOSEABLE;
        case EX_CLOSED:    return CONT_CLOSED;
        case EX_LOCKED:    return CONT_LOCKED;
        case EX_PICKPROOF: return CONT_PICKPROOF;
        default:           return 0;
    }
}
