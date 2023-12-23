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

#include "spell_move.h"

#include "act_info.h"
#include "chars.h"
#include "comm.h"
#include "db.h"
#include "ext_flags.h"
#include "fight.h"
#include "find.h"
#include "interp.h"
#include "items.h"
#include "magic.h"
#include "objs.h"
#include "players.h"
#include "rooms.h"

/* NOTE: All the BAIL_IF() or FILTER() called to "You failed." is definitely
 * more verbose than a giant if() statement with 10-or-so different conditions,
 * but splitting it this way helped find and consolidate overlapping conditions
 * between different spells. And who knows, maybe someone will want to make the
 * messages more descriptive? */

/* POSSIBLE FEATURE: You know what would be neat? If 'word of recall'
 * teleported NPCs back to their original loading room. */

bool spell_filter_is_valid_move_target (CHAR_T *ch, CHAR_T *victim,
    int level, flag_t res_type, flag_t dam_type)
{
    FILTER (victim == ch,
        "You failed.\n\r", ch);
    FILTER (victim->in_room == NULL,
        "You failed.\n\r", ch);
    FILTER (victim->level >= level + 3,
        "You failed.\n\r", ch);
    FILTER (IS_NPC (victim) && IS_SET (victim->imm_flags, res_type),
        "You failed.\n\r", ch);
    FILTER (IS_NPC (victim) && saves_spell (level, victim, dam_type),
        "You failed.\n\r", ch);
    FILTER (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL),
        "You failed.\n\r", ch);
    FILTER (IS_SET (victim->in_room->room_flags, ROOM_PRIVATE),
        "You failed.\n\r", ch);
    FILTER (IS_SET (victim->in_room->room_flags, ROOM_SAFE),
        "You failed.\n\r", ch);
    FILTER (IS_SET (victim->in_room->room_flags, ROOM_SOLITARY),
        "You failed.\n\r", ch);
    return FALSE;
}

bool spell_filter_can_go_to (CHAR_T *ch, CHAR_T *victim,
    int level, flag_t res_type, flag_t dam_type)
{
    if (spell_filter_is_valid_move_target (ch, victim, level,
            res_type, dam_type))
        return TRUE;

    FILTER (!IS_NPC (victim) && victim->level >= LEVEL_HERO, /* NOT trust */
        "You failed.\n\r", ch);
    FILTER (!char_can_see_room (ch, victim->in_room),
        "You failed.\n\r", ch);
    FILTER (IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL),
        "You failed.\n\r", ch);
    FILTER (player_has_clan (victim) && !player_in_same_clan (ch, victim),
        "You failed.\n\r", ch);
    return FALSE;
}

bool spell_filter_use_warp_stone (CHAR_T *ch) {
    OBJ_T *stone;

    if (IS_IMMORTAL (ch))
        stone = NULL;
    else {
        stone = char_get_eq_by_wear_loc (ch, WEAR_LOC_HOLD);
        FILTER (stone == NULL || !item_is_warp_stone (stone),
            "You lack the proper component for this spell.\n\r", ch);
    }
    if (stone != NULL) {
        act ("You draw upon the power of $p.",   ch, stone, NULL, TO_CHAR);
        act ("It flares brightly and vanishes!", ch, stone, NULL, TO_CHAR);
        obj_extract (stone);
    }

    return FALSE;
}

OBJ_T *spell_sub_create_portal (ROOM_INDEX_T *from_room,
    ROOM_INDEX_T *to_room, int duration)
{
    OBJ_T *portal = obj_create (obj_get_index (OBJ_VNUM_PORTAL), 0);
    portal->timer = duration;
    portal->v.portal.to_vnum = to_room->vnum;
    obj_give_to_room (portal, from_room);
    return portal;
}

void spell_do_gate_teleport (CHAR_T *ch, ROOM_INDEX_T *to_room) {
    send_to_char ("You step through a gate and vanish.\n\r", ch);
    act ("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_NOTCHAR);
    char_to_room (ch, to_room);
    act ("$n has arrived through a gate.", ch, NULL, NULL, TO_NOTCHAR);
    do_function (ch, &do_look, "auto");
}

/* RT ROM-style gate */
DEFINE_SPELL_FUN (spell_gate) {
    CHAR_T *victim;
    bool gate_pet;

    BAIL_IF ((victim = find_char_world (ch, target_name)) == NULL,
        "You failed.\n\r", ch);
    if (spell_filter_can_go_to (ch, victim, level, RES_SUMMON, DAM_OTHER))
        return;

    if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
        gate_pet = TRUE;
    else
        gate_pet = FALSE;

    spell_do_gate_teleport (ch, victim->in_room);
    if (gate_pet)
        spell_do_gate_teleport (ch->pet, victim->in_room);
}

DEFINE_SPELL_FUN (spell_summon) {
    CHAR_T *victim;

    BAIL_IF ((victim = find_char_world (ch, target_name)) == NULL,
        "You failed.\n\r", ch);
    if (spell_filter_is_valid_move_target (ch, victim, level,
            RES_SUMMON, DAM_OTHER))
        return;

    BAIL_IF (!IS_NPC (victim) && EXT_IS_SET (victim->ext_plr, PLR_NOSUMMON),
        "You failed.\n\r", ch);
    BAIL_IF (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL,
        "You failed.\n\r", ch);
    BAIL_IF (IS_NPC (victim) && EXT_IS_SET (victim->ext_mob, MOB_AGGRESSIVE),
        "You failed.\n\r", ch);
    BAIL_IF (IS_NPC (victim) && victim->mob_index->shop != NULL,
        "You failed.\n\r", ch);
    BAIL_IF (IS_SET (ch->in_room->room_flags, ROOM_SAFE),
        "You failed.\n\r", ch);
    BAIL_IF (victim->fighting != NULL,
        "You failed.\n\r", ch);

    act ("$n disappears suddenly.", victim, NULL, NULL, TO_NOTCHAR);
    char_to_room (victim, ch->in_room);
    act ("$n has summoned you!", ch, NULL, victim, TO_VICT);
    act ("$n arrives suddenly.", victim, NULL, NULL, TO_NOTCHAR);

    do_function (victim, &do_look, "auto");
}

DEFINE_SPELL_FUN (spell_teleport) {
    CHAR_T *victim = (CHAR_T *) vo;
    ROOM_INDEX_T *room_index;

    BAIL_IF (victim->in_room == NULL,
        "You failed.\n\r", ch);
    BAIL_IF (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL),
        "You failed.\n\r", ch);
    BAIL_IF (victim != ch && IS_SET (victim->imm_flags, RES_SUMMON),
        "You failed.\n\r", ch);
    BAIL_IF (!IS_NPC (ch) && victim->fighting != NULL,
        "You failed.\n\r", ch);
    BAIL_IF (victim != ch && (saves_spell (level - 5, victim, DAM_OTHER)),
        "You failed.\n\r", ch);

    room_index = room_get_random_index (victim);
    if (victim != ch)
        send_to_char ("You have been teleported!\n\r", victim);

    act ("$n vanishes!", victim, NULL, NULL, TO_NOTCHAR);
    char_to_room (victim, room_index);
    act ("$n slowly fades into existence.", victim, NULL, NULL, TO_NOTCHAR);
    do_function (victim, &do_look, "auto");
}

/* RT recall spell is back */
DEFINE_SPELL_FUN (spell_word_of_recall) {
    CHAR_T *victim = (CHAR_T *) vo;
    ROOM_INDEX_T *location;

    BAIL_IF_ACT (IS_NPC (victim),
        "Your spell ignores $N.", ch, NULL, victim);
    BAIL_IF ((location = room_get_index (ROOM_VNUM_TEMPLE)) == NULL,
        "You are completely lost.\n\r", victim);
    BAIL_IF (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL) ||
             IS_AFFECTED (victim, AFF_CURSE),
        "Spell failed.\n\r", ch);

    if (victim->fighting != NULL)
        stop_fighting (victim, TRUE);

    ch->move /= 2;
    act ("$n disappears.", victim, NULL, NULL, TO_NOTCHAR);
    char_to_room (victim, location);
    act ("$n appears in the room.", victim, NULL, NULL, TO_NOTCHAR);
    do_function (victim, &do_look, "auto");
}

DEFINE_SPELL_FUN (spell_portal) {
    CHAR_T *victim;
    OBJ_T *portal;

    BAIL_IF ((victim = find_char_world (ch, target_name)) == NULL,
        "You failed.\n\r", ch);
    if (spell_filter_can_go_to (ch, victim, level, RES_SUMMON, DAM_OTHER))
        return;
    if (spell_filter_use_warp_stone (ch))
        return;

    portal = spell_sub_create_portal (ch->in_room, victim->in_room,
        2 + level / 25);
    act2 ("$p rises up before you.",
          "$p rises up from the ground.", ch, portal, NULL, 0, POS_RESTING);
}

DEFINE_SPELL_FUN (spell_nexus) {
    CHAR_T *victim;
    OBJ_T *portal;
    ROOM_INDEX_T *to_room, *from_room;

    BAIL_IF ((victim = find_char_world (ch, target_name)) == NULL,
        "You failed.\n\r", ch);
    if (spell_filter_can_go_to (ch, victim, level, RES_SUMMON, DAM_NONE))
        return;
    BAIL_IF (!char_can_see_room (ch, ch->in_room),
        "You failed.\n\r", ch);
    BAIL_IF (IS_SET (ch->in_room->room_flags, ROOM_SAFE),
        "You failed.\n\r", ch);
    if (spell_filter_use_warp_stone (ch))
        return;

    from_room = ch->in_room;
    to_room = victim->in_room;

    /* portal one */
    portal = spell_sub_create_portal (from_room, to_room, 1 + level / 10);
    act2 ("$p rises up before you.",
          "$p rises up from the ground.", ch, portal, NULL, 0, POS_RESTING);

    /* no second portal if rooms are the same */
    if (to_room == from_room)
        return;

    /* portal two */
    portal = spell_sub_create_portal (to_room, from_room, 1 + level / 10);
    if (to_room->people_first != NULL)
        act ("$p rises up from the ground.", to_room->people_first, portal,
            NULL, TO_ALL);
}
