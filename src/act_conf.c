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

/*   QuickMUD - The Lazy Man's ROM - $Id: act_info.c,v 1.3 2000/12/01 10:48:33 ring0 Exp $ */

#include "act_conf.h"

#include "chars.h"
#include "colour.h"
#include "comm.h"
#include "db.h"
#include "do_sub.h"
#include "groups.h"
#include "interp.h"
#include "lookup.h"
#include "memory.h"
#include "tables.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

void do_colour_one (CHAR_T *ch, const COLOUR_SETTING_T *setting,
    const COLOUR_T *colour, bool use_default, char *buf)
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
    if (EXT_IS_SET (ch->ext_plr, PLR_COLOUR))
        colour_to_ansi (*col_flag, buf + strlen(buf), MAX_STRING_LENGTH);

    /* Arrange the color nicely. */
    colour_to_full_name (*col_flag, colour_buf, sizeof(colour_buf));
    for (argument = colour_buf; *argument != '\0'; ) {
        argument = one_argument (argument, arg);
        sprintf (buf + strlen(buf), " %-15s", arg);
    }

    if (EXT_IS_SET (ch->ext_plr, PLR_COLOUR))
        colour_to_ansi (CC_CLEAR, buf + strlen(buf), MAX_STRING_LENGTH);

    /* Done! */
    strcat (buf, "\n\r");
}

void do_colour_codes (CHAR_T *ch, char *argument) {
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
DEFINE_DO_FUN (do_scroll) {
    char arg[MAX_INPUT_LENGTH];
    int lines;

    one_argument (argument, arg);
    if (arg[0] == '\0') {
        if (ch->lines == 0)
            send_to_char ("You do not page long messages.\n\r", ch);
        else {
            printf_to_char (ch, "You currently display %d lines per page.\n\r",
                ch->lines + 2);
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

    printf_to_char (ch, "Scroll set to %d lines.\n\r", lines);
    ch->lines = lines - 2;
}

/* ColoUr setting and unsetting, way cool, Ant Oct 94
 * revised to include config colour, Ant Feb 95
 * Modified by Synival */
DEFINE_DO_FUN (do_colour) {
    char buf[MAX_STRING_LENGTH];
    char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
    const COLOUR_SETTING_T *setting;
    const COLOUR_T *colour;
    bool use_default;

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
            EXT_IS_SET (ch->ext_plr, PLR_COLOUR) ? "ON" : "OFF");
        return;
    }

    if (!str_cmp (arg1, "on")) {
        EXT_SET (ch->ext_plr, PLR_COLOUR);
        send_to_char("{RCo{ylor {Yis {Gno{gw {cON{b. A{Bma{Mzi{mng{r!!{x\n\r", ch);
        return;
    }
    if (!str_cmp (arg1, "off")) {
        EXT_UNSET (ch->ext_plr, PLR_COLOUR);
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
        for (i = 0; i < COLOUR_SETTING_MAX; i++)
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
DEFINE_DO_FUN (do_autolist) {
    /* lists most player flags */
    BAIL_IF (IS_NPC (ch),
        "NPCs can't use player flags.\n\r", ch);

    send_to_char ("   action         status\n\r", ch);
    send_to_char ("---------------------------\n\r", ch);

    do_autolist_ext_flag ("autoassist",   ch, ch->ext_plr,  PLR_AUTOASSIST);
    do_autolist_ext_flag ("autoexit",     ch, ch->ext_plr,  PLR_AUTOEXIT);
    do_autolist_ext_flag ("autogold",     ch, ch->ext_plr,  PLR_AUTOGOLD);
    do_autolist_ext_flag ("autoloot",     ch, ch->ext_plr,  PLR_AUTOLOOT);
    do_autolist_ext_flag ("autosac",      ch, ch->ext_plr,  PLR_AUTOSAC);
    do_autolist_ext_flag ("autosplit",    ch, ch->ext_plr,  PLR_AUTOSPLIT);

    send_to_char ("---------------------------\n\r", ch);
    do_autolist_flag ("telnetga",     ch, ch->comm, COMM_TELNET_GA);
    do_autolist_flag ("brief",        ch, ch->comm, COMM_BRIEF);
    do_autolist_flag ("compactmode",  ch, ch->comm, COMM_COMPACT);
    do_autolist_flag ("showaffects",  ch, ch->comm, COMM_SHOW_AFFECTS);
    do_autolist_flag ("prompt",       ch, ch->comm, COMM_PROMPT);
    do_autolist_flag ("combineitems", ch, ch->comm, COMM_COMBINE);
#ifdef BASEMUD_MATERIALS_COMMAND
    do_autolist_flag ("materials",    ch, ch->comm, COMM_MATERIALS);
#endif

    send_to_char ("---------------------------\n\r", ch);
    do_autolist_ext_flag ("noloot",       ch, EXT_INVERTED (ch->ext_plr), PLR_CANLOOT);
    do_autolist_ext_flag ("nosummon",     ch, ch->ext_plr,  PLR_NOSUMMON);
    do_autolist_ext_flag ("nofollow",     ch, ch->ext_plr,  PLR_NOFOLLOW);
}

DEFINE_DO_FUN (do_autoassist) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_AUTOASSIST,
        "Autoassist removed.\n\r",
        "You will now assist when needed.\n\r");
}

DEFINE_DO_FUN (do_autoexit) {
    do_ext_flag_toggle (ch, FALSE, &(ch->ext_plr), PLR_AUTOEXIT,
        "Exits will no longer be displayed.\n\r",
        "Exits will now be displayed.\n\r");
}

DEFINE_DO_FUN (do_autogold) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_AUTOGOLD,
        "Autogold removed.\n\r",
        "Automatic gold looting set.\n\r");
}

DEFINE_DO_FUN (do_autoloot) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_AUTOLOOT,
        "Autolooting removed.\n\r",
        "Automatic corpse looting set.\n\r");
}

DEFINE_DO_FUN (do_autosac) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_AUTOSAC,
        "Autosacrificing removed.\n\r",
        "Automatic corpse sacrificing set.\n\r");
}

DEFINE_DO_FUN (do_autosplit) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_AUTOSPLIT,
        "Autosplitting removed.\n\r",
        "Automatic gold splitting set.\n\r");
}

DEFINE_DO_FUN (do_noloot) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_CANLOOT,
        "Your corpse is now safe from thieves.\n\r",
        "Your corpse may now be looted.\n\r");
}

DEFINE_DO_FUN (do_nofollow) {
    do_ext_flag_toggle (ch, TRUE, &(ch->ext_plr), PLR_NOFOLLOW,
        "You now accept followers.\n\r",
        "You no longer accept followers.\n\r");
    if (EXT_IS_SET (ch->ext_plr, PLR_NOFOLLOW))
        die_follower (ch);
}

DEFINE_DO_FUN (do_telnetga) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_TELNET_GA,
        "Telnet GA removed.\n\r",
        "Telnet GA enabled.\n\r");
}

DEFINE_DO_FUN (do_brief) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_BRIEF,
        "Full descriptions activated.\n\r",
        "Short descriptions activated.\n\r");
}

DEFINE_DO_FUN (do_compact) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_COMPACT,
        "Compact mode removed.\n\r",
        "Compact mode set.\n\r");
}

DEFINE_DO_FUN (do_show_affects) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_SHOW_AFFECTS,
        "Affects will no longer be shown in score.\n\r",
        "Affects will now be shown in score.\n\r");
}

DEFINE_DO_FUN (do_combine) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_COMBINE,
        "Long inventory selected.\n\r",
        "Combined inventory selected.\n\r");
}

DEFINE_DO_FUN (do_materials) {
    do_flag_toggle (ch, FALSE, &(ch->comm), COMM_MATERIALS,
        "Object and character materials will no longer be displayed.\n\r",
        "You will now see object and character materials.\n\r");
}

DEFINE_DO_FUN (do_nosummon) {
    if (IS_NPC (ch)) {
        do_flag_toggle (ch, FALSE, &(ch->imm_flags), RES_SUMMON,
            "You are no longer immune to summon.\n\r",
            "You are now immune to summoning.\n\r");
    }
    else {
        do_ext_flag_toggle (ch, FALSE, &(ch->ext_plr), PLR_NOSUMMON,
            "You are no longer immune to summon.\n\r",
            "You are now immune to summoning.\n\r");
    }
}

DEFINE_DO_FUN (do_autoall) {
    BAIL_IF (IS_NPC (ch),
        "NPCs can't use player flags.\n\r", ch);

    if (!strcmp (argument, "on")) {
        EXT_SET (ch->ext_plr, PLR_AUTOASSIST);
        EXT_SET (ch->ext_plr, PLR_AUTOEXIT);
        EXT_SET (ch->ext_plr, PLR_AUTOGOLD);
        EXT_SET (ch->ext_plr, PLR_AUTOLOOT);
        EXT_SET (ch->ext_plr, PLR_AUTOSAC);
        EXT_SET (ch->ext_plr, PLR_AUTOSPLIT);
        send_to_char ("All autos turned on.\n\r", ch);
    }
    else if (!strcmp (argument, "off")) {
        EXT_UNSET (ch->ext_plr, PLR_AUTOASSIST);
        EXT_UNSET (ch->ext_plr, PLR_AUTOEXIT);
        EXT_UNSET (ch->ext_plr, PLR_AUTOGOLD);
        EXT_UNSET (ch->ext_plr, PLR_AUTOLOOT);
        EXT_UNSET (ch->ext_plr, PLR_AUTOSAC);
        EXT_UNSET (ch->ext_plr, PLR_AUTOSPLIT);
        send_to_char ("All autos turned off.\n\r", ch);
    }
    else
        send_to_char ("Usage: autoall [on|off]\n\r", ch);
}

DEFINE_DO_FUN (do_prompt) {
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
        strcpy (buf, DEFAULT_PROMPT);
    else {
        if (strlen (argument) > 50)
            argument[50] = '\0';
        strcpy (buf, argument);
        str_smash_tilde (buf);
        if (str_suffix ("%c", buf))
            strcat (buf, " ");
    }

    str_free (&(ch->prompt));
    ch->prompt = str_dup (buf);
    printf_to_char (ch, "Prompt set to %s\n\r", ch->prompt);
}

DEFINE_DO_FUN (do_alia) {
    send_to_char ("I'm sorry, alias must be entered in full.\n\r", ch);
}

DEFINE_DO_FUN (do_alias) {
    CHAR_T *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;

    str_smash_tilde (argument);
    rch = REAL_CH (ch);
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
            str_free (&(rch->pcdata->alias_sub[pos]));
            rch->pcdata->alias_sub[pos] = str_dup (argument);
            printf_to_char (ch, "%s is now realiased to '%s'.\n\r",
                arg, argument);
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

DEFINE_DO_FUN (do_unalias) {
    CHAR_T *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;

    rch = REAL_CH (ch);
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
            str_free (&(rch->pcdata->alias[pos]));
            str_free (&(rch->pcdata->alias_sub[pos]));
            rch->pcdata->alias[pos] = NULL;
            rch->pcdata->alias_sub[pos] = NULL;
            found = TRUE;
        }
    }

    if (!found)
        send_to_char ("No alias of that name to remove.\n\r", ch);
}
