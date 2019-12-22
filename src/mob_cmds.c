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

/***************************************************************************
 *                                                                         *
 *  Based on MERC 2.2 MOBprograms by N'Atas-ha.                            *
 *  Written and adapted to ROM 2.4 by                                      *
 *          Markku Nylander (markku.nylander@uta.fi)                       *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include "lookup.h"
#include "interp.h"
#include "groups.h"
#include "utils.h"
#include "comm.h"
#include "fight.h"
#include "mob_prog.h"
#include "db.h"
#include "chars.h"
#include "objs.h"
#include "rooms.h"
#include "find.h"
#include "act_info.h"
#include "globals.h"

#include "mob_cmds.h"

/* Command table. */
const MOB_CMD_T mob_cmd_table[] = {
    {"asound",     do_mpasound},
    {"gecho",      do_mpgecho},
    {"zecho",      do_mpzecho},
    {"kill",       do_mpkill},
    {"assist",     do_mpassist},
    {"junk",       do_mpjunk},
    {"echo",       do_mpecho},
    {"echoaround", do_mpechoaround},
    {"echoat",     do_mpechoat},
    {"mload",      do_mpmload},
    {"oload",      do_mpoload},
    {"purge",      do_mppurge},
    {"goto",       do_mpgoto},
    {"at",         do_mpat},
    {"transfer",   do_mptransfer},
    {"gtransfer",  do_mpgtransfer},
    {"otransfer",  do_mpotransfer},
    {"force",      do_mpforce},
    {"gforce",     do_mpgforce},
    {"vforce",     do_mpvforce},
    {"cast",       do_mpcast},
    {"damage",     do_mpdamage},
    {"remember",   do_mpremember},
    {"forget",     do_mpforget},
    {"delay",      do_mpdelay},
    {"cancel",     do_mpcancel},
    {"call",       do_mpcall},
    {"flee",       do_mpflee},
    {"remove",     do_mpremove},
    {NULL, 0}
};

/* Mob command interpreter. Implemented separately for security and speed
 * reasons. A trivial hack of interpret() */
void mob_interpret (CHAR_T *ch, char *argument) {
    char command[MAX_INPUT_LENGTH];
    int cmd;

    argument = one_argument (argument, command);

    /* Look for command in command table. */
    for (cmd = 0; mob_cmd_table[cmd].name != NULL; cmd++) {
        if (command[0] == mob_cmd_table[cmd].name[0]
            && !str_prefix (command, mob_cmd_table[cmd].name))
        {
            (*mob_cmd_table[cmd].do_fun) (ch, argument);
            tail_chain ();
            return;
        }
    }
    bugf ("mob_interpret: invalid cmd from mob %d: '%s'",
        CH_VNUM (ch), command);
}

char *mprog_type_to_name (flag_t type) {
    switch (type) {
        case TRIG_ACT:    return "ACT";
        case TRIG_BRIBE:  return "BRIBE";
        case TRIG_DEATH:  return "DEATH";
        case TRIG_ENTRY:  return "ENTRY";
        case TRIG_FIGHT:  return "FIGHT";
        case TRIG_GIVE:   return "GIVE";
        case TRIG_GREET:  return "GREET";
        case TRIG_GRALL:  return "GRALL";
        case TRIG_KILL:   return "KILL";
        case TRIG_HPCNT:  return "HPCNT";
        case TRIG_RANDOM: return "RANDOM";
        case TRIG_SPEECH: return "SPEECH";
        case TRIG_EXIT:   return "EXIT";
        case TRIG_EXALL:  return "EXALL";
        case TRIG_DELAY:  return "DELAY";
        case TRIG_SURR:   return "SURRENDER";
        default:          return "ERROR";
    }
}

DEFINE_DO_FUN (do_mob) {
    /* Security check! */
    if (ch->desc != NULL && char_get_trust (ch) < MAX_LEVEL)
        return;
    mob_interpret (ch, argument);
}

/* Prints the argument to all active players in the game
 * Syntax: mob gecho [string] */
DEFINE_DO_FUN (do_mpgecho) {
    DESCRIPTOR_T *d;

    BAIL_IF_BUG (argument[0] == '\0',
        "do_mpgecho: missing argument from vnum %d", CH_VNUM (ch));

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING) {
            if (IS_IMMORTAL (d->character))
                send_to_char ("Mob echo> ", d->character);
            send_to_char (argument, d->character);
            send_to_char ("\n\r", d->character);
        }
    }
}

/* Prints the argument to all players in the same area as the mob
 * Syntax: mob zecho [string] */
DEFINE_DO_FUN (do_mpzecho) {
    DESCRIPTOR_T *d;

    BAIL_IF_BUG (argument[0] == '\0',
        "do_mpzecho: missing argument from vnum %d", CH_VNUM (ch));

    if (ch->in_room == NULL)
        return;

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING
            && d->character->in_room != NULL
            && d->character->in_room->area == ch->in_room->area)
        {
            if (IS_IMMORTAL (d->character))
                send_to_char ("Mob echo> ", d->character);
            send_to_char (argument, d->character);
            send_to_char ("\n\r", d->character);
        }
    }
}

/* Prints the argument to all the rooms aroud the mobile
 * Syntax: mob asound [string] */
DEFINE_DO_FUN (do_mpasound) {
    ROOM_INDEX_T *was_in_room;
    int door;

    if (argument[0] == '\0')
        return;

    was_in_room = ch->in_room;
    for (door = 0; door < DIR_MAX; door++) {
        EXIT_T *pexit;

        if ((pexit = was_in_room->exit[door]) != NULL
            && pexit->to_room != NULL && pexit->to_room != was_in_room)
        {
            ch->in_room = pexit->to_room;
            trigger_mobs = FALSE;
            act (argument, ch, NULL, NULL, TO_NOTCHAR);
            trigger_mobs = TRUE;
        }
    }
    ch->in_room = was_in_room;
}

/* Lets the mobile kill any player or mobile without murder
 * Syntax: mob kill [victim] */
DEFINE_DO_FUN (do_mpkill) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
        return;
    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;
    if (victim == ch || IS_NPC (victim) || ch->position == POS_FIGHTING)
        return;

    BAIL_IF_BUG (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim,
        "MpKill - Charmed mob attacking master from vnum %d.", CH_VNUM (ch));

    multi_hit (ch, victim, ATTACK_DEFAULT);
}

/* Lets the mobile assist another mob or player
 * Syntax: mob assist [character] */
DEFINE_DO_FUN (do_mpassist) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);

    if (arg[0] == '\0')
        return;
    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;
    if (victim == ch || ch->fighting != NULL || victim->fighting == NULL)
        return;

    multi_hit (ch, victim->fighting, ATTACK_DEFAULT);
}


/* Lets the mobile destroy an object in its inventory
 * it can also destroy a worn object and it can destroy
 * items using all.xxxxx or just plain all of them
 *
 * Syntax: mob junk [item] */
DEFINE_DO_FUN (do_mpjunk) {
    char arg[MAX_INPUT_LENGTH];
    OBJ_T *obj;
    OBJ_T *obj_next;

    one_argument (argument, arg);

    if (arg[0] == '\0')
        return;
    if (str_cmp (arg, "all") && str_prefix ("all.", arg)) {
        if ((obj = find_obj_own_worn (ch, arg)) != NULL) {
            char_unequip_obj (ch, obj);
            obj_extract (obj);
            return;
        }
        if ((obj = find_obj_own_inventory (ch, arg)) == NULL)
            return;
        obj_extract (obj);
    }
    else {
        for (obj = ch->carrying; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (arg[3] == '\0' || str_in_namelist (&arg[4], obj->name)) {
                if (obj->wear_loc != WEAR_NONE)
                    char_unequip_obj (ch, obj);
                obj_extract (obj);
            }
        }
    }
}

/* Prints the message to everyone in the room other than the mob and victim
 * Syntax: mob echoaround [victim] [string] */
DEFINE_DO_FUN (do_mpechoaround) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    argument = one_argument (argument, arg);
    if (arg[0] == '\0')
        return;
    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;
    act (argument, ch, NULL, victim, TO_OTHERS);
}

/* Prints the message to only the victim
 * Syntax: mob echoat [victim] [string] */
DEFINE_DO_FUN (do_mpechoat) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    argument = one_argument (argument, arg);
    if (arg[0] == '\0' || argument[0] == '\0')
        return;
    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;
    act (argument, ch, NULL, victim, TO_VICT);
}

/* Prints the message to the room at large
 * Syntax: mpecho [string] */
DEFINE_DO_FUN (do_mpecho) {
    if (argument[0] == '\0')
        return;
    act (argument, ch, NULL, NULL, TO_NOTCHAR);
}

/* Lets the mobile load another mobile.
 * Syntax: mob mload [vnum] */
DEFINE_DO_FUN (do_mpmload) {
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_T *mob_index;
    CHAR_T *victim;
    int vnum;

    one_argument (argument, arg);
    if (ch->in_room == NULL || arg[0] == '\0' || !is_number (arg))
        return;

    vnum = atoi (arg);
    BAIL_IF_BUGF ((mob_index = get_mob_index (vnum)) == NULL,
        "Mpmload: bad mob index (%d) from mob %d", vnum, CH_VNUM (ch));

    victim = char_create_mobile (mob_index);
    char_to_room (victim, ch->in_room);
}

/* Lets the mobile load an object
 * Syntax: mob oload [vnum] [level] {R} */
DEFINE_DO_FUN (do_mpoload) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    OBJ_INDEX_T *obj_index;
    OBJ_T *obj;
    int level;
    bool to_room = FALSE, wear = FALSE;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    one_argument (argument, arg3);

    BAIL_IF_BUG (arg1[0] == '\0' || !is_number (arg1),
        "Mpoload - Bad syntax from vnum %d.", CH_VNUM (ch));

    if (arg2[0] == '\0')
        level = char_get_trust (ch);
    else {
        /* New feature from Alander. */
        BAIL_IF_BUG (!is_number (arg2),
            "Mpoload - Bad syntax from vnum %d.", CH_VNUM (ch));

        level = atoi (arg2);
        BAIL_IF_BUG (level < 0 || level > char_get_trust (ch),
            "Mpoload - Bad level from vnum %d.", CH_VNUM (ch));
    }

    /* Added 3rd argument
     * omitted - load to mobile's inventory
     * 'R'     - load to room
     * 'W'     - load to mobile and force wear */
    if (arg3[0] == 'R' || arg3[0] == 'r')
        to_room = TRUE;
    else if (arg3[0] == 'W' || arg3[0] == 'w')
        wear = TRUE;

    BAIL_IF_BUG ((obj_index = get_obj_index (atoi (arg1))) == NULL,
        "Mpoload - Bad vnum arg from vnum %d.", CH_VNUM (ch));

    obj = obj_create (obj_index, level);
    if ((wear || !to_room) && CAN_WEAR_FLAG (obj, ITEM_TAKE)) {
        obj_give_to_char (obj, ch);
        if (wear)
            char_wear_obj (ch, obj, TRUE);
    }
    else
        obj_give_to_room (obj, ch->in_room);
}

/* Lets the mobile purge all objects and other npcs in the room,
 * or purge a specified object or mob in the room. The mobile cannot
 * purge itself for safety reasons.
 *
 * syntax mob purge {target} */
DEFINE_DO_FUN (do_mppurge) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        /* 'purge' */
        CHAR_T *vnext;
        OBJ_T *obj_next;

        for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
            vnext = victim->next_in_room;
            if (IS_NPC (victim) && victim != ch
                && !IS_SET (victim->mob, MOB_NOPURGE))
                char_extract (victim, TRUE);
        }
        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (!IS_SET (obj->extra_flags, ITEM_NOPURGE))
                obj_extract (obj);
        }
        return;
    }
    if ((victim = find_char_same_room (ch, arg)) == NULL) {
        BAIL_IF_BUG ((obj = find_obj_here (ch, arg)) == NULL,
            "Mppurge - Bad argument from vnum %d.", CH_VNUM (ch));
        obj_extract (obj);
        return;
    }

    BAIL_IF_BUG (!IS_NPC (victim),
        "Mppurge - Purging a PC from vnum %d.", CH_VNUM (ch));
    char_extract (victim, TRUE);
}

/* Lets the mobile goto any location it wishes that is not private.
 * Syntax: mob goto [location] */
DEFINE_DO_FUN (do_mpgoto) {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_T *location;

    one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0',
        "Mpgoto - No argument from vnum %d.", CH_VNUM (ch));
    BAIL_IF_BUG ((location = find_location (ch, arg)) == NULL,
        "Mpgoto - No such location from vnum %d.", CH_VNUM (ch));

    if (ch->fighting != NULL)
        stop_fighting (ch, TRUE);

    char_from_room (ch);
    char_to_room (ch, location);
}

/* Lets the mobile do a command at another location.
 * Syntax: mob at [location] [commands] */
DEFINE_DO_FUN (do_mpat) {
    char arg[MAX_INPUT_LENGTH];
    ROOM_INDEX_T *location;
    ROOM_INDEX_T *original;
    CHAR_T *wch;
    OBJ_T *on;

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0' || argument[0] == '\0',
        "Mpat - Bad argument from vnum %d.", CH_VNUM (ch));
    BAIL_IF_BUG ((location = find_location (ch, arg)) == NULL,
        "Mpat - No such location from vnum %d.", CH_VNUM (ch));

    original = ch->in_room;
    on = ch->on;
    char_from_room (ch);
    char_to_room (ch, location);
    interpret (ch, argument);

    /* See if 'ch' still exists before continuing!
     * Handles 'at XXXX quit' case. */
    for (wch = char_list; wch != NULL; wch = wch->next) {
        if (wch == ch) {
            char_from_room (ch);
            char_to_room (ch, original);
            ch->on = on;
            break;
        }
    }
}

/* Lets the mobile transfer people.  The 'all' argument transfers
 *  everyone in the current room to the specified location
 *
 * Syntax: mob transfer [target|'all'] [location] */
DEFINE_DO_FUN (do_mptransfer) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    ROOM_INDEX_T *location;
    CHAR_T *victim;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF_BUG (arg1[0] == '\0',
        "Mptransfer - Bad syntax from vnum %d.", CH_VNUM (ch));

    if (!str_cmp (arg1, "all")) {
        CHAR_T *victim_next;

        for (victim = ch->in_room->people; victim != NULL;
             victim = victim_next)
        {
            victim_next = victim->next_in_room;
            if (!IS_NPC (victim)) {
                sprintf (buf, "%s %s", victim->name, arg2);
                do_mptransfer (ch, buf);
            }
        }
        return;
    }

    /* Thanks to Grodyn for the optional location parameter. */
    if (arg2[0] == '\0')
        location = ch->in_room;
    else {
        BAIL_IF_BUG ((location = find_location (ch, arg2)) == NULL,
            "Mptransfer - No such location from vnum %d.", CH_VNUM (ch));
        if (room_is_private (location))
            return;
    }

    if ((victim = find_char_world (ch, arg1)) == NULL)
        return;
    if (victim->in_room == NULL)
        return;

    if (victim->fighting != NULL)
        stop_fighting (victim, TRUE);
    char_from_room (victim);
    char_to_room (victim, location);
    do_look (victim, "auto");
}

/* Lets the mobile transfer all chars in same group as the victim.
 * Syntax: mob gtransfer [victim] [location] */
DEFINE_DO_FUN (do_mpgtransfer) {
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_T *who, *victim, *victim_next;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF_BUG (arg1[0] == '\0',
        "Mpgtransfer - Bad syntax from vnum %d.", CH_VNUM (ch));
    if ((who = find_char_same_room (ch, arg1)) == NULL)
        return;

    for (victim = ch->in_room->people; victim; victim = victim_next) {
        victim_next = victim->next_in_room;
        if (is_same_group (who, victim)) {
            sprintf (buf, "%s %s", victim->name, arg2);
            do_mptransfer (ch, buf);
        }
    }
}

/* Lets the mobile force someone to do something. Must be mortal level
 * and the all argument only affects those in the room with the mobile.
 *
 * Syntax: mob force [victim] [commands] */
DEFINE_DO_FUN (do_mpforce) {
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0' || argument[0] == '\0',
        "Mpforce - Bad syntax from vnum %d.", CH_VNUM (ch));

    if (!str_cmp (arg, "all")) {
        CHAR_T *vch;
        CHAR_T *vch_next;
        for (vch = char_list; vch != NULL; vch = vch_next) {
            vch_next = vch->next;
            if (char_get_trust (vch) < char_get_trust (ch) &&
                    char_can_see_in_room (ch, vch))
                interpret (vch, argument);
        }
    }
    else {
        CHAR_T *victim;
        if ((victim = find_char_same_room (ch, arg)) == NULL)
            return;
        if (victim == ch)
            return;
        interpret (victim, argument);
    }
}

/* Lets the mobile force a group something. Must be mortal level.
 * Syntax: mob gforce [victim] [commands] */
DEFINE_DO_FUN (do_mpgforce) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim, *vch, *vch_next;

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0' || argument[0] == '\0',
        "MpGforce - Bad syntax from vnum %d.", CH_VNUM (ch));

    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;
    if (victim == ch)
        return;

    for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
        vch_next = vch->next_in_room;
        if (is_same_group (victim, vch))
            interpret (vch, argument);
    }
}

/* Forces all mobiles of certain vnum to do something (except ch)
 * Syntax: mob vforce [vnum] [commands] */
DEFINE_DO_FUN (do_mpvforce) {
    CHAR_T *victim, *victim_next;
    char arg[MAX_INPUT_LENGTH];
    int vnum;

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0' || argument[0] == '\0',
        "MpVforce - Bad syntax from vnum %d.", CH_VNUM (ch));
    BAIL_IF_BUG (!is_number (arg),
        "MpVforce - Non-number argument vnum %d.", CH_VNUM (ch));
    vnum = atoi (arg);

    for (victim = char_list; victim; victim = victim_next) {
        victim_next = victim->next;
        if (IS_NPC (victim) && victim->index_data->vnum == vnum &&
                ch != victim && victim->fighting == NULL)
            interpret (victim, argument);
    }
}

/* Lets the mobile cast spells --
 * Beware: this does only crude checking on the target validity
 * and does not account for mana etc., so you should do all the
 * necessary checking in your mob program before issuing this cmd!
 *
 * Syntax: mob cast [spell] {target} */

DEFINE_DO_FUN (do_mpcast) {
    CHAR_T *vch;
    OBJ_T *obj;
    void *victim = NULL;
    char spell[MAX_INPUT_LENGTH], target[MAX_INPUT_LENGTH];
    int sn;

    argument = one_argument (argument, spell);
    one_argument (argument, target);

    BAIL_IF_BUG (spell[0] == '\0',
        "MpCast - Bad syntax from vnum %d.", CH_VNUM (ch));
    BAIL_IF_BUG ((sn = skill_lookup (spell)) < 0,
        "MpCast - No such spell from vnum %d.", CH_VNUM (ch));

    vch = find_char_same_room (ch, target);
    obj = find_obj_here (ch, target);

    switch (skill_table[sn].target) {
        case SKILL_TARGET_IGNORE:
            break;

        case SKILL_TARGET_CHAR_OFFENSIVE:
            if (vch == NULL || vch == ch)
                return;
            victim = (void *) vch;
            break;

        case SKILL_TARGET_CHAR_DEFENSIVE:
            victim = vch == NULL ? (void *) ch : (void *) vch;
            break;

        case SKILL_TARGET_CHAR_SELF:
            victim = (void *) ch;
            break;

        case SKILL_TARGET_OBJ_CHAR_DEF:
        case SKILL_TARGET_OBJ_CHAR_OFF:
        case SKILL_TARGET_OBJ_INV:
            if (obj == NULL)
                return;
            victim = (void *) obj;
            break;

        default:
            return;
    }
    (*skill_table[sn].spell_fun) (sn, ch->level, ch, victim,
        skill_table[sn].target, "");
}

/* Lets mob cause unconditional damage to someone. Nasty, use with caution.
 * Also, this is silent, you must show your own damage message...
 * Syntax: mob damage [victim] [min] [max] {kill} */

DEFINE_DO_FUN (do_mpdamage) {
    CHAR_T *victim = NULL, *victim_next;
    char target[MAX_INPUT_LENGTH],
        min[MAX_INPUT_LENGTH], max[MAX_INPUT_LENGTH];
    int low, high;
    bool all = FALSE, kill = FALSE;

    argument = one_argument (argument, target);
    argument = one_argument (argument, min);
    argument = one_argument (argument, max);

    BAIL_IF_BUG (target[0] == '\0',
        "MpDamage - Bad syntax from vnum %d.", CH_VNUM (ch));

    if (!str_cmp (target, "all"))
        all = TRUE;
    else if ((victim = find_char_same_room (ch, target)) == NULL)
        return;

    BAIL_IF_BUG (!is_number (min),
        "MpDamage - Bad damage min vnum %d.", CH_VNUM (ch));
    low = atoi (min);

    BAIL_IF_BUG (!is_number (max),
        "MpDamage - Bad damage max vnum %d.", CH_VNUM (ch));
    high = atoi (max);

    one_argument (argument, target);

    /* If kill parameter is omitted, this command is "safe" and will not
     * kill the victim.  */
    if (target[0] != '\0')
        kill = TRUE;
    if (all) {
        for (victim = ch->in_room->people; victim; victim = victim_next) {
            victim_next = victim->next_in_room;
            if (victim != ch) {
                damage_quiet (victim, victim,
                    kill ? number_range (low, high)
                          : UMIN (victim->hit, number_range (low, high)),
                    ATTACK_DEFAULT, DAM_NONE);
            }
        }
    }
    else {
        damage_quiet (victim, victim,
            kill ? number_range (low, high)
                  : UMIN (victim->hit, number_range (low, high)),
            ATTACK_DEFAULT, DAM_NONE);
    }
}

/* Lets the mobile to remember a target. The target can be referred to
 * with $q and $Q codes in MOBprograms. See also "mob forget".
 *
 * Syntax: mob remember [victim] */
DEFINE_DO_FUN (do_mpremember) {
    char arg[MAX_INPUT_LENGTH];
    one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0',
        "do_mpremember: missing argument from vnum %d.", CH_VNUM (ch));
    ch->mprog_target = find_char_world (ch, arg);
}

/* Reverse of "mob remember".
 * Syntax: mob forget */
DEFINE_DO_FUN (do_mpforget) {
    ch->mprog_target = NULL;
}

/* Sets a delay for MOBprogram execution. When the delay time expires,
 * the mobile is checked for a MObprogram with DELAY trigger, and if
 * one is found, it is executed. Delay is counted in PULSE_MOBILE
 *
 * Syntax: mob delay [pulses] */
DEFINE_DO_FUN (do_mpdelay) {
    char arg[MAX_INPUT_LENGTH];
    one_argument (argument, arg);
    BAIL_IF_BUG (!is_number (arg),
        "do_mp_delay: invalid arg from vnum %d.", CH_VNUM (ch));
    ch->mprog_delay = atoi (arg);
}

/* Reverse of "mob delay", deactivates the timer.
 * Syntax: mob cancel */
DEFINE_DO_FUN (do_mpcancel) {
    ch->mprog_delay = -1;
}

/* Lets the mobile to call another MOBprogram withing a MOBprogram.
 * This is a crude way to implement subroutines/functions. Beware of
 * nested loops and unwanted triggerings... Stack usage might be a problem.
 * Characters and objects referred to must be in the same room with the
 * mobile.
 *
 * Syntax: mob call [vnum] [victim|'null'] [object1|'null'] [object2|'null'] */
DEFINE_DO_FUN (do_mpcall) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *vch;
    OBJ_T *obj1, *obj2;
    MPROG_CODE_T *prg;
    extern void program_flow (sh_int, char *, CHAR_T *, CHAR_T *,
                              const void *, const void *);

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0',
        "do_mpcall: missing arguments from vnum %d.", CH_VNUM (ch));
    BAIL_IF_BUG ((prg = get_mprog_index (atoi (arg))) == NULL,
        "do_mpcall: invalid prog from vnum %d.", CH_VNUM (ch));

    vch = NULL;
    obj1 = obj2 = NULL;
    argument = one_argument (argument, arg);
    if (arg[0] != '\0')
        vch = find_char_same_room (ch, arg);
    argument = one_argument (argument, arg);
    if (arg[0] != '\0')
        obj1 = find_obj_here (ch, arg);
    argument = one_argument (argument, arg);
    if (arg[0] != '\0')
        obj2 = find_obj_here (ch, arg);
    program_flow (prg->vnum, prg->code, ch, vch, (void *) obj1,
                  (void *) obj2);
}

/* Forces the mobile to flee.
 * Syntax: mob flee */
DEFINE_DO_FUN (do_mpflee) {
    ROOM_INDEX_T *was_in;
    EXIT_T *pexit;
    int door, attempt;

    if (ch->fighting != NULL)
        return;

    if ((was_in = ch->in_room) == NULL)
        return;

    for (attempt = 0; attempt < 6; attempt++) {
        door = number_door ();
        if ((pexit = was_in->exit[door]) == 0
            || pexit->to_room == NULL
            || IS_SET (pexit->exit_flags, EX_CLOSED) ||
            (IS_NPC (ch) && IS_SET (pexit->to_room->room_flags, ROOM_NO_MOB)))
            continue;

        char_move (ch, door, FALSE);
        if (ch->in_room != was_in)
            return;
    }
}

/* Lets the mobile to transfer an object. The object must be in the same
 * room with the mobile.
 *
 * Syntax: mob otransfer [item name] [location] */
DEFINE_DO_FUN (do_mpotransfer) {
    OBJ_T *obj;
    ROOM_INDEX_T *location;
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    BAIL_IF_BUG (arg[0] == '\0',
        "MpOTransfer - Missing argument from vnum %d.", CH_VNUM (ch));

    one_argument (argument, buf);
    BAIL_IF_BUG ((location = find_location (ch, buf)) == NULL,
        "MpOTransfer - No such location from vnum %d.", CH_VNUM (ch));

    if ((obj = find_obj_here (ch, arg)) == NULL)
        return;
    if (obj->carried_by == NULL)
        obj_take_from_room (obj);
    else {
        if (obj->wear_loc != WEAR_NONE)
            char_unequip_obj (ch, obj);
        obj_take_from_char (obj);
    }
    obj_give_to_room (obj, location);
}

/* Lets the mobile to strip an object or all objects from the victim.
 * Useful for removing e.g. quest objects from a character.
 *
 * Syntax: mob remove [victim] [object vnum|'all'] */
DEFINE_DO_FUN (do_mpremove) {
    CHAR_T *victim;
    OBJ_T *obj, *obj_next;
    sh_int vnum = 0;
    bool all = FALSE;
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    if ((victim = find_char_same_room (ch, arg)) == NULL)
        return;

    one_argument (argument, arg);
    if (!str_cmp (arg, "all"))
        all = TRUE;
    else {
        BAIL_IF_BUG (!is_number (arg),
            "do_mpremove: Invalid object from vnum %d.", CH_VNUM (ch));
        vnum = atoi (arg);
    }

    for (obj = victim->carrying; obj; obj = obj_next) {
        obj_next = obj->next_content;
        if (all || obj->index_data->vnum == vnum) {
            char_unequip_obj (ch, obj);
            obj_take_from_char (obj);
            obj_extract (obj);
        }
    }
}
