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
 *    ROM 2.4 is copyright 1993-1998 Russ Taylor                           *
 *    ROM has been brought to you by the ROM consortium                    *
 *        Russ Taylor (rtaylor@hypercube.org)                              *
 *        Gabrielle Taylor (gtaylor@hypercube.org)                         *
 *        Brian Moore (zump@rom.org)                                       *
 *    By using this code, you have agreed to follow the terms of the       *
 *    ROM license, in the file Rom24/doc/rom.license                       *
 ***************************************************************************/

#ifndef __ROM_COMM_H
#define __ROM_COMM_H

#include "merc.h"

/* Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it. */
#if defined(apollo)
    #define __attribute(x)
#endif

#if defined(apollo)
    #undef __attribute
#endif

extern const char echo_off_str[];
extern const char echo_on_str[];
extern const char go_ahead_str[];

/* OS-dependent declarations. */
#if defined(_AIX)
    #include <sys/select.h>
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int setsockopt args ((int s, int level, int optname, void *optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
#endif

#if defined(apollo)
    #include <unistd.h>
    void bzero args ((char *b, int length));
#endif

#if defined(__hpux)
    int accept args ((int s, void *addr, int *addrlen));
    int bind args ((int s, const void *addr, int addrlen));
    void bzero args ((char *b, int length));
    int getpeername args ((int s, void *addr, int *addrlen));
    int getsockname args ((int s, void *name, int *addrlen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int setsockopt args ((int s, int level, int optname,
                          const void *optval, int optlen));
    int socket args ((int domain, int type, int protocol));
#endif

#if defined(interactive)
    #include <net/errno.h>
    #include <sys/fnctl.h>
#endif

#if defined(linux)
/* Linux shouldn't need these. If you have a problem compiling, try
   uncommenting these functions. */
/*
    int accept args( ( int s, struct sockaddr *addr, int *addrlen ) );
    int bind args( ( int s, struct sockaddr *name, int namelen ) );
    int getpeername args( ( int s, struct sockaddr *name, int *namelen ) );
    int getsockname args( ( int s, struct sockaddr *name, int *namelen ) );
    int listen args( ( int s, int backlog ) );
*/

    int close args ((int fd));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    /* int read args( ( int fd, char *buf, int nbyte ) ); */
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int socket args ((int domain, int type, int protocol));
    /* int write args( ( int fd, char *buf, int nbyte ) ); *//* read,write in unistd.h */
#endif

#if defined(macintosh)
    #include <console.h>
    #include <fcntl.h>
    #include <unix.h>
    struct timeval {
        time_t tv_sec;
        time_t tv_usec;
    };
    #if !defined(isascii)
        #define isascii(c) ( (c) < 0200 )
    #endif

    int gettimeofday (struct timeval *tp, void *tzp) {
        tp->tv_sec = time (NULL);
        tp->tv_usec = 0;
    }
#endif

#if defined(MIPS_OS)
    extern int errno;
#endif

#if defined(MSDOS)
    int gettimeofday args ((struct timeval * tp, void *tzp));
    int kbhit args ((void));
#endif

#if defined(NeXT)
    int close args ((int fd));
    int fcntl args ((int fd, int cmd, int arg));
    #if !defined(htons)
        u_short htons args ((u_short hostshort));
    #endif
    #if !defined(ntohl)
        u_long ntohl args ((u_long hostlong));
    #endif
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int write args ((int fd, char *buf, int nbyte));
#endif

#if defined(sequent)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    int close args ((int fd));
    int fcntl args ((int fd, int cmd, int arg));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    #if !defined(htons)
        u_short htons args ((u_short hostshort));
    #endif
    int listen args ((int s, int backlog));
    #if !defined(ntohl)
        u_long ntohl args ((u_long hostlong));
    #endif
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int setsockopt args ((int s, int level, int optname, caddr_t optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

/* This includes Solaris Sys V as well */
#if defined(sun)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int close args ((int fd));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int listen args ((int s, int backlog));
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));

    #if !defined(__SVR4)
        int gettimeofday args ((struct timeval * tp, struct timezone * tzp));

        #if defined(SYSV)
            int setsockopt args ((int s, int level, int optname,
                                  const char *optval, int optlen));
        #else
            int setsockopt args ((int s, int level, int optname, void *optval,
                                  int optlen));
        #endif
    #endif
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

#if defined(ultrix)
    int accept args ((int s, struct sockaddr * addr, int *addrlen));
    int bind args ((int s, struct sockaddr * name, int namelen));
    void bzero args ((char *b, int length));
    int close args ((int fd));
    int getpeername args ((int s, struct sockaddr * name, int *namelen));
    int getsockname args ((int s, struct sockaddr * name, int *namelen));
    int gettimeofday args ((struct timeval * tp, struct timezone * tzp));
    int listen args ((int s, int backlog));
    int read args ((int fd, char *buf, int nbyte));
    int select args ((int width, fd_set * readfds, fd_set * writefds,
                      fd_set * exceptfds, struct timeval * timeout));
    int setsockopt args ((int s, int level, int optname, void *optval,
                          int optlen));
    int socket args ((int domain, int type, int protocol));
    int write args ((int fd, char *buf, int nbyte));
#endif

/* Function prototypes. */
void bust_a_prompt (CHAR_T *ch);
void send_to_char_bw (const char *txt, CHAR_T *ch);
void page_to_char_bw (const char *txt, CHAR_T *ch);
void send_to_char (const char *txt, CHAR_T *ch);
void page_to_char (const char *txt, CHAR_T *ch);
void act2 (const char *to_char, const char *to_room, CHAR_T *ch,
    const void *arg1, const void *arg2, flag_t flags, int min_pos);
void act3 (const char *to_char, const char *to_vict, const char *to_room,
    CHAR_T *ch, const void *arg1, const void *arg2, flag_t flags, int min_pos);
bool act_is_valid_recipient (CHAR_T *to, flag_t flags, CHAR_T *ch, CHAR_T *vch);
char *act_code (char code, CHAR_T *ch, CHAR_T *vch, CHAR_T *to, OBJ_T *obj1,
    OBJ_T *obj2, const void *arg1, const void *arg2, char *out_buf,
    size_t size);
void act_new (const char *format, CHAR_T *ch, const void *arg1,
    const void *arg2, flag_t flags, int min_pos);
void printf_to_char (CHAR_T *ch, const char *fmt, ...);
void wiznet (const char *string, CHAR_T *ch, OBJ_T *obj, flag_t flag,
    flag_t flag_skip, int min_level);
void wiznetf (CHAR_T *ch, OBJ_T *obj, flag_t flag, flag_t flag_skip,
    int min_level, const char *fmt, ...);
bool position_change_send_message (CHAR_T *ch, int from, int to, OBJ_T *obj);
bool position_change_send_message_to_standing (CHAR_T *ch, int from,
    OBJ_T *obj);
bool position_change_send_message_to_fighting (CHAR_T *ch, int from,
    OBJ_T *obj);
bool position_change_send_message_to_resting (CHAR_T *ch, int from,
    OBJ_T *obj);
bool position_change_send_message_to_sitting (CHAR_T *ch, int from,
    OBJ_T *obj);
bool position_change_send_message_to_sleeping (CHAR_T *ch, int from,
    OBJ_T *obj);
void echo_to_char (CHAR_T *to, CHAR_T *from, const char *type, const char *msg);

#endif
