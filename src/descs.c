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

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>

#if defined(unix)
    #include <signal.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#endif

#include "ban.h"
#include "chars.h"
#include "colour.h"
#include "comm.h"
#include "db.h"
#include "recycle.h"
#include "utils.h"
#include "interp.h"
#include "lookup.h"
#include "globals.h"
#include "memory.h"

#include "descs.h"

#if defined(unix)
int init_socket (int port) {
    static struct sockaddr_in sa_zero;
    struct sockaddr_in sa;
    int x = 1;
    int fd;

    if ((fd = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("Init_socket: socket");
        exit (1);
    }

    if (setsockopt (fd, SOL_SOCKET, SO_REUSEADDR,
                    (char *) &x, sizeof (x)) < 0)
    {
        perror ("Init_socket: SO_REUSEADDR");
        close (fd);
        exit (1);
    }

    #if defined(SO_DONTLINGER) && !defined(SYSV)
    {
        struct linger ld;

        ld.l_onoff = 1;
        ld.l_linger = 1000;

        if (setsockopt (fd, SOL_SOCKET, SO_DONTLINGER,
                        (char *) &ld, sizeof (ld)) < 0)
        {
            perror ("Init_socket: SO_DONTLINGER");
            close (fd);
            exit (1);
        }
    }
    #endif

    sa = sa_zero;
    sa.sin_family = AF_INET;
    sa.sin_port = htons (port);
    sa.sin_addr.s_addr = inet_addr( mud_ipaddress );
    log_f("Set IP address to %s", mud_ipaddress);

    if (bind (fd, (struct sockaddr *) &sa, sizeof (sa)) < 0) {
        perror ("Init socket: bind");
        close (fd);
        exit (1);
    }

    if (listen (fd, 3) < 0) {
        perror ("Init socket: listen");
        close (fd);
        exit (1);
    }

    return fd;
}
#endif

#if defined(unix)
void init_descriptor (int control) {
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_T *dnew;
    struct sockaddr_in sock;
    struct hostent *from;
    size_t desc;
    socklen_t size = sizeof(struct sockaddr_in);

    getsockname (control, (struct sockaddr *) &sock, &size);
    if ((desc = accept (control, (struct sockaddr *) &sock, &size)) < 0) {
        perror ("New_descriptor: accept");
        return;
    }

    #if !defined(FNDELAY)
        #define FNDELAY O_NDELAY
    #endif

    if (fcntl (desc, F_SETFL, FNDELAY) == -1) {
        perror ("New_descriptor: fcntl: FNDELAY");
        return;
    }

    /* Cons a new descriptor. */
    dnew = descriptor_new ();
    dnew->descriptor = desc;
    if (!mud_ansiprompt)
        dnew->connected = CON_GET_NAME;
    else
        dnew->connected = CON_ANSI;

    dnew->ansi          = mud_ansicolor;
    dnew->showstr_head  = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize       = 2000;
    dnew->olc_edit      = NULL; /* OLC */
    dnew->string_edit   = NULL; /* OLC */
    dnew->editor        = 0;    /* OLC */
    dnew->outbuf        = mem_alloc (dnew->outsize);

    size = sizeof (sock);
    if (getpeername (desc, (struct sockaddr *) &sock, &size) < 0) {
        perror ("New_descriptor: getpeername");
        dnew->host = str_dup ("(unknown)");
    }
    else {
        /* Would be nice to use inet_ntoa here but it takes a struct arg,
         * which ain't very compatible between gcc and system libraries. */
        int addr;

        addr = ntohl (sock.sin_addr.s_addr);
        sprintf (buf, "%d.%d.%d.%d",
            (addr >> 24) & 0xFF, (addr >> 16) & 0xFF,
            (addr >> 8) & 0xFF, (addr) & 0xFF);
        log_f ("Sock.sinaddr:  %s", buf);
        from = gethostbyaddr ((char *) &sock.sin_addr,
                              sizeof (sock.sin_addr), AF_INET);
        dnew->host = str_dup (from ? from->h_name : buf);
    }

    /* Swiftest: I added the following to ban sites.  I don't
     * endorse banning of sites, but Copper has few descriptors now
     * and some people from certain sites keep abusing access by
     * using automated 'autodialers' and leaving connections hanging.
     *
     * Furey: added suffix check by request of Nickel of HiddenWorlds. */

    if (check_ban (dnew->host, BAN_ALL)) {
        write_to_descriptor (desc, "Your site has been banned from this mud.\n\r", 0);
        close (desc);
        descriptor_free (dnew);
        return;
    }

    /* Init descriptor data. */
    LIST_FRONT (dnew, next, descriptor_list);

    /* First Contact! */
    if (!mud_ansiprompt) {
        extern char *help_greeting;
        if (help_greeting[0] == '.')
            send_to_desc (help_greeting+1, dnew);
        else
            send_to_desc (help_greeting  , dnew);
    }
    else
        send_to_desc ("Do you want ANSI? (Y/n) ", dnew);
}
#endif

void close_socket (DESCRIPTOR_T *dclose) {
    CHAR_T *ch;

    if (dclose->outtop > 0)
        process_output (dclose, FALSE);
    if (dclose->snoop_by != NULL)
        write_to_buffer (dclose->snoop_by, "Your victim has left the game.\n\r", 0);

    {
        DESCRIPTOR_T *d;
        for (d = descriptor_list; d != NULL; d = d->next)
            if (d->snoop_by == dclose)
                d->snoop_by = NULL;
    }

    if ((ch = dclose->character) != NULL) {
        log_f ("Closing link to %s.", ch->name);

        /* cut down on wiznet spam when rebooting */
        /* If ch is writing note or playing, just lose link otherwise clear char */
        if ((dclose->connected == CON_PLAYING && !merc_down)
                || ((dclose->connected >= CON_NOTE_TO)
                        && (dclose->connected <= CON_NOTE_FINISH)))
        {
            act ("$n has lost $s link.", ch, NULL, NULL, TO_NOTCHAR);
            wiznet ("Net death has claimed $N.", ch, NULL, WIZ_LINKS, 0, 0);
            ch->desc = NULL;
        }
        else
            char_free (dclose->original ? dclose->original : dclose->character);
    }

    if (d_next == dclose)
        d_next = d_next->next;

    LIST_REMOVE (dclose, next, descriptor_list, DESCRIPTOR_T, NO_FAIL);

    close (dclose->descriptor);
    descriptor_free (dclose);
    #if defined(MSDOS) || defined(macintosh)
        exit (1);
    #endif
}

bool read_from_descriptor (DESCRIPTOR_T *d) {
    int start;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return TRUE;

    /* Check for overflow. */
    start = strlen (d->inbuf);
    if (start >= sizeof (d->inbuf) - 10) {
        log_f ("%s input overflow!", d->host);
        write_to_descriptor (d->descriptor,
            "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
        return FALSE;
    }

    /* Snarf input. */
    #if defined(macintosh)
    while (1) {
        int c;
        c = getc (stdin);
        if (c == '\0' || c == EOF)
            break;
        putc (c, stdout);
        if (c == '\r')
            putc ('\n', stdout);
        d->inbuf[start++] = c;
        if (start > sizeof (d->inbuf) - 10)
            break;
    }
    #endif

#if defined(MSDOS) || defined(unix)
    while (1) {
        int bytes_read;
        bytes_read = read (d->descriptor, d->inbuf + start,
                      sizeof (d->inbuf) - 10 - start);
        if (bytes_read > 0) {
            start += bytes_read;
            if (d->inbuf[start - 1] == '\n' || d->inbuf[start - 1] == '\r')
                break;
        }
        else if (bytes_read == 0) {
            log_string ("EOF encountered on read.");
            return FALSE;
        }
        else if (errno == EWOULDBLOCK)
            break;
        else {
            perror ("Read_from_descriptor");
            return FALSE;
        }
    }
#endif

    d->inbuf[start] = '\0';
    return TRUE;
}

/* Transfer one line from input buffer to input line. */
void read_from_buffer (DESCRIPTOR_T *d) {
    int i, j, k;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return;

    /* Look for at least one new line. */
    for (i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
        if (d->inbuf[i] == '\0')
            return;

    /* Canonical input processing. */
    for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++) {
        if (k >= MAX_INPUT_LENGTH - 2) {
            write_to_descriptor (d->descriptor, "Line too long.\n\r", 0);

            /* skip the rest of the line */
            for (; d->inbuf[i] != '\0'; i++)
                if (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
                    break;

            d->inbuf[i] = '\n';
            d->inbuf[i + 1] = '\0';
            break;
        }

        if (d->inbuf[i] == '\b' && k > 0)
            --k;
        else if (isascii (d->inbuf[i]) && isprint (d->inbuf[i]))
            d->incomm[k++] = d->inbuf[i];
    }

    /* Finish off the line. */
    if (k == 0)
        d->incomm[k++] = ' ';
    d->incomm[k] = '\0';

    /* Deal with bozos with #repeat 1000 ... */
    if (k > 1 || d->incomm[0] == '!') {
        if (d->incomm[0] != '!' && strcmp (d->incomm, d->inlast))
            d->repeat = 0;
        else {
            if (++d->repeat >= 25 && d->character
                && d->connected == CON_PLAYING)
            {
                log_f ("%s input spamming!", d->host);
                wiznet ("Spam spam spam $N spam spam spam spam spam!",
                        d->character, NULL, WIZ_SPAM, 0,
                        char_get_trust (d->character));
                if (d->incomm[0] == '!')
                    wiznet (d->inlast, d->character, NULL, WIZ_SPAM, 0,
                            char_get_trust (d->character));
                else
                    wiznet (d->incomm, d->character, NULL, WIZ_SPAM, 0,
                            char_get_trust (d->character));

                d->repeat = 0;
            }
        }
    }

    /* Do '!' substitution. */
    if (d->incomm[0] == '!')
        strcpy (d->incomm, d->inlast);
    else
        strcpy (d->inlast, d->incomm);

    /* Shift the input buffer. */
    while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
        i++;
    for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
        ;
}

/* Low level output function. */
bool process_output (DESCRIPTOR_T *d, bool prompt) {
    extern bool merc_down;

    /* Bust a prompt. */
    if (!merc_down) {
        if (d->showstr_point && *d->showstr_point == '\0') {
            d->lines_written = 0;
            clear_page (d);
        }

        if (d->showstr_point)
            write_to_buffer (d, "[Hit Return to continue] ", 0);
        else if (prompt && d->string_edit && d->connected == CON_PLAYING)
            write_to_buffer (d, "> ", 2);
        else if (prompt && d->connected == CON_PLAYING) {
            CHAR_T *ch, *victim;
            ch = d->character;

            /* battle prompt */
            if ((victim = ch->fighting) != NULL && char_can_see_in_room (ch, victim)) {
                char *pbuff;
                char buf[MSL];
                char buffer[MSL*2];

                pbuff = buffer;
                char_format_condition_or_pos_msg (buf, sizeof (buf), ch, victim,
                    FALSE);
                colour_puts (CH(d), d->ansi, buf, pbuff, MAX_STRING_LENGTH);
                write_to_buffer (d, buffer, 0);
            }

            ch = d->original ? d->original : d->character;
            if (!IS_SET (ch->comm, COMM_COMPACT))
                write_to_buffer (d, "\n\r", 2);
            if (IS_SET (ch->comm, COMM_PROMPT))
                bust_a_prompt (d->character);
            if (IS_SET (ch->comm, COMM_TELNET_GA))
                write_to_buffer (d, go_ahead_str, 0);
        }
    }

    /* Short-circuit if nothing to write. */
    if (d->outtop == 0)
        return TRUE;

    /* Snoop-o-rama. */
    if (d->snoop_by != NULL) {
        if (d->character != NULL)
            write_to_buffer (d->snoop_by, d->character->name, 0);
        write_to_buffer (d->snoop_by, "> ", 2);
        write_to_buffer (d->snoop_by, d->outbuf, d->outtop);
    }

    /* OS-dependent output. */
    return desc_flush_output (d);
}

bool desc_flush_output (DESCRIPTOR_T *d) {
    if (d == NULL)
        return FALSE;

    if (!write_to_descriptor (d->descriptor, d->outbuf, d->outtop)) {
        d->outtop = 0;
        return FALSE;
    }
    else {
        d->outtop = 0;
        return TRUE;
    }
}

/* Append onto an output buffer. */
void write_to_buffer (DESCRIPTOR_T *d, const char *txt, int length) {
    /* Find length in case caller didn't. */
    if (length <= 0)
        length = strlen (txt);

    /* Initial \n\r if needed. */
    if (d->outtop == 0 && !d->fcommand) {
        d->outbuf[0] = '\n';
        d->outbuf[1] = '\r';
        d->outtop = 2;
    }

    /* Expand the buffer as needed. */
    while (d->outtop + length >= d->outsize) {
        char *outbuf;

        if (d->outsize >= 32000) {
            bug ("Buffer overflow. Closing.\n\r", 0);
            close_socket (d);
            return;
        }
        outbuf = mem_alloc (2 * d->outsize);
        strncpy (outbuf, d->outbuf, d->outtop);
        mem_free (d->outbuf, d->outsize);
        d->outbuf = outbuf;
        d->outsize *= 2;
    }

    /* Copy. */
    strncpy (d->outbuf + d->outtop, txt, length);
    d->outtop += length;
}

/* Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size. */
bool write_to_descriptor (int desc, char *txt, int length) {
    int start;
    int bytes_written;
    int block_size;

#if defined(macintosh) || defined(MSDOS)
    if (desc == 0)
        desc = 1;
#endif

    if (length <= 0)
        length = strlen (txt);

    for (start = 0; start < length; start += bytes_written) {
        block_size = UMIN (length - start, 4096);
        if ((bytes_written = write (desc, txt + start, block_size)) < 0) {
            perror ("Write_to_descriptor");
            return FALSE;
        }
    }

    return TRUE;
}

/* Look for link-dead player to reconnect. */
bool check_reconnect (DESCRIPTOR_T *d, char *name, bool conn) {
    CHAR_T *ch;

    for (ch = char_list; ch != NULL; ch = ch->next) {
        if (!IS_NPC (ch)
            && (!conn || ch->desc == NULL)
            && !str_cmp (d->character->name, ch->name))
        {
            if (conn == FALSE) {
                str_replace_dup (&(d->character->pcdata->pwd), ch->pcdata->pwd);
            }
            else {
                char_free (d->character);
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;
                send_to_char ("Reconnecting. Type replay to see missed tells.\n\r", ch);
                act ("$n has reconnected.", ch, NULL, NULL, TO_NOTCHAR);

                log_f ("%s@%s reconnected.", ch->name, d->host);
                wiznet ("$N groks the fullness of $S link.",
                        ch, NULL, WIZ_LINKS, 0, 0);
                d->connected = CON_PLAYING;

                /* Inform the character of a note in progress and the possbility
                 * of continuation! */
                if (ch->pcdata->in_progress)
                    send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r", ch);
            }
            return TRUE;
        }
    }

    return FALSE;
}

/* Check if already playing. */
bool check_playing (DESCRIPTOR_T *d, char *name) {
    DESCRIPTOR_T *dold;

    for (dold = descriptor_list; dold; dold = dold->next) {
        if (dold != d
            && dold->character != NULL
            && dold->connected != CON_GET_NAME
            && dold->connected != CON_GET_OLD_PASSWORD
            && !str_cmp (name, dold->original
                         ? dold->original->name : dold->character->name))
        {
            write_to_buffer (d, "That character is already playing.\n\r", 0);
            write_to_buffer (d, "Do you wish to connect anyway (Y/N)?", 0);
            d->connected = CON_BREAK_CONNECT;
            return TRUE;
        }
    }

    return FALSE;
}

/* Page to one descriptor using Lope's color. */
void send_to_desc (const char *txt, DESCRIPTOR_T *d) {
    char buf[MAX_STRING_LENGTH * 4];
    int len;

    if (txt == NULL || d == NULL)
        return;
    buf[0] = '\0';
    len = colour_puts (NULL, d->ansi, txt, buf, sizeof(buf));
    write_to_buffer (d, buf, len);
}

void clear_page (DESCRIPTOR_T *d) {
    if (d->showstr_head) {
        mem_free (d->showstr_head, strlen (d->showstr_head));
        d->showstr_head = NULL;
    }
    d->showstr_point = NULL;
}

void append_to_page (DESCRIPTOR_T *d, const char *txt) {
    int len;
    if (d == NULL || txt == NULL)
        return;

    len = strlen (txt);
    if (d->showstr_head == NULL) {
        d->showstr_head  = mem_alloc (len + 1);
        d->showstr_point = d->showstr_head;
        strcpy (d->showstr_head, txt);
    }
    else {
        int offset = d->showstr_point - d->showstr_head;
        char *new_buf = mem_alloc (strlen (d->showstr_head) + len + 1);
        strcpy (new_buf, d->showstr_head);
        strcat (new_buf, txt);

        clear_page (d);
        d->showstr_head = new_buf;
        d->showstr_point = d->showstr_head + offset;
        strcpy (d->showstr_head, txt);
    }

    /* write what we can immediately. */
    show_page (d);
}

/* string pager */
int show_page (DESCRIPTOR_T *d) {
    char buffer[4 * MAX_STRING_LENGTH];
    register char *scan;
    int crlf = 0;
    int show_lines;

    show_lines = d->character ? d->character->lines : 0;
    for (scan = buffer; *d->showstr_point != '\0';
         scan++, d->showstr_point++)
    {
        if (show_lines != 0 && d->lines_written >= show_lines)
            break;
        *scan = *d->showstr_point;
        if (*scan == '\n') crlf |= 0x01;
        if (*scan == '\r') crlf |= 0x02;
        if (crlf == 0x03) {
            crlf = 0;
            d->lines_written++;
        }
    }
    *scan = '\0';
    write_to_buffer (d, buffer, strlen (buffer));
    return (*d->showstr_point) == '\0';
}

/* source: EOD, by John Booth <???> */
void printf_to_desc (DESCRIPTOR_T *d, char *fmt, ...) {
    char buf[MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    send_to_desc (buf, d);
}

/* does aliasing and other fun stuff */
void desc_substitute_alias (DESCRIPTOR_T *d, char *argument) {
    CHAR_T *ch;
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
