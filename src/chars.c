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
#include <stdlib.h>

#include "utils.h"
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
#include "wiz_l6.h"
#include "materials.h"
#include "globals.h"
#include "memory.h"
#include "board.h"
#include "update.h"
#include "items.h"
#include "players.h"
#include "mobiles.h"
#include "save.h"

#include "act_move.h"
#include "act_info.h"
#include "act_group.h"
#include "act_player.h"

#include "chars.h"

/* used for saving */
static int save_number = 0;

OBJ_T *char_get_weapon (const CHAR_T *ch) {
    OBJ_T *wield;

    wield = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD);
    if (wield == NULL || !item_can_wield (wield))
        return NULL;
    return wield;
}

/* for returning weapon information */
int char_get_weapon_sn (const CHAR_T *ch) {
    const WEAPON_T *weapon;
    const OBJ_T *wield;

    wield = char_get_weapon (ch);
    if (wield == NULL)
        return SN(HAND_TO_HAND);

    weapon = weapon_get (wield->v.weapon.weapon_type);
    if (weapon == NULL || weapon->skill_index < 0)
        return -1;

    return weapon->skill_index;
}

int char_get_weapon_skill (const CHAR_T *ch, int sn) {
    if (IS_NPC (ch))
        return mobile_get_weapon_skill (ch, sn);
    else
        return player_get_weapon_skill (ch, sn);
}

/* Retrieve a character's trusted level for permission checking. */
int char_get_trust (const CHAR_T *ch) {
    ch = REAL_CH (ch);
    if (ch->trust)
        return ch->trust;
    if (IS_NPC (ch) && ch->level >= LEVEL_HERO)
        return LEVEL_HERO - 1;
    else
        return ch->level;
}

/* command for retrieving stats */
int char_get_curr_stat (const CHAR_T *ch, int stat) {
    const PC_RACE_T *pc_race;
    int max;

    if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
        max = 25;
    else {
        pc_race = pc_race_get_by_race (ch->race);
        max = pc_race->max_stats[stat] + 4 + pc_race->bonus_max;

        if (class_table[ch->class].attr_prime == stat)
            max += 2;
        max = UMIN (max, 25);
    }
    return URANGE (3, ch->perm_stat[stat] + ch->mod_stat[stat], max);
}

/* command for returning max training score */
int char_get_max_train (const CHAR_T *ch, int stat) {
    const PC_RACE_T *pc_race;
    int max;

    if (IS_NPC (ch) || ch->level > LEVEL_IMMORTAL)
        return 25;

    pc_race = pc_race_get_by_race (ch->race);
    max = pc_race->max_stats[stat];
    if (class_table[ch->class].attr_prime == stat)
        max += 2 + pc_race->bonus_max;
    return UMIN (max, 25);
}

long int char_get_carry_weight (const CHAR_T *ch) {
    return ch->carry_weight + (ch->silver / 10) + (ch->gold * 2 / 5);
}

/* Retrieve a character's carry capacity. */
int char_get_max_carry_count (const CHAR_T *ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 1000;
    if (IS_PET (ch))
        return 0;
    return WEAR_LOC_MAX + (2 * char_get_curr_stat (ch, STAT_DEX)) + ch->level;
}

/* Retrieve a character's carry capacity. */
long int char_get_max_carry_weight (const CHAR_T *ch) {
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return 10000000;
    if (IS_PET (ch))
        return 0;
    return char_str_carry_bonus (ch) * 10 + ch->level * 25;
}

OBJ_T *char_get_active_light (const CHAR_T *ch) {
    OBJ_T *obj;
    if ((obj = char_get_eq_by_wear_loc (ch, WEAR_LOC_LIGHT)) == NULL)
        return NULL;
    if (!item_is_lit (obj))
        return NULL;
    return obj;
}

bool char_has_active_light (const CHAR_T *ch)
    { return char_get_active_light (ch) ? TRUE : FALSE; }

/* Move a char out of a room. */
void char_from_room (CHAR_T *ch) {
    BAIL_IF_BUG (ch->in_room == NULL,
        "char_from_room: NULL.", 0);

    if (!IS_NPC (ch))
        --ch->in_room->area->nplayer;
    if (char_has_active_light (ch) && ch->in_room->light > 0)
        --ch->in_room->light;

    LIST2_REMOVE (ch, room_prev, room_next,
        ch->in_room->people_first, ch->in_room->people_last);
    ch->in_room = NULL;
    ch->on = NULL; /* sanity check! */
}

void char_to_room_apply_plague (CHAR_T *ch) {
    AFFECT_T *af, plague;
    CHAR_T *vch;

    for (af = ch->affect_first; af != NULL; af = af->on_next)
        if (af->type == SN(PLAGUE))
            break;
    if (af == NULL) {
        REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
        return;
    }
    if (af->level == 1)
        return;

    affect_init (&plague, AFF_TO_AFFECTS, SN(PLAGUE), af->level - 1, number_range (1, 2 * (af->level - 1)), APPLY_STR, -5, AFF_PLAGUE);

    for (vch = ch->in_room->people_first; vch != NULL; vch = vch->room_next) {
        if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
            && !IS_IMMORTAL (vch) &&
            !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (6) == 0)
        {
            printf_to_char (vch, "You feel hot and feverish.\n\r");
            act ("$n shivers and looks very ill.", vch, NULL, NULL,
                TO_NOTCHAR);
            affect_join_char (&plague, vch);
        }
    }
}

/* Move a char into a room. */
void char_to_room (CHAR_T *ch, ROOM_INDEX_T *room_index) {
    if (room_index == NULL) {
        ROOM_INDEX_T *room;
        bug ("char_to_room: NULL.", 0);
        if ((room = room_get_index (ROOM_VNUM_TEMPLE)) != NULL)
            char_to_room (ch, room);
        return;
    }

    if (ch->in_room != NULL)
        char_from_room (ch);
    ch->in_room = room_index;
    LIST2_FRONT (ch, room_prev, room_next,
        room_index->people_first, room_index->people_last);

    if (!IS_NPC (ch)) {
        if (!ch->in_room->area->had_players) {
            ch->in_room->area->had_players = TRUE;
            ch->in_room->area->age = 0;
        }
        ++ch->in_room->area->nplayer;
    }

    if (char_has_active_light (ch))
        ++ch->in_room->light;

    if (IS_AFFECTED (ch, AFF_PLAGUE))
        char_to_room_apply_plague (ch);
}

/* Find a piece of eq on a character. */
OBJ_T *char_get_eq_by_wear_loc (const CHAR_T *ch, flag_t wear_loc) {
    OBJ_T *obj;
    if (ch == NULL)
        return NULL;
    for (obj = ch->content_first; obj != NULL; obj = obj->content_next)
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
        obj_give_to_room (obj, ch->in_room);
        return FALSE;
    }

    for (i = 0; i < 4; i++)
        ch->armor[i] -= obj_get_ac_type (obj, wear_loc, i);
    obj->wear_loc = wear_loc;

    if (!obj->enchanted) {
        for (paf = obj->obj_index->affect_first; paf; paf = paf->on_next)
            if (paf->apply != APPLY_SPELL_AFFECT)
                affect_modify_char (paf, ch, TRUE);
    }
    for (paf = obj->affect_first; paf != NULL; paf = paf->on_next) {
        if (paf->apply == APPLY_SPELL_AFFECT)
            affect_copy_to_char (paf, ch);
        else
            affect_modify_char (paf, ch, TRUE);
    }

    if (item_is_lit (obj) && ch->in_room != NULL)
        ++ch->in_room->light;

    return TRUE;
}

/* Unequip a char with an obj. */
bool char_unequip_obj (CHAR_T *ch, OBJ_T *obj) {
    AFFECT_T *paf = NULL;
    AFFECT_T *lpaf = NULL;
    AFFECT_T *lpaf_next = NULL;
    int i;

    RETURN_IF_BUG (obj->wear_loc == WEAR_LOC_NONE,
        "char_unequip_obj: already unequipped.", 0, FALSE);

    for (i = 0; i < 4; i++)
        ch->armor[i] += obj_get_ac_type (obj, obj->wear_loc, i);
    obj->wear_loc = WEAR_LOC_NONE;

    if (!obj->enchanted) {
        for (paf = obj->obj_index->affect_first; paf; paf = paf->on_next) {
            if (paf->apply == APPLY_SPELL_AFFECT) {
                for (lpaf = ch->affect_first; lpaf != NULL; lpaf = lpaf_next) {
                    lpaf_next = lpaf->on_next;
                    if ((lpaf->type == paf->type) &&
                        (lpaf->level == paf->level) &&
                        (lpaf->apply == APPLY_SPELL_AFFECT))
                    {
                        affect_remove (lpaf);
                        lpaf_next = NULL;
                    }
                }
            }
            else {
                affect_modify_char (paf, ch, FALSE);
                affect_check_char (ch, paf->bit_type, paf->bits);
            }
        }
    }

    for (paf = obj->affect_first; paf != NULL; paf = paf->on_next) {
        if (paf->apply == APPLY_SPELL_AFFECT) {
            bug ("norm-Apply: %d", 0);
            for (lpaf = ch->affect_first; lpaf != NULL; lpaf = lpaf_next) {
                lpaf_next = lpaf->on_next;
                if ((lpaf->type == paf->type) &&
                    (lpaf->level == paf->level) &&
                    (lpaf->apply == APPLY_SPELL_AFFECT))
                {
                    bug ("location = %d", lpaf->apply);
                    bug ("type = %d", lpaf->type);
                    affect_remove (lpaf);
                    lpaf_next = NULL;
                }
            }
        }
        else {
            affect_modify_char (paf, ch, FALSE);
            affect_check_char (ch, paf->bit_type, paf->bits);
        }
    }

    if (item_is_lit (obj) && ch->in_room != NULL && ch->in_room->light > 0)
        --ch->in_room->light;

    return TRUE;
}

/* Extract a char from the world. */
void char_extract (CHAR_T *ch) {
    /* char_extract() should only be used for characters currently in the
     * playable world. If they're not, char_free() should be used. */
    if (ch->in_room == NULL)
        bugf ("char_extract: %s is not in the real world.", ch->short_descr);

    nuke_pets (ch);
    die_follower (ch);

    stop_fighting (ch, TRUE);
    while (ch->content_first)
        obj_extract (ch->content_first);

    if (ch->desc != NULL && ch->desc->original != NULL) {
        do_function (ch, &do_return, "");
        ch->desc = NULL;
    }
    if (ch->in_room != NULL)
        char_from_room (ch);

    char_free (ch);
}

/* visibility on a room -- for entering and exits */
bool char_can_see_room (const CHAR_T *ch, const ROOM_INDEX_T *room_index) {
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
bool char_can_see_anywhere (const CHAR_T *ch, const CHAR_T *victim) {
    if (ch == victim)
        return TRUE;
    if (char_get_trust (ch) < victim->invis_level)
        return FALSE;
    if (char_get_trust (ch) < victim->incog_level &&
        ch->in_room != victim->in_room)
        return FALSE;
    if (  (!IS_NPC (ch) && EXT_IS_SET (ch->ext_plr, PLR_HOLYLIGHT))
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
        chance = char_get_skill (victim, SN(SNEAK));
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
bool char_can_see_in_room (const CHAR_T *ch, const CHAR_T *victim) {
    if (ch == victim)
        return TRUE;
    if (char_get_trust (ch) < victim->incog_level &&
        ch->in_room != victim->in_room)
        return FALSE;
    if (!char_can_see_anywhere (ch, victim))
        return FALSE;
    return TRUE;
}

/* True if char can see obj. */
bool char_can_see_obj (const CHAR_T *ch, const OBJ_T *obj) {
    if (!IS_NPC (ch) && EXT_IS_SET (ch->ext_plr, PLR_HOLYLIGHT))
        return TRUE;
    if (IS_SET (obj->extra_flags, ITEM_VIS_DEATH))
        return FALSE;
    if (IS_AFFECTED (ch, AFF_BLIND) && !item_is_visible_when_blind (obj))
        return FALSE;
    if (item_is_lit (obj))
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
bool char_can_drop_obj (const CHAR_T *ch, const OBJ_T *obj) {
    if (!IS_SET (obj->extra_flags, ITEM_NODROP))
        return TRUE;
    if (!IS_NPC (ch) && ch->level >= LEVEL_IMMORTAL)
        return TRUE;
    return FALSE;
}

bool char_has_boat (const CHAR_T *ch) {
    OBJ_T *boat;
    for (boat = ch->content_first; boat != NULL; boat = boat->content_next)
        if (item_is_boat (boat))
            return TRUE;
    return FALSE;
}

void char_move (CHAR_T *ch, int door, bool follow) {
    CHAR_T *fch, *fch_next;
    ROOM_INDEX_T *in_room, *to_room;
    EXIT_T *pexit;
    bool pass_door;

    BAIL_IF_BUG (door < 0 || door >= DIR_MAX,
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
    if (!IS_NPC (ch) && player_move_filter (ch, in_room, to_room))
        return;

    /* Force pets to stand before leaving the room. */
    if (in_room != to_room) {
        for (fch = in_room->people_first; fch != NULL; fch = fch_next) {
            fch_next = fch->room_next;
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
    for (fch = in_room->people_first; fch != NULL; fch = fch_next) {
        fch_next = fch->room_next;

        if (fch->master == ch && fch->position == POS_STANDING
            && char_can_see_room (fch, to_room))
        {

            if (IS_SET (ch->in_room->room_flags, ROOM_LAW)
                && (IS_NPC (fch) && EXT_IS_SET (fch->ext_mob, MOB_AGGRESSIVE)))
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

char *char_format_to_char (const CHAR_T *victim, const CHAR_T *ch) {
    const CHAR_T *real_ch;
    static char buf[MAX_STRING_LENGTH];
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
    if (!IS_NPC (victim) && EXT_IS_SET (victim->ext_plr, PLR_KILLER))
        strcat (buf, "({RKILLER{k) ");
    if (!IS_NPC (victim) && EXT_IS_SET (victim->ext_plr, PLR_THIEF))
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

    real_ch = REAL_CH (ch);
    if (IS_SET (real_ch->comm, COMM_MATERIALS))
        material_strcat (buf, material_get (victim->material));

    if (victim->position == victim->start_pos && victim->long_descr[0] != '\0')
        strcat (buf, victim->long_descr);
    else {
        size_t buf_len = strlen (buf);
        char_format_pos_msg (buf + buf_len, sizeof (buf) - buf_len,
            ch, victim, FALSE);
    }

    return buf;
}

size_t char_format_condition_or_pos_msg (char *buf, size_t size,
    const CHAR_T *ch, const CHAR_T *victim, bool use_pronoun)
{
    size_t written;
    written = char_format_condition_msg (buf, size, ch, victim, use_pronoun);
    if (written == 0)
        char_format_pos_msg (buf, size, ch, victim, use_pronoun);
    return written;
}

size_t char_format_condition_msg (char *buf, size_t size, const CHAR_T *ch,
    const CHAR_T *victim, bool use_pronoun)
{
    const HP_COND_T *cond;
    const char *name;
    size_t written;

    name = (use_pronoun)
        ? act_code_pronoun (victim, 'e')
        : PERS_AW (victim, ch);

    cond = hp_cond_get_for_char (victim);
    if (cond == NULL || cond->message == NULL) {
        buf[0] = '\0';
        return 0;
    }

    written = str_inject_args (buf, size, cond->message, name, NULL);
    written += snprintf (buf + written, size - written, "\n\r");
    buf[0] = UPPER (buf[0]);
    return written;
}

size_t char_format_pos_msg (char *buf, size_t size, const CHAR_T *ch,
    const CHAR_T *victim, bool use_pronoun)
{
    const POSITION_T *pos;
    const char *msg_arg1, *msg_arg2, *msg_arg3;
    char *victim_name, name[256];
    size_t written;

    victim_name = (use_pronoun)
        ? act_code_pronoun (victim, 'e')
        : PERS_AW (victim, ch);
    if (!use_pronoun && !IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF) &&
        victim->position == POS_STANDING && ch->on == NULL)
    {
        snprintf (name, sizeof (name), "%s%s",
            victim_name, victim->pcdata->title);
        msg_arg1 = name;
    }
    else
        msg_arg1 = victim_name;

    /* Format special messages for fighting. */
    pos = position_get (victim->position);
    if (pos->pos == POS_FIGHTING) {
        if (victim->fighting == NULL) {
            msg_arg2 = "thin air";
            msg_arg3 = "??";
        }
        else if (victim->fighting == ch) {
            msg_arg2 = "YOU";
            msg_arg3 = "!";
        }
        else if (victim->in_room == victim->fighting->in_room) {
            msg_arg2 = PERS_AW (victim->fighting, ch);
            msg_arg3 = ".";
        }
        else {
            msg_arg2 = "someone who left";
            msg_arg3 = "??";
        }
        written = str_inject_args (buf, size,
            pos->room_msg, msg_arg1, msg_arg2, msg_arg3, NULL);
    }
    /* Format special messages for furniture. */
    else if (victim->on != NULL && pos->room_msg_furniture != NULL) {
        msg_arg2 = obj_furn_preposition (victim->on, victim->position);
        msg_arg3 = victim->on->short_descr; /* TODO: invisible chairs? */
        written = str_inject_args (buf, size,
            pos->room_msg_furniture, msg_arg1, msg_arg2, msg_arg3, NULL);
    }
    /* Format standard messages. */
    else
        written = str_inject_args (buf, size,
            pos->room_msg, msg_arg1, NULL);

    written += snprintf (buf + written, size - written, "\n\r");
    buf[0] = UPPER (buf[0]);
    return written;
}

void char_look_at_char (CHAR_T *victim, CHAR_T *ch) {
    const WEAR_LOC_T *wear_loc;
    char buf[MAX_STRING_LENGTH];
    OBJ_T *obj;
    flag_t wl;
    bool found;

    if (ch == victim)
        act ("$n looks at $mself.", ch, NULL, NULL, TO_NOTCHAR);
    else {
        if (char_can_see_in_room (victim, ch))
            act ("$n looks at you.", ch, NULL, victim, TO_VICT);
        act ("$n looks at $N.", ch, NULL, victim, TO_OTHERS);
    }

    if (victim->description[0] != '\0')
        printf_to_char (ch, victim->description);
    else
        act ("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);

    buf[0] = '\0';
#ifdef BASEMUD_SHOW_POSITION_IN_LOOK
    char_format_pos_msg (buf, sizeof (buf), ch, victim, TRUE);
    if (buf[0] != '\0')
        printf_to_char (ch, buf);

    buf[0] = '\0';
    char_format_condition_msg (buf, sizeof (buf), ch, victim, FALSE);
#else
    char_format_condition_or_pos_msg (buf, sizeof (buf), ch, victim, FALSE);
#endif

    if (buf[0] != '\0')
        printf_to_char (ch, buf);

    found = FALSE;
    for (wl = 0; wl < WEAR_LOC_MAX; wl++) {
        if ((obj = char_get_eq_by_wear_loc (victim, wl)) == NULL)
            continue;
        if (!char_can_see_obj (ch, obj))
            continue;
        if ((wear_loc = wear_loc_get (wl)) == NULL)
            continue;

        if (!found) {
            printf_to_char (ch, "\n\r");
            act ("$N is using:", ch, NULL, victim, TO_CHAR);
            found = TRUE;
        }
        printf_to_char (ch, "%-21s %s\n\r",
            wear_loc->look_msg, obj_format_to_char (obj, ch, TRUE));
    }

    if (victim != ch && !IS_NPC (ch) &&
        number_percent () < char_get_skill (ch, SN(PEEK)))
    {
        printf_to_char (ch, "\n\rYou peek at the inventory:\n\r");
        player_try_skill_improve (ch, SN(PEEK), TRUE, 4);
        obj_list_show_to_char (victim->content_first, ch, TRUE, TRUE);
    }
}

void char_list_show_to_char (const CHAR_T *list, CHAR_T *ch) {
    const CHAR_T *rch;

    for (rch = list; rch != NULL; rch = rch->room_next) {
        if (rch == ch)
            continue;
        if (char_get_trust (ch) < rch->invis_level)
            continue;

        if (char_can_see_anywhere (ch, rch))
            send_to_char (char_format_to_char (rch, ch), ch);
        else if (room_is_dark (ch->in_room)
                 && IS_AFFECTED (rch, AFF_INFRARED))
            printf_to_char (ch, "You see glowing red eyes watching YOU!\n\r");
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
bool char_can_loot (const CHAR_T *ch, const OBJ_T *obj) {
    CHAR_T *owner, *wch;
    if (IS_IMMORTAL (ch))
        return TRUE;
    if (!obj->owner || obj->owner == NULL)
        return TRUE;

    owner = NULL;
    for (wch = char_first; wch != NULL; wch = wch->global_next)
        if (!str_cmp (wch->name, obj->owner))
            owner = wch;

    if (owner == NULL)
        return TRUE;
    if (!str_cmp (ch->name, owner->name))
        return TRUE;
    if (!IS_NPC (owner) && EXT_IS_SET (owner->ext_plr, PLR_CANLOOT))
        return TRUE;
    if (is_same_group (ch, owner))
        return TRUE;
    return FALSE;
}

void char_take_obj (CHAR_T *ch, OBJ_T *obj, OBJ_T *container) {
    CHAR_T *gch;

    BAIL_IF_ACT (!obj_can_wear_flag (obj, ITEM_TAKE),
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
        for (gch = obj->in_room->people_first; gch != NULL; gch = gch->room_next)
            BAIL_IF_ACT (gch->on == obj,
                "$N appears to be using $p.", ch, obj, gch);

    if (container != NULL) {
        bool is_pit = obj_is_donation_pit (container);
        BAIL_IF (is_pit && char_get_trust (ch) < obj->level,
            "You are not powerful enough to use it.\n\r", ch);

        if (is_pit && !obj_can_wear_flag (container, ITEM_TAKE) &&
            !IS_OBJ_STAT (obj, ITEM_HAD_TIMER))
        {
            obj->timer = 0;
        }

        act2 ("You get $p from $P.", "$n gets $p from $P.",
            ch, obj, container, 0, POS_RESTING);
        REMOVE_BIT (obj->extra_flags, ITEM_HAD_TIMER);
    }
    else {
        act2 ("You get $p.", "$n gets $p.",
            ch, obj, container, 0, POS_RESTING);
    }

    if (!item_take_effect (obj, ch))
        act ("...but it refuses to budge.", ch, obj, container, TO_ALL);
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

bool char_has_available_wear_loc (const CHAR_T *ch, flag_t wear_loc) {
    if (ch == NULL)
        return FALSE;
    return (char_get_eq_by_wear_loc (ch, wear_loc) == NULL);
}

bool char_has_available_wear_flag (const CHAR_T *ch, flag_t wear_flag) {
    int i;
    if (ch == NULL)
        return FALSE;
    for (i = 0; i < WEAR_LOC_MAX; i++)
        if (wear_loc_table[i].wear_flag == wear_flag)
            if (char_has_available_wear_loc (ch, i))
                return TRUE;
    return FALSE;
}

/* Wear one object.
 * Optional replacement of existing objects. */
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
        if (!obj_can_wear_flag (obj, wear_loc_table[i].wear_flag))
            continue;
        wear_locs[locs++] = &(wear_loc_table[i]);
    }
    if (locs == 0) {
        if (replace)
            printf_to_char (ch, "You can't wear, wield, or hold that.\n\r");
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
        OBJ_T *weapon = char_get_eq_by_wear_loc (ch, WEAR_LOC_WIELD);
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
                char_get_eq_by_wear_loc (ch, WEAR_LOC_SHIELD) != NULL,
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
        if (sn != SN(HAND_TO_HAND)) {
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

void char_get_who_string (const CHAR_T *ch, const CHAR_T *wch, char *buf,
    size_t len)
{
    const PC_RACE_T *pc_race;
    const CLAN_T *clan;
    const char *class_name;

    /* Figure out what to print for class. */
    class_name = wiz_class_by_level (wch->level);
    if (class_name == NULL)
        class_name = class_get (wch->class)->who_name;

    /* get information we need. */
    clan    = clan_get (ch->clan);
    pc_race = pc_race_get_by_race (ch->race);

    /* Format it up. */
    snprintf (buf, len, "[%2d %6s %s] %s%s%s%s%s%s%s%s\n\r",
        wch->level,
        (pc_race) ? pc_race->who_name : "     ",
        class_name,
        (wch->incog_level >= LEVEL_HERO) ? "(Incog) " : "",
        (wch->invis_level >= LEVEL_HERO) ? "(Wizi) "  : "",
        (clan) ? clan->who_name : "",
        IS_SET (wch->comm, COMM_AFK) ? "[AFK] " : "",
        EXT_IS_SET (wch->ext_plr, PLR_KILLER) ? "(KILLER) " : "",
        EXT_IS_SET (wch->ext_plr, PLR_THIEF)  ? "(THIEF) "  : "",
        wch->name,
        IS_NPC (wch) ? "" : wch->pcdata->title);
}

bool char_has_key (const CHAR_T *ch, int key) {
    OBJ_T *obj;
    for (obj = ch->content_first; obj != NULL; obj = obj->content_next)
        if (obj->obj_index->vnum == key)
            return TRUE;
    return FALSE;
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

CHAR_T *char_get_keeper_room (const CHAR_T *ch) {
    CHAR_T *k;
    for (k = ch->in_room->people_first; k; k = k->room_next)
        if (IS_NPC (k) && mobile_get_shop (k))
            return k;
    return NULL;
}

CHAR_T *char_get_trainer_room (const CHAR_T *ch) {
    CHAR_T *t;
    for (t = ch->in_room->people_first; t; t = t->room_next)
        if (IS_NPC (t) && EXT_IS_SET (t->ext_mob, MOB_TRAIN))
            return t;
    return NULL;
}

CHAR_T *char_get_practicer_room (const CHAR_T *ch) {
    CHAR_T *p;
    for (p = ch->in_room->people_first; p; p = p->room_next)
        if (IS_NPC (p) && EXT_IS_SET (p->ext_mob, MOB_PRACTICE))
            return p;
    return NULL;
}

CHAR_T *char_get_gainer_room (const CHAR_T *ch) {
    CHAR_T *g;
    for (g = ch->in_room->people_first; g; g = g->room_next)
        if (IS_NPC (g) && EXT_IS_SET (g->ext_mob, MOB_GAIN))
            return g;
    return NULL;
}

int char_format_exit_string (const CHAR_T *ch, const ROOM_INDEX_T *room,
    int mode, char *out_buf, size_t out_size)
{
    EXIT_T *pexit;
    int door, isdoor, closed, count;
    size_t len;

    out_buf[0] = '\0';
    len = 0;
    count = 0;

    for (door = 0; door < DIR_MAX; door++) {
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
                str_capitalized (door_get_name(door)));

            if (isdoor) {
                char dbuf[64];
                char *dname = room_get_door_name (pexit->keyword,
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
    if (ch == NULL                         ||
        ch->desc == NULL                   ||
        ch->desc->connected != CON_PLAYING ||
        ch->was_in_room == NULL            ||
        ch->in_room != room_get_index (ROOM_VNUM_LIMBO)
    )
        return;

    ch->timer = 0;
    char_to_room (ch, ch->was_in_room);
    ch->was_in_room = NULL;
    act ("$n has returned from the void.", ch, NULL, NULL, TO_NOTCHAR);
}

const char *char_get_class_name (const CHAR_T *ch)
    { return (IS_NPC (ch)) ? "mobile" : class_get (ch->class)->name; }

const char *char_get_position_str (const CHAR_T *ch, int position,
    const OBJ_T *on, int with_punct)
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
bool char_is_trusted (const CHAR_T *ch, int level)
    { return char_get_trust (ch) >= level ? TRUE : FALSE; }

bool char_is_npc (const CHAR_T *ch)
    { return EXT_IS_SET(ch->ext_mob, MOB_IS_NPC) ? TRUE : FALSE; }
bool char_is_immortal (const CHAR_T *ch)
    { return (char_is_trusted (ch, LEVEL_IMMORTAL)) ? TRUE : FALSE; }
bool char_is_hero (const CHAR_T *ch)
    { return (char_is_trusted (ch, LEVEL_HERO)) ? TRUE : FALSE; }
bool char_is_affected (const CHAR_T *ch, flag_t sn)
    { return IS_SET((ch)->affected_by, sn) ? TRUE : FALSE; }

DEFINE_COND_FUN (char_is_sober) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->cond_hours[COND_DRUNK] <= 0) ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_drunk) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->cond_hours[COND_DRUNK] > 10) ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_thirsty) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->cond_hours[COND_THIRST] <= 0) ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_quenched) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->cond_hours[COND_THIRST] > COND_HOURS_STUFFED)
        ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_hungry) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->cond_hours[COND_HUNGER] <= 0) ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_fed) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return TRUE;
    return (ch->pcdata->cond_hours[COND_HUNGER] > COND_HOURS_STUFFED)
        ? TRUE : FALSE;
}

DEFINE_COND_FUN (char_is_full) {
    if (char_is_npc (ch) || ch->pcdata == NULL)
        return FALSE;
    return (ch->pcdata->cond_hours[COND_FULL] > COND_HOURS_STUFFED)
        ? TRUE : FALSE;
}

bool char_is_pet (const CHAR_T *ch)
    { return (char_is_npc (ch) && EXT_IS_SET (ch->ext_mob, MOB_PET)) ? TRUE : FALSE; }

bool char_is_good (const CHAR_T *ch)
    { return (ch->alignment >=  350) ? TRUE : FALSE; }
bool char_is_evil (const CHAR_T *ch)
    { return (ch->alignment <= -350) ? TRUE : FALSE; }
bool char_is_really_good (const CHAR_T *ch)
    { return (ch->alignment >=  750) ? TRUE : FALSE; }
bool char_is_really_evil (const CHAR_T *ch)
    { return (ch->alignment <= -750) ? TRUE : FALSE; }
bool char_is_neutral (const CHAR_T *ch)
    { return (!IS_GOOD(ch) && !IS_EVIL(ch)) ? TRUE : FALSE; }

bool char_is_outside (const CHAR_T *ch) {
    if (ch->in_room == NULL)
        return FALSE;
    return (!IS_SET ((ch)->in_room->room_flags, ROOM_INDOORS))
        ? TRUE : FALSE;
}
bool char_is_awake (const CHAR_T *ch)
    { return (ch->position > POS_SLEEPING) ? TRUE : FALSE; }

int char_get_age (const CHAR_T *ch) {
    int age = 17 + (ch->played + (int) (current_time - ch->logon)) / 72000;
    return age;
}

bool char_is_same_align (const CHAR_T *ch1, const CHAR_T *ch2) {
    if (EXT_IS_SET (ch1->ext_mob, MOB_NOALIGN))
        return FALSE;
    if (EXT_IS_SET (ch2->ext_mob, MOB_NOALIGN))
        return FALSE;
    if (char_is_good (ch1) && char_is_good (ch2))
        return TRUE;
    if (char_is_evil (ch1) && char_is_evil (ch2))
        return TRUE;
    if (char_is_neutral (ch1) && char_is_neutral (ch2))
        return TRUE;
    return FALSE;
}

int char_get_ac (const CHAR_T *ch, int type) {
    return ch->armor[type] + (char_is_awake (ch)
        ? char_dex_defense_bonus (ch) : 0);
}

int char_get_hitroll (const CHAR_T *ch)
    { return ch->hitroll + char_str_hitroll_bonus (ch); }
int char_get_damroll (const CHAR_T *ch)
    { return ch->damroll + char_str_damroll_bonus (ch); }

bool char_is_switched (const CHAR_T *ch)
    { return (ch->desc && ch->desc->original) ? TRUE : FALSE; }

bool char_is_builder (const CHAR_T *ch, const AREA_T *area) {
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

int char_get_vnum (const CHAR_T *ch)
    { return IS_NPC(ch) ? (ch)->mob_index->vnum : 0; }

int char_set_max_wait_state (CHAR_T *ch, int npulse) {
    if (!char_is_trusted (ch, IMPLEMENTOR) && npulse > ch->wait)
        ch->wait = npulse;
    return ch->wait;
}

int char_set_max_daze_state (CHAR_T *ch, int npulse) {
    if (!char_is_trusted (ch, IMPLEMENTOR) && npulse > ch->daze)
        ch->daze = npulse;
    return ch->daze;
}

char *char_get_short_descr (const CHAR_T *ch)
    { return char_is_npc (ch) ? ch->short_descr : ch->name; }

char *char_get_look_short_descr_anywhere (const CHAR_T *looker,
    const CHAR_T *ch)
{
    if (!char_can_see_anywhere (looker, ch))
        return "someone";
    return char_get_short_descr (ch);
}

char *char_get_look_short_descr (const CHAR_T *looker, const CHAR_T *ch) {
    if (!char_can_see_in_room (looker, ch))
        return "someone";
    return char_get_short_descr (ch);
}

/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */
int char_get_immunity (CHAR_T *ch, int dam_type) {
    const DAM_T *dam;
    int def;
    flag_t bit;

    /* no resistance for unclassed damage. sorry, hand-to-hand fighters! */
    if (dam_type == DAM_NONE)
        return IS_NORMAL;

    dam = dam_get (dam_type);
    if (dam == NULL)
        dam = &(dam_table[DAM_NONE]);

    /* determine default resistance.  if it's a physical attack (bash, pierce,
     * slash), check weapon resistance.  otherwise, check magic resistance. */
    bit = (dam->dam_flags & DAM_MAGICAL) ? RES_MAGIC : RES_WEAPON;
         if (IS_SET (ch->imm_flags,  bit)) def = IS_IMMUNE;
    else if (IS_SET (ch->res_flags,  bit)) def = IS_RESISTANT;
    else if (IS_SET (ch->vuln_flags, bit)) def = IS_VULNERABLE;
    else                                   def = IS_NORMAL;

    /* check for resistance to the attack's specific resistance bit.
     * if there is none, we're done here. */
    if (dam == NULL || dam->res <= 0)
        return def;

    /* if the character is immune, override default immunity. */
    if (IS_SET (ch->imm_flags, dam->res))
        return IS_IMMUNE;

    /* if the character is resistant, upgrade to resistant (don't override
     * complete immunity). */
    if (IS_SET (ch->res_flags, dam->res))
        return (def == IS_IMMUNE) ? IS_IMMUNE : IS_RESISTANT;

    /* if vulnerable, downgrade default immunity by one level. */
    if (IS_SET (ch->vuln_flags, dam->res)) {
        if (def == IS_IMMUNE)
            return IS_RESISTANT;
        else if (def == IS_RESISTANT)
            return IS_NORMAL;
        else
            return IS_VULNERABLE;
    }

    /* no specific immunity - use the default. */
    return def;
}

/* Stat bonuses. */
const STR_APP_T *char_get_curr_str_app (const CHAR_T *ch)
    { return str_app_get (char_get_curr_stat (ch, STAT_STR)); }
const INT_APP_T *char_get_curr_int_app (const CHAR_T *ch)
    { return int_app_get (char_get_curr_stat (ch, STAT_INT)); }
const WIS_APP_T *char_get_curr_wis_app (const CHAR_T *ch)
    { return wis_app_get (char_get_curr_stat (ch, STAT_WIS)); }
const DEX_APP_T *char_get_curr_dex_app (const CHAR_T *ch)
    { return dex_app_get (char_get_curr_stat (ch, STAT_DEX)); }
const CON_APP_T *char_get_curr_con_app (const CHAR_T *ch)
    { return con_app_get (char_get_curr_stat (ch, STAT_CON)); }

int char_str_carry_bonus (const CHAR_T *ch)
    { return char_get_curr_str_app (ch)->carry; }
int char_str_hitroll_bonus (const CHAR_T *ch)
    { return char_get_curr_str_app (ch)->tohit; }
int char_str_damroll_bonus (const CHAR_T *ch)
    { return char_get_curr_str_app (ch)->todam; }
int char_str_max_wield_weight (const CHAR_T *ch)
    { return char_get_curr_str_app (ch)->wield; }
int char_int_learn_rate (const CHAR_T *ch)
    { return char_get_curr_int_app (ch)->learn; }
int char_wis_level_practices (const CHAR_T *ch)
    { return char_get_curr_wis_app (ch)->practice; }
int char_dex_defense_bonus (const CHAR_T *ch)
    { return char_get_curr_dex_app (ch)->defensive; }
int char_con_level_hp (const CHAR_T *ch)
    { return char_get_curr_con_app (ch)->hitp; }
int char_con_shock (const CHAR_T *ch)
    { return char_get_curr_con_app (ch)->shock; }

/* tries to change to the next accessible board */
bool char_change_to_next_board (CHAR_T *ch) {
    int i;

    for (i = board_number (ch->pcdata->board) + 1; i < BOARD_MAX; i++)
        if (board_get_unread_notes_for_char (&board_table[i], ch) != BOARD_NOACCESS)
            break;
    if (i == BOARD_MAX)
        return FALSE;

    ch->pcdata->board = &board_table[i];
    return TRUE;
}

/* for returning skill information */
int char_get_skill (const CHAR_T *ch, int sn) {
    const SKILL_T *skill;
    int ch_skill;

    /* shorthand for level based skills */
    if (sn == -1) {
        skill = NULL;
        ch_skill = ch->level * 5 / 2;
    }
    /* ignore invalid skills. */
    else if (sn < -1 || sn >= SKILL_MAX || (skill = skill_get (sn)) == NULL) {
        bug ("char_get_skill: Bad sn %d.", sn);
        return 0;
    }

    /* get the skill rating. */
    if (skill != NULL) {
        ch_skill = IS_NPC (ch)
            ? mobile_get_skill_learned (ch, sn)
            : player_get_skill_learned (ch, sn);
    }

    /* dazed characters have bad skills. */
    if (ch->daze > 0) {
        if (skill == NULL || skill->spell_fun == spell_null)
            ch_skill = 2 * ch_skill / 3;
        else
            ch_skill /= 2;
    }

    /* drunks are bad at things. */
    if (IS_DRUNK (ch))
        ch_skill = 9 * ch_skill / 10;

    return URANGE (0, ch_skill, 100);
}

OBJ_T *char_die (CHAR_T *ch) {
    OBJ_T *corpse;

    stop_fighting (ch, TRUE);
    death_cry (ch);
    corpse = make_corpse (ch);

    if (IS_NPC (ch))
        mobile_die (ch);
    else
        player_die (ch);

    return corpse;
}

void char_damage_if_wounded (CHAR_T *ch) {
    CHAR_T *rch;
    int div = 2;

    if (ch->position > POS_INCAP)
        return;
    if (ch->position == POS_INCAP)
        div *= 2;

    /* NPC's shouldn't get damaged during stun while somebody is
     * fighting them. Too much kill theft!! */
    if (IS_NPC (ch)) {
        for (rch = ch->in_room->people_first; rch; rch = rch->room_next)
            if (rch->fighting == ch)
                break;
        if (rch)
            return;
    }

    damage_quiet (ch, ch, (ch->level / div) + 1, ATTACK_DEFAULT, DAM_NONE);
}

/* Update all chars, including mobs. */
void char_update_all (void) {
    CHAR_T *ch, *ch_next, *ch_quit;
    ch_quit = NULL;

    /* update save counter */
    save_number++;
    if (save_number > 29)
        save_number = 0;

    for (ch = char_first; ch != NULL; ch = ch_next) {
        ch_next = ch->global_next;
        char_update (ch);
        if (ch->timer > 30)
            ch_quit = ch;
    }

    /* Autosave and autoquit.
     * Check that these chars still exist. */
    for (ch = char_first; ch != NULL; ch = ch_next) {
        /* Edwin's fix for possible pet-induced problem
         * JR -- 10/15/00 */
        BAIL_IF_BUG (!IS_VALID (ch),
            "update_char: Trying to work with an invalidated character.\n", 0);

        ch_next = ch->global_next;
        if (ch->desc != NULL && ch->desc->descriptor % 30 == save_number)
            save_char_obj (ch);
        if (ch == ch_quit)
            do_function (ch, &do_quit, "");
    }
}

void char_update (CHAR_T *ch) {
    AFFECT_T *paf, *paf_next;

    if (ch->position >= POS_STUNNED) {
        /* check to see if we need to go home */
        if (IS_NPC (ch) && ch->area != NULL
            && ch->area != ch->in_room->area && ch->desc == NULL
            && ch->fighting == NULL && !IS_AFFECTED (ch, AFF_CHARM)
            && number_percent () < 5)
        {
            act ("$n wanders on home.", ch, NULL, NULL, TO_NOTCHAR);
            char_extract (ch);
            return;
        }
    }

    if (ch->position == POS_STUNNED)
        update_pos (ch);

    if (!IS_NPC (ch))
        player_update (ch);

    for (paf = ch->affect_first; paf; paf = paf_next) {
        paf_next = paf->on_next;
        if (paf->duration > 0) {
            paf->duration--;
            if (number_range (0, 4) == 0 && paf->level > 0)
                paf->level--;    /* spell strength fades with time */
        }
        else if (paf->duration < 0)
            ;
        else {
            if (paf_next == NULL
                || paf_next->type != paf->type || paf_next->duration > 0)
            {
                if (paf->type > 0 && skill_table[paf->type].msg_off) {
                    printf_to_char (ch, skill_table[paf->type].msg_off);
                    printf_to_char (ch, "\n\r");
                }
            }
            affect_remove (paf);
        }
    }

    /* Careful with the damages here,
     * MUST NOT refer to ch after damage taken,
     * as it may be lethal damage (on NPC). */
    if (affect_is_char_affected (ch, SN(PLAGUE)) && ch != NULL) {
        AFFECT_T *af, plague;
        CHAR_T *vch;
        int dam;

        if (ch->in_room == NULL)
            return;

        printf_to_char (ch, "You writhe in agony from the plague.\n\r");
        act ("$n writhes in agony as plague sores erupt from $s skin.",
             ch, NULL, NULL, TO_NOTCHAR);
        for (af = ch->affect_first; af; af = af->on_next)
            if (af->type == SN(PLAGUE))
                break;
        if (af == NULL) {
            REMOVE_BIT (ch->affected_by, AFF_PLAGUE);
            return;
        }
        if (af->level == 1)
            return;

        affect_init (&plague, AFF_TO_AFFECTS, SN(PLAGUE), af->level - 1, number_range (1, 2 * plague.level), APPLY_STR, -5, AFF_PLAGUE);

        for (vch = ch->in_room->people_first; vch != NULL;
             vch = vch->room_next)
        {
            if (!saves_spell (plague.level - 2, vch, DAM_DISEASE)
                && !IS_IMMORTAL (vch)
                && !IS_AFFECTED (vch, AFF_PLAGUE) && number_bits (4) == 0)
            {
                printf_to_char (vch, "You feel hot and feverish.\n\r");
                act ("$n shivers and looks very ill.", vch, NULL, NULL,
                     TO_NOTCHAR);
                affect_join_char (&plague, vch);
            }
        }

        dam = UMIN (ch->level, af->level / 5 + 1);
        ch->mana -= dam;
        ch->move -= dam;
        damage_quiet (ch, ch, dam, SN(PLAGUE), DAM_DISEASE);
    }
    else if (IS_AFFECTED (ch, AFF_POISON) && ch != NULL
             && !IS_AFFECTED (ch, AFF_SLOW))
    {
        AFFECT_T *poison;
        poison = affect_find (ch->affect_first, SN(POISON));

        if (poison != NULL) {
            printf_to_char (ch, "You shiver and suffer.\n\r");
            act ("$n shivers and suffers.", ch, NULL, NULL, TO_NOTCHAR);
            damage_quiet (ch, ch, poison->level / 10 + 1, SN(POISON),
                    DAM_POISON);
        }
    }
    else
        char_damage_if_wounded (ch);
}

bool char_drop_weapon_if_too_heavy (CHAR_T *ch) {
    if (IS_NPC (ch))
        return FALSE;
    return player_drop_weapon_if_too_heavy (ch);
}

void char_change_conditions (CHAR_T *ch, int drunk, int full, int thirst,
    int hunger)
{
    if (IS_NPC (ch))
        return;
    return player_change_conditions (ch, drunk, full, thirst, hunger);
}
