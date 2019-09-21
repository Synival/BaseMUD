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

/* TODO: VVVV review that! VVVV */

/* This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "colour.h"
#include "recycle.h"
#include "utils.h"
#include "fight.h"
#include "interp.h"
#include "db.h"
#include "olc.h"
#include "save.h"
#include "mob_prog.h"
#include "lookup.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "descs.h"

#include "comm.h"

/* TODO: move definitions to header files. */
/* TODO: review names and usages of global variables. */
/* TODO: review most of this, it's been largely untouched. */
/* TODO: compatibility is probably lost :-( */
/* TODO: split socket / descriptor related functions to a different file. */
/* TODO: move game loop to 'main.c' */
/* TODO: there's a lot of random functions that don't belong in here.
 *       move them to somewhere appropriate! */

/* Socket and TCP/IP stuff. */
#if defined(macintosh) || defined(MSDOS)
    const char echo_off_str[] = { '\0' };
    const char echo_on_str[]  = { '\0' };
    const char go_ahead_str[] = { '\0' };
#endif

#if defined(unix)
    #include "telnet.h"
    const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
    const char echo_on_str[]  = { IAC, WONT, TELOPT_ECHO, '\0' };
    const char go_ahead_str[] = { IAC, GA, '\0' };
#endif

/* Malloc debugging stuff. */
#if defined(sun)
    #undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
    #include <malloc.h>
    extern int malloc_debug args ((int));
    extern int malloc_verify args ((void));
#endif

/* Global variables. */
DESCRIPTOR_DATA *descriptor_list; /* All open descriptors     */
DESCRIPTOR_DATA *d_next;          /* Next descriptor in loop  */
FILE *fpReserve;                  /* Reserved file handle     */
bool god;                         /* All new chars are gods!  */
bool merc_down;                   /* Shutdown         */
bool wizlock;                     /* Game is wizlocked        */
bool newlock;                     /* Game is newlocked        */
char str_boot_time[MAX_INPUT_LENGTH];
time_t current_time;              /* time of this pulse */
bool MOBtrigger = TRUE;           /* act() switch */

/* Needs to be global because of do_copyover */
int port, control;

/* Put global mud config values here. Look at qmconfig command for clues.     */
/*   -- JR 09/23/2000                                                         */
/* Set values for all but IP address in ../area/qmconfig.rc file.             */
/*   -- JR 05/10/2001                                                         */
int mud_ansiprompt, mud_ansicolor, mud_telnetga;

/* Set this to the IP address you want to listen on (127.0.0.1 is good for    */
/* paranoid types who don't want the 'net at large peeking at their MUD)      */
char *mud_ipaddress = "0.0.0.0";

/* Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud */
void bust_a_prompt (CHAR_DATA *ch) {
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MAX_STRING_LENGTH * 2];

    if (ch == NULL || ch->desc == NULL)
        return;

    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0') {
        printf_to_char (ch, "{p<%dhp %dm %dmv>{x %s",
            ch->hit, ch->mana, ch->move, ch->prefix);
        return;
    }

    if (IS_SET (ch->comm, COMM_AFK)) {
        send_to_char ("{p<AFK>{x ", ch);
        return;
    }

    while (*str != '\0') {
        if (*str != '%') {
            *point++ = *str++;
            continue;
        }
        ++str;
        switch (*str) {
            case '%': buf2[0] = '%'; buf2[1] = '\0';           i = buf2; break;
            case 'c': sprintf (buf2, "%s",  "\n\r");           i = buf2; break;
            case 'h': sprintf (buf2, "%d",  ch->hit);          i = buf2; break;
            case 'H': sprintf (buf2, "%d",  ch->max_hit);      i = buf2; break;
            case 'm': sprintf (buf2, "%d",  ch->mana);         i = buf2; break;
            case 'M': sprintf (buf2, "%d",  ch->max_mana);     i = buf2; break;
            case 'v': sprintf (buf2, "%d",  ch->move);         i = buf2; break;
            case 'V': sprintf (buf2, "%d",  ch->max_move);     i = buf2; break;
            case 'x': sprintf (buf2, "%d",  ch->exp);          i = buf2; break;
            case 'g': sprintf (buf2, "%ld", ch->gold);         i = buf2; break;
            case 's': sprintf (buf2, "%ld", ch->silver);       i = buf2; break;
            case 'o': sprintf (buf2, "%s",  olc_ed_name (ch)); i = buf2; break;
            case 'O': sprintf (buf2, "%s",  olc_ed_vnum (ch)); i = buf2; break;

            case 'e':
                char_exit_string (ch, ch->in_room, EXITS_PROMPT,
                    buf2, sizeof (buf2));
                i = buf2;
                break;

            case 'X':
                sprintf (buf2, "%d", IS_NPC (ch) ? 0 : (ch->level + 1)
                    * exp_per_level (ch, ch->pcdata->points) - ch->exp);
                i = buf2;
                break;

            case 'a':
                if (ch->level > 9)
                    sprintf (buf2, "%d", ch->alignment);
                else
                    sprintf (buf2, "%s",
                        IS_GOOD (ch) ? "good" :
                        IS_EVIL (ch) ? "evil" :
                                       "neutral");
                i = buf2;
                break;

            case 'r':
                if (ch->in_room != NULL) {
                    sprintf (buf2, "%s",
                        ((!IS_NPC (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT)) ||
                         (!IS_AFFECTED (ch, AFF_BLIND) &&
                             !room_is_dark (ch->in_room))
                        ) ? ch->in_room-> name : "darkness"
                    );
                    i = buf2;
                }
                else
                    i = " ";
                break;

            case 'R':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL) {
                    sprintf (buf2, "%d", ch->in_room->vnum);
                    i = buf2;
                }
                else
                    i = " ";
                break;

            case 'z':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%s", ch->in_room->area->title);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;

            case 'p':
                sprintf (buf2, "%s%s", ch->fighting ? "!" : "",
                    get_character_position_str (ch, ch->position, ch->on, FALSE));
                i = buf2;
                break;

            default:
                i = " ";
                break;
        }
        ++str;
        while ((*point = *i) != '\0')
            ++point, ++i;
    }

    *point = '\0';
    pbuff = buffer;
    colour_puts (ch, ch->desc->ansi, buf, pbuff, MAX_STRING_LENGTH);
    send_to_char ("{p", ch);
    write_to_buffer (ch->desc, buffer, 0);
    send_to_char ("{x", ch);

    if (ch->prefix[0] != '\0')
        write_to_buffer (ch->desc, ch->prefix, 0);
}

/* Parse a name for acceptability. */
bool check_parse_name (char *name) {
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

    if (str_cmp (capitalize (name), "Alander") && (!str_prefix ("Alan", name)
            || !str_suffix ("Alander", name)))
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

    /* Alphanumerics only.
     * Lock out IllIll twits. */
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
            sprintf (log_buf,"Double newbie alert (%s)", name);
            wiznet (log_buf, NULL, NULL, WIZ_LOGINS, 0, 0);
            return FALSE;
        }
    }

    return TRUE;
}

void stop_idling (CHAR_DATA * ch) {
    if (ch == NULL
        || ch->desc == NULL
        || ch->desc->connected != CON_PLAYING
        || ch->was_in_room == NULL
        || ch->in_room != get_room_index (ROOM_VNUM_LIMBO)
    )
        return;

    ch->timer = 0;
    char_from_room (ch);
    char_to_room (ch, ch->was_in_room);
    ch->was_in_room = NULL;
    act ("$n has returned from the void.", ch, NULL, NULL, TO_NOTCHAR);
}

/* Write to one char. */
void send_to_char_bw (const char *txt, CHAR_DATA *ch) {
    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    write_to_buffer (ch->desc, txt, strlen (txt));
}

/* Send a page to one char. */
void page_to_char_bw (const char *txt, CHAR_DATA *ch) {
    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    if (ch->lines == 0) {
        send_to_char_bw (txt, ch);
        return;
    }

#if defined(macintosh)
    send_to_char_bw (txt, ch);
#else
    append_to_page (ch->desc, txt);
#endif
}

/* Page to one char, new colour version, by Lope. */
void send_to_char (const char *txt, CHAR_DATA * ch) {
    char buf[MAX_STRING_LENGTH * 4];
    int len;

    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    buf[0] = '\0';
    len = colour_puts (ch, ch->desc->ansi, txt, buf, sizeof(buf));
    write_to_buffer (ch->desc, buf, len);
}

void page_to_char (const char *txt, CHAR_DATA * ch) {
#if !defined(macintosh)
    char buf[MAX_STRING_LENGTH * 4];
#endif

    if (txt == NULL || ch == NULL || ch->desc == NULL)
        return;
    if (ch->lines == 0) {
        send_to_char (txt, ch);
        return;
    }

#if defined(macintosh)
    send_to_char (txt, ch);
#else
    buf[0] = '\0';
    colour_puts (ch, ch->desc->ansi, txt, buf, sizeof(buf));
    append_to_page (ch->desc, buf);
#endif
}

/* quick sex fixer */
void fix_sex (CHAR_DATA * ch) {
    if (ch->sex < 0 || ch->sex > 2)
        ch->sex = IS_NPC (ch) ? 0 : ch->pcdata->true_sex;
}

void act2 (const char *to_char, const char *to_room, CHAR_DATA *ch,
           const void *arg1, const void *arg2, flag_t flags, int min_pos)
{
    if (to_char)
        act_new (to_char, ch, arg1, arg2, flags | TO_CHAR, min_pos);
    if (to_room)
        act_new (to_room, ch, arg1, arg2, flags | TO_NOTCHAR, min_pos);
}

void act3 (const char *to_char, const char *to_vict, const char *to_room,
           CHAR_DATA *ch, const void *arg1, const void *arg2, flag_t flags,
           int min_pos)
{
    if (to_char)
        act_new (to_char, ch, arg1, arg2, flags | TO_CHAR, min_pos);
    if (to_vict && arg2) /* arg2 represents the victim */
        act_new (to_vict, ch, arg1, arg2, flags | TO_VICT, min_pos);
    if (to_room)
        act_new (to_room, ch, arg1, arg2, flags | TO_OTHERS, min_pos);
}

bool act_is_valid_recipient (CHAR_DATA *to, flag_t flags,
    CHAR_DATA *ch, CHAR_DATA *vch)
{
    if ((flags & TO_CHAR) && to == ch)
        return TRUE;
    if ((flags & TO_VICT) && to == vch && to != ch)
        return TRUE;
    if ((flags & TO_OTHERS) && (to != ch && to != vch))
        return TRUE;
    return FALSE;
}

char *act_code (char code, CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
    OBJ_DATA *obj1, OBJ_DATA *obj2, const void *arg1, const void *arg2,
    char *out_buf, size_t size)
{
    static char *const he_she[]  = { "it",  "he",  "she" };
    static char *const him_her[] = { "it",  "him", "her" };
    static char *const his_her[] = { "its", "his", "her" };

    #define FILTER_BAD_CODE(true_cond, message) \
        do { \
            if (!(true_cond)) { \
                bug ("act: " message, 0); \
                return " <@@@> "; \
            } \
        } while (0)

    switch (code) {
        /* Added checking of pointers to each case after reading about the
         * bug on Edwin's page. JR -- 10/15/00 */

        /* Thx alex for 't' idea */
        case 't':
            FILTER_BAD_CODE (arg1, "bad code $t for 'arg1'");
            return (char *) arg1;
        case 'T':
            FILTER_BAD_CODE (arg2, "bad code $T for 'arg2'");
            return (char *) arg2;
        case 'n':
            FILTER_BAD_CODE (ch && to, "bad code $n for 'ch' or 'to'");
            return PERS_AW (ch, to);
        case 'N':
            FILTER_BAD_CODE (vch && to, "bad code $N for 'vch' or 'to'");
            return PERS_AW (vch, to);
        case 'e':
            FILTER_BAD_CODE (ch, "bad code $e for 'ch'");
            return he_she[URANGE (0, ch->sex, 2)];
        case 'E':
            FILTER_BAD_CODE (vch, "bad code $E for 'vch'");
            return he_she[URANGE (0, vch->sex, 2)];
        case 'm':
            FILTER_BAD_CODE (ch, "bad code $m for 'ch'");
            return him_her[URANGE (0, ch->sex, 2)];
        case 'M':
            FILTER_BAD_CODE (vch, "bad code $M for 'vch'");
            return him_her[URANGE (0, vch->sex, 2)];
        case 's':
            FILTER_BAD_CODE (ch, "bad code $s for 'ch'");
            return his_her[URANGE (0, ch->sex, 2)];
        case 'S':
            FILTER_BAD_CODE (vch, "bad code $S for 'vch'");
            return his_her[URANGE (0, vch->sex, 2)];
        case 'p':
            FILTER_BAD_CODE (to && obj1, "bad code $p for 'to' or 'obj1'");
            return char_can_see_obj (to, obj1) ? obj1->short_descr : "something";
        case 'P':
            FILTER_BAD_CODE (to && obj2, "bad code $P for 'to' or 'obj2'");
            return char_can_see_obj (to, obj2) ? obj2->short_descr : "something";
        case 'd':
            return door_keyword_to_name ((char *) arg2, out_buf, size);

        default:
            bug ("bad code %d.", code);
            return " <@@@> ";
    }
}

void act_new (const char *format, CHAR_DATA * ch, const void *arg1,
              const void *arg2, flag_t flags, int min_pos)
{
    char buf[MAX_STRING_LENGTH];
    char code_buf[MAX_INPUT_LENGTH];
    CHAR_DATA *to;
    CHAR_DATA *vch = (CHAR_DATA *) arg2;
    OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
    OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
    const char *str;
    const char *i;
    char *point;
    char *pbuff;
    char buffer[MSL * 2];

    /* Discard null and zero-length messages. */
    if (format == NULL || format[0] == '\0')
        return;

    /* discard null rooms and chars */
    if (ch == NULL || ch->in_room == NULL)
        return;

    to = ch->in_room->people;
    if (flags == TO_VICT) {
        if (vch == NULL) {
            bug ("act: null vch with TO_VICT.", 0);
            return;
        }
        if (vch->in_room == NULL)
            return;
        to = vch->in_room->people;
    }

    for (; to != NULL; to = to->next_in_room) {
        if (!IS_NPC (to) && to->desc == NULL)
            continue;
        if (IS_NPC (to) && to->desc == NULL && !HAS_TRIGGER (to, TRIG_ACT))
            continue;
        if (to->position < min_pos)
            continue;
        if (!act_is_valid_recipient (to, flags, ch, vch))
            continue;

        point = buf;
        str = format;
        while (*str != '\0') {
            if (*str != '$') {
                *point++ = *str++;
                continue;
            }
            ++str;
            i = " <@@@> ";

            if (arg2 == NULL && *str >= 'A' && *str <= 'Z')
                bug ("act: missing arg2 for code %d.", *str);
            else
                i = act_code (*str, ch, vch, to, obj1, obj2, arg1, arg2,
                    code_buf, sizeof (code_buf));

            ++str;
            while ((*point = *i) != '\0')
                ++point, ++i;
        }

        *point++ = '\n';
        *point++ = '\r';
        *point = '\0';

        /* Kludge to capitalize first letter of buffer, trying
         * to account for { color codes. -- JR 09/09/00 */
        if (buf[0] == '{' && buf[1] != '{')
            buf[2] = UPPER (buf[2]);
        else
            buf[0] = UPPER (buf[0]);
        pbuff = buffer;
        colour_puts (to, to->desc ? to->desc->ansi : 0,
            buf, pbuff, MAX_STRING_LENGTH);
        if (to->desc && (to->desc->connected == CON_PLAYING))
            write_to_buffer (to->desc, buffer, 0); /* changed to buffer to reflect prev. fix */
        else if (MOBtrigger)
            mp_act_trigger (buf, to, ch, arg1, arg2, TRIG_ACT);
    }
}

/* Macintosh support functions. */
#if defined(macintosh)
int gettimeofday (struct timeval *tp, void *tzp) {
    tp->tv_sec = time (NULL);
    tp->tv_usec = 0;
}
#endif

void printf_to_char (CHAR_DATA * ch, char *fmt, ...) {
    char buf[MAX_STRING_LENGTH];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    send_to_char (buf, ch);
}

void wiznet (char *string, CHAR_DATA * ch, OBJ_DATA * obj,
             flag_t flag, flag_t flag_skip, int min_level)
{
    DESCRIPTOR_DATA *d;

    for (d = descriptor_list; d != NULL; d = d->next) {
        if (d->connected == CON_PLAYING && IS_IMMORTAL (d->character)
            && IS_SET (d->character->wiznet, WIZ_ON)
            && (!flag || IS_SET (d->character->wiznet, flag))
            && (!flag_skip || !IS_SET (d->character->wiznet, flag_skip))
            && char_get_trust (d->character) >= min_level && d->character != ch)
        {
            if (IS_SET (d->character->wiznet, WIZ_PREFIX))
                send_to_char ("{Z--> ", d->character);
            else
                send_to_char ("{Z", d->character);
            act_new (string, d->character, obj, ch, TO_CHAR, POS_DEAD);
            send_to_char ("{x", d->character);
        }
    }
}

void qmconfig_read (void) {
    FILE *fp;
    bool fMatch;
    char *word;
    extern int mud_ansiprompt, mud_ansicolor, mud_telnetga;

    log_f ("Loading configuration settings from %s.", QMCONFIG_FILE);
    fp = fopen(QMCONFIG_FILE, "r");
    if (!fp) {
        log_f ("%s not found. Using compiled-in defaults.", QMCONFIG_FILE);
        return;
    }

    while (1) {
        word = feof (fp) ? "END" : fread_word(fp);
        fMatch = FALSE;

        switch (UPPER(word[0])) {
            case '#':
                /* This is a comment line! */
                fMatch = TRUE;
                fread_to_eol (fp);
                break;
            case '*':
                fMatch = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                KEY ("Ansicolor", mud_ansicolor, fread_number(fp));
                KEY ("Ansiprompt", mud_ansiprompt, fread_number(fp));
                break;
            case 'E':
                if (!str_cmp(word, "END"))
                    return;
                break;
            case 'T':
                KEY ("Telnetga", mud_telnetga, fread_number(fp));
                break;
        }
        if (!fMatch) {
            log_f ("qmconfig_read: no match for %s!", word);
            fread_to_eol(fp);
        }
    }
    log_f ("Settings have been read from %s", QMCONFIG_FILE);
    exit(0);
}

const char *get_align_name (int align) {
         if (align >  900) return "angelic";
    else if (align >  700) return "saintly";
    else if (align >  350) return "good";
    else if (align >  100) return "kind";
    else if (align > -100) return "neutral";
    else if (align > -350) return "mean";
    else if (align > -700) return "evil";
    else if (align > -900) return "demonic";
    else                   return "satanic";
}

const char *get_sex_name (int sex) {
    switch (sex) {
        case SEX_NEUTRAL: return "sexless";
        case SEX_MALE:    return "male";
        case SEX_FEMALE:  return "female";
        default:          return "(unknown sex)";
    }
}

const char *get_ch_class_name (CHAR_DATA * ch) {
    if (IS_NPC (ch))
        return "mobile";
    else
        return class_table[ch->class].name;
}

const char *get_ac_type_name (int type) {
    return flag_string (ac_types, type);
}

const char *get_position_name (int position) {
    if (position < POS_DEAD || position > POS_STANDING)
        return "an unknown position (this is a bug!)";
    return position_table[position].long_name;
}

const char *get_character_position_str (CHAR_DATA * ch, int position,
    OBJ_DATA * on, int with_punct)
{
    static char buf[MAX_STRING_LENGTH];
    const char *name = get_position_name (position);

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

const char *get_ac_rating_phrase (int ac) {
         if (ac >= 101)  return "hopelessly vulnerable to";
    else if (ac >=  80)  return "defenseless against";
    else if (ac >=  60)  return "barely protected from";
    else if (ac >=  40)  return "slightly armored against";
    else if (ac >=  20)  return "somewhat armored against";
    else if (ac >=   0)  return "armored against";
    else if (ac >= -20)  return "well-armored against";
    else if (ac >= -40)  return "very well-armored against";
    else if (ac >= -60)  return "heavily armored against";
    else if (ac >= -80)  return "superbly armored against";
    else if (ac >= -100) return "almost invulnerable to";
    else                 return "divinely armored against";
}

/* Recover from a copyover - load players */
void copyover_recover () {
    DESCRIPTOR_DATA *d;
    FILE *fp;
    char name[100];
    char host[MSL];
    int desc;
    bool fOld;

    log_f ("Copyover recovery initiated");
    fp = fopen (COPYOVER_FILE, "r");

    /* there are some descriptors open which will hang forever then ? */
    if (!fp) {
        perror ("copyover_recover:fopen");
        log_f ("Copyover file not found. Exitting.\n\r");
        exit (1);
    }

    /* In case something crashes - doesn't prevent reading  */
    unlink (COPYOVER_FILE);
    while (1) {
        int errorcheck = fscanf (fp, "%d %s %s\n", &desc, name, host);
        if (errorcheck < 0)
            break;
        if (desc == -1)
            break;

        /* Write something, and check if it goes error-free */
        if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r", 0)) {
            close (desc); /* nope */
            continue;
        }

        d = descriptor_new ();
        d->descriptor = desc;

        d->host = str_dup (host);
        LIST_FRONT (d, next, descriptor_list);
        d->connected = CON_COPYOVER_RECOVER;    /* -15, so close_socket frees the char */

        /* Now, find the pfile */
        fOld = load_char_obj (d, name);

        /* Player file not found?! */
        if (!fOld) {
            write_to_descriptor (desc,
                "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
            close_socket (d);
        }
        /* ok! */
        else {
            write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r", 0);

            /* Just In Case */
            if (!d->character->in_room)
                d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);

            /* Insert in the char_list */
            LIST_FRONT (d->character, next, char_list);

            char_to_room (d->character, d->character->in_room);
            do_look (d->character, "auto");
            act ("$n materializes!", d->character, NULL, NULL, TO_NOTCHAR);
            d->connected = CON_PLAYING;

            if (d->character->pet != NULL) {
                char_to_room (d->character->pet, d->character->in_room);
                act ("$n materializes!.", d->character->pet, NULL, NULL,
                     TO_NOTCHAR);
            }
        }
    }
    fclose (fp);
}

bool position_change_message (CHAR_DATA * ch, int from, int to,
    OBJ_DATA *obj)
{
    switch (to) {
        case POS_SLEEPING:
            return position_change_message_to_sleeping(ch, from, obj);
        case POS_RESTING:
            return position_change_message_to_resting(ch, from, obj);
        case POS_SITTING:
            return position_change_message_to_sitting(ch, from, obj);
        case POS_STANDING:
            return position_change_message_to_standing(ch, from, obj);
        case POS_FIGHTING:
            return position_change_message_to_fighting(ch, from, obj);
    }
    return FALSE;
}

bool position_change_message_to_standing (CHAR_DATA * ch, int from, OBJ_DATA *obj) {
    const char *prep = obj_furn_preposition (obj, POS_STANDING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake and stand up.\n\r", ch);
                act ("$n wakes and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake and stand $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n wakes and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_RESTING:
            if (obj == NULL) {
                send_to_char ("You stop resting and stand up.\n\r", ch);
                act ("$n stops resting and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You stop resting and stand $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n stops resting and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_SITTING:
            if (obj == NULL) {
                send_to_char ("You stop sitting and stand up.\n\r", ch);
                act ("$n stops sitting and stands up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You stop sitting and stand $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n stops sitting and stands $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (ch->on == obj)
                break;
            if (obj == NULL && ch->on != NULL) {
                prep = obj_furn_preposition_base (ch->on, POS_STANDING,
                    "away from", "off of", "outside of", "away from");
                act ("You step $T $p.", ch, ch->on, prep, TO_CHAR);
                act ("$n steps $T $p.", ch, ch->on, prep, TO_NOTCHAR);
            }
            else if (obj != NULL) {
                prep = obj_furn_preposition_base (obj, POS_STANDING,
                    "toward", "onto", "inside", "beside");
                act ("You step $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n steps $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            else
                return FALSE;
            return TRUE;
    }
    return FALSE;
}

bool position_change_message_to_fighting (CHAR_DATA * ch, int from, OBJ_DATA *obj) {
    switch (from) {
        case POS_SLEEPING:
            send_to_char ("You wake up, stand up, and fight!\n\r", ch);
            act ("$n wakes up, stands up, and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;

        case POS_RESTING:
            send_to_char ("You stop resting, stand up, and fight!\n\r", ch);
            act ("$n stops resting, stands up, and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;

        case POS_SITTING:
            send_to_char ("You stand up and fight!\n\r", ch);
            act ("$n stands up and fights!", ch, NULL, NULL, TO_NOTCHAR);
            return TRUE;
    }
    return FALSE;
}

bool position_change_message_to_resting (CHAR_DATA * ch, int from, OBJ_DATA *obj) {
    const char *prep = obj_furn_preposition (obj, POS_RESTING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake up and start resting.\n\r", ch);
                act ("$n wakes up and starts resting.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake up and rest $T $p.", ch, obj, prep, TO_CHAR, POS_SLEEPING);
                act ("$n wakes up and rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_SITTING:
            if (obj == NULL) {
                send_to_char ("You rest.\n\r", ch);
                act ("$n rests.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You rest $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You sit down and rest.\n\r", ch);
                act ("$n sits down and rests.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You rest $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n rests $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

bool position_change_message_to_sitting (CHAR_DATA * ch, int from, OBJ_DATA *obj) {
    const char *prep = obj_furn_preposition (obj, POS_RESTING);
    switch (from) {
        case POS_SLEEPING:
            if (obj == NULL) {
                send_to_char ("You wake and sit up.\n\r", ch);
                act ("$n wakes and sits up.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act_new ("You wake and sit $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n wakes and sits $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_RESTING:
            if (obj == NULL)
                send_to_char ("You stop resting.\n\r", ch);
            else {
                act_new ("You sit $T $p.", ch, obj, prep, TO_CHAR, POS_DEAD);
                act ("$n sits $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;

        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You sit down.\n\r", ch);
                act ("$n sits down on the ground.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You sit down $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n sits down $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

bool position_change_message_to_sleeping (CHAR_DATA * ch, int from, OBJ_DATA *obj) {
    const char *prep = obj_furn_preposition (obj, POS_SLEEPING);
    switch (from) {
        case POS_RESTING:
        case POS_SITTING:
        case POS_STANDING:
            if (obj == NULL) {
                send_to_char ("You lie down and go to sleep.\n\r", ch);
                act ("$n lies down and goes to sleep.", ch, NULL, NULL, TO_NOTCHAR);
            }
            else {
                act ("You lie down and go to sleep $T $p.", ch, obj, prep, TO_CHAR);
                act ("$n lies down and goes to sleep $T $p.", ch, obj, prep, TO_NOTCHAR);
            }
            return TRUE;
    }
    return FALSE;
}

/* does aliasing and other fun stuff */
void substitute_alias (DESCRIPTOR_DATA * d, char *argument) {
    CHAR_DATA *ch;
    char buf[MAX_STRING_LENGTH], prefix[MAX_INPUT_LENGTH],
        name[MAX_INPUT_LENGTH];
    char *point;
    int alias;

    ch = d->original ? d->original : d->character;

    /* check for prefix */
    if (ch->prefix[0] != '\0' && str_prefix ("prefix", argument)) {
        if (strlen (ch->prefix) + strlen (argument) > MAX_INPUT_LENGTH - 2)
            send_to_char ("Line to long, prefix not processed.\r\n", ch);
        else
        {
            sprintf (prefix, "%s %s", ch->prefix, argument);
            argument = prefix;
        }
    }

    if (IS_NPC (ch) || ch->pcdata->alias[0] == NULL
        || !str_prefix ("alias", argument) || !str_prefix ("una", argument)
        || !str_prefix ("prefix", argument))
    {
        interpret (d->character, argument);
        return;
    }

    strcpy (buf, argument);

    for (alias = 0; alias < MAX_ALIAS; alias++) { /* go through the aliases */
        if (ch->pcdata->alias[alias] == NULL)
            break;

        if (!str_prefix (ch->pcdata->alias[alias], argument)) {
            point = one_argument (argument, name);
            if (!strcmp (ch->pcdata->alias[alias], name)) {
                /* More Edwin inspired fixes. JR -- 10/15/00 */
                buf[0] = '\0';
                strcat(buf,ch->pcdata->alias_sub[alias]);
                if (point[0]) {
                    strcat(buf," ");
                    strcat(buf,point);
                }

                if (strlen (buf) > MAX_INPUT_LENGTH - 1) {
                    send_to_char
                        ("Alias substitution too long. Truncated.\r\n", ch);
                    buf[MAX_INPUT_LENGTH - 1] = '\0';
                }
                break;
            }
        }
    }
    interpret (d->character, buf);
}

void echo_to_char (CHAR_DATA *to, CHAR_DATA *from, const char *type,
    const char *msg)
{
    if (char_get_trust (to) >= char_get_trust (from)) {
        send_to_char (type, to);
        send_to_char ("> ", to);
    }
    send_to_char (msg, to);
    send_to_char ("\n\r", to);
}
