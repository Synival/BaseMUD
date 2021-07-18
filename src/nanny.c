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
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/****************************************************************************
 *   This file is just the stock nanny() function ripped from comm.c. It    *
 *   seems to be a popular task for new mud coders, so what the heck?       *
 ***************************************************************************/

#include <ctype.h>
#include <string.h>

#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "save.h"
#include "ban.h"
#include "fight.h"
#include "act_info.h"
#include "act_skills.h"
#include "act_board.h"
#include "act_obj.h"
#include "chars.h"
#include "objs.h"
#include "descs.h"
#include "globals.h"
#include "memory.h"
#include "magic.h"
#include "players.h"
#include "rooms.h"

#include "nanny.h"

/* Parse a name for acceptability. */
bool new_player_name_is_valid (char *name) {
    int clan;

    /* Reserved words. */
    if (str_in_namelist_exact (name,
            "all auto immortal self someone something the you loner none"))
        return FALSE;

    /* check clans */
    for (clan = 0; clan < CLAN_MAX; clan++) {
        if (LOWER (name[0]) == LOWER (clan_table[clan].name[0])
            && !str_cmp (name, clan_table[clan].name))
            return FALSE;
    }

    /* Restrict certain specific names. */
    if (str_cmp (str_capitalized (name), "Alander") &&
            (!str_prefix ("Alan", name) || !str_suffix ("Alander", name)))
        return FALSE;

    /* Length restrictions. */
    if (strlen (name) < 2)
        return FALSE;

    #if defined(MSDOS)
        if (strlen (name) > 8)
            return FALSE;
    #endif

    #if defined(macintosh) || defined(unix)
        if (strlen (name) > 12)
            return FALSE;
    #endif

    /* Alphanumerics only.  Lock out IllIll twits. */
    {
        char *pc;
        bool ill, adjcaps = FALSE, cleancaps = FALSE;
        int total_caps = 0;

        ill = TRUE;
        for (pc = name; *pc != '\0'; pc++) {
            if (!isalpha (*pc))
                return FALSE;

            if (isupper (*pc)) {
                /* ugly anti-caps hack */
                if (adjcaps)
                    cleancaps = TRUE;
                total_caps++;
                adjcaps = TRUE;
            }
            else
                adjcaps = FALSE;

            if (LOWER (*pc) != 'i' && LOWER (*pc) != 'l')
                ill = FALSE;
        }

        if (ill)
            return FALSE;
        if (cleancaps || (total_caps > (strlen (name)) / 2 && strlen (name) < 3))
            return FALSE;
    }

    /* Prevent players from naming themselves after mobs. */
    {
        extern MOB_INDEX_T *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_T *mob_index;
        int hash;

        for (hash = 0; hash < MAX_KEY_HASH; hash++) {
            for (mob_index = mob_index_hash[hash];
                 mob_index != NULL; mob_index = mob_index->hash_next)
            {
                if (str_in_namelist (name, mob_index->name))
                    return FALSE;
            }
        }
    }

    /* Edwin's been here too. JR -- 10/15/00
     *
     * Check names of people playing. Yes, this is necessary for multiple
     * newbies with the same name (thanks Saro) */
    if (descriptor_first) {
        int count = 0;
        DESCRIPTOR_T *d, *dnext;

        for (d = descriptor_first; d != NULL; d = dnext) {
            dnext = d->global_next;
            if (d->connected!=CON_PLAYING&&d->character&&d->character->name
                && d->character->name[0] && !str_cmp(d->character->name,name))
            {
                count++;
                close_socket(d);
            }
        }
        if (count) {
            wiznetf (NULL, NULL, WIZ_LOGINS, 0, 0,
                "newbie alert (%s)", name);
            return FALSE;
        }
    }

    /* All checks passed - this name is valid. */
    return TRUE;
}

/* Deal with sockets that haven't logged in yet. */
void nanny (DESCRIPTOR_T *d, char *argument) {
    const NANNY_HANDLER_T *handler;

    /* Delete leading spaces UNLESS character is writing a note */
    if (d->connected != CON_NOTE_TEXT)
        while (isspace(*argument))
            argument++;

    if ((handler = nanny_get (d->connected)) == NULL) {
        bug ("nanny: bad d->connected %d.", d->connected);
        close_socket (d);
        return;
    }
    handler->action (d, argument);
}

DEFINE_NANNY_FUN (nanny_ansi) {
    extern char *help_greeting;
    if (argument[0] == '\0' || UPPER (argument[0]) == 'Y') {
        d->ansi = TRUE;
        send_to_desc ("{RAnsi enabled!{x\n\r", d);
    }
    else if (UPPER (argument[0]) == 'N') {
        d->ansi = FALSE;
        send_to_desc ("Ansi disabled!\n\r", d);
    }
    else {
        send_to_desc ("Do you want ANSI? (Y/n) ", d);
        return;
    }

    if (help_greeting[0] == '.')
        send_to_desc (help_greeting + 1, d);
    else
        send_to_desc (help_greeting, d);
    d->connected = CON_GET_NAME;
}

DEFINE_NANNY_FUN (nanny_get_player_name) {
    bool old;
    CHAR_T *ch;

    if (argument[0] == '\0') {
        close_socket (d);
        return;
    }

    argument[0] = UPPER (argument[0]);
    if (!new_player_name_is_valid (argument)) {
        send_to_desc ("Illegal name, try another.\n\rName: ", d);
        return;
    }

    old = load_char_obj (d, argument);
    ch = d->character;

    if (EXT_IS_SET (ch->ext_plr, PLR_DENY)) {
        log_f ("Denying access to %s@%s.", argument, d->host);
        send_to_desc ("You are denied access.\n\r", d);
        close_socket (d);
        return;
    }

    if (ban_check (d->host, BAN_PERMIT) && !EXT_IS_SET (ch->ext_plr, PLR_PERMIT)) {
        send_to_desc ("Your site has been banned from this mud.\n\r", d);
        close_socket (d);
        return;
    }

    if (check_reconnect (d, argument, FALSE))
        old = TRUE;
    else {
        if (wizlock && !IS_IMMORTAL (ch)) {
            send_to_desc ("The game is wizlocked.\n\r", d);
            close_socket (d);
            return;
        }
    }

    if (old) {
        /* Old player */
        send_to_desc ("Password: ", d);
        write_to_buffer (d, echo_off_str, 0);
        d->connected = CON_GET_OLD_PASSWORD;
        return;
    }
    else {
        /* New player */
        if (newlock) {
            send_to_desc ("The game is newlocked.\n\r", d);
            close_socket (d);
            return;
        }

        if (ban_check (d->host, BAN_NEWBIES)) {
            send_to_desc (
                "New players are not allowed from your site.\n\r", 0);
            close_socket (d);
            return;
        }

        printf_to_desc (d, "Did I get that right, %s (Y/N)? ", argument);
        d->connected = CON_CONFIRM_NEW_NAME;
        return;
    }
}

DEFINE_NANNY_FUN (nanny_get_old_password) {
    CHAR_T *ch = d->character;

#if defined(unix)
    write_to_buffer (d, "\n\r", 2);
#endif

    if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
        send_to_desc ("Wrong password.\n\r", d);
        close_socket (d);
        return;
    }

    write_to_buffer (d, echo_on_str, 0);
    if (check_playing (d, ch->name))
        return;
    if (check_reconnect (d, ch->name, TRUE))
        return;

    sprintf (log_buf, "%s@%s has connected.", ch->name, d->host);
    log_string (log_buf);
    wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, char_get_trust (ch));

    if (ch->desc->ansi)
        EXT_SET (ch->ext_plr, PLR_COLOUR);
    else
        EXT_UNSET (ch->ext_plr, PLR_COLOUR);

    if (IS_IMMORTAL (ch)) {
        do_function (ch, &do_help, "imotd");
        d->connected = CON_READ_IMOTD;
    }
    else {
        do_function (ch, &do_help, "motd");
        d->connected = CON_READ_MOTD;
    }
}

DEFINE_NANNY_FUN (nanny_break_connect) {
    switch (UPPER(argument[0])) {
        case 'Y':
            nanny_break_connect_confirm (d, argument);
            break;

        case 'N':
            send_to_desc ("Name: ", d);
            if (d->character != NULL) {
                if (d->character->in_room)
                    char_extract (d->character);
                else
                    char_free (d->character);
                d->character = NULL;
            }
            d->connected = CON_GET_NAME;
            break;

        default:
            send_to_desc ("Please type Y or N? ", d);
            break;
    }
}

DEFINE_NANNY_FUN (nanny_break_connect_confirm) {
    CHAR_T *ch = d->character;
    DESCRIPTOR_T *d_old, *d_next;

    for (d_old = descriptor_first; d_old != NULL; d_old = d_next) {
        d_next = d_old->global_next;
        if (d_old == d || d_old->character == NULL)
            continue;

        if (str_cmp (ch->name, d_old->original
                ? d_old->original->name : d_old-> character->name))
            continue;

        close_socket (d_old);
    }
    if (check_reconnect (d, ch->name, TRUE))
        return;

    send_to_desc ("Reconnect attempt failed.\n\rName: ", d);
    if (d->character != NULL) {
        if (d->character->in_room)
            char_extract (d->character);
        else
            char_free (d->character);
        d->character = NULL;
    }
    d->connected = CON_GET_NAME;
}

DEFINE_NANNY_FUN (nanny_confirm_new_name) {
    CHAR_T *ch = d->character;

    switch (UPPER(argument[0])) {
        case 'Y':
            printf_to_desc (d, "New character.\n\r"
                "Give me a password for %s: %s", ch->name, echo_off_str);
            d->connected = CON_GET_NEW_PASSWORD;
            if (ch->desc->ansi)
                EXT_SET (ch->ext_plr, PLR_COLOUR);
            break;

        case 'N':
            send_to_desc ("Ok, what IS it, then? ", d);
            char_free (d->character);
            d->character = NULL;
            d->connected = CON_GET_NAME;
            break;

        default:
            send_to_desc ("Please type Yes or No? ", d);
            break;
    }
}

DEFINE_NANNY_FUN (nanny_get_new_password) {
    CHAR_T *ch = d->character;
    char *pwdnew, *p;

#if defined(unix)
    write_to_buffer (d, "\n\r", 2);
#endif

    if (strlen (argument) < 5) {
        send_to_desc (
            "Password must be at least five characters long.\n\r"
            "Password: ", d);
        return;
    }

    pwdnew = crypt (argument, ch->name);
    for (p = pwdnew; *p != '\0'; p++) {
        if (*p == '~') {
            send_to_desc (
                "New password not acceptable, try again.\n\r"
                "Password: ", d);
            return;
        }
    }

    str_replace_dup (&(ch->pcdata->pwd), pwdnew);
    send_to_desc ("Please retype password: ", d);
    d->connected = CON_CONFIRM_PASSWORD;
}

DEFINE_NANNY_FUN (nanny_confirm_new_password) {
    const PC_RACE_T *pc_race;
    CHAR_T *ch = d->character;
    int i;

#if defined(unix)
    write_to_buffer (d, "\n\r", 2);
#endif

    if (strcmp (crypt (argument, ch->pcdata->pwd), ch->pcdata->pwd)) {
        send_to_desc ("Passwords don't match.\n\rRetype password: ", d);
        d->connected = CON_GET_NEW_PASSWORD;
        return;
    }

    write_to_buffer (d, echo_on_str, 0);
    send_to_desc ("The following races are available:\n\r  ", d);
    for (i = 0; i < PC_RACE_MAX; i++) {
        if ((pc_race = pc_race_get (i)) == NULL)
            break;
        write_to_buffer (d, pc_race->name, 0);
        write_to_buffer (d, " ", 1);
    }
    write_to_buffer (d, "\n\r", 0);
    send_to_desc ("What is your race (help for more information)? ", d);
    d->connected = CON_GET_NEW_RACE;
}

DEFINE_NANNY_FUN (nanny_get_new_race) {
    const PC_RACE_T *pc_race;
    const RACE_T *race;
    char arg[MAX_STRING_LENGTH];
    CHAR_T *ch = d->character;
    int i, race_num;

    one_argument (argument, arg);
    if (!strcmp (arg, "help")) {
        argument = one_argument (argument, arg);
        if (argument[0] == '\0')
            do_function (ch, &do_help, "race help");
        else
            do_function (ch, &do_help, argument);
        send_to_desc ("What is your race (help for more information)? ", d);
        return;
    }

    pc_race = pc_race_get_by_name (argument);
    race_num = (pc_race == NULL)
        ? TYPE_NONE : race_lookup_exact (pc_race->name);
    if (race_num < 0) {
        send_to_desc ("That is not a valid race.\n\r", d);
        send_to_desc ("The following races are available:\n\r  ", d);
        for (i = 0; i < PC_RACE_MAX; i++) {
            if ((pc_race = pc_race_get (i)) == NULL)
                break;
            write_to_buffer (d, pc_race->name, 0);
            write_to_buffer (d, " ", 1);
        }
        write_to_buffer (d, "\n\r", 0);
        send_to_desc ("What is your race? (help for more information) ", d);
        return;
    }

    ch->race = race_num;
    pc_race = pc_race_get_by_race (race_num);
    race = race_get (race_num);

    /* initialize stats */
    for (i = 0; i < STAT_MAX; i++)
        ch->perm_stat[i] = pc_race->stats[i];

    ch->affected_by = ch->affected_by | race->aff;
    ch->imm_flags   = ch->imm_flags   | race->imm;
    ch->res_flags   = ch->res_flags   | race->res;
    ch->vuln_flags  = ch->vuln_flags  | race->vuln;
    ch->form        = race->form;
    ch->parts       = race->parts;

    /* add skills */
    for (i = 0; i < PC_RACE_SKILL_MAX; i++) {
        if (pc_race->skills[i] == NULL)
            break;
        player_add_skill_or_group (ch, pc_race->skills[i], FALSE);
    }

    /* add cost */
    ch->pcdata->creation_points = pc_race->creation_points;
    ch->size = pc_race->size;

    send_to_desc ("What is your sex (M/F)? ", d);
    d->connected = CON_GET_NEW_SEX;
}

DEFINE_NANNY_FUN (nanny_get_new_sex) {
    const CLASS_T *class;
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    switch (UPPER(argument[0])) {
        case 'M':
            ch->sex = SEX_MALE;
            ch->pcdata->true_sex = SEX_MALE;
            break;
        case 'F':
            ch->sex = SEX_FEMALE;
            ch->pcdata->true_sex = SEX_FEMALE;
            break;
        default:
            send_to_desc ("That's not a sex.\n\rWhat IS your sex? ", d);
            return;
    }

    strcpy (buf, "Select a class [");
    for (i = 0; (class = class_get (i)) != NULL; i++) {
        if (i > 0)
            strcat (buf, " ");
        strcat (buf, class->name);
    }
    strcat (buf, "]: ");
    write_to_buffer (d, buf, 0);
    d->connected = CON_GET_NEW_CLASS;
}

DEFINE_NANNY_FUN (nanny_get_new_class) {
    CHAR_T *ch = d->character;
    int class_n;

    if ((class_n = class_lookup (argument)) < 0) {
        send_to_desc ("That's not a class.\n\rWhat IS your class? ", d);
        return;
    }
    ch->class = class_n;

    sprintf (log_buf, "%s@%s new player.", ch->name, d->host);
    log_string (log_buf);
    wiznet ("Newbie alert!  $N sighted.", ch, NULL, WIZ_NEWBIE, 0, 0);
    wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, char_get_trust (ch));

    write_to_buffer (d, "\n\r", 2);
    send_to_desc ("You may be good, neutral, or evil.\n\r", d);
    send_to_desc ("Which alignment (G/N/E)? ", d);
    d->connected = CON_GET_ALIGNMENT;
}

DEFINE_NANNY_FUN (nanny_get_alignment) {
    CHAR_T *ch = d->character;
    switch (UPPER(argument[0])) {
        case 'G': ch->alignment =  750; break;
        case 'N': ch->alignment =    0; break;
        case 'E': ch->alignment = -750; break;

        default:
            send_to_desc ("That's not a valid alignment.\n\r", d);
            send_to_desc ("Which alignment (G/N/E)? ", d);
            return;
    }

    write_to_buffer (d, "\n\r", 0);

    player_add_skill_or_group (ch, "rom basics", FALSE);
    if (class_table[ch->class].base_group != NULL)
        player_add_skill_or_group (ch, class_table[ch->class].base_group, FALSE);

    player_set_default_skills (ch);

    send_to_desc ("Do you wish to customize this character?\n\r", d);
    send_to_desc
        ("Customization takes time, but allows a wider range of skills "
         "and abilities.\n\r", d);
    send_to_desc ("Customize (Y/N)? ", d);
    d->connected = CON_DEFAULT_CHOICE;
}

DEFINE_NANNY_FUN (nanny_default_choice) {
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer (d, "\n\r", 2);
    switch (UPPER(argument[0])) {
        case 'Y':
            ch->gen_data = gen_data_new ();
            do_function (ch, &do_help, "group header");
            write_to_buffer (d, "\n\r", 0);
            player_list_skills_and_groups (ch, FALSE);

            write_to_buffer (d,
                "\n\rYou already have the following skills and spells:\n\r", 0);
            do_function (ch, &do_abilities, "all");
            write_to_buffer (d, "\n\r", 0);
            do_function (ch, &do_help, "menu choice");
            d->connected = CON_GEN_GROUPS;
            break;

        case 'N':
            if (class_table[ch->class].default_group != NULL)
                player_add_skill_or_group (ch, class_table[ch->class].default_group, TRUE);
            write_to_buffer (d, "\n\r", 2);
            write_to_buffer (d,
                "Please pick a weapon from the following choices:\n\r", 0);

            buf[0] = '\0';
            for (i = 0; weapon_table[i].name != NULL; i++) {
                if (ch->pcdata->learned[weapon_table[i].skill_index] > 0) {
                    strcat (buf, weapon_table[i].name);
                    strcat (buf, " ");
                }
            }
            strcat (buf, "\n\rYour choice? ");
            write_to_buffer (d, buf, 0);

            d->connected = CON_PICK_WEAPON;
            break;

        default:
            write_to_buffer (d, "Please answer (Y/N)? ", 0);
            return;
    }
}

DEFINE_NANNY_FUN (nanny_pick_weapon) {
    CHAR_T *ch = d->character;
    const WEAPON_T *weapon;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer (d, "\n\r", 2);

    weapon = weapon_get_by_name (argument);
    if (weapon == NULL || ch->pcdata->learned[weapon->skill_index] <= 0) {
        write_to_buffer (d, "That's not a valid selection. Choices are:\n\r", 0);
        buf[0] = '\0';
        for (i = 0; weapon_table[i].name != NULL; i++) {
            if (ch->pcdata->learned[weapon_table[i].skill_index] > 0) {
                strcat (buf, weapon_table[i].name);
                strcat (buf, " ");
            }
        }
        strcat (buf, "\n\rYour choice? ");
        write_to_buffer (d, buf, 0);
        return;
    }

    ch->pcdata->learned[weapon->skill_index] = 40;
    write_to_buffer (d, "\n\r", 2);
    do_function (ch, &do_help, "motd");
    d->connected = CON_READ_MOTD;
}

DEFINE_NANNY_FUN (nanny_gen_groups) {
    CHAR_T *ch = d->character;

    if (!str_cmp (argument, "done")) {
        nanny_gen_groups_done (d, argument);
        return;
    }

    if (nanny_parse_gen_groups (ch, argument))
        printf_to_char (ch, "\n\r");
    do_function (ch, &do_help, "menu choice");
}

/* this procedure handles the input parsing for the skill generator */
bool nanny_parse_gen_groups (CHAR_T *ch, char *argument) {
    const SKILL_T *skill;
    const SKILL_GROUP_T *group;
    char arg[MAX_INPUT_LENGTH];
    int num;

    if (argument[0] == '\0')
        return FALSE;

    argument = one_argument (argument, arg);
    if (!str_prefix (arg, "help")) {
        if (argument[0] == '\0') {
            do_function (ch, &do_help, "group help");
            return TRUE;
        }

        do_function (ch, &do_help, argument);
        return TRUE;
    }

    if (!str_prefix (arg, "add")) {
        if (argument[0] == '\0') {
            printf_to_char (ch, "You must provide a skill name.\n\r");
            return TRUE;
        }

        num = skill_group_lookup (argument);
        if (num != -1) {
            group = skill_group_get (num);
            if (ch->gen_data->group_chosen[num] || ch->pcdata->group_known[num]) {
                printf_to_char (ch, "You already know that group!\n\r");
                return TRUE;
            }
            if (group->classes[ch->class].cost < 1) {
                printf_to_char (ch, "That group is not available.\n\r");
                return TRUE;
            }

            /* Close security hole */
            if (ch->pcdata->creation_points + group->classes[ch->class].cost > 300) {
                printf_to_char (ch, "You cannot take more than 300 creation points.\n\r");
                return TRUE;
            }

            printf_to_char (ch, "Group '%s' added.\n\r", group->name);
            player_add_skill_group (ch, num, TRUE);
            ch->gen_data->group_chosen[num] = TRUE;
            return TRUE;
        }

        num = skill_lookup (argument);
        if (num != -1) {
            skill = skill_get (num);
            if (ch->gen_data->skill_chosen[num] || ch->pcdata->skill_known[num] > 0 ||
                ch->pcdata->learned[num] > 0)
            {
                printf_to_char (ch, "You already know that skill!\n\r");
                return TRUE;
            }
            if (skill->classes[ch->class].effort < 1 || skill->spell_fun != spell_null) {
                printf_to_char (ch, "That skill is not available.\n\r");
                return TRUE;
            }

            /* Close security hole */
            if (ch->pcdata->creation_points + skill->classes[ch->class].effort > 300) {
                printf_to_char (ch, "You cannot take more than 300 creation points.\n\r");
                return TRUE;
            }

            printf_to_char (ch, "Skill '%s' added.\n\r", skill->name);
            player_add_skill (ch, num, TRUE);
            ch->gen_data->skill_chosen[num] = TRUE;
            return TRUE;
        }

        printf_to_char (ch, "No skills or groups by that name...\n\r");
        return TRUE;
    }

    if (!strcmp (arg, "drop")) {
        if (argument[0] == '\0') {
            printf_to_char (ch, "You must provide a skill to drop.\n\r");
            return TRUE;
        }

        num = skill_group_lookup (argument);
        if (num != -1 && ch->gen_data->group_chosen[num]) {
            group = skill_group_get (num);
            printf_to_char (ch, "Group '%s' dropped.\n\r", group->name);
            player_remove_skill_group (ch, num, TRUE);
            ch->gen_data->group_chosen[num] = FALSE;
            return TRUE;
        }

        num = skill_lookup (argument);
        if (num != -1 && ch->gen_data->skill_chosen[num]) {
            skill = skill_get (num);
            printf_to_char (ch, "Skill '%s' dropped.\n\r", skill->name);
            player_remove_skill (ch, num, TRUE);
            ch->gen_data->skill_chosen[num] = FALSE;
            return TRUE;
        }

        printf_to_char (ch, "You haven't bought any such skill or group.\n\r");
        return TRUE;
    }

    if (!str_prefix (arg, "premise")) {
        do_function (ch, &do_help, "premise");
        return TRUE;
    }
    if (!str_prefix (arg, "list")) {
        player_list_skills_and_groups (ch, FALSE);
        return TRUE;
    }
    if (!str_prefix (arg, "learned")) {
        player_list_skills_and_groups (ch, TRUE);
        return TRUE;
    }
    if (!str_prefix (arg, "abilities")) {
        do_function (ch, &do_abilities, "all");
        return TRUE;
    }
    if (!str_prefix (arg, "info")) {
        do_function (ch, &do_groups, argument);
        return TRUE;
    }

    return FALSE;
}

DEFINE_NANNY_FUN (nanny_gen_groups_done) {
    const PC_RACE_T *pc_race;
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    pc_race = pc_race_get_by_race (ch->race);
    if (ch->pcdata->creation_points == pc_race->creation_points) {
        printf_to_char (ch, "You didn't pick anything.\n\r\n\r");
        do_function (ch, &do_help, "menu choice");
        return;
    }

    if (ch->pcdata->creation_points < 40 + pc_race->creation_points) {
        printf_to_char (ch,
            "You must take at least %d points of skills and groups.\n\r\n\r",
            40 + pc_race->creation_points);
        do_function (ch, &do_help, "menu choice");
        return;
    }

    printf_to_char (ch, "Creation points: %d\n\r",
        ch->pcdata->creation_points);
    printf_to_char (ch, "Experience per level: %d\n\r",
        player_get_exp_per_level (ch));

    if (ch->pcdata->creation_points < 40)
        ch->train = (40 - ch->pcdata->creation_points + 1) / 2;
    gen_data_free (ch->gen_data);
    ch->gen_data = NULL;

    write_to_buffer (d, "\n\r", 2);
    write_to_buffer (d, "Please pick a weapon from the following choices:\n\r", 0);
    buf[0] = '\0';
    for (i = 0; weapon_table[i].name != NULL; i++) {
        if (ch->pcdata->learned[weapon_table[i].skill_index] > 0) {
            strcat (buf, weapon_table[i].name);
            strcat (buf, " ");
        }
    }
    strcat (buf, "\n\rYour choice? ");
    write_to_buffer (d, buf, 0);
    d->connected = CON_PICK_WEAPON;
}

DEFINE_NANNY_FUN (nanny_read_imotd) {
    CHAR_T *ch = d->character;

    write_to_buffer (d, "\n\r", 2);
    do_function (ch, &do_help, "motd");
    d->connected = CON_READ_MOTD;
}

DEFINE_NANNY_FUN (nanny_read_motd) {
    CHAR_T *ch = d->character;
    char buf[MAX_STRING_LENGTH];

    extern int mud_telnetga, mud_ansicolor;
    if (ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0') {
        write_to_buffer (d,
            "Warning! Null password!\n\r"
            "Please report old password with bug.\n\r"
            "Type 'password null <new password>' to fix.\n\r", 0);
    }

    write_to_buffer (d,
        "\n\rWelcome to ROM 2.4.  Please don't feed the mobiles!\n\r", 0);
    LIST2_FRONT (ch, global_prev, global_next, char_first, char_last);

    d->connected = CON_PLAYING;
    player_reset (ch);

    if (ch->level == 0) {
        if(mud_ansicolor)
            EXT_SET (ch->ext_plr, PLR_COLOUR);
        if(mud_telnetga)
            SET_BIT (ch->comm, COMM_TELNET_GA);

        ch->perm_stat[class_table[ch->class].attr_prime] += 3;

        ch->level = 1;
        ch->exp   = player_get_exp_per_level (ch);
        ch->hit   = ch->max_hit;
        ch->mana  = ch->max_mana;
        ch->move  = ch->max_move;
        ch->train = 3;
        ch->practice = 5;
        sprintf (buf, "the %s", title_table[ch->class][ch->level]
                 [ch->sex == SEX_FEMALE ? 1 : 0]);
        player_set_title (ch, buf);

        do_function (ch, &do_outfit, "");
        obj_give_to_char (obj_create (obj_get_index (OBJ_VNUM_MAP), 0), ch);

        char_to_room (ch, room_get_index (ROOM_VNUM_SCHOOL));
        printf_to_char (ch, "\n\r");
        do_function (ch, &do_help, "newbie info");
        printf_to_char (ch, "\n\r");
    }
    else if (ch->in_room != NULL)
        char_to_room (ch, ch->in_room);
    else if (IS_IMMORTAL (ch))
        char_to_room (ch, room_get_index (ROOM_VNUM_CHAT));
    else
        char_to_room (ch, room_get_index (ROOM_VNUM_TEMPLE));

    act ("$n has entered the game.", ch, NULL, NULL, TO_NOTCHAR);
    do_function (ch, &do_look, "auto");

    wiznet ("$N has left real life behind.", ch, NULL,
            WIZ_LOGINS, WIZ_SITES, char_get_trust (ch));

    if (ch->pet != NULL) {
        char_to_room (ch->pet, ch->in_room);
        act ("$n has entered the game.", ch->pet, NULL, NULL,
             TO_NOTCHAR);
    }

    send_to_char("\n", ch);
    do_function (ch, &do_board, "");
}
