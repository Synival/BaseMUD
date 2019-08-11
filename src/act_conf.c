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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_info.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "interp.h"
#include "colour.h"
#include "groups.h"
#include "comm.h"
#include "utils.h"
#include "lookup.h"
#include "do_sub.h"

#include "act_conf.h"

void do_colour_one (CHAR_DATA * ch, const COLOUR_SETTING_TYPE * setting,
    const COLOUR_TYPE * colour, bool use_default, char *buf)
{
    char colour_buf[256], *argument, arg[256];
    flag_t *col_flag;

    /* Is this the beginning of our menu? */
    if (buf[0] == '\0') {
        if (colour != NULL)
            send_to_char ("The following colours have been set:\n\r", ch);
        strcat (buf,
            " Code | Name            | Color\n\r"
            "-------------------------------------------------------------------------\n\r");
    }

    /* Show the colour code + name. */
    sprintf (buf + strlen(buf), "  {{%c  | %-15s |",
        setting->act_char, setting->name);

    /* Get our colour. Should we set it? */
    col_flag = &(ch->pcdata->colour[setting->index]);
    if (use_default)
        *col_flag = setting->default_colour;
    if (colour != NULL) {
        *col_flag &= ~(colour->mask);
        *col_flag |= colour->code;
    }

    /* Show us the colour... /in colour!/ */
    if (IS_SET (ch->plr, PLR_COLOUR))
        colour_to_ansi (*col_flag, buf + strlen(buf), MAX_STRING_LENGTH);

    /* Arrange the color nicely. */
    colour_to_full_name (*col_flag, colour_buf, sizeof(colour_buf));
    for (argument = colour_buf; *argument != '\0'; ) {
        argument = one_argument (argument, arg);
        sprintf (buf + strlen(buf), " %-15s", arg);
    }

    if (IS_SET (ch->plr, PLR_COLOUR))
        colour_to_ansi (CC_CLEAR, buf + strlen(buf), MAX_STRING_LENGTH);

    /* Done! */
    strcat (buf, "\n\r");
}

void do_colour_codes (CHAR_DATA * ch, char *argument) {
    flag_t last_mask = 0;
    int i, col = 0;

    send_to_char ("Valid colours:\n\r", ch);
    for (i = 0; colour_table[i].name != NULL; i++) {
        if (last_mask != 0 && colour_table[i].mask != last_mask) {
            if (col != 0)
                send_to_char ("\n\r", ch);
            col = 0;
        }
        if (col == 0)
            send_to_char ("    ", ch);

        last_mask = colour_table[i].mask;
        printf_to_char (ch, "%-15s", colour_table[i].name);

        if (++col == 4) {
            send_to_char ("\n\r", ch);
            col = 0;
        }
    }
    if (col != 0)
        send_to_char ("\n\r", ch);
}

/* changes your scroll */
void do_scroll (CHAR_DATA * ch, char *argument) {
    char arg[MAX_INPUT_LENGTH];
    char buf[100];
    int lines;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        if (ch->lines == 0)
            send_to_char ("You do not page long messages.\n\r", ch);
        else {
            sprintf (buf, "You currently display %d lines per page.\n\r",
                     ch->lines + 2);
            send_to_char (buf, ch);
        }
        return;
    }
    BAIL_IF (!is_number (arg),
        "You must provide a number.\n\r", ch);

    lines = atoi (arg);
    if (lines == 0) {
        send_to_char ("Paging disabled.\n\r", ch);
        ch->lines = 0;
        return;
    }
    BAIL_IF (lines < 10 || lines > 100,
        "You must provide a reasonable number.\n\r", ch);

    sprintf (buf, "Scroll set to %d lines.\n\r", lines);
    send_to_char (buf, ch);
    ch->lines = lines - 2;
}

/* ColoUr setting and unsetting, way cool, Ant Oct 94
 * revised to include config colour, Ant Feb 95
 * Modified by Synival */
void do_colour (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    const COLOUR_SETTING_TYPE *setting;
    const COLOUR_TYPE *colour;
    bool use_default;

    /* TODO: could we use the switched character? */
    if (IS_NPC (ch)) {
        send_to_char_bw ("Nice try, you dumb NPC!\n\r", ch);
        return;
    }

    argument = one_argument (argument, arg1);
    if (!*arg1) {
        printf_to_char (ch,
            "Colour is currently %s.\n\r"
            "Colour syntax:\n\r"
            "    colour on\n\r"
            "    colour off\n\r"
            "    colour codes\n\r"
            "    colour <field>|all\n\r"
            "    colour <field>|all default\n\r"
            "    colour <field>|all <colour>\n\r"
            "    colour <field>|all beep|nobeep\n\r",
            IS_SET (ch->plr, PLR_COLOUR) ? "ON" : "OFF");
        return;
    }

    if (!str_cmp (arg1, "on")) {
        SET_BIT (ch->plr, PLR_COLOUR);
        send_to_char("{RCo{ylor {Yis {Gno{gw {cON{b. A{Bma{Mzi{mng{r!!{x\n\r", ch);
        return;
    }
    if (!str_cmp (arg1, "off")) {
        REMOVE_BIT (ch->plr, PLR_COLOUR);
        send_to_char("Color is now OFF. Lame!\n\r", ch);
        return;
    }
    if (!str_cmp (arg1, "codes")) {
        do_colour_codes (ch, argument);
        return;
    }

    /* What are we modifying the colour to? */
    argument = one_argument (argument, arg2);
    if (*arg2 == '\0') {
        colour = NULL;
        use_default = FALSE;
    }
    else if (!str_cmp (arg2, "default")) {
        colour = NULL;
        use_default = TRUE;
    }
    else {
        if ((colour = colour_get_by_name (arg2)) == NULL) {
            printf_to_char (ch,
                "Unrecognized colour '%s'. "
                "Type 'colour codes' for valid colours.\n\r", arg2);
            return;
        }
        use_default = FALSE;
    }

    /* View/modify all colours */
    buf[0] = '\0';
    if (!str_cmp (arg1, "all")) {
        int i;
        for (i = 0; i < COLOUR_MAX; i++)
            do_colour_one (ch, &(colour_setting_table[i]),
                colour, use_default, buf);
        page_to_char (buf, ch);
        return;
    }

    /* Colour specified - look it up! */
    if ((setting = colour_setting_get_by_name (arg1)) == NULL) {
        printf_to_char (ch,
            "Unrecognized colour setting '%s'. "
            "Type 'colour all' for valid settings.\n\r", arg1);
        return;
    }

    do_colour_one (ch, setting, colour, use_default, buf);
    page_to_char (buf, ch);
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
void do_autolist (CHAR_DATA * ch, char *argument) {
    /* lists most player flags */
    BAIL_IF (IS_NPC (ch),
        "NPCs can't use player flags.\n\r", ch);

    send_to_char ("   action         status\n\r", ch);
    send_to_char ("---------------------------\n\r", ch);

    do_autolist_flag ("autoassist",   ch, ch->plr,  PLR_AUTOASSIST);
    do_autolist_flag ("autoexit",     ch, ch->plr,  PLR_AUTOEXIT);
    do_autolist_flag ("autogold",     ch, ch->plr,  PLR_AUTOGOLD);
    do_autolist_flag ("autoloot",     ch, ch->plr,  PLR_AUTOLOOT);
    do_autolist_flag ("autosac",      ch, ch->plr,  PLR_AUTOSAC);
    do_autolist_flag ("autosplit",    ch, ch->plr,  PLR_AUTOSPLIT);

    send_to_char ("---------------------------\n\r", ch);
    do_autolist_flag ("telnetga",     ch, ch->comm, COMM_TELNET_GA);
    do_autolist_flag ("brief",        ch, ch->comm, COMM_BRIEF);
    do_autolist_flag ("compactmode",  ch, ch->comm, COMM_COMPACT);
    do_autolist_flag ("showaffects",  ch, ch->comm, COMM_SHOW_AFFECTS);
    do_autolist_flag ("prompt",       ch, ch->comm, COMM_PROMPT);
    do_autolist_flag ("combineitems", ch, ch->comm, COMM_COMBINE);
#ifndef VANILLA
    do_autolist_flag ("materials",    ch, ch->comm, COMM_MATERIALS);
#endif

    send_to_char ("---------------------------\n\r", ch);
    do_autolist_flag ("noloot",       ch, ~ch->plr, PLR_CANLOOT);
    do_autolist_flag ("nosummon",     ch, ch->plr,  PLR_NOSUMMON);
    do_autolist_flag ("nofollow",     ch, ch->plr,  PLR_NOFOLLOW);
}

void do_autoassist (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_AUTOASSIST,
        "Autoassist removed.\n\r",
        "You will now assist when needed.\n\r");
}

void do_autoexit (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->plr), PLR_AUTOEXIT,
        "Exits will no longer be displayed.\n\r",
        "Exits will now be displayed.\n\r");
}

void do_autogold (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_AUTOGOLD,
        "Autogold removed.\n\r",
        "Automatic gold looting set.\n\r");
}

void do_autoloot (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_AUTOLOOT,
        "Autolooting removed.\n\r",
        "Automatic corpse looting set.\n\r");
}

void do_autosac (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_AUTOSAC,
        "Autosacrificing removed.\n\r",
        "Automatic corpse sacrificing set.\n\r");
}

void do_autosplit (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_AUTOSPLIT,
        "Autosplitting removed.\n\r",
        "Automatic gold splitting set.\n\r");
}

void do_noloot (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_CANLOOT,
        "Your corpse is now safe from thieves.\n\r",
        "Your corpse may now be looted.\n\r");
}

void do_nofollow (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, TRUE, &(ch->plr), PLR_NOFOLLOW,
        "You now accept followers.\n\r",
        "You no longer accept followers.\n\r");
    if (IS_SET (ch->plr, PLR_NOFOLLOW))
        die_follower (ch);
}

void do_telnetga (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_TELNET_GA,
        "Telnet GA removed.\n\r",
        "Telnet GA enabled.\n\r");
}

void do_brief (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_BRIEF,
        "Full descriptions activated.\n\r",
        "Short descriptions activated.\n\r");
}

void do_compact (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_COMPACT,
        "Compact mode removed.\n\r",
        "Compact mode set.\n\r");
}

void do_show_affects (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_SHOW_AFFECTS,
        "Affects will no longer be shown in score.\n\r",
        "Affects will now be shown in score.\n\r");
}

void do_combine (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_COMBINE,
        "Long inventory selected.\n\r",
        "Combined inventory selected.\n\r");
}

void do_materials (CHAR_DATA * ch, char *argument) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_MATERIALS,
        "Object and character materials will no longer be displayed.\n\r",
        "You will now see object and character materials.\n\r");
}

void do_nosummon (CHAR_DATA * ch, char *argument) {
    if (IS_NPC (ch)) {
        do_flag_toggle (ch, FALSE, &(ch->imm_flags), RES_SUMMON,
            "You are no longer immune to summon.\n\r",
            "You are now immune to summoning.\n\r");
    }
    else {
        do_flag_toggle (ch, FALSE, &(ch->plr), PLR_NOSUMMON,
            "You are no longer immune to summon.\n\r",
            "You are now immune to summoning.\n\r");
    }
}

void do_autoall (CHAR_DATA *ch, char * argument) {
    BAIL_IF (IS_NPC (ch),
        "NPCs can't use player flags.\n\r", ch);

    if (!strcmp (argument, "on")) {
        SET_BIT(ch->plr, PLR_AUTOASSIST);
        SET_BIT(ch->plr, PLR_AUTOEXIT);
        SET_BIT(ch->plr, PLR_AUTOGOLD);
        SET_BIT(ch->plr, PLR_AUTOLOOT);
        SET_BIT(ch->plr, PLR_AUTOSAC);
        SET_BIT(ch->plr, PLR_AUTOSPLIT);
        send_to_char("All autos turned on.\n\r",ch);
    }
    else if (!strcmp (argument, "off")) {
        REMOVE_BIT (ch->plr, PLR_AUTOASSIST);
        REMOVE_BIT (ch->plr, PLR_AUTOEXIT);
        REMOVE_BIT (ch->plr, PLR_AUTOGOLD);
        REMOVE_BIT (ch->plr, PLR_AUTOLOOT);
        REMOVE_BIT (ch->plr, PLR_AUTOSAC);
        REMOVE_BIT (ch->plr, PLR_AUTOSPLIT);
        send_to_char("All autos turned off.\n\r", ch);
    }
    else
        send_to_char("Usage: autoall [on|off]\n\r", ch);
}

void do_prompt (CHAR_DATA * ch, char *argument) {
    char buf[MAX_STRING_LENGTH];

    if (argument[0] == '\0') {
        do_flag_toggle (ch, FALSE, &(ch->comm), COMM_PROMPT,
            "You will no longer see prompts.\n\r",
            "You will now see prompts.\n\r");
            return;
    }

    if (!strcmp (argument, "show")) {
        printf_to_char (ch, "%s\n\r", ch->prompt);
        return;
    }

    if (!strcmp (argument, "all"))
        strcpy (buf, "<%hhp %mm %vmv> ");
    else {
        if (strlen (argument) > 50)
            argument[50] = '\0';
        strcpy (buf, argument);
        smash_tilde (buf);
        if (str_suffix ("%c", buf))
            strcat (buf, " ");
    }

    str_free (ch->prompt);
    ch->prompt = str_dup (buf);
    sprintf (buf, "Prompt set to %s\n\r", ch->prompt);
    send_to_char (buf, ch);
}

void do_alia (CHAR_DATA * ch, char *argument) {
    send_to_char ("I'm sorry, alias must be entered in full.\n\r", ch);
    return;
}

void do_alias (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
    int pos;

    smash_tilde (argument);
    rch = OCH(ch);
    if (IS_NPC (rch))
        return;

    argument = one_argument (argument, arg);
    if (arg[0] == '\0') {
        BAIL_IF (rch->pcdata->alias[0] == NULL,
            "You have no aliases defined.\n\r", ch);

        send_to_char ("Your current aliases are:\n\r", ch);
        for (pos = 0; pos < MAX_ALIAS; pos++) {
            if (rch->pcdata->alias[pos] == NULL
                || rch->pcdata->alias_sub[pos] == NULL)
                break;

            printf_to_char (ch, "    %s:  %s\n\r", rch->pcdata->alias[pos],
                rch->pcdata->alias_sub[pos]);
        }
        return;
    }

    BAIL_IF (!str_prefix ("una", arg) || !str_cmp ("alias", arg),
        "Sorry, that word is reserved.\n\r", ch);

    /* More Edwin-inspired fixes. JR -- 10/15/00 */
    BAIL_IF (strchr (arg, ' ') || strchr (arg, '"') || strchr (arg, '\''),
        "The word to be aliased should not contain a space, "
            "a tick or a double-quote.\n\r", ch);

    if (argument[0] == '\0') {
        for (pos = 0; pos < MAX_ALIAS; pos++) {
            if (rch->pcdata->alias[pos] == NULL
                || rch->pcdata->alias_sub[pos] == NULL)
                break;

            if (!str_cmp (arg, rch->pcdata->alias[pos])) {
                printf_to_char (ch, "%s aliases to '%s'.\n\r",
                         rch->pcdata->alias[pos],
                         rch->pcdata->alias_sub[pos]);
                return;
            }
        }
        send_to_char ("That alias is not defined.\n\r", ch);
        return;
    }

    BAIL_IF (!str_prefix(argument, "delete") || !str_prefix(argument, "prefix"),
        "That shall not be done!\n\r", ch);

    for (pos = 0; pos < MAX_ALIAS; pos++) {
        if (rch->pcdata->alias[pos] == NULL)
            break;

        if (!str_cmp (arg, rch->pcdata->alias[pos])) { /* redefine an alias */
            str_free (rch->pcdata->alias_sub[pos]);
            rch->pcdata->alias_sub[pos] = str_dup (argument);
            sprintf (buf, "%s is now realiased to '%s'.\n\r", arg, argument);
            send_to_char (buf, ch);
            return;
        }
    }

    BAIL_IF (pos >= MAX_ALIAS,
        "Sorry, you have reached the alias limit.\n\r", ch);

    /* make a new alias */
    rch->pcdata->alias[pos] = str_dup (arg);
    rch->pcdata->alias_sub[pos] = str_dup (argument);
    printf_to_char (ch, "%s is now aliased to '%s'.\n\r", arg, argument);
}

void do_unalias (CHAR_DATA * ch, char *argument) {
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

    rch = OCH(ch);
    if (IS_NPC (rch))
        return;
    DO_REQUIRE_ARG (arg, "Unalias what?\n\r");

    for (pos = 0; pos < MAX_ALIAS; pos++) {
        if (rch->pcdata->alias[pos] == NULL)
            break;

        if (found) {
            rch->pcdata->alias[pos - 1] = rch->pcdata->alias[pos];
            rch->pcdata->alias_sub[pos - 1] = rch->pcdata->alias_sub[pos];
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            continue;
        }

        if (!strcmp (arg, rch->pcdata->alias[pos])) {
            send_to_char ("Alias removed.\n\r", ch);
            str_free (rch->pcdata->alias[pos]);
            str_free (rch->pcdata->alias_sub[pos]);
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            found = TRUE;
        }
    }

    if (!found)
        send_to_char ("No alias of that name to remove.\n\r", ch);
}
