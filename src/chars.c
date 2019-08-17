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

#include "chars.h"

/* TODO: lots of code can be put into tables. */
/* TODO: review the function names for consistency. */
/* TODO: remove any redundant functions, like simple lookup functions. */
/* TODO: char_wear_obj() is pretty awful :( */
/* TODO: replace fReplace in char_wear_obj() and char_remove_obj() with
 *       flags for EQUIP_ONLY_IF_EMPTY and EQUIP_QUIET. */

bool char_has_clan (CHAR_DATA * ch) {
    return ch->clan;
}

bool char_in_same_clan (CHAR_DATA * ch, CHAR_DATA * victim) {
    if (clan_table[ch->clan].independent)
        return FALSE;
    else
        return (ch->clan == victim->clan);
}

/* for returning weapon information */
int char_get_weapon_sn (CHAR_DATA * ch) {
    OBJ_DATA *wield;
    int sn;

    wield = char_get_eq_by_wear (ch, WEAR_WIELD);
    if (wield == NULL || wield->item_type != ITEM_WEAPON)
        sn = gsn_hand_to_hand;
    else {
        switch (wield->value[0]) {
            case WEAPON_SWORD:   sn = gsn_sword;   break;
            case WEAPON_DAGGER:  sn = gsn_dagger;  break;
            case WEAPON_SPEAR:   sn = gsn_spear;   break;
            case WEAPON_MACE:    sn = gsn_mace;    break;
            case WEAPON_AXE:     sn = gsn_axe;     break;
            case WEAPON_FLAIL:   sn = gsn_flail;   break;
            case WEAPON_WHIP:    sn = gsn_whip;    break;
            case WEAPON_POLEARM: sn = gsn_polearm; break;
            default:             sn = -1;          break;
        }
    }
    return sn;
}

int char_get_weapon_skill (CHAR_DATA * ch, int sn) {
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
/* TODO: this doesn't check imm/res/vuln flags... */
void char_reset (CHAR_DATA * ch) {
    int loc, mod, stat;
    OBJ_DATA *obj;
    AFFECT_DATA *af;
    int i;

    if (IS_NPC (ch))
        return;

    if (ch->pcdata->perm_hit == 0
        || ch->pcdata->perm_mana == 0
        || ch->pcdata->perm_move == 0 || ch->pcdata->last_level == 0)
    {
        /* do a FULL reset */
        for (loc = 0; loc < WEAR_MAX; loc++) {
            obj = char_get_eq_by_wear (ch, loc);
            if (obj == NULL)
                continue;
            if (!obj->enchanted) {
                for (af = obj->pIndexData->affected; af != NULL;
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
    ch->sex = ch->pcdata->true_sex;
    ch->max_hit = ch->pcdata->perm_hit;
    ch->max_mana = ch->pcdata->perm_mana;
    ch->max_move = ch->pcdata->perm_move;

    for (i = 0; i < 4; i++)
        ch->armor[i] = 100;

    ch->hitroll = 0;
    ch->damroll = 0;
    ch->saving_throw = 0;

    /* now start adding back the effects */
    for (loc = 0; loc < WEAR_MAX; loc++) {
        obj = char_get_eq_by_wear (ch, loc);
        if (obj == NULL)
            continue;
        for (i = 0; i < 4; i++)
            ch->armor[i] -= obj_get_ac_type (obj, loc, i);
        if (obj->enchanted)
            continue;
        for (af = obj->pIndexData->affected; af != NULL; af = af->next)
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
int char_get_trust (CHAR_DATA * ch) {
    if (ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;
    if (ch->trust)
        return ch->trust;
    if (IS_NPC (ch) && ch->level >= LEVEL_HERO)
        return LEVEL_HERO - 1;
    else
        return ch->level;
}

/* Retrieve a character's age. */
int char_get_age (CHAR_DATA * ch) {
    return 17 + (ch->played + (int) (current_time - ch->logon)) / 72000;
}

/* command for retrieving stats */
int char_get_curr_stat (CHAR_DATA * ch, int stat) {
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
int char_get_max_train (CHAR_DATA * ch, int stat) {
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

long int char_get_carry_weight (CHAR_DATA *ch) {
    return ch->carry_weight + (ch->silver / 10) + (ch->gold * 2 / 5);
}

/* Retrieve a character's carry capacity. */
int char_get_max_carry_count (CHAR_DATA * ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 1000;
    if (IS_PET (ch))
        return 0;
    return WEAR_MAX + 2 * char_get_curr_stat (ch, STAT_DEX) + ch->level;
}

/* Retrieve a character's carry capacity. */
long int char_get_max_carry_weight (CHAR_DATA * ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 10000000;
    if (IS_PET (ch))
        return 0;
    return str_app[char_get_curr_stat (ch, STAT_STR)].carry * 10 +
        ch->level * 25;
}

/* Move a char out of a room. */
void char_from_room (CHAR_DATA * ch) {
    OBJ_DATA *obj;

    if (ch->in_room == NULL) {
        bug ("char_from_room: NULL.", 0);
        return;
    }

    if (!IS_NPC (ch))
        --ch->in_room->area->nplayer;

    if ((obj = char_get_eq_by_wear (ch, WEAR_LIGHT)) != NULL
        && obj->item_type == ITEM_LIGHT
        && obj->value[2] != 0 && ch->in_room->light > 0)
        --ch->in_room->light;

    LIST_REMOVE (ch, next_in_room, ch->in_room->people, CHAR_DATA, NO_FAIL);
    ch->in_room = NULL;
    ch->on = NULL; /* sanity check! */
}

void char_to_room_apply_plague (CHAR_DATA * ch) {
    AFFECT_DATA *af, plague;
    CHAR_DATA *vch;

    for (af = ch->affected; af != NULL; af = af->next)
        if (af->type == gsn_plague)
            break;
    if (af == NULL) {
        REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
        return;
    }
    if (af->level == 1)
        return;

    affect_init (&plague, TO_AFFECTS, gsn_plague, af->level - 1, number_range (1, 2 * (af->level - 1)), APPLY_STR, -5, AFF_PLAGUE);

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
void char_to_room (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex) {
    OBJ_DATA *obj;

    if (pRoomIndex == NULL) {
        ROOM_INDEX_DATA *room;
        bug ("char_to_room: NULL.", 0);
        if ((room = get_room_index (ROOM_VNUM_TEMPLE)) != NULL)
            char_to_room (ch, room);
        return;
    }

    ch->in_room = pRoomIndex;
    LIST_FRONT (ch, next_in_room, pRoomIndex->people);

    if (!IS_NPC (ch)) {
        if (ch->in_room->area->empty) {
            ch->in_room->area->empty = FALSE;
            ch->in_room->area->age = 0;
        }
        ++ch->in_room->area->nplayer;
    }

    if ((obj = char_get_eq_by_wear (ch, WEAR_LIGHT)) != NULL
            && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
        ++ch->in_room->light;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        char_to_room_apply_plague (ch);
}

/* Find a piece of eq on a character. */
OBJ_DATA *char_get_eq_by_wear (CHAR_DATA * ch, int iWear) {
    OBJ_DATA *obj;
    if (ch == NULL)
        return NULL;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        if (obj->wear_loc == iWear)
            return obj;
    return NULL;
}

/* Equip a char with an obj. */
void char_equip (CHAR_DATA * ch, OBJ_DATA * obj, int iWear) {
    AFFECT_DATA *paf;
    int i;

    if (char_get_eq_by_wear (ch, iWear) != NULL) {
        bug ("char_equip: already equipped (%d).", iWear);
        return;
    }

    if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL)    && IS_EVIL (ch)) ||
        (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD)    && IS_GOOD (ch)) ||
        (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
    {
        /*
         * Thanks to Morgenes for the bug fix here!
         */
        act ("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
        act ("$n is zapped by $p and drops it.", ch, obj, NULL, TO_NOTCHAR);
        obj_from_char (obj);
        obj_to_room (obj, ch->in_room);
        return;
    }

    for (i = 0; i < 4; i++)
        ch->armor[i] -= obj_get_ac_type (obj, iWear, i);
    obj->wear_loc = iWear;

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
            if (paf->apply != APPLY_SPELL_AFFECT)
                affect_modify (ch, paf, TRUE);
    for (paf = obj->affected; paf != NULL; paf = paf->next)
        if (paf->apply == APPLY_SPELL_AFFECT)
            affect_to_char (ch, paf);
        else
            affect_modify (ch, paf, TRUE);

    if (obj->item_type == ITEM_LIGHT
        && obj->value[2] != 0 && ch->in_room != NULL) ++ch->in_room->light;
}

/* Unequip a char with an obj. */
void char_unequip (CHAR_DATA * ch, OBJ_DATA * obj) {
    AFFECT_DATA *paf = NULL;
    AFFECT_DATA *lpaf = NULL;
    AFFECT_DATA *lpaf_next = NULL;
    int i;

    if (obj->wear_loc == WEAR_NONE) {
        bug ("char_unequip: already unequipped.", 0);
        return;
    }

    for (i = 0; i < 4; i++)
        ch->armor[i] += obj_get_ac_type (obj, obj->wear_loc, i);
    obj->wear_loc = -1;

    if (!obj->enchanted) {
        for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
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

    if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0
        && ch->in_room != NULL && ch->in_room->light > 0)
    {
        --ch->in_room->light;
    }
}

/* Extract a char from the world. */
void char_extract (CHAR_DATA * ch, bool fPull) {
    CHAR_DATA *wch;
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    /* doesn't seem to be necessary
    if ( ch->in_room == NULL ) {
            bug( "Extract_char: NULL.", 0 );
            return;
    }
    */

    nuke_pets (ch);
    ch->pet = NULL;                /* just in case */

    if (fPull)
        die_follower (ch);

    stop_fighting (ch, TRUE);
    for (obj = ch->carrying; obj != NULL; obj = obj_next) {
        obj_next = obj->next_content;
        obj_extract (obj);
    }

    if (ch->in_room != NULL)
        char_from_room (ch);

    /* Death room is set in the clan table now */
    if (!fPull) {
        char_to_room (ch, get_room_index (clan_table[ch->clan].hall));
        return;
    }

    if (IS_NPC (ch))
        --ch->pIndexData->count;

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

    LIST_REMOVE (ch, next, char_list, CHAR_DATA, return);
    if (ch->desc != NULL)
        ch->desc->character = NULL;

    char_free (ch);
}

/* visibility on a room -- for entering and exits */
bool char_can_see_room (CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex) {
    int flags = pRoomIndex->room_flags;
    if (ch == NULL || pRoomIndex == NULL)
        return FALSE;
    if (IS_SET (flags, ROOM_IMP_ONLY) && char_get_trust (ch) < MAX_LEVEL)
        return FALSE;
    if (IS_SET (flags, ROOM_GODS_ONLY) && !IS_IMMORTAL (ch))
        return FALSE;
    if (IS_SET (flags, ROOM_HEROES_ONLY) && !IS_IMMORTAL (ch))
        return FALSE;
    if (IS_SET (flags, ROOM_NEWBIES_ONLY) && ch->level > 5 && !IS_IMMORTAL (ch))
        return FALSE;
    if (!IS_IMMORTAL (ch) && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
        return FALSE;
    return TRUE;
}

/* True if char can see victim, regardless of room. */
bool char_can_see_anywhere (CHAR_DATA * ch, CHAR_DATA * victim) {
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
bool char_can_see_in_room (CHAR_DATA * ch, CHAR_DATA * victim) {
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
bool char_can_see_obj (CHAR_DATA * ch, OBJ_DATA * obj) {
    if (!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT))
        return TRUE;
    if (IS_SET (obj->extra_flags, ITEM_VIS_DEATH))
        return FALSE;
    if (IS_AFFECTED (ch, AFF_BLIND) && obj->item_type != ITEM_POTION)
        return FALSE;
    if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
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
bool char_can_drop_obj (CHAR_DATA * ch, OBJ_DATA * obj) {
    if (!IS_SET (obj->extra_flags, ITEM_NODROP))
        return TRUE;
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return TRUE;
    return FALSE;
}

/* Config Colour stuff */
void char_reset_colour (CHAR_DATA * ch) {
    int i;
    if (ch == NULL || ch->pcdata == NULL)
        return;
    for (i = 0; i < COLOUR_MAX; i++)
        ch->pcdata->colour[i] = colour_setting_table[i].default_colour;
}

void char_move (CHAR_DATA * ch, int door, bool follow) {
    CHAR_DATA *fch;
    CHAR_DATA *fch_next;
    ROOM_INDEX_DATA *in_room;
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    bool pass_door;

    if (door < 0 || door > 5) {
        bug ("char_move: bad door %d.", door);
        return;
    }
    if (ch->daze > 0) {
        send_to_char ("You're too dazed to move...\n\r", ch);
        return;
    }
    if (ch->wait > 0) {
        send_to_char ("You haven't quite recovered yet...\n\r", ch);
        return;
    }

    /* Exit trigger, if activated, bail out. Only PCs are triggered. */
    if (!IS_NPC (ch) && mp_exit_trigger (ch, door))
        return;

    in_room = ch->in_room;
    if ((pexit = in_room->exit[door]) == NULL
        || (to_room = pexit->to_room) == NULL
        || !char_can_see_room (ch, pexit->to_room))
    {
        send_to_char ("Alas, you cannot go that way.\n\r", ch);
        return;
    }

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

    if (IS_SET (pexit->exit_flags, EX_CLOSED) && !pass_door) {
        act ("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
        return;
    }
    if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL &&
        in_room == ch->master->in_room)
    {
        send_to_char ("What?  And leave your beloved master?\n\r", ch);
        return;
    }
    if (!room_is_owner (to_room, ch) && room_is_private (to_room)) {
        send_to_char ("That room is private right now.\n\r", ch);
        return;
    }

    /* Special conditions for players. */
    if (!IS_NPC (ch)) {
        int iClass, iGuild;
        int move, in_sect, to_sect, flying;

        for (iClass = 0; iClass < CLASS_MAX; iClass++) {
            for (iGuild = 0; iGuild < MAX_GUILD; iGuild++) {
                if (iClass != ch->class
                    && to_room->vnum == class_table[iClass].guild[iGuild])
                {
                    send_to_char ("You aren't allowed in there.\n\r", ch);
                    return;
                }
            }
        }

        in_sect = in_room->sector_type;
        to_sect = to_room->sector_type;
        flying = IS_AFFECTED (ch, AFF_FLYING) || IS_IMMORTAL (ch);

        if ((in_sect == SECT_AIR || to_sect == SECT_AIR) && !flying) {
            send_to_char ("You can't fly.\n\r", ch);
            return;
        }

        if ((in_sect == SECT_WATER_NOSWIM || to_sect == SECT_WATER_NOSWIM) &&
             !flying)
        {
            /* Look for a boat. */
            OBJ_DATA *boat;
            for (boat = ch->carrying; boat != NULL; boat = boat->next_content)
                if (boat->item_type == ITEM_BOAT)
                    break;
            if (boat == NULL) {
                send_to_char ("You need a boat to go there.\n\r", ch);
                return;
            }
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

        if (ch->move < move) {
            send_to_char ("You are too exhausted.\n\r", ch);
            return;
        }

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
#ifndef VANILLA
        EXIT_DATA *pexit_rev;
        int rev_dir = REV_DIR(door);

        pexit_rev = room_get_opposite_exit (in_room, door, NULL);
        if (!pexit_rev)
            act ("$n has arrived.", ch, NULL, NULL, TO_NOTCHAR);
        else {
            if (IS_SET (pexit_rev->exit_flags, EX_CLOSED))
                act ("$n enters from $t, passing through the $d.", ch,
                    door_table[rev_dir].from, pexit_rev->keyword, TO_NOTCHAR);
            else
                act ("$n enters from $t.", ch,
                    door_table[rev_dir].from, NULL, TO_NOTCHAR);
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

char *char_format_to_char (CHAR_DATA *victim, CHAR_DATA *ch) {
    static char buf[MAX_STRING_LENGTH];
    char message[MAX_STRING_LENGTH];
    buf[0] = '\0';

#ifndef VANILLA
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
        if (IS_REALLY_EVIL (victim))
            strcat (buf, "({DBlack Aura{x) ");
        else if (IS_EVIL (victim))
            strcat (buf, "({rRed Aura{x) ");
    }
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD)) {
        if (IS_REALLY_GOOD (victim))
            strcat (buf, "({YGolden Aura{x) ");
        else if (IS_GOOD (victim))
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
    if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_EVIL (victim))
        strcat (buf, "(Red Aura) ");
    if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_GOOD (victim))
        strcat (buf, "(Silver Aura) ");
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

void char_look_at_char (CHAR_DATA * victim, CHAR_DATA * ch) {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    int iWear;
    int percent;
    bool found;

    if (char_can_see_in_room (victim, ch)) {
        if (ch == victim)
            act ("$n looks at $mself.", ch, NULL, NULL, TO_NOTCHAR);
        else {
            act ("$n looks at you.", ch, NULL, victim, TO_VICT);
            act ("$n looks at $N.", ch, NULL, victim, TO_OTHERS);
        }
    }

    if (victim->description[0] != '\0')
        send_to_char (victim->description, ch);
    else
        act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);

    if (victim->max_hit > 0)
        percent = (100 * victim->hit) / victim->max_hit;
    else
        percent = -1;

    switch (victim->position) {
        case POS_DEAD:
            act ("$E is... dead?!", ch, NULL, victim, TO_CHAR);
            break;
        case POS_MORTAL:
            act ("$E is on the ground, mortally wounded.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_INCAP:
            act ("$E is on the ground, incapacitated.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_STUNNED:
            act ("$E is on the ground, stunned.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_SLEEPING:
            act ("$E is on the ground, sleeping.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_RESTING:
            act ("$E is sitting and resting.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_SITTING:
            act ("$E is sitting.", ch, NULL, victim, TO_CHAR);
            break;
        case POS_STANDING:
            act ("$E is standing.", ch, NULL, victim, TO_CHAR);
            break;
    }

    sprintf (buf, "%s %s.\n\r", PERS_AW (victim, ch),
        condition_string (percent));
    buf[0] = UPPER (buf[0]);
    send_to_char (buf, ch);

    found = FALSE;
    for (iWear = 0; iWear < WEAR_MAX; iWear++) {
        if ((obj = char_get_eq_by_wear (victim, iWear)) != NULL
            && char_can_see_obj (ch, obj))
        {
            if (!found) {
                send_to_char ("\n\r", ch);
                act ("$N is using:", ch, NULL, victim, TO_CHAR);
                found = TRUE;
            }
            send_to_char (wear_table[iWear].look_msg, ch);
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

void char_list_show_to_char (CHAR_DATA * list, CHAR_DATA * ch) {
    CHAR_DATA *rch;

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
bool char_is_friend (CHAR_DATA * ch, CHAR_DATA * victim) {
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
        && ch->pIndexData == victim->pIndexData)
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
bool char_can_loot (CHAR_DATA * ch, OBJ_DATA * obj) {
    CHAR_DATA *owner, *wch;
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

void char_take_obj (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container) {
    /* variables for AUTOSPLIT */
    CHAR_DATA *gch;
    int members;
    char buffer[100];

    BAIL_IF_ACT (!CAN_WEAR (obj, ITEM_TAKE),
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
        BAIL_IF (container->pIndexData->vnum == OBJ_VNUM_PIT &&
                char_get_trust (ch) < obj->level,
            "You are not powerful enough to use it.\n\r", ch);

        if (container->pIndexData->vnum == OBJ_VNUM_PIT
            && !CAN_WEAR (container, ITEM_TAKE)
            && !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
            obj->timer = 0;
        act ("You get $p from $P.", ch, obj, container, TO_CHAR);
        act ("$n gets $p from $P.", ch, obj, container, TO_NOTCHAR);
        REMOVE_BIT (obj->extra_flags, ITEM_HAD_TIMER);
        obj_from_obj (obj);
    }
    else {
        act ("You get $p.", ch, obj, container, TO_CHAR);
        act ("$n gets $p.", ch, obj, container, TO_NOTCHAR);
        obj_from_room (obj);
    }

    if (obj->item_type == ITEM_MONEY) {
        ch->silver += obj->value[0];
        ch->gold += obj->value[1];
        if (IS_SET (ch->plr, PLR_AUTOSPLIT)) { /* AUTOSPLIT code */
            members = 0;
            for (gch = ch->in_room->people; gch; gch = gch->next_in_room)
                if (!IS_AFFECTED (gch, AFF_CHARM) && is_same_group (gch, ch))
                    members++;

            if (members > 1 && (obj->value[0] > 1 || obj->value[1])) {
                sprintf (buffer, "%d %d", obj->value[0], obj->value[1]);
                do_function (ch, &do_split, buffer);
            }
        }

        obj_extract (obj);
    }
    else
        obj_to_char (obj, ch);
}

/* Remove an object. */
bool char_remove_obj (CHAR_DATA * ch, int iWear, bool fReplace, bool quiet) {
    OBJ_DATA *obj;

    if ((obj = char_get_eq_by_wear (ch, iWear)) == NULL)
        return TRUE;
    if (!fReplace)
        return FALSE;
    if (IS_SET (obj->extra_flags, ITEM_NOREMOVE)) {
        if (!quiet)
            act ("You can't remove $p.", ch, obj, NULL, TO_CHAR);
        return FALSE;
    }

    char_unequip (ch, obj);
    if (!quiet) {
        act2 ("You stop using $p.",
              "$n stops using $p.", ch, obj, NULL, 0, POS_RESTING);
    }
    return TRUE;
}

/* Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick. */
void char_wear_obj (CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace) {
    char buf[MAX_STRING_LENGTH];
    if (ch->level < obj->level) {
        sprintf (buf, "You must be level %d to use this object.\n\r",
                 obj->level);
        send_to_char (buf, ch);
        act ("$n tries to use $p, but is too inexperienced.",
             ch, obj, NULL, TO_NOTCHAR);
        return;
    }
    if (obj->item_type == ITEM_LIGHT) {
        if (!char_remove_obj (ch, WEAR_LIGHT, fReplace, FALSE))
            return;
        act ("You light $p and hold it.", ch, obj, NULL, TO_CHAR);
        act ("$n lights $p and holds it.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_LIGHT);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_FINGER)) {
        if (char_get_eq_by_wear (ch, WEAR_FINGER_L) != NULL
            && char_get_eq_by_wear (ch, WEAR_FINGER_R) != NULL
            && !char_remove_obj (ch, WEAR_FINGER_L, fReplace, FALSE)
            && !char_remove_obj (ch, WEAR_FINGER_R, fReplace, FALSE))
            return;

        if (char_get_eq_by_wear (ch, WEAR_FINGER_L) == NULL) {
            act ("You wear $p on your left finger.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p on $s left finger.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_FINGER_L);
            return;
        }

        if (char_get_eq_by_wear (ch, WEAR_FINGER_R) == NULL) {
            act ("You wear $p on your right finger.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p on $s right finger.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_FINGER_R);
            return;
        }

        bug ("char_wear_obj: no free finger.", 0);
        send_to_char ("You already wear two rings.\n\r", ch);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_NECK)) {
        if (char_get_eq_by_wear (ch, WEAR_NECK_1) != NULL
            && char_get_eq_by_wear (ch, WEAR_NECK_2) != NULL
            && !char_remove_obj (ch, WEAR_NECK_1, fReplace, FALSE)
            && !char_remove_obj (ch, WEAR_NECK_2, fReplace, FALSE))
            return;

        if (char_get_eq_by_wear (ch, WEAR_NECK_1) == NULL) {
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_NECK_1);
            return;
        }
        if (char_get_eq_by_wear (ch, WEAR_NECK_2) == NULL) {
            act ("You wear $p around your neck.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p around $s neck.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_NECK_2);
            return;
        }

        bug ("char_wear_obj: no free neck.", 0);
        send_to_char ("You already wear two neck items.\n\r", ch);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_BODY)) {
        if (!char_remove_obj (ch, WEAR_BODY, fReplace, FALSE))
            return;
        act ("You wear $p on your torso.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s torso.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_BODY);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_HEAD)) {
        if (!char_remove_obj (ch, WEAR_HEAD, fReplace, FALSE))
            return;
        act ("You wear $p on your head.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s head.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_HEAD);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_LEGS)) {
        if (!char_remove_obj (ch, WEAR_LEGS, fReplace, FALSE))
            return;
        act ("You wear $p on your legs.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s legs.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_LEGS);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_FEET)) {
        if (!char_remove_obj (ch, WEAR_FEET, fReplace, FALSE))
            return;
        act ("You wear $p on your feet.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s feet.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_FEET);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_HANDS)) {
        if (!char_remove_obj (ch, WEAR_HANDS, fReplace, FALSE))
            return;
        act ("You wear $p on your hands.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s hands.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_HANDS);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_ARMS)) {
        if (!char_remove_obj (ch, WEAR_ARMS, fReplace, FALSE))
            return;
        act ("You wear $p on your arms.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p on $s arms.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_ARMS);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_ABOUT)) {
        if (!char_remove_obj (ch, WEAR_ABOUT, fReplace, FALSE))
            return;
        act ("You wear $p about your torso.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p about $s torso.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_ABOUT);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_WAIST)) {
        if (!char_remove_obj (ch, WEAR_WAIST, fReplace, FALSE))
            return;
        act ("You wear $p about your waist.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p about $s waist.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_WAIST);
        return;
    }
    if (CAN_WEAR (obj, ITEM_WEAR_WRIST)) {
        if (char_get_eq_by_wear (ch, WEAR_WRIST_L) != NULL
            && char_get_eq_by_wear (ch, WEAR_WRIST_R) != NULL
            && !char_remove_obj (ch, WEAR_WRIST_L, fReplace, FALSE)
            && !char_remove_obj (ch, WEAR_WRIST_R, fReplace, FALSE))
            return;

        if (char_get_eq_by_wear (ch, WEAR_WRIST_L) == NULL) {
            act ("You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p around $s left wrist.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_WRIST_L);
            return;
        }
        if (char_get_eq_by_wear (ch, WEAR_WRIST_R) == NULL) {
            act ("You wear $p around your right wrist.", ch, obj, NULL, TO_CHAR);
            act ("$n wears $p around $s right wrist.", ch, obj, NULL, TO_NOTCHAR);
            char_equip (ch, obj, WEAR_WRIST_R);
            return;
        }

        bug ("char_wear_obj: no free wrist.", 0);
        send_to_char ("You already wear two wrist items.\n\r", ch);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_SHIELD)) {
        OBJ_DATA *weapon;
        if (!char_remove_obj (ch, WEAR_SHIELD, fReplace, FALSE))
            return;

        weapon = char_get_eq_by_wear (ch, WEAR_WIELD);
        BAIL_IF (weapon != NULL && ch->size < SIZE_LARGE &&
                IS_WEAPON_STAT (weapon, WEAPON_TWO_HANDS),
            "Your hands are tied up with your weapon!\n\r", ch);

        act ("You wear $p as a shield.", ch, obj, NULL, TO_CHAR);
        act ("$n wears $p as a shield.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_SHIELD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WIELD)) {
        char *msg;
        int sn, skill;

        if (!char_remove_obj (ch, WEAR_WIELD, fReplace, FALSE))
            return;
        BAIL_IF (!IS_NPC (ch) && obj_get_weight (obj) >
                (str_app[char_get_curr_stat (ch, STAT_STR)].wield * 10),
            "It is too heavy for you to wield.\n\r", ch);
        BAIL_IF (!IS_NPC (ch) && ch->size < SIZE_LARGE &&
                IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS) &&
                char_get_eq_by_wear (ch, WEAR_SHIELD) != NULL,
            "You need two hands free for that weapon.\n\r", ch);

        act ("You wield $p.", ch, obj, NULL, TO_CHAR);
        act ("$n wields $p.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_WIELD);

        sn = char_get_weapon_sn (ch);
        if (sn == gsn_hand_to_hand)
            return;

        skill = char_get_weapon_skill (ch, sn);

             if (skill >= 100) msg = "$p feels like a part of you!";
        else if (skill >   85) msg = "You feel quite confident with $p.";
        else if (skill >   70) msg = "You are skilled with $p.";
        else if (skill >   50) msg = "Your skill with $p is adequate.";
        else if (skill >   25) msg = "$p feels a little clumsy in your hands.";
        else if (skill >    1) msg = "You fumble and almost drop $p.";
        else                   msg = "You don't even know which end is up on $p.";
        act (msg, ch, obj, NULL, TO_CHAR);
        return;
    }

    if (CAN_WEAR (obj, ITEM_HOLD)) {
        if (!char_remove_obj (ch, WEAR_HOLD, fReplace, FALSE))
            return;
        act ("You hold $p in your hand.", ch, obj, NULL, TO_CHAR);
        act ("$n holds $p in $s hand.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_HOLD);
        return;
    }

    if (CAN_WEAR (obj, ITEM_WEAR_FLOAT)) {
        if (!char_remove_obj (ch, WEAR_FLOAT, fReplace, FALSE))
            return;
        act ("You release $p and it floats next to you.", ch, obj, NULL, TO_CHAR);
        act ("$n releases $p to float next to $m.", ch, obj, NULL, TO_NOTCHAR);
        char_equip (ch, obj, WEAR_FLOAT);
        return;
    }

    if (fReplace)
        send_to_char ("You can't wear, wield, or hold that.\n\r", ch);
}

void char_get_who_string (CHAR_DATA * ch, CHAR_DATA *wch, char *buf,
    size_t len)
{
    const char *class;

    /* Figure out what to print for class. */
    class = get_wiz_class(wch->level);
    if (class == NULL)
        class = class_table[wch->class].who_name;

    /* Format it up. */
    snprintf (buf, len, "[%2d %6s %s] %s%s%s%s%s%s%s%s\n\r",
        wch->level,
        wch->race < PC_RACE_MAX ? pc_race_table[wch->race].who_name : "     ",
        class,
        wch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
        wch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
        clan_table[wch->clan].who_name,
        IS_SET (wch->comm, COMM_AFK) ? "[AFK] " : "",
        IS_SET (wch->plr, PLR_KILLER) ? "(KILLER) " : "",
        IS_SET (wch->plr, PLR_THIEF) ? "(THIEF) " : "",
        wch->name, IS_NPC (wch) ? "" : wch->pcdata->title);
}

void char_set_title (CHAR_DATA * ch, char *title) {
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC (ch)) {
        bug ("char_set_title: NPC.", 0);
        return;
    }
    if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?') {
        buf[0] = ' ';
        strcpy (buf + 1, title);
    }
    else
        strcpy (buf, title);

    str_free (ch->pcdata->title);
    ch->pcdata->title = str_dup (buf);
}

bool char_has_key (CHAR_DATA * ch, int key) {
    OBJ_DATA *obj;
    for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
        if (obj->pIndexData->vnum == key)
            return TRUE;
    return FALSE;
}

bool char_drop_weapon_if_too_heavy (CHAR_DATA *ch) {
    OBJ_DATA *wield;
    static int depth;

    /* Check for weapon wielding.
     * Guard against recursion (for weapons with affects). */
    if (IS_NPC (ch))
        return FALSE;
    if ((wield = char_get_eq_by_wear (ch, WEAR_WIELD)) == NULL)
        return FALSE;
    if (obj_get_weight (wield) <= (
            str_app[char_get_curr_stat (ch, STAT_STR)].wield * 10))
        return FALSE;
    if (depth != 0)
        return FALSE;

    depth++;
    act ("You drop $p.", ch, wield, NULL, TO_CHAR);
    act ("$n drops $p.", ch, wield, NULL, TO_NOTCHAR);
    obj_from_char (wield);
    obj_to_room (wield, ch->in_room);
    depth--;
    return TRUE;
}

void char_reduce_money (CHAR_DATA *ch, int cost) {
    int silver = 0, gold = 0;

    silver = UMIN (ch->silver, cost);
    if (silver < cost) {
        gold = ((cost - silver + 99) / 100);
        silver = cost - 100 * gold;
    }

    ch->gold -= gold;
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

int char_get_obj_cost (CHAR_DATA *ch, OBJ_DATA *obj, bool fBuy) {
    SHOP_DATA *pShop;
    int cost;

    if (obj == NULL || (pShop = ch->pIndexData->pShop) == NULL)
        return 0;

    if (fBuy)
        cost = obj->cost * pShop->profit_buy / 100;
    else {
        OBJ_DATA *obj2;
        int itype;

        cost = 0;
        for (itype = 0; itype < MAX_TRADE; itype++) {
            if (obj->item_type == pShop->buy_type[itype]) {
                cost = obj->cost * pShop->profit_sell / 100;
                break;
            }
        }

        if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)) {
            for (obj2 = ch->carrying; obj2; obj2 = obj2->next_content) {
                if (obj->pIndexData == obj2->pIndexData
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
    if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND) {
        if (obj->value[1] == 0)
            cost /= 4;
        else
            cost = cost * obj->value[2] / obj->value[1];
    }
    return cost;
}

SHOP_DATA *char_get_shop (CHAR_DATA *ch) {
    if (ch->pIndexData == NULL)
        return NULL;
    return ch->pIndexData->pShop;
}

CHAR_DATA *char_get_keeper_room (CHAR_DATA *ch) {
    CHAR_DATA *k;
    for (k = ch->in_room->people; k; k = k->next_in_room)
        if (IS_NPC (k) && char_get_shop (k))
            return k;
    return NULL;
}

CHAR_DATA *char_get_trainer_room (CHAR_DATA *ch) {
    CHAR_DATA *t;
    for (t = ch->in_room->people; t; t = t->next_in_room)
        if (IS_NPC (t) && IS_SET (t->mob, MOB_TRAIN))
            return t;
    return NULL;
}

CHAR_DATA *char_get_practicer_room (CHAR_DATA *ch) {
    CHAR_DATA *p;
    for (p = ch->in_room->people; p; p = p->next_in_room)
        if (IS_NPC (p) && IS_SET (p->mob, MOB_PRACTICE))
            return p;
    return NULL;
}

CHAR_DATA *char_get_gainer_room (CHAR_DATA *ch) {
    CHAR_DATA *g;
    for (g = ch->in_room->people; g; g = g->next_in_room)
        if (IS_NPC (g) && IS_SET (g->mob, MOB_GAIN))
            return g;
    return NULL;
}

char *condition_string (int percent) {
#ifndef VANILLA
         if (percent >= 100) return "is in excellent condition";
    else if (percent >=  90) return "has a few scratches";
    else if (percent >=  80) return "has a few bruises";
    else if (percent >=  70) return "has some small wounds and bruises";
    else if (percent >=  60) return "has some large wounds";
    else if (percent >=  50) return "has quite a large few wounds";
    else if (percent >=  40) return "has some big nasty wounds and scratches";
    else if (percent >=  30) return "looks seriously wounded";
    else if (percent >=  20) return "looks pretty hurt";
    else if (percent >=  10) return "is in awful condition";
    else if (percent >    0) return "is in critical condition";
    else if (percent >  -10) return "is stunned on the floor";
    else if (percent >  -20) return "is incapacitated and bleeding to death";
    else                     return "is mortally wounded";
#else
         if (percent >= 100) return "is in excellent condition";
    else if (percent >=  90) return "has a few scratches";
    else if (percent >=  75) return "has some small wounds and bruises";
    else if (percent >=  50) return "has quite a few wounds";
    else if (percent >=  30) return "has some big nasty wounds and scratches";
    else if (percent >=  15) return "looks pretty hurt";
    else if (percent >=   0) return "is in awful condition";
    else                     return "is bleeding to death";
#endif
}

const char *get_wiz_class (int level) {
    switch (level) {
        case IMPLEMENTOR: return "IMP";
        case CREATOR:     return "CRE";
        case SUPREME:     return "SUP";
        case DEITY:       return "DEI";
        case GOD:         return "GOD";
        case IMMORTAL:    return "IMM";
        case DEMI:        return "DEM";
        case ANGEL:       return "ANG";
        case AVATAR:      return "AVA";
        default:          return NULL;
    }
}
