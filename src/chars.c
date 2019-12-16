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
 *   ROM 2.4 is copyright 1993-1998 Russ Taylor                            *
 *   ROM has been brought to you by the ROM consortium                     *
 *       Russ Taylor (rtaylor@hypercube.org)                               *
 *       Gabrielle Taylor (gtaylor@hypercube.org)                          *
 *       Brian Moore (zump@rom.org)                                        *
 *   By using this code, you have agreed to follow the terms of the        *
 *   ROM license, in the file Rom24/doc/rom.license                        *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "skills.h"
#include "comm.h"
#include "objs.h"
#include "affects.h"
#include "lookup.h"
#include "magic.h"
#include "db.h"
#include "groups.h"
#include "fight.h"
#include "interp.h"
#include "recycle.h"
#include "rooms.h"
#include "mob_prog.h"
#include "act_move.h"
#include "act_info.h"
#include "act_group.h"
#include "wiz_l6.h"
#include "materials.h"
#include "globals.h"
#include "memory.h"

#include "chars.h"

/* Create an instance of a mobile. */
CHAR_T *char_create_mobile (MOB_INDEX_T *mob_index) {
    CHAR_T *mob;
    int i;
    AFFECT_T af;

    mobile_count++;
    EXIT_IF_BUG (mob_index == NULL,
        "char_create_mobile: NULL mob_index.", 0);

    mob = char_new ();

    mob->index_data = mob_index;
    str_replace_dup (&mob->name,        mob_index->name);        /* OLC */
    str_replace_dup (&mob->short_descr, mob_index->short_descr); /* OLC */
    str_replace_dup (&mob->long_descr,  mob_index->long_descr);  /* OLC */
    str_replace_dup (&mob->description, mob_index->description); /* OLC */
    mob->id           = get_mob_id ();
    mob->spec_fun     = mob_index->spec_fun;
    mob->prompt       = NULL;
    mob->mprog_target = NULL;
    mob->class        = CLASS_NONE;

    if (mob_index->wealth == 0) {
        mob->silver = 0;
        mob->gold = 0;
    }
    else {
        long wealth;
        wealth = number_range (mob_index->wealth / 2, 3 * mob_index->wealth / 2);
        mob->gold = number_range (wealth / 200, wealth / 100);
        mob->silver = wealth - (mob->gold * 100);
    }

    /* load in new style */
    if (mob_index->new_format) {
        /* read from prototype */
        mob->group    = mob_index->group;
        mob->mob      = mob_index->mob_final;
        mob->plr      = 0;
        mob->comm     = COMM_NOCHANNELS | COMM_NOSHOUT | COMM_NOTELL;
        mob->affected_by = mob_index->affected_by_final;
        mob->alignment = mob_index->alignment;
        mob->level    = mob_index->level;
        mob->hitroll  = mob_index->hitroll;
        mob->damroll  = mob_index->damage.bonus;
        mob->max_hit  = dice (mob_index->hit.number, mob_index->hit.size)
            + mob_index->hit.bonus;
        mob->hit      = mob->max_hit;
        mob->max_mana = dice (mob_index->mana.number, mob_index->mana.size)
            + mob_index->mana.bonus;
        mob->mana     = mob->max_mana;
        mob->damage   = mob_index->damage;
        mob->damage.bonus = 0;
        mob->attack_type = mob_index->attack_type;

        if (mob->attack_type == 0) {
            switch (number_range (1, 3)) {
                case 1: mob->attack_type = ATTACK_SLASH;  break;
                case 2: mob->attack_type = ATTACK_POUND;  break;
                case 3: mob->attack_type = ATTACK_PIERCE; break;
            }
        }
        for (i = 0; i < 4; i++)
            mob->armor[i] = mob_index->ac[i];
        mob->off_flags   = mob_index->off_flags_final;
        mob->imm_flags   = mob_index->imm_flags_final;
        mob->res_flags   = mob_index->res_flags_final;
        mob->vuln_flags  = mob_index->vuln_flags_final;
        mob->start_pos   = mob_index->start_pos;
        mob->default_pos = mob_index->default_pos;

        mob->sex = mob_index->sex;
        if (mob->sex == 3) /* random sex */
            mob->sex = number_range (1, 2);
        mob->race     = mob_index->race;
        mob->form     = mob_index->form_final;
        mob->parts    = mob_index->parts_final;
        mob->size     = mob_index->size;
        mob->material = mob_index->material;

        /* computed on the spot */
        for (i = 0; i < STAT_MAX; i++)
            mob->perm_stat[i] = UMIN (25, 11 + mob->level / 4);

        if (IS_SET (mob->mob, MOB_WARRIOR)) {
            mob->perm_stat[STAT_STR] += 3;
            mob->perm_stat[STAT_INT] -= 1;
            mob->perm_stat[STAT_CON] += 2;
        }
        if (IS_SET (mob->mob, MOB_THIEF)) {
            mob->perm_stat[STAT_DEX] += 3;
            mob->perm_stat[STAT_INT] += 1;
            mob->perm_stat[STAT_WIS] -= 1;
        }
        if (IS_SET (mob->mob, MOB_CLERIC)) {
            mob->perm_stat[STAT_WIS] += 3;
            mob->perm_stat[STAT_DEX] -= 1;
            mob->perm_stat[STAT_STR] += 1;
        }
        if (IS_SET (mob->mob, MOB_MAGE)) {
            mob->perm_stat[STAT_INT] += 3;
            mob->perm_stat[STAT_STR] -= 1;
            mob->perm_stat[STAT_DEX] += 1;
        }
        if (IS_SET (mob->off_flags, OFF_FAST))
            mob->perm_stat[STAT_DEX] += 2;

        mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
        mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

        /* let's get some spell action */
        if (IS_AFFECTED (mob, AFF_SANCTUARY)) {
            affect_init (&af, AFF_TO_AFFECTS, skill_lookup_exact ("sanctuary"), mob->level, -1, APPLY_NONE, 0, AFF_SANCTUARY);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_HASTE)) {
            affect_init (&af, AFF_TO_AFFECTS, skill_lookup_exact ("haste"), mob->level, -1, APPLY_DEX, 1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32), AFF_HASTE);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_PROTECT_EVIL)) {
            affect_init (&af, AFF_TO_AFFECTS, skill_lookup_exact ("protection evil"), mob->level, -1, APPLY_SAVES, -1, AFF_PROTECT_EVIL);
            affect_to_char (mob, &af);
        }
        if (IS_AFFECTED (mob, AFF_PROTECT_GOOD)) {
            affect_init (&af, AFF_TO_AFFECTS, skill_lookup_exact ("protection good"), mob->level, -1, APPLY_SAVES, -1, AFF_PROTECT_GOOD);
            affect_to_char (mob, &af);
        }
    }
    /* read in old format and convert */
    else {
        mob->mob         = mob_index->mob_final;
        mob->plr         = 0;
        mob->affected_by = mob_index->affected_by_final;
        mob->alignment   = mob_index->alignment;
        mob->level       = mob_index->level;
        mob->hitroll     = mob_index->hitroll;
        mob->damroll     = 0;
        mob->max_hit     = (mob->level * 8 + number_range
            (mob->level * mob->level / 4, mob->level * mob->level)) * 9 / 10;
        mob->hit         = mob->max_hit;
        mob->max_mana    = 100 + dice (mob->level, 10);
        mob->mana        = mob->max_mana;
        switch (number_range (1, 3)) {
            case 1: mob->attack_type = ATTACK_SLASH;  break;
            case 2: mob->attack_type = ATTACK_POUND;  break;
            case 3: mob->attack_type = ATTACK_PIERCE; break;
        }
        for (i = 0; i < 3; i++)
            mob->armor[i] = interpolate (mob->level, 100, -100);
        mob->armor[3]    = interpolate (mob->level, 100, 0);
        mob->race        = mob_index->race;
        mob->off_flags   = mob_index->off_flags_final;
        mob->imm_flags   = mob_index->imm_flags_final;
        mob->res_flags   = mob_index->res_flags_final;
        mob->vuln_flags  = mob_index->vuln_flags_final;
        mob->start_pos   = mob_index->start_pos;
        mob->default_pos = mob_index->default_pos;
        mob->sex         = mob_index->sex;
        mob->form        = mob_index->form_final;
        mob->parts       = mob_index->parts_final;
        mob->size        = SIZE_MEDIUM;

        for (i = 0; i < STAT_MAX; i++)
            mob->perm_stat[i] = 11 + mob->level / 4;
    }
    mob->position = mob->start_pos;

    /* link the mob to the world list */
    LIST_FRONT (mob, next, char_list);
    mob_index->count++;
    return mob;
}

/* duplicate a mobile exactly -- except inventory */
void char_clone_mobile (CHAR_T *parent, CHAR_T *clone) {
    int i;
    AFFECT_T *paf;

    if (parent == NULL || clone == NULL || !IS_NPC (parent))
        return;

    /* start fixing values */
    str_replace_dup (&clone->name, parent->name);
    clone->version     = parent->version;
    str_replace_dup (&clone->short_descr, parent->short_descr);
    str_replace_dup (&clone->long_descr,  parent->long_descr);
    str_replace_dup (&clone->description, parent->description);
    clone->group       = parent->group;
    clone->sex         = parent->sex;
    clone->class       = parent->class;
    clone->race        = parent->race;
    clone->level       = parent->level;
    clone->trust       = 0;
    clone->timer       = parent->timer;
    clone->wait        = parent->wait;
    clone->hit         = parent->hit;
    clone->max_hit     = parent->max_hit;
    clone->mana        = parent->mana;
    clone->max_mana    = parent->max_mana;
    clone->move        = parent->move;
    clone->max_move    = parent->max_move;
    clone->gold        = parent->gold;
    clone->silver      = parent->silver;
    clone->exp         = parent->exp;
    clone->mob         = parent->mob;
    clone->plr         = parent->plr;
    clone->comm        = parent->comm;
    clone->imm_flags   = parent->imm_flags;
    clone->res_flags   = parent->res_flags;
    clone->vuln_flags  = parent->vuln_flags;
    clone->invis_level = parent->invis_level;
    clone->affected_by = parent->affected_by;
    clone->position    = parent->position;
    clone->practice    = parent->practice;
    clone->train       = parent->train;
    clone->saving_throw = parent->saving_throw;
    clone->alignment   = parent->alignment;
    clone->hitroll     = parent->hitroll;
    clone->damroll     = parent->damroll;
    clone->wimpy       = parent->wimpy;
    clone->form        = parent->form;
    clone->parts       = parent->parts;
    clone->size        = parent->size;
    clone->material    = parent->material;
    clone->off_flags   = parent->off_flags;
    clone->attack_type = parent->attack_type;
    clone->start_pos   = parent->start_pos;
    clone->default_pos = parent->default_pos;
    clone->spec_fun    = parent->spec_fun;
    clone->damage      = parent->damage;

    for (i = 0; i < 4; i++)
        clone->armor[i] = parent->armor[i];

    for (i = 0; i < STAT_MAX; i++) {
        clone->perm_stat[i] = parent->perm_stat[i];
        clone->mod_stat[i]  = parent->mod_stat[i];
    }

    /* now add the affects */
    for (paf = parent->affected; paf != NULL; paf = paf->next)
        affect_to_char (clone, paf);
}

bool char_has_clan (CHAR_T *ch) {
    return ch->clan;
}

bool char_in_same_clan (CHAR_T *ch, CHAR_T *victim) {
    if (clan_table[ch->clan].independent)
        return FALSE;
    else
        return (ch->clan == victim->clan);
}

OBJ_T *char_get_weapon (CHAR_T *ch) {
    OBJ_T *wield;

    wield = char_get_eq_by_wear_loc (ch, WEAR_WIELD);
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        return NULL;
    return wield;
}

/* for returning weapon information */
int char_get_weapon_sn (CHAR_T *ch) {
    const WEAPON_T *weapon;
    const OBJ_T *wield;

    wield = char_get_weapon (ch);
    if (wield == NULL)
        return gsn_hand_to_hand;

    weapon = weapon_get (wield->v.weapon.weapon_type);
    if (weapon == NULL || weapon->gsn == NULL)
        return -1;

    return *(weapon->gsn);
}

int char_get_weapon_skill (CHAR_T *ch, int sn) {
    int skill;

    /* -1 is exotic */
    if (IS_NPC (ch)) {
        if (sn == -1)
            skill = 3 * ch->level;
        else if (sn == gsn_hand_to_hand)
            skill = 40 + 2 * ch->level;
        else
            skill = 40 + 5 * ch->level / 2;
    }
    else {
        if (sn == -1)
            skill = 3 * ch->level;
        else
            skill = ch->pcdata->learned[sn];
    }
    return URANGE (0, skill, 100);
}

/* used to de-screw characters */
void char_reset (CHAR_T *ch) {
    int loc, mod, stat;
    OBJ_T *obj;
    AFFECT_T *af;
    int i;

    if (IS_NPC (ch))
        return;

    if (ch->pcdata->perm_hit   == 0 ||
        ch->pcdata->perm_mana  == 0 ||
        ch->pcdata->perm_move  == 0 ||
        ch->pcdata->last_level == 0)
    {
        /* do a FULL reset */
        for (loc = 0; loc < WEAR_LOC_MAX; loc++) {
            obj = char_get_eq_by_wear_loc (ch, loc);
            if (obj == NULL)
                continue;
            if (!obj->enchanted) {
                for (af = obj->index_data->affected; af != NULL;
                     af = af->next)
                {
                    mod = af->modifier;
                    switch (af->apply) {
                        case APPLY_SEX:
                            ch->sex -= mod;
                            if (ch->sex < 0 || ch->sex > 2)
                                ch->sex = IS_NPC (ch) ? 0 : ch->pcdata->true_sex;
                            break;
                        case APPLY_MANA: ch->max_mana -= mod; break;
                        case APPLY_HIT:  ch->max_hit  -= mod; break;
                        case APPLY_MOVE: ch->max_move -= mod; break;
                    }
                }
            }

            for (af = obj->affected; af != NULL; af = af->next) {
                mod = af->modifier;
                switch (af->apply) {
                    case APPLY_SEX:  ch->sex      -= mod; break;
                    case APPLY_MANA: ch->max_mana -= mod; break;
                    case APPLY_HIT:  ch->max_hit  -= mod; break;
                    case APPLY_MOVE: ch->max_move -= mod; break;
                }
            }
        }

        /* now reset the permanent stats */
        ch->pcdata->perm_hit = ch->max_hit;
        ch->pcdata->perm_mana = ch->max_mana;
        ch->pcdata->perm_move = ch->max_move;
        ch->pcdata->last_level = ch->played / 3600;

        if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2) {
            if (ch->sex > 0 && ch->sex < 3)
                ch->pcdata->true_sex = ch->sex;
            else
                ch->pcdata->true_sex = 0;
        }
    }

    /* now restore the character to his/her true condition */
    for (stat = 0; stat < STAT_MAX; stat++)
        ch->mod_stat[stat] = 0;

    if (ch->pcdata->true_sex < 0 || ch->pcdata->true_sex > 2)
        ch->pcdata->true_sex = 0;
    ch->sex      = ch->pcdata->true_sex;
    ch->max_hit  = ch->pcdata->perm_hit;
    ch->max_mana = ch->pcdata->perm_mana;
    ch->max_move = ch->pcdata->perm_move;

    for (i = 0; i < 4; i++)
        ch->armor[i] = 100;

    ch->hitroll = 0;
    ch->damroll = 0;
    ch->saving_throw = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < WEAR_LOC_MAX; loc++) {
        obj = char_get_eq_by_wear_loc (ch, loc);
        if (obj == NULL)
            continue;
        for (i = 0; i < 4; i++)
            ch->armor[i] -= obj_get_ac_type (obj, loc, i);
        if (obj->enchanted)
            continue;
        for (af = obj->index_data->affected; af != NULL; af = af->next)
            affect_modify_apply (ch, af, TRUE);
        for (af = obj->affected; af != NULL; af = af->next)
            affect_modify_apply (ch, af, TRUE);
    }

    /* now add back spell effects */
    for (af = ch->affected; af != NULL; af = af->next)
        affect_modify_apply (ch, af, TRUE);

    /* make sure sex is RIGHT!!!! */
    if (ch->sex < 0 || ch->sex > 2)
        ch->sex = ch->pcdata->true_sex;
}

/* Retrieve a character's trusted level for permission checking. */
int char_get_trust (CHAR_T *ch) {
    if (ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;
    if (ch->trust)
        return ch->trust;
    if (IS_NPC (ch) && ch->level >= LEVEL_HERO)
        return LEVEL_HERO - 1;
    else
        return ch->level;
}

/* command for retrieving stats */
int char_get_curr_stat (CHAR_T *ch, int stat) {
    int max;

    if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
        max = 25;
    else {
        max = pc_race_table[ch->race].max_stats[stat] + 4;
        if (class_table[ch->class].attr_prime == stat)
            max += 2;
        if (ch->race == race_lookup ("human"))
            max += 1;
        max = UMIN (max, 25);
    }
    return URANGE (3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int char_get_max_train (CHAR_T *ch, int stat) {
    int max;

    if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
        return 25;

    max = pc_race_table[ch->race].max_stats[stat];
    if (class_table[ch->class].attr_prime == stat) {
        if (ch->race == race_lookup ("human"))
            max += 3;
        else
            max += 2;
    }
    return UMIN (max, 25);
}

long int char_get_carry_weight (CHAR_T *ch) {
    return ch->carry_weight + (ch->silver / 10) + (ch->gold * 2 / 5);
}

/* Retrieve a character's carry capacity. */
int char_get_max_carry_count (CHAR_T *ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 1000;
    if (IS_PET (ch))
        return 0;
    return WEAR_LOC_MAX + (2 * char_get_curr_stat (ch, STAT_DEX)) + ch->level;
}

/* Retrieve a character's carry capacity. */
long int char_get_max_carry_weight (CHAR_T *ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 10000000;
    if (IS_PET (ch))
        return 0;
    return char_str_carry_bonus (ch) * 10 + ch->level * 25;
}

/* Move a char out of a room. */
void char_from_room (CHAR_T *ch) {
    OBJ_T *obj;

    BAIL_IF_BUG (ch->in_room == NULL,
        "char_from_room: NULL.", 0);

    if (!IS_NPC (ch))
        --ch->in_room->area->nplayer;

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LIGHT)) != NULL
        && obj->item_type == ITEM_LIGHT && obj->v.light.duration != 0
        && ch->in_room->light > 0)
    {
        --ch->in_room->light;
    }

    LIST_REMOVE (ch, next_in_room, ch->in_room->people, CHAR_T, NO_FAIL);
    ch->in_room = NULL;
    ch->on = NULL; /* sanity check! */
}

void char_to_room_apply_plague (CHAR_T *ch) {
    AFFECT_T *af, plague;
    CHAR_T *vch;

    for (af = ch->affected; af != NULL; af = af->next)
        if (af->type == gsn_plague)
            break;
    if (af == NULL) {
        REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
        return;
    }
    if (af->level == 1)
        return;

    affect_init (&plague, AFF_TO_AFFECTS, gsn_plague, af->level - 1, number_range (1, 2 * (af->level - 1)), APPLY_STR, -5, AFF_PLAGUE);

    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
        if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
            && !IS_IMMORTAL (vch) &&
            !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (6) == 0)
        {
            send_to_char ("You feel hot and feverish.\n\r", vch);
            act ("$n shivers and looks very ill.", vch, NULL, NULL,
                TO_NOTCHAR);
            affect_join (vch, &plague);
        }
    }
}

/* Move a char into a room. */
void char_to_room (CHAR_T *ch, ROOM_INDEX_T *room_index) {
    OBJ_T *obj;

    if (room_index == NULL) {
        ROOM_INDEX_T *room;
        bug ("char_to_room: NULL.", 0);
        if ((room = get_room_index (ROOM_VNUM_TEMPLE)) != NULL)
            char_to_room (ch, room);
        return;
    }

    ch->in_room = room_index;
    LIST_FRONT (ch, next_in_room, room_index->people);

    if (!IS_NPC (ch)) {
        if (ch->in_room->area->empty) {
            ch->in_room->area->empty = FALSE;
            ch->in_room->area->age = 0;
        }
        ++ch->in_room->area->nplayer;
    }

    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LIGHT)) != NULL
        && obj->item_type == ITEM_LIGHT && obj->v.light.duration != 0)
    {
        ++ch->in_room->light;
    }

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        char_to_room_apply_plague (ch);
}

/* Find a piece of eq on a character. */
OBJ_T *char_get_eq_by_wear_loc (CHAR_T *ch, flag_t wear_loc) {
    OBJ_T *obj;
    if (ch == NULL)
        return NULL;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        if (obj->wear_loc == wear_loc)
            return obj;
    return NULL;
}

/* Equip a char with an obj. */
bool char_equip_obj (CHAR_T *ch, OBJ_T *obj, flag_t wear_loc) {
    AFFECT_T *paf;
    int i;

    RETURN_IF_BUG (char_get_eq_by_wear_loc (ch, wear_loc) != NULL,
        "char_equip_obj: already equipped (%d).", wear_loc, FALSE);

    if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL)    && IS_EVIL (ch)) ||
        (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD)    && IS_GOOD (ch)) ||
        (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
    {
        /* Thanks to Morgenes for the bug fix here! */
        act2 ("You are zapped by $p and drop it.",
              "$n is zapped by $p and drops it.", ch, obj, NULL, 0, POS_RESTING);
        obj_take_from_char (obj);
        obj_give_to_room (obj, ch->in_room);
        return FALSE;
    }

    for (i = 0; i < 4; i++)
        ch->armor[i] -= obj_get_ac_type (obj, wear_loc, i);
    obj->wear_loc = wear_loc;

    if (!obj->enchanted) {
        for (paf = obj->index_data->affected; paf != NULL; paf = paf->next)
            if (paf->apply != APPLY_SPELL_AFFECT)
                affect_modify (ch, paf, TRUE);
    }
    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (paf->apply == APPLY_SPELL_AFFECT)
            affect_to_char (ch, paf);
        else
            affect_modify (ch, paf, TRUE);
    }

    if (obj->item_type == ITEM_LIGHT && obj->v.light.duration != 0 &&
        ch->in_room != NULL)
    {
        ++ch->in_room->light;
    }

    return TRUE;
}

/* Unequip a char with an obj. */
bool char_unequip_obj (CHAR_T *ch, OBJ_T *obj) {
    AFFECT_T *paf = NULL;
    AFFECT_T *lpaf = NULL;
    AFFECT_T *lpaf_next = NULL;
    int i;

    RETURN_IF_BUG (obj->wear_loc == WEAR_NONE,
        "char_unequip_obj: already unequipped.", 0, FALSE);

    for (i = 0; i < 4; i++)
        ch->armor[i] += obj_get_ac_type (obj, obj->wear_loc, i);
    obj->wear_loc = -1;

    if (!obj->enchanted) {
        for (paf = obj->index_data->affected; paf != NULL; paf = paf->next) {
            if (paf->apply == APPLY_SPELL_AFFECT) {
                for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next) {
                    lpaf_next = lpaf->next;
                    if ((lpaf->type == paf->type) &&
                        (lpaf->level == paf->level) &&
                        (lpaf->apply == APPLY_SPELL_AFFECT))
                    {
                        affect_remove (ch, lpaf);
                        lpaf_next = NULL;
                    }
                }
            }
            else {
                affect_modify (ch, paf, FALSE);
                affect_check (ch, paf->bit_type, paf->bits);
            }
        }
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (paf->apply == APPLY_SPELL_AFFECT) {
            bug ("norm-Apply: %d", 0);
            for (lpaf = ch->affected; lpaf != NULL; lpaf = lpaf_next) {
                lpaf_next = lpaf->next;
                if ((lpaf->type == paf->type) &&
                    (lpaf->level == paf->level) &&
                    (lpaf->apply == APPLY_SPELL_AFFECT))
                {
                    bug ("location = %d", lpaf->apply);
                    bug ("type = %d", lpaf->type);
                    affect_remove (ch, lpaf);
                    lpaf_next = NULL;
                }
            }
        }
        else {
            affect_modify (ch, paf, FALSE);
            affect_check (ch, paf->bit_type, paf->bits);
        }
    }

    if (obj->item_type == ITEM_LIGHT && obj->v.light.duration != 0 &&
        ch->in_room != NULL && ch->in_room->light > 0)
    {
        --ch->in_room->light;
    }

    return TRUE;
}

/* Extract a char from the world. */
void char_extract (CHAR_T *ch, bool pull) {
    CHAR_T *wch;
    OBJ_T *obj;
    OBJ_T *obj_next;

    /* doesn't seem to be necessary */
#if 0
    BAIL_IF_BUG (ch->in_room == NULL,
        "Extract_char: NULL.", 0);
#endif

    nuke_pets (ch);
    ch->pet = NULL; /* just in case */

    if (pull)
        die_follower (ch);

    stop_fighting (ch, TRUE);
    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        obj_extract (obj);
    }

    if (ch->in_room != NULL)
        char_from_room (ch);

    /* Death room is set in the clan table now */
    if (!pull) {
        char_to_room (ch, get_room_index (clan_table[ch->clan].hall));
        return;
    }

    if (IS_NPC (ch))
        --ch->index_data->count;

    if (ch->desc != NULL && ch->desc->original != NULL) {
        do_function (ch, &do_return, "");
        ch->desc = NULL;
    }

    for (wch = char_list; wch != NULL; wch = wch->next) {
        if (wch->reply == ch)
            wch->reply = NULL;
        if (ch->mprog_target == wch)
            wch->mprog_target = NULL;
    }

    LIST_REMOVE (ch, next, char_list, CHAR_T, return);
    if (ch->desc != NULL)
        ch->desc->character = NULL;

    char_free (ch);
}

/* visibility on a room -- for entering and exits */
bool char_can_see_room (CHAR_T *ch, ROOM_INDEX_T *room_index) {
    int flags = room_index->room_flags;
    if (ch == NULL || room_index == NULL)
        return FALSE;
    if (IS_SET (flags, ROOM_IMP_ONLY) && char_get_trust (ch) < MAX_LEVEL)
        return FALSE;
    if (IS_SET (flags, ROOM_GODS_ONLY) && !IS_IMMORTAL (ch))
        return FALSE;
    if (IS_SET (flags, ROOM_HEROES_ONLY) && !IS_IMMORTAL (ch))
        return FALSE;
    if (IS_SET (flags, ROOM_NEWBIES_ONLY) && ch->level > 5 && !IS_IMMORTAL (ch))
        return FALSE;
    if (!IS_IMMORTAL (ch) && room_index->clan && ch->clan != room_index->clan)
        return FALSE;
    return TRUE;
}

/* True if char can see victim, regardless of room. */
bool char_can_see_anywhere (CHAR_T *ch, CHAR_T *victim) {
    if (ch == victim)
        return TRUE;
    if (char_get_trust (ch) < victim->invis_level)
        return FALSE;
    if (char_get_trust (ch) < victim->incog_level &&
        ch->in_room != victim->in_room)
        return FALSE;
    if (  (!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT))
        || (IS_NPC (ch) && IS_IMMORTAL (ch)))
        return TRUE;
    if (IS_AFFECTED (ch, AFF_BLIND))
        return FALSE;
    if (room_is_dark (victim->in_room) && !IS_AFFECTED (ch, AFF_INFRARED))
        return FALSE;
    if (IS_AFFECTED (victim, AFF_INVISIBLE)
        && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
        return FALSE;

    /* sneaking */
    if (IS_AFFECTED (victim, AFF_SNEAK)
        && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN)
        && victim->fighting == NULL
        && victim->position > POS_STUNNED)
    {
        int chance;
        chance = get_skill (victim, gsn_sneak);
        chance += char_get_curr_stat (victim, STAT_DEX) * 3 / 2;
        chance -= char_get_curr_stat (ch, STAT_INT) * 2;
        chance -= ch->level - victim->level * 3 / 2;

        if (number_percent () < chance)
            return FALSE;
    }

    if (IS_AFFECTED (victim, AFF_HIDE)
        && !IS_AFFECTED (ch, AFF_DETECT_HIDDEN) && victim->fighting == NULL)
        return FALSE;

    return TRUE;
}

/* True if char can see victim in the same room. */
bool char_can_see_in_room (CHAR_T *ch, CHAR_T *victim) {
    if (ch == victim)
        return TRUE;
    if (char_get_trust (ch) < victim->incog_level &&
        ch->in_room != victim->in_room)
        return FALSE;
    if (!char_can_see_anywhere(ch, victim))
        return FALSE;
    return TRUE;
}

/* True if char can see obj. */
bool char_can_see_obj (CHAR_T *ch, OBJ_T *obj) {
    if (!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT))
        return TRUE;
    if (IS_SET (obj->extra_flags, ITEM_VIS_DEATH))
        return FALSE;
    if (IS_AFFECTED (ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
        return FALSE;
    if (obj->item_type == ITEM_LIGHT && obj->v.light.duration != 0)
        return TRUE;
    if (IS_SET (obj->extra_flags, ITEM_INVIS)
        && !IS_AFFECTED (ch, AFF_DETECT_INVIS))
        return FALSE;
    if (IS_OBJ_STAT (obj, ITEM_GLOW))
        return TRUE;
    if (room_is_dark (ch->in_room) && !IS_AFFECTED (ch, AFF_DARK_VISION))
        return FALSE;
    return TRUE;
}

/* True if char can drop obj. */
bool char_can_drop_obj (CHAR_T *ch, OBJ_T *obj) {
    if (!IS_SET (obj->extra_flags, ITEM_NODROP))
        return TRUE;
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return TRUE;
    return FALSE;
}

/* Config Colour stuff */
void char_reset_colour (CHAR_T *ch) {
    int i;
    if (ch == NULL || ch->pcdata == NULL)
        return;
    for (i = 0; i < COLOUR_MAX; i++)
        ch->pcdata->colour[i] = colour_setting_table[i].default_colour;
}

void char_move (CHAR_T *ch, int door, bool follow) {
    CHAR_T *fch;
    CHAR_T *fch_next;
    ROOM_INDEX_T *in_room;
    ROOM_INDEX_T *to_room;
    EXIT_T *pexit;
    bool pass_door;

    BAIL_IF_BUG (door < 0 || door > 5,
        "char_move: bad door %d.", door);

    BAIL_IF (ch->daze > 0,
        "You're too dazed to move...\n\r", ch);
    BAIL_IF (ch->wait > 0,
        "You haven't quite recovered yet...\n\r", ch);

    /* Exit trigger, if activated, bail out. Only PCs are triggered. */
    if (!IS_NPC (ch) && mp_exit_trigger (ch, door))
        return;

    in_room = ch->in_room;
    BAIL_IF ((pexit = in_room->exit[door]) == NULL ||
             (to_room = pexit->to_room) == NULL ||
             !char_can_see_room (ch, pexit->to_room),
        "Alas, you cannot go that way.\n\r", ch);

    /* Determine if we're passing through a door. */
    if (!IS_SET (pexit->exit_flags, EX_CLOSED))
        pass_door = FALSE;
    else if (IS_TRUSTED (ch, ANGEL))
        pass_door = TRUE;
    else if (IS_AFFECTED (ch, AFF_PASS_DOOR) &&
             !IS_SET (pexit->exit_flags, EX_NOPASS))
        pass_door = TRUE;
    else
        pass_door = FALSE;

    BAIL_IF_ACT (IS_SET (pexit->exit_flags, EX_CLOSED) && !pass_door,
        "The $d is closed.", ch, NULL, pexit->keyword);
    BAIL_IF (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL &&
            in_room == ch->master->in_room,
        "What?  And leave your beloved master?\n\r", ch);
    BAIL_IF (!room_is_owner (to_room, ch) && room_is_private (to_room),
        "That room is private right now.\n\r", ch);

    /* Special conditions for players. */
    if (!IS_NPC (ch)) {
        int class_n, guild;
        int move, in_sect, to_sect, flying;

        for (class_n = 0; class_n < CLASS_MAX; class_n++) {
            for (guild = 0; guild < MAX_GUILD; guild++) {
                BAIL_IF (class_n != ch->class &&
                         to_room->vnum == class_table[class_n].guild[guild],
                    "You aren't allowed in there.\n\r", ch);
            }
        }

        in_sect = in_room->sector_type;
        to_sect = to_room->sector_type;
        flying = IS_AFFECTED (ch, AFF_FLYING) || IS_IMMORTAL (ch);

        BAIL_IF ((in_sect == SECT_AIR || to_sect == SECT_AIR) && !flying,
            "You can't fly.\n\r", ch);

        if ((in_sect == SECT_WATER_NOSWIM || to_sect == SECT_WATER_NOSWIM) &&
             !flying)
        {
            /* Look for a boat. */
            OBJ_T *boat;
            for (boat = ch->carrying; boat != NULL; boat = boat->next_content)
                if (boat->item_type == ITEM_BOAT)
                    break;
            BAIL_IF (boat == NULL,
                "You need a boat to go there.\n\r", ch);
        }

        move = sector_table[UMIN (SECT_MAX - 1, in_room->sector_type)].move_loss
             + sector_table[UMIN (SECT_MAX - 1, to_room->sector_type)].move_loss;
        move /= 2; /* the average */

        /* conditional effects */
        if (IS_AFFECTED (ch, AFF_FLYING) || IS_AFFECTED (ch, AFF_HASTE))
            move /= 2;
        if (IS_AFFECTED (ch, AFF_SLOW))
            move *= 2;
        if (move < 1)
            move = 1;

        BAIL_IF (ch->move < move,
            "You are too exhausted.\n\r", ch);

        WAIT_STATE (ch, 1);
        ch->move -= move;
    }

    /* Force pets to stand before leaving the room. */
    if (in_room != to_room) {
        for (fch = in_room->people; fch != NULL; fch = fch_next) {
            fch_next = fch->next_in_room;
            if (fch->master == ch && IS_AFFECTED (fch, AFF_CHARM)
                    && fch->position < POS_STANDING)
                do_function (fch, &do_stand, "");
        }
    }

    if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) {
        if (IS_SET (pexit->exit_flags, EX_CLOSED)) {
            act ("You pass through the $d.", ch,
                NULL, pexit->keyword, TO_CHAR);
            act ("$n leaves $t, passing through the $d.", ch,
                door_table[door].name, pexit->keyword, TO_NOTCHAR);
        }
        else
            act ("$n leaves $t.", ch, door_table[door].name, NULL, TO_NOTCHAR);
    }

    char_from_room (ch);
    char_to_room (ch, to_room);

    if (!IS_AFFECTED (ch, AFF_SNEAK) && ch->invis_level < LEVEL_HERO) {
#ifdef BASEMUD_SHOW_ARRIVAL_DIRECTIONS
        EXIT_T *pexit_rev;
        int rev_dir = REV_DIR(door);

        pexit_rev = room_get_opposite_exit (in_room, door, NULL);
        if (!pexit_rev)
            act ("$n has arrived.", ch, NULL, NULL, TO_NOTCHAR);
        else {
            if (IS_SET (pexit_rev->exit_flags, EX_CLOSED))
                act ("$n enters $t, passing through the $d.", ch,
                    door_table[rev_dir].from_phrase, pexit_rev->keyword,
                    TO_NOTCHAR);
            else
                act ("$n enters $t.", ch,
                    door_table[rev_dir].from_phrase, NULL, TO_NOTCHAR);
        }
#else
        act ("$n has arrived.", ch, NULL, NULL, TO_NOTCHAR);
#endif
    }

    do_function (ch, &do_look, "auto");
    if (in_room == to_room) /* no circular follows */
        return;

    /* Move pets, but don't allow aggressive pets into lawful rooms. */
    for (fch = in_room->people; fch != NULL; fch = fch_next) {
        fch_next = fch->next_in_room;

        if (fch->master == ch && fch->position == POS_STANDING
            && char_can_see_room (fch, to_room))
        {

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
            char_move (fch, door, TRUE);
        }
    }

    /* If someone is following the char, these triggers get activated
     * for the followers before the char, but it's safer this way... */
    if (IS_NPC (ch) && HAS_TRIGGER (ch, TRIG_ENTRY))
        mp_percent_trigger (ch, NULL, NULL, NULL, TRIG_ENTRY);
    if (!IS_NPC (ch))
        mp_greet_trigger (ch);
}

char *char_format_to_char (CHAR_T *victim, CHAR_T *ch) {
    static char buf[MAX_STRING_LENGTH];
    char message[MAX_STRING_LENGTH];
    buf[0] = '\0';

#ifdef BASEMUD_COLOR_STATUS_EFFECTS
    if (IS_SET (victim->comm, COMM_AFK))
        strcat (buf, "[{RAFK{x] ");
    if (IS_AFFECTED (victim, AFF_INVISIBLE))
        strcat (buf, "({DInvis{x) ");
    if (victim->invis_level >= LEVEL_HERO)
        strcat (buf, "({CWizi{x) ");
    if (IS_AFFECTED (victim, AFF_HIDE))
        strcat (buf, "({gHide{x) ");
    if (IS_AFFECTED (victim, AFF_CHARM))
        strcat (buf, "({yCharmed{x) ");
    if (IS_AFFECTED (victim, AFF_PASS_DOOR))
        strcat (buf, "({cTranslucent{x) ");
    if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
        strcat (buf, "({MPink Aura{x) ");
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL)) {
#ifdef BASEMUD_DETECT_EXTREME_ALIGNMENTS
        if (IS_REALLY_EVIL (victim))
            strcat (buf, "({DBlack Aura{x) ");
        else
#endif
        if (IS_EVIL (victim))
            strcat (buf, "({rRed Aura{x) ");
    }
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD)) {
#ifdef BASEMUD_DETECT_EXTREME_ALIGNMENTS
        if (IS_REALLY_GOOD (victim))
            strcat (buf, "({YGolden Aura{x) ");
        else
#endif
        if (IS_GOOD (victim))
            strcat (buf, "({wSilver Aura{x) ");
    }
    if (IS_AFFECTED (victim, AFF_SANCTUARY))
        strcat (buf, "({WWhite Aura{x) ");
    if (!IS_NPC (victim) && IS_SET (victim->plr, PLR_KILLER))
        strcat (buf, "({RKILLER{k) ");
    if (!IS_NPC (victim) && IS_SET (victim->plr, PLR_THIEF))
        strcat (buf, "({RTHIEF{k) ");
#else
    if (IS_SET (victim->comm, COMM_AFK))
        strcat (buf, "[AFK] ");
    if (IS_AFFECTED (victim, AFF_INVISIBLE))
        strcat (buf, "(Invis) ");
    if (victim->invis_level >= LEVEL_HERO)
        strcat (buf, "(Wizi) ");
    if (IS_AFFECTED (victim, AFF_HIDE))
        strcat (buf, "(Hide) ");
    if (IS_AFFECTED (victim, AFF_CHARM))
        strcat (buf, "(Charmed) ");
    if (IS_AFFECTED (victim, AFF_PASS_DOOR))
        strcat (buf, "(Translucent) ");
    if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
        strcat (buf, "(Pink Aura) ");
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL)) {
#ifdef BASEMUD_DETECT_EXTREME_ALIGNMENTS
        if (IS_REALLY_EVIL (victim))
            strcat (buf, "(Black Aura) ");
        else
#endif
        if (IS_EVIL (victim))
            strcat (buf, "(Red Aura) ");
    }
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD)) {
#ifdef BASEMUD_DETECT_EXTREME_ALIGNMENTS
        if (IS_REALLY_GOOD (victim))
            strcat (buf, "(Golden Aura) ");
        else
#endif
        if (IS_GOOD (victim))
            strcat (buf, "(Silver Aura) ");
    }
    if (IS_AFFECTED (victim, AFF_SANCTUARY))
        strcat (buf, "(White Aura) ");
    if (!IS_NPC (victim) && IS_SET (victim->plr, PLR_KILLER))
        strcat (buf, "(KILLER) ");
    if (!IS_NPC (victim) && IS_SET (victim->plr, PLR_THIEF))
        strcat (buf, "(THIEF) ");
#endif

    if (IS_SET (ch->comm, COMM_MATERIALS))
        material_strcat (buf, material_get (victim->material));

    if (victim->position == victim->start_pos
        && victim->long_descr[0] != '\0')
    {
        strcat (buf, victim->long_descr);
        return buf;
    }

    strcat (buf, PERS_AW (victim, ch));
    if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
        && victim->position == POS_STANDING && ch->on == NULL)
        strcat (buf, victim->pcdata->title);

    switch (victim->position) {
        case POS_DEAD:    strcat (buf, " is lying here, DEAD!!"); break;
        case POS_MORTAL:  strcat (buf, " is lying here, mortally wounded."); break;
        case POS_INCAP:   strcat (buf, " is lying here, incapacitated."); break;
        case POS_STUNNED: strcat (buf, " is lying here, stunned."); break;

        case POS_SLEEPING:
            if (victim->on != NULL) {
                sprintf (message, " is sleeping %s %s.",
                    obj_furn_preposition (victim->on, victim->position),
                    victim->on->short_descr);
                strcat (buf, message);
            }
            else
                strcat (buf, " is sleeping here.");
            break;

        case POS_RESTING:
            if (victim->on != NULL) {
                sprintf (message, " is resting %s %s.",
                    obj_furn_preposition (victim->on, victim->position),
                    victim->on->short_descr);
                strcat (buf, message);
            }
            else
                strcat (buf, " is resting here.");
            break;

        case POS_SITTING:
            if (victim->on != NULL) {
                sprintf (message, " is sitting %s %s.",
                    obj_furn_preposition (victim->on, victim->position),
                    victim->on->short_descr);
                strcat (buf, message);
            }
            else
                strcat (buf, " is sitting here.");
            break;

        case POS_STANDING:
            if (victim->on != NULL) {
                sprintf (message, " is standing %s %s.",
                    obj_furn_preposition (victim->on, victim->position),
                    victim->on->short_descr);
                strcat (buf, message);
            }
            else
                strcat (buf, " is here.");
            break;

        case POS_FIGHTING:
            strcat (buf, " is here, fighting ");
            if (victim->fighting == NULL)
                strcat (buf, "thin air??");
            else if (victim->fighting == ch)
                strcat (buf, "YOU!");
            else if (victim->in_room == victim->fighting->in_room) {
                strcat (buf, PERS_AW (victim->fighting, ch));
                strcat (buf, ".");
            }
            else
                strcat (buf, "someone who left??");
            break;
    }

    strcat (buf, "\n\r");
    buf[0] = UPPER (buf[0]);
    return buf;
}

void char_look_at_char (CHAR_T *victim, CHAR_T *ch) {
    char buf[MAX_STRING_LENGTH], *msg;
    OBJ_T *obj;
    flag_t wear_loc;
    int percent;
    bool found;

    if (ch == victim)
        act ("$n looks at $mself.", ch, NULL, NULL, TO_NOTCHAR);
    else {
        if (char_can_see_in_room (victim, ch))
            act ("$n looks at you.", ch, NULL, victim, TO_VICT);
        act ("$n looks at $N.", ch, NULL, victim, TO_OTHERS);
    }

    if (victim->description[0] != '\0')
        send_to_char (victim->description, ch);
    else
        act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);

    if (victim->max_hit > 0)
        percent = (100 * victim->hit) / victim->max_hit;
    else
        percent = -1;

    msg = NULL;
    switch (victim->position) {
        case POS_DEAD:     msg = "$E is... dead?!"; break;
        case POS_MORTAL:   msg = "$E is on the ground, mortally wounded."; break;
        case POS_INCAP:    msg = "$E is on the ground, incapacitated."; break;
        case POS_STUNNED:  msg = "$E is on the ground, stunned."; break;
        case POS_SLEEPING: msg = "$E is on the ground, sleeping."; break;
        case POS_RESTING:  msg = "$E is sitting and resting."; break;
        case POS_SITTING:  msg = "$E is sitting."; break;
        case POS_STANDING: msg = "$E is standing."; break;
    }
    if (msg != NULL)
        act (msg, ch, NULL, victim, TO_CHAR);

    sprintf (buf, "%s %s.\n\r", PERS_AW (victim, ch),
        condition_name_by_percent (percent));
    buf[0] = UPPER (buf[0]);
    send_to_char (buf, ch);

    found = FALSE;
    for (wear_loc = 0; wear_loc < WEAR_LOC_MAX; wear_loc++) {
        if ((obj = char_get_eq_by_wear_loc (victim, wear_loc)) != NULL
            && char_can_see_obj (ch, obj))
        {
            if (!found) {
                send_to_char ("\n\r", ch);
                act ("$N is using:", ch, NULL, victim, TO_CHAR);
                found = TRUE;
            }
            send_to_char (wear_loc_table[wear_loc].look_msg, ch);
            send_to_char (obj_format_to_char (obj, ch, TRUE), ch);
            send_to_char ("\n\r", ch);
        }
    }

    if (victim != ch && !IS_NPC (ch)
        && number_percent () < get_skill (ch, gsn_peek))
    {
        send_to_char ("\n\rYou peek at the inventory:\n\r", ch);
        check_improve (ch, gsn_peek, TRUE, 4);
        obj_list_show_to_char (victim->carrying, ch, TRUE, TRUE);
    }
}

void char_list_show_to_char (CHAR_T *list, CHAR_T *ch) {
    CHAR_T *rch;

    for (rch = list; rch != NULL; rch = rch->next_in_room) {
        if (rch == ch)
            continue;
        if (char_get_trust (ch) < rch->invis_level)
            continue;

        if (char_can_see_anywhere (ch, rch))
            send_to_char (char_format_to_char (rch, ch), ch);
        else if (room_is_dark (ch->in_room)
                 && IS_AFFECTED (rch, AFF_INFRARED))
            send_to_char ("You see glowing red eyes watching YOU!\n\r", ch);
    }
}

#if 0
/* friend stuff -- for NPC's mostly */
bool char_is_friend (CHAR_T *ch, CHAR_T *victim) {
    if (is_same_group (ch, victim))
        return TRUE;
    if (!IS_NPC (ch))
        return FALSE;
    if (!IS_NPC (victim)) {
        if (IS_SET (ch->off_flags, ASSIST_PLAYERS))
            return TRUE;
        else
            return FALSE;
    }
    if (IS_AFFECTED (ch, AFF_CHARM))
        return FALSE;
    if (IS_SET (ch->off_flags, ASSIST_ALL))
        return TRUE;
    if (ch->group && ch->group == victim->group)
        return TRUE;
    if (IS_SET (ch->off_flags, ASSIST_VNUM)
        && ch->index_data == victim->index_data)
        return TRUE;
    if (IS_SET (ch->off_flags, ASSIST_RACE) && ch->race == victim->race)
        return TRUE;
    if (IS_SET (ch->off_flags, ASSIST_ALIGN)
        && IS_SAME_ALIGN (ch, victim))
        return TRUE;

    return FALSE;
}
#endif

/* RT part of the corpse looting code */
bool char_can_loot (CHAR_T *ch, OBJ_T *obj) {
    CHAR_T *owner, *wch;
    if (IS_IMMORTAL (ch))
        return TRUE;
    if (!obj->owner || obj->owner == NULL)
        return TRUE;

    owner = NULL;
    for (wch = char_list; wch != NULL; wch = wch->next)
        if (!str_cmp (wch->name, obj->owner))
            owner = wch;

    if (owner == NULL)
        return TRUE;
    if (!str_cmp (ch->name, owner->name))
        return TRUE;
    if (!IS_NPC (owner) && IS_SET (owner->plr, PLR_CANLOOT))
        return TRUE;
    if (is_same_group (ch, owner))
        return TRUE;
    return FALSE;
}

void char_take_obj (CHAR_T *ch, OBJ_T *obj, OBJ_T *container) {
    CHAR_T *gch;

    BAIL_IF_ACT (!CAN_WEAR_FLAG (obj, ITEM_TAKE),
        "$p: You can't take that.", ch, obj, NULL);
    BAIL_IF_ACT (ch->carry_number + obj_get_carry_number (obj) >
            char_get_max_carry_count (ch),
        "$p: you can't carry that many items.", ch, obj, NULL);
    BAIL_IF_ACT ((!obj->in_obj || obj->in_obj->carried_by != ch) &&
            (char_get_carry_weight (ch) + obj_get_weight (obj) >
             char_get_max_carry_weight (ch)),
        "$p: you can't carry that much weight.", ch, obj, NULL);
    BAIL_IF (!char_can_loot (ch, obj),
        "Corpse looting is not permitted.", ch);

    if (obj->in_room != NULL)
        for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
            BAIL_IF_ACT (gch->on == obj,
                "$N appears to be using $p.", ch, obj, gch);

    if (container != NULL) {
        BAIL_IF (container->index_data->vnum == OBJ_VNUM_PIT &&
                char_get_trust (ch) < obj->level,
            "You are not powerful enough to use it.\n\r", ch);

        if (container->index_data->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR_FLAG (container, ITEM_TAKE)
            && !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
            obj->timer = 0;
        act ("You get $p from $P.", ch, obj, container, TO_CHAR);
        act ("$n gets $p from $P.", ch, obj, container, TO_NOTCHAR);
        REMOVE_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        obj_take_from_obj (obj);
    }
    else {
        act ("You get $p.", ch, obj, container, TO_CHAR);
        act ("$n gets $p.", ch, obj, container, TO_NOTCHAR);
        obj_take_from_room (obj);
    }

    if (obj->item_type == ITEM_MONEY) {
        ch->silver += obj->v.money.silver;
        ch->gold   += obj->v.money.gold;

        if (IS_SET (ch->plr, PLR_AUTOSPLIT)) {
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
    }
    else
        obj_give_to_char (obj, ch);
}

/* Remove an object. */
bool char_remove_obj (CHAR_T *ch, flag_t wear_loc, bool replace, bool quiet) {
    OBJ_T *obj;

    if ((obj = char_get_eq_by_wear_loc (ch, wear_loc)) == NULL)
        return TRUE;
    if (!replace)
        return FALSE;
    if (IS_SET (obj->extra_flags, ITEM_NOREMOVE)) {
        if (!quiet)
            act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        return FALSE;
    }
    if (!char_unequip_obj (ch, obj))
        return FALSE;

    if (!quiet) {
        act2 ("You stop using $p.",
              "$n stops using $p.", ch, obj, NULL, 0, POS_RESTING);
    }
    return TRUE;
}

bool char_has_available_wear_loc (CHAR_T *ch, flag_t wear_loc) {
    if (ch == NULL)
        return FALSE;
    return (char_get_eq_by_wear_loc (ch, wear_loc) == NULL);
}

bool char_has_available_wear_flag (CHAR_T *ch, flag_t wear_flag) {
    int i;
    if (ch == NULL)
        return FALSE;
    for (i = 0; i < WEAR_LOC_MAX; i++) {
        if (wear_loc_table[i].wear_flag == wear_flag)
            if (char_has_available_wear_loc (ch, i))
                return TRUE;
    }
    return FALSE;
}

/* Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick. */
bool char_wear_obj (CHAR_T *ch, OBJ_T *obj, bool replace) {
    const WEAR_LOC_T *wear_loc, *wear_locs[WEAR_LOC_MAX];
    int i, loc, locs;

    if (ch->level < obj->level) {
        if (replace) {
            printf_to_char (ch,
                "You must be level %d to use this object.\n\r", obj->level);
            act ("$n tries to use $p, but is too inexperienced.",
                 ch, obj, NULL, TO_NOTCHAR);
        }
        return FALSE;
    }

    /* Find where this item can be worn / wielded / held / etc. */
    locs = 0;
    for (i = 0; wear_loc_table[i].name != NULL; i++) {
        if (!CAN_WEAR_FLAG (obj, wear_loc_table[i].wear_flag))
            continue;
        wear_locs[locs++] = &(wear_loc_table[i]);
    }
    if (locs == 0) {
        if (replace)
            send_to_char ("You can't wear, wield, or hold that.\n\r", ch);
        return FALSE;
    }

    /* First, see if there's a free location. */
    for (i = 0; i < locs; i++)
        if (char_has_available_wear_loc (ch, wear_locs[i]->type))
            break;
    loc = i;

    /* If we didn't find a free location, try removing items. */
    if (loc == locs)
        for (i = 0; i < locs; i++)
            if (char_remove_obj (ch, wear_locs[i]->type, replace, FALSE))
                break;
    loc = i;

    /* If we STILL didn't find anything, give up. */
    if (loc == locs)
        return FALSE;

    wear_loc = wear_locs[loc];

    /* Shields cannot be worn while wielding a two-handed weapon. */
    if (wear_loc->wear_flag == ITEM_WEAR_SHIELD) {
        OBJ_T *weapon = char_get_eq_by_wear_loc (ch, WEAR_WIELD);
        RETURN_IF (weapon != NULL && ch->size < SIZE_LARGE &&
                IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS),
            "Your hands are tied up with your weapon!\n\r", ch, FALSE);
    }

    /* Likewise, two-handed weapons can't be wielded without a shield. */
    if (wear_loc->wear_flag == ITEM_WIELD) {
        RETURN_IF (!IS_NPC (ch) && obj_get_weight (obj) >
                char_str_max_wield_weight (ch) * 10,
            "It is too heavy for you to wield.\n\r", ch, FALSE);
        RETURN_IF (!IS_NPC (ch) && ch->size < SIZE_LARGE &&
                IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS) &&
                char_get_eq_by_wear_loc (ch, WEAR_SHIELD) != NULL,
            "You need two hands free for that weapon.\n\r", ch, FALSE);
    }

    /* Let everyone know we're wearing this. */
    act2 (wear_loc->msg_wear_self, wear_loc->msg_wear_room,
        ch, obj, NULL, 0, POS_RESTING);
    if (!char_equip_obj (ch, obj, wear_loc->type))
        return FALSE;

    /* If wielding something, let us know how good we are at using it. */
    if (wear_loc->wear_flag == ITEM_WIELD) {
        int sn = char_get_weapon_sn (ch);
        if (sn != gsn_hand_to_hand) {
            char *msg;
            int skill = char_get_weapon_skill (ch, sn);
                 if (skill >= 100) msg = "$p feels like a part of you!";
            else if (skill >   85) msg = "You feel quite confident with $p.";
            else if (skill >   70) msg = "You are skilled with $p.";
            else if (skill >   50) msg = "Your skill with $p is adequate.";
            else if (skill >   25) msg = "$p feels a little clumsy in your hands.";
            else if (skill >    1) msg = "You fumble and almost drop $p.";
            else                   msg = "You don't even know which end is up on $p.";
            act (msg, ch, obj, NULL, TO_CHAR);
        }
    }
    return TRUE;
}

void char_get_who_string (CHAR_T *ch, CHAR_T *wch, char *buf,
    size_t len)
{
    const char *class;

    /* Figure out what to print for class. */
    class = wiz_class_by_level (wch->level);
    if (class == NULL)
        class = class_table[wch->class].who_name;

    /* Format it up. */
    snprintf (buf, len, "[%2d %6s %s] %s%s%s%s%s%s%s%s\n\r",
        wch->level,
        (wch->race < PC_RACE_MAX) ? pc_race_table[wch->race].who_name : "     ",
        class,
        (wch->incog_level >= LEVEL_HERO) ? "(Incog) " : "",
        (wch->invis_level >= LEVEL_HERO) ? "(Wizi) " : "",
        clan_table[wch->clan].who_name,
        IS_SET (wch->comm, COMM_AFK) ? "[AFK] " : "",
        IS_SET (wch->plr, PLR_KILLER) ? "(KILLER) " : "",
        IS_SET (wch->plr, PLR_THIEF) ? "(THIEF) " : "",
        wch->name,
        IS_NPC (wch) ? "" : wch->pcdata->title);
}

void char_set_title (CHAR_T *ch, char *title) {
    char buf[MAX_STRING_LENGTH];

    BAIL_IF_BUG (IS_NPC (ch),
        "char_set_title: NPC.", 0);

    if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?') {
        buf[0] = ' ';
        strcpy (buf + 1, title);
    }
    else
        strcpy (buf, title);

    str_replace_dup (&(ch->pcdata->title), buf);
}

bool char_has_key (CHAR_T *ch, int key) {
    OBJ_T *obj;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        if (obj->index_data->vnum == key)
            return TRUE;
    return FALSE;
}

bool char_drop_weapon_if_too_heavy (CHAR_T *ch) {
    OBJ_T *wield;
    static int depth;

    /* Check for weapon wielding.
     * Guard against recursion (for weapons with affects). */
    if (IS_NPC (ch))
        return FALSE;
    if ((wield = char_get_eq_by_wear_loc (ch, WEAR_WIELD)) == NULL)
        return FALSE;
    if (obj_get_weight (wield) <= char_str_max_wield_weight (ch) * 10)
        return FALSE;
    if (depth != 0)
        return FALSE;

    depth++;
    act2 ("You drop $p.", "$n drops $p.", ch, wield, NULL, 0, POS_RESTING);
    obj_take_from_char (wield);
    obj_give_to_room (wield, ch->in_room);
    depth--;
    return TRUE;
}

void char_reduce_money (CHAR_T *ch, int cost) {
    int silver = 0, gold = 0;

    silver = UMIN (ch->silver, cost);
    if (silver < cost) {
        gold = ((cost - silver + 99) / 100);
        silver = cost - 100 * gold;
    }

    ch->gold   -= gold;
    ch->silver -= silver;

    if (ch->gold < 0) {
        bug ("deduct costs: gold %d < 0", ch->gold);
        ch->gold = 0;
    }
    if (ch->silver < 0) {
        bug ("deduct costs: silver %d < 0", ch->silver);
        ch->silver = 0;
    }
}

int char_get_obj_cost (CHAR_T *ch, OBJ_T *obj, bool buy) {
    SHOP_T *shop;
    int cost;

    if (obj == NULL || (shop = ch->index_data->shop) == NULL)
        return 0;

    if (buy)
        cost = obj->cost * shop->profit_buy / 100;
    else {
        OBJ_T *obj2;
        int itype;

        cost = 0;
        for (itype = 0; itype < MAX_TRADE; itype++) {
            if (obj->item_type == shop->buy_type[itype]) {
                cost = obj->cost * shop->profit_sell / 100;
                break;
            }
        }

        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)) {
            for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content) {
                if (obj->index_data == obj2->index_data
                    && !str_cmp (obj->short_descr, obj2->short_descr))
                {
                    if (IS_OBJ_STAT (obj2, ITEM_INVENTORY))
                        cost /= 2;
                    else
                        cost = cost * 3 / 4;
                }
            }
        }
    }

    switch (obj->item_type) {
        case ITEM_STAFF:
            if (obj->v.staff.recharge == 0)
                cost /= 4;
            else
                cost = cost * obj->v.staff.charges / obj->v.staff.recharge;
            break;

        case ITEM_WAND:
            if (obj->v.wand.recharge == 0)
                cost /= 4;
            else
                cost = cost * obj->v.wand.charges / obj->v.wand.recharge;
            break;
    }

    return cost;
}

SHOP_T *char_get_shop (CHAR_T *ch) {
    if (ch->index_data == NULL)
        return NULL;
    return ch->index_data->shop;
}

CHAR_T *char_get_keeper_room (CHAR_T *ch) {
    CHAR_T *k;
    for (k = ch->in_room->people; k; k = k->next_in_room)
        if (IS_NPC (k) && char_get_shop (k))
            return k;
    return NULL;
}

CHAR_T *char_get_trainer_room (CHAR_T *ch) {
    CHAR_T *t;
    for (t = ch->in_room->people; t; t = t->next_in_room)
        if (IS_NPC (t) && IS_SET (t->mob, MOB_TRAIN))
            return t;
    return NULL;
}

CHAR_T *char_get_practicer_room (CHAR_T *ch) {
    CHAR_T *p;
    for (p = ch->in_room->people; p; p = p->next_in_room)
        if (IS_NPC (p) && IS_SET (p->mob, MOB_PRACTICE))
            return p;
    return NULL;
}

CHAR_T *char_get_gainer_room (CHAR_T *ch) {
    CHAR_T *g;
    for (g = ch->in_room->people; g; g = g->next_in_room)
        if (IS_NPC (g) && IS_SET (g->mob, MOB_GAIN))
            return g;
    return NULL;
}

int char_exit_string (CHAR_T *ch, ROOM_INDEX_T *room, int mode,
    char *out_buf, size_t out_size)
{
    EXIT_T *pexit;
    int door, isdoor, closed, count;
    size_t len;

    out_buf[0] = '\0';
    len = 0;
    count = 0;

    for (door = 0; door <= 5; door++) {
        pexit = room->exit[door];
        if (pexit == NULL)
            continue;
        if (pexit->to_room == NULL)
            continue;
        if (!char_can_see_room (ch, pexit->to_room))
            continue;

        isdoor = IS_SET (pexit->exit_flags, EX_ISDOOR);
        closed = IS_SET (pexit->exit_flags, EX_CLOSED);

#ifndef BASEMUD_SHOW_DOORS
        if (closed)
            continue;
#endif

        if (mode == EXITS_PROMPT) {
            len += snprintf (out_buf + len, out_size - len, "%s%s",
#ifdef BASEMUD_SHOW_DOORS
                closed ? "#" : isdoor ? "-" : ""
#else
                ""
#endif
                , door_get(door)->short_name
            );
        }
        if (mode == EXITS_AUTO) {
            if (out_buf[0] != '\0')
                len += snprintf (out_buf + len, out_size - len, " ");
#ifdef BASEMUD_SHOW_DOORS
            len += snprintf (out_buf + len, out_size - len,
                closed ? "#" : isdoor ? "-" : "");
#endif

            len += snprintf (out_buf + len, out_size - len, "%s",
                door_get_name(door));
        }
        else if (mode == EXITS_LONG) {
            /* exit information */
            len += snprintf (out_buf + len, out_size - len, "%-5s ",
                capitalize (door_get_name(door)));

            if (isdoor) {
                char dbuf[64];
                char *dname = door_keyword_to_name (pexit->keyword,
                    dbuf, sizeof (dbuf));
                len += snprintf (out_buf + len, out_size - len, "(%s %s) ",
                    closed ? "closed" : "open", dname);
            }

            /* room information (only if open) */
            if (!closed || IS_IMMORTAL (ch)) {
                ROOM_INDEX_T *room = pexit->to_room;
                if (room_is_dark (room)) {
                    len += snprintf (out_buf + len, out_size - len,
                        "- {DToo dark to tell{x");
                }
                else {
                    len += snprintf (out_buf + len, out_size - len, "- {%c%s{x",
                        room_colour_char(room), room->name);
                }
            }

            /* wiz info */
            if (IS_IMMORTAL (ch))
                len += snprintf (out_buf + len, out_size - len,
                         " (room %d)\n\r", pexit->to_room->vnum);
            else
                len += snprintf (out_buf + len, out_size - len, "\n\r");
        }
        count++;
    }

    if (count == 0) {
        len += snprintf (out_buf + len, out_size - len,
            (mode == EXITS_LONG) ? "None.\n\r" : "none");
    }

    return count;
}

void char_stop_idling (CHAR_T *ch) {
    if (ch == NULL
        || ch->desc == NULL
        || ch->desc->connected != CON_PLAYING
        || ch->was_in_room == NULL
        || ch->in_room != get_room_index (ROOM_VNUM_LIMBO)
    )
        return;

    ch->timer = 0;
    char_from_room (ch);
    char_to_room (ch, ch->was_in_room);
    ch->was_in_room = NULL;
    act ("$n has returned from the void.", ch, NULL, NULL, TO_NOTCHAR);
}

const char *char_get_class_name (CHAR_T *ch)
    { return (IS_NPC (ch)) ? "mobile" : class_table[ch->class].name; }

const char *char_get_position_str (CHAR_T *ch, int position,
    OBJ_T *on, int with_punct)
{
    static char buf[MAX_STRING_LENGTH];
    const char *name = position_name (position);

    if (on == NULL)
        snprintf (buf, sizeof(buf), "%s", name);
    else {
        snprintf (buf, sizeof(buf), "%s %s %s",
            name, obj_furn_preposition (on, position),
            char_can_see_obj (ch, on) ? on->short_descr : "something");
    }

    if (with_punct)
        strcat (buf, (position == POS_DEAD) ? "!!" : ".");
    return buf;
}

/* Former character macros. */
bool char_is_trusted (CHAR_T *ch, int level)
    { return char_get_trust (ch) >= level ? TRUE : FALSE; }

bool char_is_npc (CHAR_T *ch)
    { return IS_SET((ch)->mob, MOB_IS_NPC) ? TRUE : FALSE; }
bool char_is_immortal (CHAR_T *ch)
    { return (char_is_trusted (ch, LEVEL_IMMORTAL)) ? TRUE : FALSE; }
bool char_is_hero (CHAR_T *ch)
    { return (char_is_trusted (ch, LEVEL_HERO)) ? TRUE : FALSE; }
bool char_is_affected (CHAR_T *ch, flag_t sn)
    { return IS_SET((ch)->affected_by, sn) ? TRUE : FALSE; }

bool char_is_sober (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->condition[COND_DRUNK] <= 0) ? TRUE : FALSE;
}

bool char_is_drunk (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->condition[COND_DRUNK] > 10) ? TRUE : FALSE;
}

bool char_is_thirsty (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->condition[COND_THIRST] <= 0) ? TRUE : FALSE;
}

bool char_is_quenched (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->condition[COND_THIRST] > 40) ? TRUE : FALSE;
}

bool char_is_hungry (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->condition[COND_HUNGER] <= 0) ? TRUE : FALSE;
}

bool char_is_fed (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->condition[COND_HUNGER] > 40) ? TRUE : FALSE;
}

bool char_is_full (CHAR_T *ch) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->condition[COND_FULL] > 40) ? TRUE : FALSE;
}

bool char_is_pet (CHAR_T *ch)
    { return (char_is_npc (ch) && IS_SET ((ch)->mob, MOB_PET)) ? TRUE : FALSE; }

bool char_is_good (CHAR_T *ch)
    { return (ch->alignment >=  350) ? TRUE : FALSE; }
bool char_is_evil (CHAR_T *ch)
    { return (ch->alignment <= -350) ? TRUE : FALSE; }
bool char_is_really_good (CHAR_T *ch)
    { return (ch->alignment >=  750) ? TRUE : FALSE; }
bool char_is_really_evil (CHAR_T *ch)
    { return (ch->alignment <= -750) ? TRUE : FALSE; }
bool char_is_neutral (CHAR_T *ch)
    { return (!IS_GOOD(ch) && !IS_EVIL(ch)) ? TRUE : FALSE; }

bool char_is_outside (CHAR_T *ch) {
    if (ch->in_room == NULL)
        return FALSE;
    return (!IS_SET ((ch)->in_room->room_flags, ROOM_INDOORS))
        ? TRUE : FALSE;
}
bool char_is_awake (CHAR_T *ch)
    { return (ch->position > POS_SLEEPING) ? TRUE : FALSE; }

int char_get_age (CHAR_T *ch) {
    int age = 17 + (ch->played + (int) (current_time - ch->logon)) / 72000;
    return age;
}

bool char_is_same_align (CHAR_T *ch1, CHAR_T *ch2) {
    if (IS_SET (ch1->mob, MOB_NOALIGN || IS_SET (ch2->mob, MOB_NOALIGN)))
        return FALSE;
    if (char_is_good (ch1) && char_is_good (ch2))
        return TRUE;
    if (char_is_evil (ch1) && char_is_evil (ch2))
        return TRUE;
    if (char_is_neutral (ch1) && char_is_neutral (ch2))
        return TRUE;
    return FALSE;
}

int char_get_ac (CHAR_T *ch, int type) {
    return ch->armor[type] + (char_is_awake (ch)
        ? char_dex_defense_bonus (ch) : 0);
}

int char_get_hitroll (CHAR_T *ch)
    { return ch->hitroll + char_str_hitroll_bonus (ch); }
int char_get_damroll (CHAR_T *ch)
    { return ch->damroll + char_str_damroll_bonus (ch); }

bool char_is_switched (CHAR_T *ch)
    { return (ch->desc && ch->desc->original) ? TRUE : FALSE; }

bool char_is_builder (CHAR_T *ch, AREA_T *area) {
    if (char_is_npc (ch) || char_is_switched (ch))
        return FALSE;
    if (ch->pcdata->security >= area->security)
        return TRUE;
    if (strstr (area->builders, ch->name))
        return TRUE;
    if (strstr (area->builders, "All"))
        return TRUE;
    return FALSE;
}

int char_get_vnum (CHAR_T *ch)
    { return IS_NPC(ch) ? (ch)->index_data->vnum : 0; }

int char_set_max_wait_state (CHAR_T *ch, int npulse)
{
    if (!char_is_trusted (ch, IMPLEMENTOR) && npulse > ch->wait)
        ch->wait = npulse;
    return ch->wait;
}

int char_set_max_daze_state (CHAR_T *ch, int npulse)
{
    if (!char_is_trusted (ch, IMPLEMENTOR) && npulse > ch->daze)
        ch->daze = npulse;
    return ch->daze;
}

char *char_get_short_descr (CHAR_T *ch)
    { return char_is_npc (ch) ? ch->short_descr : ch->name; }

char *char_get_look_short_descr_anywhere (CHAR_T *looker, CHAR_T *ch) {
    if (!char_can_see_anywhere (looker, ch))
        return "someone";
    return char_get_short_descr (ch);
}

char *char_get_look_short_descr (CHAR_T *looker, CHAR_T *ch) {
    if (!char_can_see_in_room (looker, ch))
        return "someone";
    return char_get_short_descr (ch);
}

/* Stat bonuses. */
const STR_APP_T *char_get_curr_str_app (CHAR_T *ch)
    { return str_app_get (char_get_curr_stat (ch, STAT_STR)); }
const INT_APP_T *char_get_curr_int_app (CHAR_T *ch)
    { return int_app_get (char_get_curr_stat (ch, STAT_INT)); }
const WIS_APP_T *char_get_curr_wis_app (CHAR_T *ch)
    { return wis_app_get (char_get_curr_stat (ch, STAT_WIS)); }
const DEX_APP_T *char_get_curr_dex_app (CHAR_T *ch)
    { return dex_app_get (char_get_curr_stat (ch, STAT_DEX)); }
const CON_APP_T *char_get_curr_con_app (CHAR_T *ch)
    { return con_app_get (char_get_curr_stat (ch, STAT_CON)); }

int char_str_carry_bonus (CHAR_T *ch)
    { return char_get_curr_str_app (ch)->carry; }
int char_str_hitroll_bonus (CHAR_T *ch)
    { return char_get_curr_str_app (ch)->tohit; }
int char_str_damroll_bonus (CHAR_T *ch)
    { return char_get_curr_str_app (ch)->todam; }
int char_str_max_wield_weight (CHAR_T *ch)
    { return char_get_curr_str_app (ch)->wield; }
int char_int_learn_rate (CHAR_T *ch)
    { return char_get_curr_int_app (ch)->learn; }
int char_wis_level_practices (CHAR_T *ch)
    { return char_get_curr_wis_app (ch)->practice; }
int char_dex_defense_bonus (CHAR_T *ch)
    { return char_get_curr_dex_app (ch)->defensive; }
int char_con_level_hp (CHAR_T *ch)
    { return char_get_curr_con_app (ch)->hitp; }
int char_con_shock (CHAR_T *ch)
    { return char_get_curr_con_app (ch)->shock; }
