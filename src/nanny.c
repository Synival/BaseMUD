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
*    ROM 2.4 is copyright 1993-1998 Russ Taylor                             *
*    ROM has been brought to you by the ROM consortium                      *
*        Russ Taylor (rtaylor@hypercube.org)                                *
*        Gabrielle Taylor (gtaylor@hypercube.org)                           *
*        Brian Moore (zump@rom.org)                                         *
*    By using this code, you have agreed to follow the terms of the         *
*    ROM license, in the file Rom24/doc/rom.license                         *
****************************************************************************/

/****************************************************************************
 *   This file is just the stock nanny() function ripped from comm.c. It    *
 *   seems to be a popular task for new mud coders, so what the heck?       *
 ***************************************************************************/

#include <ctype.h>
#include <string.h>

#include "interp.h"
#include "recycle.h"
#include "lookup.h"
#include "skills.h"
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

#include "nanny.h"

/* Parse a name for acceptability. */
bool new_player_name_is_valid (char *name) {
    int clan;

    /* Reserved words. */
    if (is_exact_name (name,
            "all auto immortal self someone something the you loner none"))
        return FALSE;

    /* check clans */
    for (clan = 0; clan < CLAN_MAX; clan++) {
        if (LOWER (name[0]) == LOWER (clan_table[clan].name[0])
            && !str_cmp (name, clan_table[clan].name))
            return FALSE;
    }

    /* Restrict certain specific names. */
    if (str_cmp (capitalize (name), "Alander") &&
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
        bool fIll, adjcaps = FALSE, cleancaps = FALSE;
        int total_caps = 0;

        fIll = TRUE;
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
                fIll = FALSE;
        }

        if (fIll)
            return FALSE;
        if (cleancaps || (total_caps > (strlen (name)) / 2 && strlen (name) < 3))
            return FALSE;
    }

    /* Prevent players from naming themselves after mobs. */
    {
        extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
        MOB_INDEX_DATA *pMobIndex;
        int iHash;

        for (iHash = 0; iHash < MAX_KEY_HASH; iHash++) {
            for (pMobIndex = mob_index_hash[iHash];
                 pMobIndex != NULL; pMobIndex = pMobIndex->next)
            {
                if (is_name (name, pMobIndex->name))
                    return FALSE;
            }
        }
    }

    /* Edwin's been here too. JR -- 10/15/00
     *
     * Check names of people playing. Yes, this is necessary for multiple
     * newbies with the same name (thanks Saro) */
    if (descriptor_list) {
        int count = 0;
        DESCRIPTOR_DATA *d, *dnext;

        for (d = descriptor_list; d != NULL; d = dnext) {
            dnext=d->next;
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
void nanny (DESCRIPTOR_DATA * d, char *argument) {
    const NANNY_HANDLER *handler;

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

void nanny_ansi (DESCRIPTOR_DATA * d, char *argument) {
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

void nanny_get_player_name (DESCRIPTOR_DATA * d, char *argument) {
    bool fOld;
    CHAR_DATA *ch;

    if (argument[0] == '\0') {
        close_socket (d);
        return;
    }

    argument[0] = UPPER (argument[0]);
    if (!new_player_name_is_valid (argument)) {
        send_to_desc ("Illegal name, try another.\n\rName: ", d);
        return;
    }

    fOld = load_char_obj (d, argument);
    ch = d->character;

    if (IS_SET (ch->plr, PLR_DENY)) {
        log_f ("Denying access to %s@%s.", argument, d->host);
        send_to_desc ("You are denied access.\n\r", d);
        close_socket (d);
        return;
    }

    if (check_ban (d->host, BAN_PERMIT) && !IS_SET (ch->plr, PLR_PERMIT)) {
        send_to_desc ("Your site has been banned from this mud.\n\r", d);
        close_socket (d);
        return;
    }

    if (check_reconnect (d, argument, FALSE))
        fOld = TRUE;
    else {
        if (wizlock && !IS_IMMORTAL (ch)) {
            send_to_desc ("The game is wizlocked.\n\r", d);
            close_socket (d);
            return;
        }
    }

    if (fOld) {
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

        if (check_ban (d->host, BAN_NEWBIES)) {
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

void nanny_get_old_password (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;

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
        SET_BIT (ch->plr, PLR_COLOUR);
    else
        REMOVE_BIT (ch->plr, PLR_COLOUR);

    if (IS_IMMORTAL (ch)) {
        do_function (ch, &do_help, "imotd");
        d->connected = CON_READ_IMOTD;
    }
    else {
        do_function (ch, &do_help, "motd");
        d->connected = CON_READ_MOTD;
    }
}

void nanny_break_connect (DESCRIPTOR_DATA * d, char *argument) {
    switch (UPPER(argument[0])) {
        case 'Y':
            nanny_break_connect_confirm (d, argument);
            break;

        case 'N':
            send_to_desc ("Name: ", d);
            if (d->character != NULL) {
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

void nanny_break_connect_confirm (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    DESCRIPTOR_DATA *d_old, *d_next;

    for (d_old = descriptor_list; d_old != NULL; d_old = d_next) {
        d_next = d_old->next;
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
        char_free (d->character);
        d->character = NULL;
    }
    d->connected = CON_GET_NAME;
}

void nanny_confirm_new_name (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;

    switch (UPPER(argument[0])) {
        case 'Y':
            printf_to_desc (d, "New character.\n\r"
                "Give me a password for %s: %s", ch->name, echo_off_str);
            d->connected = CON_GET_NEW_PASSWORD;
            if (ch->desc->ansi)
                SET_BIT (ch->plr, PLR_COLOUR);
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

void nanny_get_new_password (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
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

    str_free (ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup (pwdnew);
    send_to_desc ("Please retype password: ", d);
    d->connected = CON_CONFIRM_PASSWORD;
}

void nanny_confirm_new_password (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    int race;

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
    for (race = 1; race_table[race].name != NULL; race++) {
        if (!race_table[race].pc_race)
            break;
        write_to_buffer (d, race_table[race].name, 0);
        write_to_buffer (d, " ", 1);
    }
    write_to_buffer (d, "\n\r", 0);
    send_to_desc ("What is your race (help for more information)? ", d);
    d->connected = CON_GET_NEW_RACE;
}

void nanny_get_new_race (DESCRIPTOR_DATA * d, char *argument) {
    char arg[MAX_STRING_LENGTH];
    CHAR_DATA * ch = d->character;
    int i, race;

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

    race = race_lookup (argument);
    if (race < 0 || !race_table[race].pc_race) {
        send_to_desc ("That is not a valid race.\n\r", d);
        send_to_desc ("The following races are available:\n\r  ", d);
        for (race = 1; race_table[race].name != NULL; race++) {
            if (!race_table[race].pc_race)
                break;
            write_to_buffer (d, race_table[race].name, 0);
            write_to_buffer (d, " ", 1);
        }
        write_to_buffer (d, "\n\r", 0);
        send_to_desc ("What is your race? (help for more information) ", d);
        return;
    }

    ch->race = race;

    /* initialize stats */
    for (i = 0; i < STAT_MAX; i++)
        ch->perm_stat[i] = pc_race_table[race].stats[i];
    ch->affected_by = ch->affected_by | race_table[race].aff;
    ch->imm_flags = ch->imm_flags | race_table[race].imm;
    ch->res_flags = ch->res_flags | race_table[race].res;
    ch->vuln_flags = ch->vuln_flags | race_table[race].vuln;
    ch->form = race_table[race].form;
    ch->parts = race_table[race].parts;

    /* add skills */
    for (i = 0; i < PC_RACE_MAX; i++) {
        if (pc_race_table[race].skills[i] == NULL)
            break;
        group_add (ch, pc_race_table[race].skills[i], FALSE);
    }

    /* add cost */
    ch->pcdata->points = pc_race_table[race].points;
    ch->size = pc_race_table[race].size;

    send_to_desc ("What is your sex (M/F)? ", d);
    d->connected = CON_GET_NEW_SEX;
}

void nanny_get_new_sex (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int iClass;

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
    for (iClass = 0; iClass < CLASS_MAX; iClass++) {
        if (iClass > 0)
            strcat (buf, " ");
        strcat (buf, class_table[iClass].name);
    }
    strcat (buf, "]: ");
    write_to_buffer (d, buf, 0);
    d->connected = CON_GET_NEW_CLASS;
}

void nanny_get_new_class (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    int iClass;

    iClass = class_lookup (argument);
    if (iClass < 0) {
        send_to_desc ("That's not a class.\n\rWhat IS your class? ", d);
        return;
    }

    ch->class = iClass;

    sprintf (log_buf, "%s@%s new player.", ch->name, d->host);
    log_string (log_buf);
    wiznet ("Newbie alert!  $N sighted.", ch, NULL, WIZ_NEWBIE, 0, 0);
    wiznet (log_buf, NULL, NULL, WIZ_SITES, 0, char_get_trust (ch));

    write_to_buffer (d, "\n\r", 2);
    send_to_desc ("You may be good, neutral, or evil.\n\r", d);
    send_to_desc ("Which alignment (G/N/E)? ", d);
    d->connected = CON_GET_ALIGNMENT;
}

void nanny_get_alignment (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
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

    group_add (ch, "rom basics", FALSE);
    group_add (ch, class_table[ch->class].base_group, FALSE);
    ch->pcdata->learned[gsn_recall] = 50;
    send_to_desc ("Do you wish to customize this character?\n\r", d);
    send_to_desc
        ("Customization takes time, but allows a wider range of skills and abilities.\n\r",
         d);
    send_to_desc ("Customize (Y/N)? ", d);
    d->connected = CON_DEFAULT_CHOICE;
}

void nanny_default_choice (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer (d, "\n\r", 2);
    switch (UPPER(argument[0])) {
        case 'Y':
            ch->gen_data = gen_data_new ();
            ch->gen_data->points_chosen = ch->pcdata->points;
            do_function (ch, &do_help, "group header");
            write_to_buffer (d, "\n\r", 0);
            list_group_costs (ch);

            write_to_buffer (d,
                "\n\rYou already have the following skills and spells:\n\r", 0);
            do_function (ch, &do_abilities, "all");
            write_to_buffer (d, "\n\r", 0);
            do_function (ch, &do_help, "menu choice");
            d->connected = CON_GEN_GROUPS;
            break;

        case 'N':
            group_add (ch, class_table[ch->class].default_group, TRUE);
            write_to_buffer (d, "\n\r", 2);
            write_to_buffer (d,
                "Please pick a weapon from the following choices:\n\r", 0);

            buf[0] = '\0';
            for (i = 0; weapon_table[i].name != NULL; i++) {
                if (ch->pcdata->learned[*weapon_table[i].gsn] > 0) {
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

void nanny_pick_weapon (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    const WEAPON_TYPE *weapon;
    char buf[MAX_STRING_LENGTH];
    int i;

    write_to_buffer (d, "\n\r", 2);

    weapon = weapon_get_by_name (argument);
    if (weapon == NULL || ch->pcdata->learned[*(weapon->gsn)] <= 0) {
        write_to_buffer (d, "That's not a valid selection. Choices are:\n\r", 0);
        buf[0] = '\0';
        for (i = 0; weapon_table[i].name != NULL; i++) {
            if (ch->pcdata->learned[*(weapon_table[i].gsn)] > 0) {
                strcat (buf, weapon_table[i].name);
                strcat (buf, " ");
            }
        }
        strcat (buf, "\n\rYour choice? ");
        write_to_buffer (d, buf, 0);
        return;
    }

    ch->pcdata->learned[*(weapon->gsn)] = 40;
    write_to_buffer (d, "\n\r", 2);
    do_function (ch, &do_help, "motd");
    d->connected = CON_READ_MOTD;
}

void nanny_gen_groups (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;

    if (!str_cmp (argument, "done")) {
        nanny_gen_groups_done (d, argument);
        return;
    }

    if (parse_gen_groups (ch, argument))
        send_to_char ("\n\r", ch);
    do_function (ch, &do_help, "menu choice");
}

void nanny_gen_groups_done (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    char buf[MAX_STRING_LENGTH];
    int i;

    BAIL_IF (ch->pcdata->points == pc_race_table[ch->race].points,
        "You didn't pick anything.\n\r", ch);

    if (ch->pcdata->points < 40 + pc_race_table[ch->race].points) {
        printf_to_char (ch,
             "You must take at least %d points of skills "
             "and groups.\n\r", 40 + pc_race_table[ch->race].points);
        return;
    }

    printf_to_char (ch, "Creation points: %d\n\r", ch->pcdata->points);
    printf_to_char (ch, "Experience per level: %d\n\r",
        exp_per_level (ch, ch->gen_data->points_chosen));

    if (ch->pcdata->points < 40)
        ch->train = (40 - ch->pcdata->points + 1) / 2;
    gen_data_free (ch->gen_data);
    ch->gen_data = NULL;

    write_to_buffer (d, "\n\r", 2);
    write_to_buffer (d, "Please pick a weapon from the following choices:\n\r", 0);
    buf[0] = '\0';
    for (i = 0; weapon_table[i].name != NULL; i++) {
        if (ch->pcdata->learned[*weapon_table[i].gsn] > 0) {
            strcat (buf, weapon_table[i].name);
            strcat (buf, " ");
        }
    }
    strcat (buf, "\n\rYour choice? ");
    write_to_buffer (d, buf, 0);
    d->connected = CON_PICK_WEAPON;
}

void nanny_read_imotd (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;

    write_to_buffer (d, "\n\r", 2);
    do_function (ch, &do_help, "motd");
    d->connected = CON_READ_MOTD;
}

void nanny_read_motd (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA * ch = d->character;
    char buf[MAX_STRING_LENGTH];

    extern int mud_telnetga, mud_ansicolor;
    if (ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0') {
        write_to_buffer (d,
            "Warning! Null password!\n\r", 0);
        write_to_buffer (d,
            "Please report old password with bug.\n\r", 0);
        write_to_buffer (d,
            "Type 'password null <new password>' to fix.\n\r", 0);
    }

    write_to_buffer (d,
        "\n\rWelcome to ROM 2.4.  Please don't feed the mobiles!\n\r", 0);
    LIST_FRONT (ch, next, char_list);

    d->connected = CON_PLAYING;
    char_reset (ch);

    if (ch->level == 0) {
        if(mud_ansicolor)
            SET_BIT (ch->plr, PLR_COLOUR);
        if(mud_telnetga)
            SET_BIT (ch->comm, COMM_TELNET_GA);

        ch->perm_stat[class_table[ch->class].attr_prime] += 3;

        ch->level = 1;
        ch->exp   = exp_per_level (ch, ch->pcdata->points);
        ch->hit   = ch->max_hit;
        ch->mana  = ch->max_mana;
        ch->move  = ch->max_move;
        ch->train = 3;
        ch->practice = 5;
        sprintf (buf, "the %s", title_table[ch->class][ch->level]
                 [ch->sex == SEX_FEMALE ? 1 : 0]);
        char_set_title (ch, buf);

        do_function (ch, &do_outfit, "");
        obj_to_char (obj_create (get_obj_index (OBJ_VNUM_MAP), 0), ch);

        char_to_room (ch, get_room_index (ROOM_VNUM_SCHOOL));
        send_to_char ("\n\r", ch);
        do_function (ch, &do_help, "newbie info");
        send_to_char ("\n\r", ch);
    }
    else if (ch->in_room != NULL)
        char_to_room (ch, ch->in_room);
    else if (IS_IMMORTAL (ch))
        char_to_room (ch, get_room_index (ROOM_VNUM_CHAT));
    else
        char_to_room (ch, get_room_index (ROOM_VNUM_TEMPLE));

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
