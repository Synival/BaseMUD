/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "comm.h"
#include "utils.h"
#include "interp.h"
#include "olc.h"
#include "db.h"
#include "olc_mpedit.h"
#include "globals.h"
#include "memory.h"

#include "string.h"

/*****************************************************************************
 Name:       string_edit
 Purpose:    Clears string and puts player into editing mode.
 Called by:  none
 ****************************************************************************/
void string_edit (CHAR_T *ch, char **string_edit) {
    printf_to_char (ch, "-========- Entering EDIT Mode -=========-\n\r");
    printf_to_char (ch, "    Type .h on a new line for help\n\r");
    printf_to_char (ch, " Terminate with a ~ or @ on a blank line.\n\r");
    printf_to_char (ch, "-=======================================-\n\r");

    if (*string_edit == NULL)
        *string_edit = str_dup ("");
    else
        **string_edit = '\0';
    ch->desc->string_edit = string_edit;
}

/*****************************************************************************
 Name:       string_append
 Purpose:    Puts player into append mode for given string.
 Called by:  (many)olc_act.c
 ****************************************************************************/
void string_append (CHAR_T *ch, char **string_edit) {
    printf_to_char (ch, "-=======- Entering APPEND Mode -========-\n\r");
    printf_to_char (ch, "    Type .h on a new line for help\n\r");
    printf_to_char (ch, " Terminate with a ~ or @ on a blank line.\n\r");
    printf_to_char (ch, "-=======================================-\n\r");

    if (*string_edit == NULL)
        *string_edit = str_dup ("");
    send_to_char (numlines (*string_edit), ch);

/* numlines sends the string with \n\r */
/*  if ( *(*string_edit + strlen( *string_edit ) - 1) != '\r' )
    send_to_char( "\n\r", ch ); */

    ch->desc->string_edit = string_edit;
}

/*****************************************************************************
 Name:       string_replace
 Purpose:    Substitutes one string for another.
 Called by:  string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
char *string_replace (char *orig, char *old, char *new) {
    char xbuf[MAX_STRING_LENGTH];
    int i;

    xbuf[0] = '\0';
    strcpy (xbuf, orig);
    if (strstr (orig, old) != NULL) {
        i = strlen (orig) - strlen (strstr (orig, old));
        xbuf[i] = '\0';
        strcat (xbuf, new);
        strcat (xbuf, &orig[i + strlen (old)]);
        str_free (&(orig));
    }

    return str_dup (xbuf);
}

/*****************************************************************************
 Name:       string_add
 Purpose:    Interpreter for string editing.
 Called by:  game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add (CHAR_T *ch, char *argument) {
    char buf[MAX_STRING_LENGTH];

    /* Thanks to James Seng */
    str_smash_tilde (argument);
    if (*argument == '.') {
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char arg3[MAX_INPUT_LENGTH];
        char tmparg3[MAX_INPUT_LENGTH];

        argument = one_argument (argument, arg1);
        argument = first_arg (argument, arg2, FALSE);
        strcpy (tmparg3, argument);
        argument = first_arg (argument, arg3, FALSE);

        if (!str_cmp (arg1, ".c")) {
            printf_to_char (ch, "String cleared.\n\r");
            str_replace_dup (ch->desc->string_edit, "");
            return;
        }
        if (!str_cmp (arg1, ".s")) {
            printf_to_char (ch, "String so far:\n\r");
            send_to_char (numlines (*ch->desc->string_edit), ch);
            return;
        }
        if (!str_cmp (arg1, ".r")) {
            if (arg2[0] == '\0') {
                printf_to_char (ch, "usage:  .r \"old string\" \"new string\"\n\r");
                return;
            }

            *ch->desc->string_edit =
                string_replace (*ch->desc->string_edit, arg2, arg3);
            printf_to_char (ch, "'%s' replaced with '%s'.\n\r", arg2, arg3);
            return;
        }
        if (!str_cmp (arg1, ".f")) {
            *ch->desc->string_edit = format_string (*ch->desc->string_edit);
            printf_to_char (ch, "String formatted.\n\r");
            return;
        }
        if (!str_cmp (arg1, ".ld")) {
            *ch->desc->string_edit =
                string_linedel (*ch->desc->string_edit, atoi (arg2));
            printf_to_char (ch, "Line deleted.\n\r");
            return;
        }
        if (!str_cmp (arg1, ".li")) {
            *ch->desc->string_edit =
                string_lineadd (*ch->desc->string_edit, tmparg3, atoi (arg2));
            printf_to_char (ch, "Line inserted.\n\r");
            return;
        }
        if (!str_cmp (arg1, ".lr")) {
            *ch->desc->string_edit =
                string_linedel (*ch->desc->string_edit, atoi (arg2));
            *ch->desc->string_edit =
                string_lineadd (*ch->desc->string_edit, tmparg3, atoi (arg2));
            printf_to_char (ch, "Line replaced.\n\r");
            return;
        }
        if (!str_cmp (arg1, ".h")) {
            printf_to_char (ch, "Sedit help (commands on blank line):   \n\r");
            printf_to_char (ch, ".r 'old' 'new'   - replace a substring \n\r");
            printf_to_char (ch, "                   (requires '', \"\") \n\r");
            printf_to_char (ch, ".h               - get help (this info)\n\r");
            printf_to_char (ch, ".s               - show string so far  \n\r");
            printf_to_char (ch, ".f               - (word wrap) string  \n\r");
            printf_to_char (ch, ".c               - clear string so far \n\r");
            printf_to_char (ch, ".ld <num>        - delete line number <num>\n\r");
            printf_to_char (ch, ".li <num> <str>  - insert <str> at line <num>\n\r");
            printf_to_char (ch, ".lr <num> <str>  - replace line <num> with <str>\n\r");
            printf_to_char (ch, "@                - end string          \n\r");
            return;
        }

        printf_to_char (ch, "SEdit:  Invalid dot command.\n\r");
        return;
    }

    if (*argument == '~' || *argument == '@') {
        if (ch->desc->editor == ED_MPCODE) { /* for the mobprogs */
            MOB_INDEX_T *mob;
            int hash;
            MPROG_LIST_T *mpl;
            MPROG_CODE_T *mpc;

            EDIT_MPCODE (ch, mpc);

            if (mpc != NULL)
                for (hash = 0; hash < MAX_KEY_HASH; hash++)
                    for (mob = mob_index_hash[hash]; mob; mob = mob->hash_next)
                        for (mpl = mob->mprog_first; mpl; mpl = mpl->mob_next)
                            if (mpl->vnum == mpc->vnum) {
                                printf_to_char (ch, "Editting mob %d.\n\r",
                                    mob->vnum);
                                mpl->code = mpc->code;
                            }
        }

        ch->desc->string_edit = NULL;
        return;
    }

    strcpy (buf, *ch->desc->string_edit);

    /* Truncate strings to MAX_STRING_LENGTH.
     * --------------------------------------
     * Edwin strikes again! Fixed avoid adding to a too-long
     * note. JR -- 10/15/00 */
    if (strlen ( *ch->desc->string_edit ) + strlen (argument) >= (MAX_STRING_LENGTH - 4)) {
        printf_to_char (ch, "String too long, last line skipped.\n\r");

        /* Force character out of editing mode. */
        ch->desc->string_edit = NULL;
        return;
    }

    /* Ensure no tilde's inside string.
     * -------------------------------- */
    str_smash_tilde (argument);

    strcat (buf, argument);
    strcat (buf, "\n\r");
    str_replace_dup (ch->desc->string_edit, buf);
}

/* Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality. */
/*****************************************************************************
 Name:       format_string
 Purpose:    Special string formating and word-wrapping.
 Called by:  string_add(string.c) (many)olc_act.c
 ****************************************************************************/
char *format_string (char *oldstring /*, bool space */ ) {
    char xbuf[MAX_STRING_LENGTH];
    char xbuf2[MAX_STRING_LENGTH];
    char *rdesc;
    int i = 0;
    bool cap = TRUE;

    xbuf[0] = xbuf2[0] = 0;
    i = 0;

    for (rdesc = oldstring; *rdesc; rdesc++) {
        if (*rdesc == '\n') {
            if (xbuf[i - 1] != ' ') {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if (*rdesc == '\r')
            ; /* empty */
        else if (*rdesc == ' ') {
            if (xbuf[i - 1] != ' ') {
                xbuf[i] = ' ';
                i++;
            }
        }
        else if (*rdesc == ')') {
            if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                 || xbuf[i - 3] == '!'))
            {
                xbuf[i - 2] = *rdesc;
                xbuf[i - 1] = ' ';
                xbuf[i] = ' ';
                i++;
            }
            else {
                xbuf[i] = *rdesc;
                i++;
            }
        }
        else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!') {
            if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' &&
                (xbuf[i - 3] == '.' || xbuf[i - 3] == '?'
                 || xbuf[i - 3] == '!'))
            {
                xbuf[i - 2] = *rdesc;
                if (*(rdesc + 1) != '\"') {
                    xbuf[i - 1] = ' ';
                    xbuf[i] = ' ';
                    i++;
                }
                else {
                    xbuf[i - 1] = '\"';
                    xbuf[i] = ' ';
                    xbuf[i + 1] = ' ';
                    i += 2;
                    rdesc++;
                }
            }
            else {
                xbuf[i] = *rdesc;
                if (*(rdesc + 1) != '\"') {
                    xbuf[i + 1] = ' ';
                    xbuf[i + 2] = ' ';
                    i += 3;
                }
                else {
                    xbuf[i + 1] = '\"';
                    xbuf[i + 2] = ' ';
                    xbuf[i + 3] = ' ';
                    i += 4;
                    rdesc++;
                }
            }
            cap = TRUE;
        }
        else {
            xbuf[i] = *rdesc;
            if (cap) {
                cap = FALSE;
                xbuf[i] = UPPER (xbuf[i]);
            }
            i++;
        }
    }
    xbuf[i] = 0;
    strcpy (xbuf2, xbuf);

    rdesc = xbuf2;
    xbuf[0] = 0;

    while (1) {
        for (i = 0; i < 77; i++)
            if (!*(rdesc + i))
                break;
        if (i < 77)
            break;
        for (i = (xbuf[0] ? 76 : 73); i; i--)
            if (*(rdesc + i) == ' ')
                break;
        if (i) {
            *(rdesc + i) = 0;
            strcat (xbuf, rdesc);
            strcat (xbuf, "\n\r");
            rdesc += i + 1;
            while (*rdesc == ' ')
                rdesc++;
        }
        else {
            bug ("No spaces", 0);
            *(rdesc + 75) = 0;
            strcat (xbuf, rdesc);
            strcat (xbuf, "-\n\r");
            rdesc += 76;
        }
    }
    while (*(rdesc + i) && (*(rdesc + i) == ' ' ||
                            *(rdesc + i) == '\n' || *(rdesc + i) == '\r'))
        i--;
    *(rdesc + i + 1) = 0;
    strcat (xbuf, rdesc);
    if (xbuf[strlen (xbuf) - 2] != '\n')
        strcat (xbuf, "\n\r");

    str_free (&(oldstring));
    return (str_dup (xbuf));
}

/* Used above in string_add.  Because this function does not
 * modify case if mod_case is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument. */

/*****************************************************************************
 Name:    first_arg
 Purpose: Pick off one argument from a string and return the rest.
          Understands quates, parenthesis (barring ) ('s) and
          percentages.
 Called by:    string_add(string.c)
 ****************************************************************************/
char *first_arg (char *argument, char *arg_first, bool mod_case) {
    char end;

    while (*argument == ' ')
        argument++;

    end = ' ';
    if (*argument == '\'' || *argument == '"'
        || *argument == '%' || *argument == '(')
    {
        if (*argument == '(') {
            end = ')';
            argument++;
        }
        else
            end = *argument++;
    }
    while (*argument != '\0') {
        if (*argument == end) {
            argument++;
            break;
        }
        if (mod_case)
            *arg_first = LOWER (*argument);
        else
            *arg_first = *argument;
        arg_first++;
        argument++;
    }
    *arg_first = '\0';

    while (*argument == ' ')
        argument++;

    return argument;
}

/* Used in olc_act.c for aedit_builders. */
char *string_unpad (char *argument) {
    char buf[MAX_STRING_LENGTH];
    char *s;

    s = argument;

    while (*s == ' ')
        s++;

    strcpy (buf, s);
    s = buf;

    if (*s != '\0') {
        while (*s != '\0')
            s++;
        s--;

        while (*s == ' ')
            s--;
        s++;
        *s = '\0';
    }

    str_free (&(argument));
    return str_dup (buf);
}

/* Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder. */
char *string_proper (char *argument) {
    char *s;

    s = argument;
    while (*s != '\0') {
        if (*s != ' ') {
            *s = UPPER (*s);
            while (*s != ' ' && *s != '\0')
                s++;
        }
        else
            s++;
    }
    return argument;
}

char *string_linedel (char *string, int line) {
    char *strtmp = string;
    char buf[MAX_STRING_LENGTH];
    int cnt = 1, tmp = 0;

    buf[0] = '\0';
    for (; *strtmp != '\0'; strtmp++) {
        if (cnt != line)
            buf[tmp++] = *strtmp;

        if (*strtmp == '\n') {
            if (*(strtmp + 1) == '\r') {
                if (cnt != line)
                    buf[tmp++] = *(++strtmp);
                else
                    ++strtmp;
            }
            cnt++;
        }
    }

    buf[tmp] = '\0';

    str_free (&(string));
    return str_dup (buf);
}

char *string_lineadd (char *string, char *newstr, int line) {
    char *strtmp = string;
    int cnt = 1, tmp = 0;
    bool done = FALSE;
    char buf[MAX_STRING_LENGTH];

    buf[0] = '\0';
    for (; *strtmp != '\0' || (!done && cnt == line); strtmp++) {
        if (cnt == line && !done) {
            strcat (buf, newstr);
            strcat (buf, "\n\r");
            tmp += strlen (newstr) + 2;
            cnt++;
            done = TRUE;
        }

        buf[tmp++] = *strtmp;
        if (done && *strtmp == '\0')
            break;

        if (*strtmp == '\n') {
            if (*(strtmp + 1) == '\r')
                buf[tmp++] = *(++strtmp);
            cnt++;
        }
        buf[tmp] = '\0';
    }

    str_free (&(string));
    return str_dup (buf);
}

char *merc_getline (char *str, char *buf) {
    int tmp = 0;
    bool found = FALSE;

    while (*str) {
        if (*str == '\n') {
            found = TRUE;
            break;
        }
        buf[tmp++] = *(str++);
    }
    if (found) {
        if (*(str + 1) == '\r')
            str += 2;
        else
            str += 1;
    }

    buf[tmp] = '\0';
    return str;
}

char *numlines (char *string) {
    int cnt = 1;
    static char buf[MAX_STRING_LENGTH * 2];
    char buf2[MAX_STRING_LENGTH], tmpb[MAX_STRING_LENGTH];

    buf[0] = '\0';
    while (*string) {
        string = merc_getline (string, tmpb);
        sprintf (buf2, "%2d. %s\n\r", cnt++, tmpb);
        strcat (buf, buf2);
    }
    return buf;
}
