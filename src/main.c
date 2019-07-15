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

#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "interp.h"
#include "comm.h"
#include "db.h"
#include "utils.h"
#include "signal.h"

/* TODO: feature - better command-line arguments? */

int main (int argc, char **argv) {
    struct timeval now_time;
    bool fCopyOver = FALSE;
    #ifdef IMC
        int imcsocket = -1;
    #endif

    /* Memory debugging if needed. */
    #if defined(MALLOC_DEBUG)
        malloc_debug (2);
    #endif

    /* Catch CTRL-C so we can do some manditory clean-up. */
    init_signal_handlers ();

    /* Init time.  */
    gettimeofday (&now_time, NULL);
    current_time = (time_t) now_time.tv_sec;
    strcpy (str_boot_time, ctime (&current_time));

    /* Macintosh console initialization. */
    #if defined(macintosh)
        console_options.nrows = 31;
        cshow (stdout);
        csetmode (C_RAW, stdin);
        cecho2file ("log file", 1, stderr);
    #endif

    /* Reserve one channel for our use. */
    if ((fpReserve = fopen (NULL_FILE, "r")) == NULL) {
        perror (NULL_FILE);
        exit (1);
    }

    /* Get the port number. */
    port = 4000;
    if (argc > 1) {
        if (!is_number (argv[1])) {
            fprintf (stderr, "Usage: %s [port #]\n", argv[0]);
            exit (1);
        }
        else if ((port = atoi (argv[1])) <= 1024) {
            fprintf (stderr, "Port number must be above 1024.\n");
            exit (1);
        }

        /* Are we recovering from a copyover? */
        if (argv[2] && argv[2][0]) {
            fCopyOver = TRUE;
            control = atoi (argv[3]);
            #ifdef IMC
                imcsocket = atoi (argv[4]);
            #endif
        }
        else
            fCopyOver = FALSE;
    }

    /* Run the game. */
    #if defined(macintosh) || defined(MSDOS)
        qmconfig_read(); /* Here because it fits, no conflicts with Linux placement -- JR 05/06/01 */
        boot_db ();
        log_string ("Merc is ready to rock.");
        game_loop_mac_msdos ();
    #endif

    #if defined(unix)
        qmconfig_read(); /* Here so we can set the IP adress. -- JR 05/06/01 */
        if (!fCopyOver)
            control = init_socket (port);
        boot_db ();
        log_f ("ROM is ready to rock on port %d (%s).", port, mud_ipaddress);

        #ifdef IMC
            /* Initialize and connect to IMC2 */
            imc_startup (FALSE, imcsocket, fCopyOver);
        #endif

        if (fCopyOver)
            copyover_recover ();

        game_loop_unix (control);
        close (control);
        #ifdef IMC
            imc_shutdown (FALSE);
        #endif
    #endif

    /* That's all, folks. */
    log_string ("Normal termination of game.");
    exit (0);
    return 0;
}
