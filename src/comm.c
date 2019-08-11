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
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>

#include "colour.h"
#include "recycle.h"
#include "utils.h"
#include "ban.h"
#include "fight.h"
#include "update.h"
#include "interp.h"
#include "db.h"
#include "string.h"
#include "olc.h"
#include "nanny.h"
#include "save.h"
#include "mob_prog.h"
#include "lookup.h"
#include "act_info.h"
#include "chars.h"
#include "rooms.h"
#include "objs.h"
#include "find.h"

#include "comm.h"

/* TODO: move definitions to header files. */
/* TODO: review names and usages of global variables. */
/* TODO: a lot of the functions are for low-level processes like sockets,
 * several others for higher-level communication functions. split them up! */
/* TODO: review most of this, it's been largely untouched. */
/* TODO: compatibility is probably lost :-( */

/* Socket and TCP/IP stuff. */
#if defined(macintosh) || defined(MSDOS)
    const char echo_off_str[] = { '\0' };
    const char echo_on_str[]  = { '\0' };
    const char go_ahead_str[] = { '\0' };
#endif

#if defined(unix)
    #include "telnet.h"
    const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
    const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
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

#if defined(unix)
    #include <signal.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <arpa/inet.h>
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

#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos (void) {
    struct timeval last_time;
    struct timeval now_time;
    static DESCRIPTOR_DATA dcon;

    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* New_descriptor analogue. */
    dcon.descriptor = 0;
    if (!mud_ansiprompt)
        dcon.connected = CON_GET_NAME;
    else
        dcon.connected = CON_ANSI;
    dcon.ansi = mud_ansicolor;
    dcon.host = str_dup ("localhost");
    dcon.outsize = 2000;
    dcon.outbuf = alloc_mem (dcon.outsize);
    dcon.next = descriptor_list;
    dcon.showstr_head = NULL;
    dcon.showstr_point = NULL;
    dcon.pEdit = NULL;            /* OLC */
    dcon.pString = NULL;        /* OLC */
    dcon.editor = 0;            /* OLC */
    descriptor_list = &dcon;

    /* First Contact! */
    if (!mud_ansiprompt) {
        extern char * help_greeting;
        if ( help_greeting[0] == '.' )
            send_to_desc ( help_greeting+1, &dcon );
        else
            send_to_desc ( help_greeting  , &dcon );
    }
    else
        write_to_buffer (&dcon, "Do you want ANSI? (Y/n) ", 0);

    /* Main loop */
    while (!merc_down) {
        DESCRIPTOR_DATA *d;

        /* Process input. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            d->fcommand = FALSE;

            #if defined(MSDOS)
                if (kbhit ())
            #endif
            {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->wait > 0)
                continue;

            read_from_buffer (d);
            if (d->incomm[0] != '\0') {
                d->fcommand = TRUE;
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }
                }

                d->incomm[0] = '\0';
            }
        }

        /* Autonomous game motion.  */
        update_handler ();

        /* Output. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            if ((d->fcommand || d->outtop > 0)) {
                if (!process_output (d, TRUE)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }

        /* Synchronize to a clock.
         * Busy wait (blargh). */
        now_time = last_time;
        while (1) {
            int delta;

            #if defined(MSDOS)
                if (kbhit ())
            #endif
            {
                if (dcon.character != NULL)
                    dcon.character->timer = 0;
                if (!read_from_descriptor (&dcon)) {
                    if (dcon.character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    dcon.outtop = 0;
                    close_socket (&dcon);
                }
                #if defined(MSDOS)
                    break;
                #endif
            }

            gettimeofday (&now_time, NULL);
            delta = (now_time.tv_sec - last_time.tv_sec) * 1000 * 1000
                + (now_time.tv_usec - last_time.tv_usec);
            if (delta >= (1000000 / PULSE_PER_SECOND) * 100 / PULSE_SPEED)
                break;
        }
        last_time = now_time;
        current_time = (time_t) last_time.tv_sec;
    }
    return;
}
#endif

#if defined(unix)
void game_loop_unix (int control) {
    static struct timeval null_time;
    struct timeval last_time;

    signal (SIGPIPE, SIG_IGN);
    gettimeofday (&last_time, NULL);
    current_time = (time_t) last_time.tv_sec;

    /* Main loop */
    while (!merc_down) {
        fd_set in_set;
        fd_set out_set;
        fd_set exc_set;
        DESCRIPTOR_DATA *d;
        int maxdesc;

#if defined(MALLOC_DEBUG)
        if (malloc_verify () != 1)
            abort ();
#endif

        /* Poll all active descriptors.  */
        FD_ZERO (&in_set);
        FD_ZERO (&out_set);
        FD_ZERO (&exc_set);
        FD_SET (control, &in_set);
        maxdesc = control;
        for (d = descriptor_list; d; d = d->next) {
            maxdesc = UMAX (maxdesc, d->descriptor);
            FD_SET (d->descriptor, &in_set);
            FD_SET (d->descriptor, &out_set);
            FD_SET (d->descriptor, &exc_set);
        }

        if (select (maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0) {
            perror ("Game_loop: select: poll");
            exit (1);
        }

        /*
         * New connection?
         */
        if (FD_ISSET (control, &in_set))
            init_descriptor (control);

        /* Kick out the freaky folks. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            if (FD_ISSET (d->descriptor, &exc_set)) {
                FD_CLR (d->descriptor, &in_set);
                FD_CLR (d->descriptor, &out_set);
                if (d->character && d->connected == CON_PLAYING)
                    save_char_obj (d->character);
                d->outtop = 0;
                close_socket (d);
            }
        }

        /* Process input. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;
            d->fcommand = FALSE;

            if (FD_ISSET (d->descriptor, &in_set)) {
                if (d->character != NULL)
                    d->character->timer = 0;
                if (!read_from_descriptor (d)) {
                    FD_CLR (d->descriptor, &out_set);
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                    continue;
                }
            }

            if (d->character != NULL && d->character->wait > 0)
                continue;

            read_from_buffer (d);
            if (d->incomm[0] != '\0') {
                d->fcommand = TRUE;
                stop_idling (d->character);

                /* OLC */
                if (d->showstr_point) {
                    d->lines_written = 0;
                    show_page (d);
                }
                else if (d->pString)
                    string_add (d->character, d->incomm);
                else {
                    switch (d->connected) {
                        case CON_PLAYING:
                            if (!run_olc_editor (d))
                                substitute_alias (d, d->incomm);
                            break;
                        default:
                            nanny (d, d->incomm);
                            break;
                    }
                }
                d->incomm[0] = '\0';
            }
        }

        #ifdef IMC
            imc_loop();
        #endif

        /* Autonomous game motion. */
        update_handler ();

        /* Output. */
        for (d = descriptor_list; d != NULL; d = d_next) {
            d_next = d->next;

            if ((d->fcommand || d->outtop > 0)
                && FD_ISSET (d->descriptor, &out_set))
            {
                if (!process_output (d, TRUE)) {
                    if (d->character != NULL && d->connected == CON_PLAYING)
                        save_char_obj (d->character);
                    d->outtop = 0;
                    close_socket (d);
                }
            }
        }

        /* Synchronize to a clock.
         * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
         * Careful here of signed versus unsigned arithmetic. */
        {
            struct timeval now_time;
            long secDelta;
            long usecDelta;

            gettimeofday (&now_time, NULL);
            usecDelta = ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
                + ((1000000 / PULSE_PER_SECOND) * 100 / PULSE_SPEED);
            secDelta = ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
            while (usecDelta < 0) {
                usecDelta += 1000000;
                secDelta -= 1;
            }

            while (usecDelta >= 1000000) {
                usecDelta -= 1000000;
                secDelta += 1;
            }

            if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
                struct timeval stall_time;

                stall_time.tv_usec = usecDelta;
                stall_time.tv_sec = secDelta;
                if (select (0, NULL, NULL, NULL, &stall_time) < 0) {
                    perror ("Game_loop: select: stall");
                    exit (1);
                }
            }
        }

        gettimeofday (&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;
    }

    return;
}
#endif

#if defined(unix)
void init_descriptor (int control) {
    char buf[MAX_STRING_LENGTH];
    DESCRIPTOR_DATA *dnew;
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
    dnew->ansi = mud_ansicolor;
    dnew->showstr_head = NULL;
    dnew->showstr_point = NULL;
    dnew->outsize = 2000;
    dnew->pEdit = NULL;            /* OLC */
    dnew->pString = NULL;        /* OLC */
    dnew->editor = 0;            /* OLC */
    dnew->outbuf = alloc_mem (dnew->outsize);

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
        sprintf (log_buf, "Sock.sinaddr:  %s", buf);
        log_string (log_buf);
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

    return;
}
#endif

void close_socket (DESCRIPTOR_DATA * dclose) {
    CHAR_DATA *ch;

    if (dclose->outtop > 0)
        process_output (dclose, FALSE);
    if (dclose->snoop_by != NULL)
        write_to_buffer (dclose->snoop_by, "Your victim has left the game.\n\r", 0);

    {
        DESCRIPTOR_DATA *d;
        for (d = descriptor_list; d != NULL; d = d->next)
            if (d->snoop_by == dclose)
                d->snoop_by = NULL;
    }

    if ((ch = dclose->character) != NULL) {
        sprintf (log_buf, "Closing link to %s.", ch->name);
        log_string (log_buf);
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

    LIST_REMOVE (dclose, next, descriptor_list, DESCRIPTOR_DATA, NO_FAIL);

    close (dclose->descriptor);
    descriptor_free (dclose);
    #if defined(MSDOS) || defined(macintosh)
        exit (1);
    #endif
    return;
}

bool read_from_descriptor (DESCRIPTOR_DATA * d) {
    int iStart;

    /* Hold horses if pending command already. */
    if (d->incomm[0] != '\0')
        return TRUE;

    /* Check for overflow. */
    iStart = strlen (d->inbuf);
    if (iStart >= sizeof (d->inbuf) - 10) {
        sprintf (log_buf, "%s input overflow!", d->host);
        log_string (log_buf);
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
        d->inbuf[iStart++] = c;
        if (iStart > sizeof (d->inbuf) - 10)
            break;
    }
    #endif

#if defined(MSDOS) || defined(unix)
    while (1) {
        int nRead;
        nRead = read (d->descriptor, d->inbuf + iStart,
                      sizeof (d->inbuf) - 10 - iStart);
        if (nRead > 0) {
            iStart += nRead;
            if (d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r')
                break;
        }
        else if (nRead == 0) {
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

    d->inbuf[iStart] = '\0';
    return TRUE;
}

/* Transfer one line from input buffer to input line. */
void read_from_buffer (DESCRIPTOR_DATA * d) {
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
                sprintf (log_buf, "%s input spamming!", d->host);
                log_string (log_buf);
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
    return;
}

/* Low level output function. */
bool process_output (DESCRIPTOR_DATA * d, bool fPrompt) {
    extern bool merc_down;

    /* Bust a prompt. */
    if (!merc_down) {
        if (d->showstr_point && *d->showstr_point == '\0') {
            d->lines_written = 0;
            clear_page (d);
        }

        if (d->showstr_point)
            write_to_buffer (d, "[Hit Return to continue] ", 0);
        else if (fPrompt && d->pString && d->connected == CON_PLAYING)
            write_to_buffer (d, "> ", 2);
        else if (fPrompt && d->connected == CON_PLAYING) {
            CHAR_DATA *ch;
            CHAR_DATA *victim;

            ch = d->character;

            /* battle prompt */
            if ((victim = ch->fighting) != NULL && char_can_see_in_room (ch, victim)) {
                int percent;
                char *pbuff;
                char buf[MSL];
                char buffer[MSL*2];

                if (victim->max_hit > 0)
                    percent = victim->hit * 100 / victim->max_hit;
                else
                    percent = -1;

                sprintf (buf, "%s %s.\n\r", PERS_IR (victim, ch),
                    condition_string(percent));
                buf[0] = UPPER (buf[0]);
                pbuff = buffer;
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
    if (!write_to_descriptor (d->descriptor, d->outbuf, d->outtop)) {
        d->outtop = 0;
        return FALSE;
    }
    else {
        d->outtop = 0;
        return TRUE;
    }
}

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
    char doors[MAX_INPUT_LENGTH];
    EXIT_DATA *pexit;
    bool found;
    const char *dir_short_name[] = { "N", "E", "S", "W", "U", "D" };
    int door;

    if (ch == NULL || ch->desc == NULL)
        return;

    point = buf;
    str = ch->prompt;
    if (str == NULL || str[0] == '\0') {
        sprintf (buf, "{p<%dhp %dm %dmv>{x %s",
                 ch->hit, ch->mana, ch->move, ch->prefix);
        send_to_char (buf, ch);
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
            default:
                i = " ";
                break;
            case 'e':
                found = FALSE;
                doors[0] = '\0';
                for (door = 0; door < 6; door++) {
                    if ((pexit = ch->in_room->exit[door]) != NULL
                        && pexit->to_room != NULL
                        && (char_can_see_room (ch, pexit->to_room)
                            || (IS_AFFECTED (ch, AFF_INFRARED)
                                && !IS_AFFECTED (ch, AFF_BLIND)))
                        && !IS_SET (pexit->exit_flags, EX_CLOSED))
                    {
                        found = TRUE;
                        strcat (doors, dir_short_name[door]);
                    }
                }
                if (!found)
                    strcat (doors, "none");
                sprintf (buf2, "%s", doors);
                i = buf2;
                break;
            case 'c':
                sprintf (buf2, "%s", "\n\r");
                i = buf2;
                break;
            case 'h':
                sprintf (buf2, "%d", ch->hit);
                i = buf2;
                break;
            case 'H':
                sprintf (buf2, "%d", ch->max_hit);
                i = buf2;
                break;
            case 'm':
                sprintf (buf2, "%d", ch->mana);
                i = buf2;
                break;
            case 'M':
                sprintf (buf2, "%d", ch->max_mana);
                i = buf2;
                break;
            case 'v':
                sprintf (buf2, "%d", ch->move);
                i = buf2;
                break;
            case 'V':
                sprintf (buf2, "%d", ch->max_move);
                i = buf2;
                break;
            case 'x':
                sprintf (buf2, "%d", ch->exp);
                i = buf2;
                break;
            case 'X':
                sprintf (buf2, "%d", IS_NPC (ch) ? 0 :
                         (ch->level + 1) * exp_per_level (ch,
                                                          ch->pcdata->
                                                          points) - ch->exp);
                i = buf2;
                break;
            case 'g':
                sprintf (buf2, "%ld", ch->gold);
                i = buf2;
                break;
            case 's':
                sprintf (buf2, "%ld", ch->silver);
                i = buf2;
                break;
            case 'a':
                if (ch->level > 9)
                    sprintf (buf2, "%d", ch->alignment);
                else
                    sprintf (buf2, "%s",
                             IS_GOOD (ch) ? "good" : IS_EVIL (ch) ? "evil" :
                             "neutral");
                i = buf2;
                break;
            case 'r':
                if (ch->in_room != NULL)
                    sprintf (buf2, "%s",
                             ((!IS_NPC
                               (ch) && IS_SET (ch->plr, PLR_HOLYLIGHT))
                              || (!IS_AFFECTED (ch, AFF_BLIND)
                                  && !room_is_dark (ch->
                                                    in_room))) ? ch->in_room->
                             name : "darkness");
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case 'R':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%d", ch->in_room->vnum);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case 'z':
                if (IS_IMMORTAL (ch) && ch->in_room != NULL)
                    sprintf (buf2, "%s", ch->in_room->area->title);
                else
                    sprintf (buf2, " ");
                i = buf2;
                break;
            case '%':
                sprintf (buf2, "%%");
                i = buf2;
                break;
            case 'o':
                sprintf (buf2, "%s", olc_ed_name (ch));
                i = buf2;
                break;
            case 'O':
                sprintf (buf2, "%s", olc_ed_vnum (ch));
                i = buf2;
                break;
            case 'p':
                sprintf (buf2, "%s%s", ch->fighting ? "!" : "",
                    get_character_position_str (ch, ch->position, ch->on, FALSE));
                i = buf2;
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
    return;
}

/* Append onto an output buffer. */
void write_to_buffer (DESCRIPTOR_DATA * d, const char *txt, int length) {
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
        outbuf = alloc_mem (2 * d->outsize);
        strncpy (outbuf, d->outbuf, d->outtop);
        mem_free (d->outbuf, d->outsize);
        d->outbuf = outbuf;
        d->outsize *= 2;
    }

    /* Copy. */
    strncpy (d->outbuf + d->outtop, txt, length);
    d->outtop += length;
    return;
}

/* Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size. */
bool write_to_descriptor (int desc, char *txt, int length) {
    int iStart;
    int nWrite;
    int nBlock;

#if defined(macintosh) || defined(MSDOS)
    if (desc == 0)
        desc = 1;
#endif

    if (length <= 0)
        length = strlen (txt);

    for (iStart = 0; iStart < length; iStart += nWrite) {
        nBlock = UMIN (length - iStart, 4096);
        if ((nWrite = write (desc, txt + iStart, nBlock)) < 0) {
            perror ("Write_to_descriptor");
            return FALSE;
        }
    }

    return TRUE;
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

/* Look for link-dead player to reconnect. */
bool check_reconnect (DESCRIPTOR_DATA * d, char *name, bool fConn) {
    CHAR_DATA *ch;

    for (ch = char_list; ch != NULL; ch = ch->next) {
        if (!IS_NPC (ch)
            && (!fConn || ch->desc == NULL)
            && !str_cmp (d->character->name, ch->name))
        {
            if (fConn == FALSE) {
                str_free (d->character->pcdata->pwd);
                d->character->pcdata->pwd = str_dup (ch->pcdata->pwd);
            }
            else {
                char_free (d->character);
                d->character = ch;
                ch->desc = d;
                ch->timer = 0;
                send_to_char ("Reconnecting. Type replay to see missed tells.\n\r", ch);
                act ("$n has reconnected.", ch, NULL, NULL, TO_NOTCHAR);

                sprintf (log_buf, "%s@%s reconnected.", ch->name, d->host);
                log_string (log_buf);
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
bool check_playing (DESCRIPTOR_DATA * d, char *name) {
    DESCRIPTOR_DATA *dold;

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

/* Page to one descriptor using Lope's color. */
void send_to_desc (const char *txt, DESCRIPTOR_DATA * d) {
    char buf[MAX_STRING_LENGTH * 4];
    int len;

    if (txt == NULL || d == NULL)
        return;
    buf[0] = '\0';
    len = colour_puts (NULL, d->ansi, txt, buf, sizeof(buf));
    write_to_buffer (d, buf, len);
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

void clear_page (DESCRIPTOR_DATA *d) {
    if (d->showstr_head) {
        mem_free (d->showstr_head, strlen (d->showstr_head));
        d->showstr_head = NULL;
    }
    d->showstr_point = NULL;
}

void append_to_page (DESCRIPTOR_DATA *d, const char *txt) {
    int len;
    if (d == NULL || txt == NULL)
        return;

    len = strlen (txt);
    if (d->showstr_head == NULL) {
        d->showstr_head  = alloc_mem (len + 1);
        d->showstr_point = d->showstr_head;
        strcpy (d->showstr_head, txt);
    }
    else {
        int offset = d->showstr_point - d->showstr_head;
        char *new_buf = alloc_mem (strlen (d->showstr_head) + len + 1);
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
int show_page (DESCRIPTOR_DATA *d) {
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
    return;
}

/* Macintosh support functions. */
#if defined(macintosh)
int gettimeofday (struct timeval *tp, void *tzp) {
    tp->tv_sec = time (NULL);
    tp->tv_usec = 0;
}
#endif

/* source: EOD, by John Booth <???> */
void printf_to_desc (DESCRIPTOR_DATA * d, char *fmt, ...) {
    char buf[MSL];
    va_list args;
    va_start (args, fmt);
    vsprintf (buf, fmt, args);
    va_end (args);

    send_to_desc (buf, d);
}

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

ROOM_INDEX_DATA *find_location (CHAR_DATA * ch, char *arg) {
    CHAR_DATA *victim;
    OBJ_DATA *obj;

    if (is_number (arg))
        return get_room_index (atoi (arg));
    if ((victim = find_char_world (ch, arg)) != NULL)
        return victim->in_room;
    if ((obj = find_obj_world (ch, arg)) != NULL)
        return obj->in_room;
    return NULL;
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
