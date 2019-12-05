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
#include <stdlib.h>

#include "db.h"
#include "comm.h"
#include "interp.h"
#include "save.h"
#include "fight.h"
#include "utils.h"
#include "lookup.h"
#include "skills.h"
#include "affects.h"
#include "chars.h"
#include "objs.h"
#include "find.h"
#include "recycle.h"
#include "descs.h"
#include "globals.h"

#include "wiz_l4.h"

DEFINE_DO_FUN (do_guild) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    char *cname;
    int clan_index;
    const CLAN_T *clan;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0' || arg2[0] == '\0',
        "Syntax: guild <char> none|<clan>\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg1)) == NULL,
        "They aren't playing.\n\r", ch);

    if (!str_cmp (arg2, "none")) {
        send_to_char ("They are now clanless.\n\r", ch);
        send_to_char ("You are now a member of no clan!\n\r", victim);
        victim->clan = 0;
        return;
    }

    BAIL_IF ((clan_index = clan_lookup (arg2)) < 0,
        "No such clan exists.\n\r", ch);
    clan = &(clan_table[clan_index]);

    if (clan->independent) {
        cname = clan->name;
        printf_to_char (ch,    "They are now a %s.\n\r", cname);
        printf_to_char (victim, "You are now a %s.\n\r", cname);
    }
    else {
        cname = capitalize (clan->name);
        printf_to_char (ch,    "They are now a member of clan %s.\n\r", cname);
        printf_to_char (victim, "You are now a member of clan %s.\n\r", cname);
    }

    victim->clan = clan_index;
}

DEFINE_DO_FUN (do_sockets) {
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_T *d;
    int count;

    count = 0;
    buf[0] = '\0';

    one_argument (argument, arg);
    for (d = descriptor_list; d != NULL; d = d->next) {
        if (d->character != NULL && char_can_see_anywhere (ch, d->character)
            && (arg[0] == '\0' || is_name (arg, d->character->name)
                || (d->original && is_name (arg, d->original->name))))
        {
            count++;
            sprintf (buf + strlen (buf), "[%3d %2d] %s@%s\n\r",
                d->descriptor, d->connected,
                d->original
                    ? d->original->name
                    : d->character ? d->character->name : "(none)",
                d->host);
        }
    }
    BAIL_IF (count == 0,
        "No one by that name is connected.\n\r", ch);

    sprintf (buf2, "%d user%s\n\r", count, count == 1 ? "" : "s");
    strcat (buf, buf2);
    page_to_char (buf, ch);
}

DEFINE_DO_FUN (do_flag) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
        arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    flag_t *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const FLAG_T *flag_table;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);
    argument = one_argument (argument, arg3);

    type = argument[0];

    if (type == '=' || type == '-' || type == '+')
        argument = one_argument (argument, word);

    if (arg1[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  flag mob  <name> <field> <flags>\n\r", ch);
        send_to_char ("  flag char <name> <field> <flags>\n\r", ch);
        send_to_char ("  mob  flags: mob, aff, off, imm, res, vuln, form, part\n\r", ch);
        send_to_char ("  char flags: plr, comm, aff, imm, res, vuln\n\r", ch);
        send_to_char ("  +: add flag, -: remove flag, = set equal to\n\r", ch);
        send_to_char ("  otherwise flag toggles the flags listed.\n\r", ch);
        return;
    }

    BAIL_IF (!(!str_prefix (arg1, "mob") || !str_prefix (arg1, "char")),
        "Must specify 'mob' or 'char'.\n\r", ch);
    BAIL_IF (arg2[0] == '\0',
        "What do you wish to set flags on?\n\r", ch);
    BAIL_IF (arg3[0] == '\0',
        "You need to specify a flag to set.\n\r", ch);
    BAIL_IF (argument[0] == '\0',
        "Which flags do you wish to change?\n\r", ch);

    victim = find_char_world (ch, arg2);
    BAIL_IF (victim == NULL,
        "You can't find them.\n\r", ch);

    /* select a flag to set */
    if (!str_prefix (arg3, "mob")) {
        BAIL_IF (!IS_NPC (victim),
            "Use plr for PCs.\n\r", ch);
        flag = &victim->mob;
        flag_table = mob_flags;
    }
    else if (!str_prefix (arg3, "plr")) {
        BAIL_IF (IS_NPC (victim),
            "Use act for NPCs.\n\r", ch);
        flag = &victim->plr;
        flag_table = plr_flags;
    }
    else if (!str_prefix (arg3, "aff")) {
        flag = &victim->affected_by;
        flag_table = affect_flags;
    }
    else if (!str_prefix (arg3, "immunity")) {
        flag = &victim->imm_flags;
        flag_table = res_flags;
    }
    else if (!str_prefix (arg3, "resist")) {
        flag = &victim->res_flags;
        flag_table = res_flags;
    }
    else if (!str_prefix (arg3, "vuln")) {
        flag = &victim->vuln_flags;
        flag_table = res_flags;
    }
    else if (!str_prefix (arg3, "form")) {
        BAIL_IF (!IS_NPC (victim),
            "Form can't be set on PCs.\n\r", ch);
        flag = &victim->form;
        flag_table = form_flags;
    }
    else if (!str_prefix (arg3, "parts")) {
        BAIL_IF (!IS_NPC (victim),
            "Parts can't be set on PCs.\n\r", ch);
        flag = &victim->parts;
        flag_table = part_flags;
    }
    else if (!str_prefix (arg3, "comm")) {
        BAIL_IF (IS_NPC (victim),
            "Comm can't be set on NPCs.\n\r", ch);
        flag = &victim->comm;
        flag_table = comm_flags;
    }
    else {
        send_to_char ("That's not an acceptable flag.\n\r", ch);
        return;
    }

    old = *flag;
    victim->zone = NULL;

    if (type != '=')
        new = old;

    /* mark the words */
    while (1) {
        argument = one_argument (argument, word);
        if (word[0] == '\0')
            break;

        pos = flag_lookup (word, flag_table);
        BAIL_IF (pos < 0,
            "That flag doesn't exist!\n\r", ch);
        SET_BIT (marked, pos);
    }

    for (pos = 0; flag_table[pos].name != NULL; pos++) {
        if (!flag_table[pos].settable
            && IS_SET (old, flag_table[pos].bit))
        {
            SET_BIT (new, flag_table[pos].bit);
            continue;
        }

        if (IS_SET (marked, flag_table[pos].bit)) {
            switch (type) {
                case '=':
                case '+':
                    SET_BIT (new, flag_table[pos].bit);
                    break;
                case '-':
                    REMOVE_BIT (new, flag_table[pos].bit);
                    break;
                default:
                    if (IS_SET (new, flag_table[pos].bit))
                        REMOVE_BIT (new, flag_table[pos].bit);
                    else
                        SET_BIT (new, flag_table[pos].bit);
            }
        }
    }
    *flag = new;
}

DEFINE_DO_FUN (do_freeze) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Freeze whom?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);
    BAIL_IF (IS_NPC (victim),
        "Not on NPC's.\n\r", ch);
    BAIL_IF (char_get_trust (victim) >= char_get_trust (ch),
        "You failed.\n\r", ch);
    if (IS_SET (victim->plr, PLR_FREEZE)) {
        REMOVE_BIT (victim->plr, PLR_FREEZE);
        send_to_char ("You can play again.\n\r", victim);
        send_to_char ("FREEZE removed.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N thaws %s.", victim->name);
    }
    else {
        SET_BIT (victim->plr, PLR_FREEZE);
        send_to_char ("You can't do ANYthing!\n\r", victim);
        send_to_char ("FREEZE set.\n\r", ch);
        wiznetf (ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0,
            "$N puts %s in the deep freeze.", victim->name);
    }

    save_char_obj (victim);
}

/* RT to replace the two load commands */
DEFINE_DO_FUN (do_load) {
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  load mob <vnum>\n\r", ch);
        send_to_char ("  load obj <vnum> <level>\n\r", ch);
        return;
    }
    BAIL_IF_EXPR (!str_cmp (arg, "mob") || !str_cmp (arg, "char"),
        do_function (ch, &do_mload, argument));
    BAIL_IF_EXPR (!str_cmp (arg, "obj"),
        do_function (ch, &do_oload, argument));

    /* echo syntax */
    do_function (ch, &do_load, "");
}

DEFINE_DO_FUN (do_mload) {
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_T *pMobIndex;
    CHAR_T *victim;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0' || !is_number (arg),
        "Syntax: load mob <vnum>.\n\r", ch);
    BAIL_IF ((pMobIndex = get_mob_index (atoi (arg))) == NULL,
        "No mob has that vnum.\n\r", ch);

    victim = char_create_mobile (pMobIndex);
    char_to_room (victim, ch->in_room);
    act ("$n has created $N!", ch, NULL, victim, TO_NOTCHAR);
    wiznetf (ch, NULL, WIZ_LOAD, WIZ_SECURE, char_get_trust (ch),
        "$N loads %s.", victim->short_descr);
    send_to_char ("Ok.\n\r", ch);
}

DEFINE_DO_FUN (do_oload) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_T *pObjIndex;
    OBJ_T *obj;
    int level;

    argument = one_argument (argument, arg1);
    one_argument (argument, arg2);

    BAIL_IF (arg1[0] == '\0' || !is_number (arg1),
        "Syntax: load obj <vnum> <level>.\n\r", ch);

    level = char_get_trust (ch); /* default */
    if (arg2[0] != '\0') {
        /* load with a level */
        BAIL_IF (!is_number (arg2),
            "Syntax: oload <vnum> <level>.\n\r", ch);
        level = atoi (arg2);
        BAIL_IF (level < 0 || level > char_get_trust (ch),
            "Level must be be between 0 and your level.\n\r", ch);
    }

    BAIL_IF ((pObjIndex = get_obj_index (atoi (arg1))) == NULL,
        "No object has that vnum.\n\r", ch);

    obj = obj_create (pObjIndex, level);
    if (CAN_WEAR_FLAG (obj, ITEM_TAKE))
        obj_to_char (obj, ch);
    else
        obj_to_room (obj, ch->in_room);
    send_to_char ("Ok.\n\r", ch);
    act ("$n has created $p!", ch, obj, NULL, TO_NOTCHAR);
    wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, char_get_trust (ch));
}

DEFINE_DO_FUN (do_pecho) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;

    argument = one_argument (argument, arg);

    BAIL_IF (argument[0] == '\0' || arg[0] == '\0',
        "Personal echo what?\n\r", ch);
    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "Target not found.\n\r", ch);

    if (char_get_trust (victim) >= char_get_trust (ch) &&
            char_get_trust (ch) != MAX_LEVEL)
        send_to_char ("personal> ", victim);

    printf_to_char (victim, "%s\n\r", argument);
    printf_to_char (ch, "personal> %s\n\r", argument);
}

DEFINE_DO_FUN (do_purge) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    OBJ_T *obj;
    DESCRIPTOR_T *d;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        /* 'purge' */
        CHAR_T *vnext;
        OBJ_T *obj_next;

        for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
            vnext = victim->next_in_room;
            if (IS_NPC (victim) && !IS_SET (victim->mob, MOB_NOPURGE)
                && victim != ch /* safety precaution */ )
                char_extract (victim, TRUE);
        }
        for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
            obj_next = obj->next_content;
            if (!IS_OBJ_STAT (obj, ITEM_NOPURGE))
                obj_extract (obj);
        }

        send_to_char ("Ok.\n\r", ch);
        act ("$n purges the room!", ch, NULL, NULL, TO_NOTCHAR);
        return;
    }

    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);

    if (!IS_NPC (victim)) {
        BAIL_IF (ch == victim,
            "Ho ho ho.\n\r", ch);

        if (char_get_trust (ch) <= char_get_trust (victim)) {
            send_to_char ("Maybe that wasn't a good idea...\n\r", ch);
            printf_to_char (victim, "%s tried to purge you!\n\r", ch->name);
            return;
        }

        act3 ("You disintegrate $N.",
              "$n disintegrates you.",
              "$n disintegrates $N.", ch, 0, victim, 0, POS_RESTING);

        if (victim->level > 1)
            save_char_obj (victim);
        d = victim->desc;
        char_extract (victim, TRUE);
        if (d != NULL)
            close_socket (d);
    }

    act3 ("You purge $N.", "$n purges you.", "$n purges $N.",
        ch, 0, victim, 0, POS_RESTING);
    char_extract (victim, TRUE);
}

void do_restore_single (CHAR_T *ch, CHAR_T *vch) {
    affect_strip (vch, gsn_plague);
    affect_strip (vch, gsn_poison);
    affect_strip (vch, gsn_blindness);
    affect_strip (vch, gsn_sleep);
    affect_strip (vch, gsn_curse);

    vch->hit = vch->max_hit;
    vch->mana = vch->max_mana;
    vch->move = vch->max_move;
    update_pos (vch);
    if (vch->in_room != NULL)
        act ("$n has restored you.", ch, NULL, vch, TO_VICT);
}

DEFINE_DO_FUN (do_restore) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_T *victim;
    DESCRIPTOR_T *d;

    one_argument (argument, arg);
    if (arg[0] == '\0' || !str_cmp (arg, "room")) {
        /* cure room */
        for (victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room)
            do_restore_single (ch, victim);

        wiznetf (ch, NULL, WIZ_RESTORE, WIZ_SECURE, char_get_trust (ch),
            "$N restored room %d.", ch->in_room->vnum);
        send_to_char ("Room restored.\n\r", ch);
        return;
    }

    if (char_get_trust (ch) >= MAX_LEVEL - 1 && !str_cmp (arg, "all")) {
        /* cure all */
        for (d = descriptor_list; d != NULL; d = d->next) {
            victim = d->character;
            if (victim == NULL || IS_NPC (victim))
                continue;
            do_restore_single (ch, victim);
        }

        wiznet ("$N restored all players.", ch, NULL, WIZ_RESTORE, WIZ_SECURE,
            char_get_trust (ch));
        send_to_char ("All active players restored.\n\r", ch);
        return;
    }

    BAIL_IF ((victim = find_char_world (ch, arg)) == NULL,
        "They aren't here.\n\r", ch);

    do_restore_single (ch, victim);

    wiznetf (ch, NULL, WIZ_RESTORE, WIZ_SECURE, char_get_trust (ch),
        "$N restored %s", PERS (victim));
    printf_to_char (ch, "You restored %s.\n\r", PERS (victim));
}

DEFINE_DO_FUN (do_echo) {
    DESCRIPTOR_T *d;

    BAIL_IF (argument[0] == '\0',
        "Global echo what?\n\r", ch);

    for (d = descriptor_list; d; d = d->next)
        if (d->connected == CON_PLAYING)
            echo_to_char (d->character, ch, "global", argument);
}

/* ofind and mfind replaced with vnum, vnum skill also added */
DEFINE_DO_FUN (do_vnum) {
    char arg[MAX_INPUT_LENGTH];
    char *string;

    string = one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  vnum obj <name>\n\r", ch);
        send_to_char ("  vnum mob <name>\n\r", ch);
        send_to_char ("  vnum skill <skill or spell>\n\r", ch);
        return;
    }

    BAIL_IF_EXPR (!str_cmp (arg, "obj"),
        do_function (ch, &do_ofind, string));
    BAIL_IF_EXPR (!str_cmp (arg, "mob") || !str_cmp (arg, "char"),
        do_function (ch, &do_mfind, string));
    BAIL_IF_EXPR (!str_cmp (arg, "skill") || !str_cmp (arg, "spell"),
        do_function (ch, &do_slookup, string));

    /* do both (not skills) */
    do_function (ch, &do_mfind, argument);
    do_function (ch, &do_ofind, argument);
}

DEFINE_DO_FUN (do_slookup) {
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Lookup which skill or spell?\n\r", ch);

    if (!str_cmp (arg, "all")) {
        for (sn = 0; sn < SKILL_MAX && skill_table[sn].name; sn++)
            printf_to_char (ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                sn, skill_table[sn].slot, skill_table[sn].name);
        return;
    }

    BAIL_IF ((sn = skill_lookup (arg)) < 0,
        "No such skill or spell.\n\r", ch);
    printf_to_char (ch, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
        sn, skill_table[sn].slot, skill_table[sn].name);
}

DEFINE_DO_FUN (do_mfind) {
    char arg[MAX_INPUT_LENGTH];
    const MOB_INDEX_T *mob;
    int matches = 0;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Find whom?\n\r", ch);

    for (mob = mob_index_get_first(); mob; mob = mob_index_get_next (mob)) {
        if (is_name (argument, mob->name)) {
            printf_to_char (ch, "[%5d] %s\n\r", mob->vnum, mob->short_descr);
            matches++;
        }
    }
    if (matches == 0)
        send_to_char ("No mobiles by that name.\n\r", ch);
    else
        printf_to_char (ch, "Found %d mobiles.\n\r", matches);
}

DEFINE_DO_FUN (do_ofind) {
    char arg[MAX_INPUT_LENGTH];
    const OBJ_INDEX_T *obj;
    int matches = 0;

    one_argument (argument, arg);
    BAIL_IF (arg[0] == '\0',
        "Find what?\n\r", ch);

    for (obj = obj_index_get_first(); obj; obj = obj_index_get_next (obj)) {
        if (is_name (argument, obj->name)) {
            printf_to_char (ch, "[%5d] %s\n\r", obj->vnum, obj->short_descr);
            matches++;
        }
    }
    if (matches == 0)
        send_to_char ("No objects by that name.\n\r", ch);
    else
        printf_to_char (ch, "Found %d objects.\n\r", matches);
}

DEFINE_DO_FUN (do_zecho) {
    DESCRIPTOR_T *d;
    CHAR_T *vch;

    BAIL_IF (argument[0] == '\0',
        "Zone echo what?\n\r", ch);
    BAIL_IF (ch->in_room == NULL,
        "Good idea - if only you were in a zone...\n\r", ch);

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;
        if ((vch = d->character) == NULL)
            continue;
        if (vch->in_room == NULL)
            continue;
        if (vch->in_room->area != ch->in_room->area)
            continue;
        echo_to_char (vch, ch, "zone", argument);
    }
}
