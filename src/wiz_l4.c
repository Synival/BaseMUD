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

#include "wiz_l4.h"

/* TODO: do_flag() is just ugly in general. */
/* TODO: review most of these functions and test them thoroughly. */
/* TODO: BAIL_IF() clauses. */
/* TODO: employ tables whenever possible */

void do_guild (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    int clan;

    argument = one_argument (argument, arg1);
    argument = one_argument (argument, arg2);

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        send_to_char ("Syntax: guild <char> <cln name>\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, arg1)) == NULL) {
        send_to_char ("They aren't playing.\n\r", ch);
        return;
    }

    if (!str_prefix (arg2, "none")) {
        send_to_char ("They are now clanless.\n\r", ch);
        send_to_char ("You are now a member of no clan!\n\r", victim);
        victim->clan = 0;
        return;
    }

    if ((clan = clan_lookup (arg2)) == 0) {
        send_to_char ("No such clan exists.\n\r", ch);
        return;
    }

    if (clan_table[clan].independent) {
        sprintf (buf, "They are now a %s.\n\r", clan_table[clan].name);
        send_to_char (buf, ch);
        sprintf (buf, "You are now a %s.\n\r", clan_table[clan].name);
        send_to_char (buf, victim);
    }
    else {
        sprintf (buf, "They are now a member of clan %s.\n\r",
                 capitalize (clan_table[clan].name));
        send_to_char (buf, ch);
        sprintf (buf, "You are now a member of clan %s.\n\r",
                 capitalize (clan_table[clan].name));
    }

    victim->clan = clan;
}

void do_sockets (CHAR_DATA * ch, char *argument) {
    char buf[2 * MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    DESCRIPTOR_DATA *d;
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
                     d->descriptor,
                     d->connected,
                     d->original ? d->original->name :
                     d->character ? d->character->name : "(none)", d->host);
        }
    }
    if (count == 0) {
        send_to_char ("No one by that name is connected.\n\r", ch);
        return;
    }

    sprintf (buf2, "%d user%s\n\r", count, count == 1 ? "" : "s");
    strcat (buf, buf2);
    page_to_char (buf, ch);
}

void do_flag (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH],
        arg3[MAX_INPUT_LENGTH];
    char word[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    flag_t *flag, old = 0, new = 0, marked = 0, pos;
    char type;
    const FLAG_TYPE *flag_table;

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
        send_to_char ("  mob  flags: act, aff, off, imm, res, vuln, form, part\n\r", ch);
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
    if (!str_prefix (arg3, "act")) {
        BAIL_IF (!IS_NPC (victim),
            "Use plr for PCs.\n\r", ch);
        flag = &victim->act;
        flag_table = act_flags;
    }
    else if (!str_prefix (arg3, "plr")) {
        BAIL_IF (IS_NPC (victim),
            "Use act for NPCs.\n\r", ch);
        flag = &victim->act;
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

void do_freeze (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Freeze whom?\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }
    if (IS_NPC (victim)) {
        send_to_char ("Not on NPC's.\n\r", ch);
        return;
    }
    if (char_get_trust (victim) >= char_get_trust (ch)) {
        send_to_char ("You failed.\n\r", ch);
        return;
    }
    if (IS_SET (victim->act, PLR_FREEZE)) {
        REMOVE_BIT (victim->act, PLR_FREEZE);
        send_to_char ("You can play again.\n\r", victim);
        send_to_char ("FREEZE removed.\n\r", ch);
        sprintf (buf, "$N thaws %s.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
    else {
        SET_BIT (victim->act, PLR_FREEZE);
        send_to_char ("You can't do ANYthing!\n\r", victim);
        send_to_char ("FREEZE set.\n\r", ch);
        sprintf (buf, "$N puts %s in the deep freeze.", victim->name);
        wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

    save_char_obj (victim);
}

/* RT to replace the two load commands */
void do_load (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Syntax:\n\r", ch);
        send_to_char ("  load mob <vnum>\n\r", ch);
        send_to_char ("  load obj <vnum> <level>\n\r", ch);
        return;
    }
    if (!str_cmp (arg, "mob") || !str_cmp (arg, "char")) {
        do_function (ch, &do_mload, argument);
        return;
    }
    if (!str_cmp (arg, "obj")) {
        do_function (ch, &do_oload, argument);
        return;
    }

    /* echo syntax */
    do_function (ch, &do_load, "");
}

void do_mload (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    CHAR_DATA *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument (argument, arg);
    if (arg[0] == '\0' || !is_number (arg)) {
        send_to_char ("Syntax: load mob <vnum>.\n\r", ch);
        return;
    }

    if ((pMobIndex = get_mob_index (atoi (arg))) == NULL) {
        send_to_char ("No mob has that vnum.\n\r", ch);
        return;
    }

    victim = create_mobile (pMobIndex);
    char_to_room (victim, ch->in_room);
    sprintf (buf, "$N loads %s.", victim->short_descr);
    act ("$n has created $N!", ch, NULL, victim, TO_NOTCHAR);
    wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, char_get_trust (ch));
    send_to_char ("Ok.\n\r", ch);
}

void do_oload (CHAR_DATA * ch, char *argument) {
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    OBJ_DATA *obj;
    int level;

    argument = one_argument (argument, arg1);
    one_argument (argument, arg2);

    if (arg1[0] == '\0' || !is_number (arg1)) {
        send_to_char ("Syntax: load obj <vnum> <level>.\n\r", ch);
        return;
    }

    level = char_get_trust (ch); /* default */
    if (arg2[0] != '\0') {
        /* load with a level */
        if (!is_number (arg2)) {
            send_to_char ("Syntax: oload <vnum> <level>.\n\r", ch);
            return;
        }
        level = atoi (arg2);
        if (level < 0 || level > char_get_trust (ch)) {
            send_to_char ("Level must be be between 0 and your level.\n\r", ch);
            return;
        }
    }

    if ((pObjIndex = get_obj_index (atoi (arg1))) == NULL) {
        send_to_char ("No object has that vnum.\n\r", ch);
        return;
    }

    obj = create_object (pObjIndex, level);
    if (CAN_WEAR (obj, ITEM_TAKE))
        obj_to_char (obj, ch);
    else
        obj_to_room (obj, ch->in_room);
    send_to_char ("Ok.\n\r", ch);
    act ("$n has created $p!", ch, obj, NULL, TO_NOTCHAR);
    wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, char_get_trust (ch));
}

void do_pecho (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;

    argument = one_argument (argument, arg);

    if (argument[0] == '\0' || arg[0] == '\0') {
        send_to_char ("Personal echo what?\n\r", ch);
        return;
    }
    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("Target not found.\n\r", ch);
        return;
    }

    if (char_get_trust (victim) >= char_get_trust (ch) &&
            char_get_trust (ch) != MAX_LEVEL)
        send_to_char ("personal> ", victim);

    send_to_char (argument, victim);
    send_to_char ("\n\r", victim);
    send_to_char ("personal> ", ch);
    send_to_char (argument, ch);
    send_to_char ("\n\r", ch);
}

void do_purge (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_DATA *victim;
    OBJ_DATA *obj;
    DESCRIPTOR_DATA *d;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        /* 'purge' */
        CHAR_DATA *vnext;
        OBJ_DATA *obj_next;

        for (victim = ch->in_room->people; victim != NULL; victim = vnext) {
            vnext = victim->next_in_room;
            if (IS_NPC (victim) && !IS_SET (victim->act, ACT_NOPURGE)
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

    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }
    if (!IS_NPC (victim)) {
        if (ch == victim) {
            send_to_char ("Ho ho ho.\n\r", ch);
            return;
        }
        if (char_get_trust (ch) <= char_get_trust (victim)) {
            send_to_char ("Maybe that wasn't a good idea...\n\r", ch);
            sprintf (buf, "%s tried to purge you!\n\r", ch->name);
            send_to_char (buf, victim);
            return;
        }

        act ("$n disintegrates $N.", ch, 0, victim, TO_OTHERS);

        if (victim->level > 1)
            save_char_obj (victim);
        d = victim->desc;
        char_extract (victim, TRUE);
        if (d != NULL)
            close_socket (d);
    }

    act ("$n purges $N.", ch, NULL, victim, TO_OTHERS);
    char_extract (victim, TRUE);
    return;
}

void do_restore (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    CHAR_DATA *victim;
    CHAR_DATA *vch;
    DESCRIPTOR_DATA *d;

    one_argument (argument, arg);
    if (arg[0] == '\0' || !str_cmp (arg, "room")) {
        /* cure room */
        for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
            affect_strip (vch, gsn_plague);
            affect_strip (vch, gsn_poison);
            affect_strip (vch, gsn_blindness);
            affect_strip (vch, gsn_sleep);
            affect_strip (vch, gsn_curse);

            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            update_pos (vch);
            act ("$n has restored you.", ch, NULL, vch, TO_VICT);
        }

        sprintf (buf, "$N restored room %d.", ch->in_room->vnum);
        wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, char_get_trust (ch));

        send_to_char ("Room restored.\n\r", ch);
        return;

    }

    if (char_get_trust (ch) >= MAX_LEVEL - 1 && !str_cmp (arg, "all")) {
        /* cure all */
        for (d = descriptor_list; d != NULL; d = d->next) {
            victim = d->character;

            if (victim == NULL || IS_NPC (victim))
                continue;

            affect_strip (victim, gsn_plague);
            affect_strip (victim, gsn_poison);
            affect_strip (victim, gsn_blindness);
            affect_strip (victim, gsn_sleep);
            affect_strip (victim, gsn_curse);

            victim->hit = victim->max_hit;
            victim->mana = victim->max_mana;
            victim->move = victim->max_move;
            update_pos (victim);
            if (victim->in_room != NULL)
                act ("$n has restored you.", ch, NULL, victim, TO_VICT);
        }
        send_to_char ("All active players restored.\n\r", ch);
        return;
    }

    if ((victim = find_char_world (ch, arg)) == NULL) {
        send_to_char ("They aren't here.\n\r", ch);
        return;
    }

    affect_strip (victim, gsn_plague);
    affect_strip (victim, gsn_poison);
    affect_strip (victim, gsn_blindness);
    affect_strip (victim, gsn_sleep);
    affect_strip (victim, gsn_curse);
    victim->hit = victim->max_hit;
    victim->mana = victim->max_mana;
    victim->move = victim->max_move;
    update_pos (victim);
    act ("$n has restored you.", ch, NULL, victim, TO_VICT);
    sprintf (buf, "$N restored %s",
             IS_NPC (victim) ? victim->short_descr : victim->name);
    wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, char_get_trust (ch));
    send_to_char ("Ok.\n\r", ch);
}

void do_echo (CHAR_DATA * ch, char *argument) {
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
        send_to_char ("Global echo what?\n\r", ch);
        return;
    }
    for (d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING) {
            if (char_get_trust (d->character) >= char_get_trust (ch))
                send_to_char ("global> ", d->character);
            send_to_char (argument, d->character);
            send_to_char ("\n\r", d->character);
        }
    }
}

/* ofind and mfind replaced with vnum, vnum skill also added */
void do_vnum (CHAR_DATA * ch, char *argument) {
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

    if (!str_cmp (arg, "obj")) {
        do_function (ch, &do_ofind, string);
        return;
    }
    if (!str_cmp (arg, "mob") || !str_cmp (arg, "char")) {
        do_function (ch, &do_mfind, string);
        return;
    }
    if (!str_cmp (arg, "skill") || !str_cmp (arg, "spell")) {
        do_function (ch, &do_slookup, string);
        return;
    }

    /* do both (not skills) */
    do_function (ch, &do_mfind, argument);
    do_function (ch, &do_ofind, argument);
}

void do_slookup (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int sn;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Lookup which skill or spell?\n\r", ch);
        return;
    }
    if (!str_cmp (arg, "all")) {
        for (sn = 0; sn < SKILL_MAX; sn++) {
            if (skill_table[sn].name == NULL)
                break;
            sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                     sn, skill_table[sn].slot, skill_table[sn].name);
            send_to_char (buf, ch);
        }
    }
    else {
        if ((sn = skill_lookup (arg)) < 0) {
            send_to_char ("No such skill or spell.\n\r", ch);
            return;
        }
        sprintf (buf, "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
                 sn, skill_table[sn].slot, skill_table[sn].name);
        send_to_char (buf, ch);
    }
}

void do_mfind (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    MOB_INDEX_DATA *pMobIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Find whom?\n\r", ch);
        return;
    }

    fAll = FALSE;                /* !str_cmp( arg, "all" ); */
    found = FALSE;
    nMatch = 0;

    /* Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_mob_index is fast, and I don't feel like threading another link.
     * Do you? -- Furey */
    for (vnum = 0; nMatch < TOP (RECYCLE_MOB_INDEX_DATA); vnum++) {
        if ((pMobIndex = get_mob_index (vnum)) != NULL) {
            nMatch++;
            if (fAll || is_name (argument, pMobIndex->name)) {
                found = TRUE;
                sprintf (buf, "[%5d] %s\n\r",
                         pMobIndex->vnum, pMobIndex->short_descr);
                send_to_char (buf, ch);
            }
        }
    }
    if (!found)
        send_to_char ("No mobiles by that name.\n\r", ch);
}

void do_ofind (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    OBJ_INDEX_DATA *pObjIndex;
    int vnum;
    int nMatch;
    bool fAll;
    bool found;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        send_to_char ("Find what?\n\r", ch);
        return;
    }

    fAll   = FALSE; /* !str_cmp( arg, "all" ); */
    found  = FALSE;
    nMatch = 0;

    /* Yeah, so iterating over all vnum's takes 10,000 loops.
     * Get_obj_index is fast, and I don't feel like threading another link.
     * Do you? -- Furey */
    for (vnum = 0; nMatch < TOP (RECYCLE_OBJ_INDEX_DATA); vnum++) {
        if ((pObjIndex = get_obj_index (vnum)) != NULL) {
            nMatch++;
            if (fAll || is_name (argument, pObjIndex->name)) {
                found = TRUE;
                sprintf (buf, "[%5d] %s\n\r",
                         pObjIndex->vnum, pObjIndex->short_descr);
                send_to_char (buf, ch);
            }
        }
    }
    if (!found)
        send_to_char ("No objects by that name.\n\r", ch);
}

void do_zecho (CHAR_DATA * ch, char *argument) {
    DESCRIPTOR_DATA *d;

    if (argument[0] == '\0') {
        send_to_char ("Zone echo what?\n\r", ch);
        return;
    }
    for (d = descriptor_list; d; d = d->next) {
        if (d->connected == CON_PLAYING
            && d->character->in_room != NULL && ch->in_room != NULL
            && d->character->in_room->area == ch->in_room->area)
        {
            if (char_get_trust (d->character) >= char_get_trust (ch))
                send_to_char ("zone> ", d->character);
            send_to_char (argument, d->character);
            send_to_char ("\n\r", d->character);
        }
    }
}
