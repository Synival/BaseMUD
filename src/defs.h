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

#ifndef __ROM_DEFS_H
#define __ROM_DEFS_H

/* Function definitions in *.c */
#define DEFINE_DO_FUN(fun) \
    void fun (CHAR_T *ch, char *argument)

#define DEFINE_SPELL_FUN(fun) \
    void fun (int sn, int level, CHAR_T *ch, void *vo, int target, \
              const char *target_name)

#define DEFINE_EFFECT_FUN(fun) \
    void fun (void *vo, int level, int dam, int target)

#define DEFINE_NANNY_FUN(fun) \
    void fun (DESCRIPTOR_T *d, char *argument)

#define DEFINE_TABLE_JSON_FUN(fun) \
    JSON_T *fun (const void *obj, const char *obj_name)

/* Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types. */
#if !defined(FALSE)
    #define FALSE 0
#endif

#if !defined(TRUE)
    #define TRUE  1
#endif

#if defined(_AIX)
    #if !defined(const)
        #define const
    #endif
#endif

/* Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file). */

#if defined(macintosh)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define NULL_FILE   "proto.are"         /* To reserve one stream */
    #define TEMP_FILE   "romtmp"
#endif

#if defined(MSDOS)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define NULL_FILE   "nul"               /* To reserve one stream */
    #define TEMP_FILE   "romtmp"
#endif

#if defined(unix)
    #define PLAYER_DIR  "player/"           /* Player files          */
    #define GOD_DIR     "gods/"             /* list of gods          */
    #define NULL_FILE   "/dev/null"         /* To reserve one stream */
    #define TEMP_FILE   "player/romtmp"
#endif

#define AREA_DIR        "area/"
#define DUMP_DIR        "dump/"
#define AREA_LIST       AREA_DIR "area.lst"     /* List of areas       */
#define BUG_FILE        AREA_DIR "bugs.txt"     /* For 'bug' and bug() */
#define TYPO_FILE       AREA_DIR "typos.txt"    /* For 'typo'          */
#define SHUTDOWN_FILE   AREA_DIR "shutdown.txt" /* For 'shutdown'      */
#define BAN_FILE        AREA_DIR "ban.txt"
#define MUSIC_FILE      AREA_DIR "music.txt"
#define OHELPS_FILE     AREA_DIR "orphaned_helps.txt" /* Unmet 'help' requests */
#define QMCONFIG_FILE   AREA_DIR "qmconfig.rc"

/* String and memory management parameters. */
#define MAX_KEY_HASH          1024
#define MAX_STRING_LENGTH     4608
#define MAX_INPUT_LENGTH       256
#define PAGELEN                 22

/* I am lazy :) */
#define MSL MAX_STRING_LENGTH
#define MIL MAX_INPUT_LENGTH

/* Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself. */
#define MAX_IN_GROUP        15
#define MAX_ALIAS           5
#define MAX_LEVEL           60
#define LEVEL_HERO          (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL      (MAX_LEVEL - 8)
#define L_IMM LEVEL_IMMORTAL

/* Added this for "orphaned help" code. Check do_help() -- JR */
#define MAX_CMD_LEN      50

#define PULSE_PER_SECOND 4
#define PULSE_SPEED      100 /* percentage; 100 = normal */

#define PULSE_VIOLENCE   (  3 * PULSE_PER_SECOND)
#define PULSE_MOBILE     (  4 * PULSE_PER_SECOND)
#define PULSE_MUSIC      (  6 * PULSE_PER_SECOND)
#define PULSE_TICK       ( 60 * PULSE_PER_SECOND)
#define PULSE_AREA       (120 * PULSE_PER_SECOND)

/* Amount of hit/mana/move we're dividing by when running health_update() */
#ifdef BASEMUD_GRADUAL_RECOVERY
    #define PULSE_DIVISOR    30
#else
    #define PULSE_DIVISOR    1 
#endif
#define PULSE_HEALTH     (PULSE_TICK / PULSE_DIVISOR)

#define IMPLEMENTOR (MAX_LEVEL - 0)
#define CREATOR     (MAX_LEVEL - 1)
#define SUPREME     (MAX_LEVEL - 2)
#define DEITY       (MAX_LEVEL - 3)
#define GOD         (MAX_LEVEL - 4)
#define IMMORTAL    (MAX_LEVEL - 5)
#define DEMI        (MAX_LEVEL - 6)
#define ANGEL       (MAX_LEVEL - 7)
#define AVATAR      (MAX_LEVEL - 8)
#define HERO        (LEVEL_HERO)

/* Shorthand for command types */
#define ML    (MAX_LEVEL - 0)  /* implementor */
#define L1    (MAX_LEVEL - 1)  /* creator */
#define L2    (MAX_LEVEL - 2)  /* supreme being */
#define L3    (MAX_LEVEL - 3)  /* deity */
#define L4    (MAX_LEVEL - 4)  /* god */
#define L5    (MAX_LEVEL - 5)  /* immortal */
#define L6    (MAX_LEVEL - 6)  /* demigod */
#define L7    (MAX_LEVEL - 7)  /* angel */
#define L8    (MAX_LEVEL - 8)  /* avatar */
#define IM    (LEVEL_IMMORTAL) /* avatar */
#define HE    (LEVEL_HERO)     /* hero */

/* Time and weather stuff. */
#define SUN_DARK       0
#define SUN_RISE       1
#define SUN_LIGHT      2
#define SUN_SET        3
#define SUN_MAX        4

#define SKY_CLOUDLESS  0
#define SKY_CLOUDY     1
#define SKY_RAINING    2
#define SKY_LIGHTNING  3
#define SKY_MAX        4

/* damage classes */
#define DAM_NONE            0
#define DAM_BASH            1
#define DAM_PIERCE          2
#define DAM_SLASH           3
#define DAM_FIRE            4
#define DAM_COLD            5
#define DAM_LIGHTNING       6
#define DAM_ACID            7
#define DAM_POISON          8
#define DAM_NEGATIVE        9
#define DAM_HOLY           10
#define DAM_ENERGY         11
#define DAM_MENTAL         12
#define DAM_DISEASE        13
#define DAM_DROWNING       14
#define DAM_LIGHT          15
#define DAM_OTHER          16
#define DAM_HARM           17
#define DAM_CHARM          18
#define DAM_SOUND          19
#define DAM_MAX            20

/* Damage flags. */
#define DAM_MAGICAL     A

/* TO types for act. */
#define TO_CHAR    (A) /* 'ch' only */
#define TO_VICT    (B) /* 'vch' only */
#define TO_OTHERS  (C) /* everyone else */
#define TO_NOTCHAR (TO_VICT | TO_OTHERS)
#define TO_ALL     (TO_CHAR | TO_VICT | TO_OTHERS)

/* Shop types. */
#define MAX_TRADE 5

/* Per-class stuff. */
#define MAX_GUILD 2

/* Stat types. */
#define STAT_STR  0
#define STAT_INT  1
#define STAT_WIS  2
#define STAT_DEX  3
#define STAT_CON  4
#define STAT_MAX  5

/* where definitions */
#define AFF_TO_AFFECTS  0
#define AFF_TO_OBJECT   1
#define AFF_TO_IMMUNE   2
#define AFF_TO_RESIST   3
#define AFF_TO_VULN     4
#define AFF_TO_WEAPON   5
#define AFF_TO_MAX      6

/* Well known mob virtual numbers.
 * Defined in #MOBILES. */
#define MOB_VNUM_FIDO       3090
#define MOB_VNUM_CITYGUARD  3060
#define MOB_VNUM_VAMPIRE    3404

#define MOB_VNUM_PATROLMAN  2106
#define GROUP_VNUM_TROLLS   2100
#define GROUP_VNUM_OGRES    2101

/* return values for check_imm */
#define IS_NORMAL          0
#define IS_IMMUNE          1
#define IS_RESISTANT       2
#define IS_VULNERABLE      3

/* Liquids. */
#define LIQ_WATER   0
/* ... lots of stuff ... */
#define LIQ_MAX     36

/* Well known object virtual numbers.
 * Defined in #OBJECTS. */
#define OBJ_VNUM_SILVER_ONE         1
#define OBJ_VNUM_GOLD_ONE           2
#define OBJ_VNUM_GOLD_SOME          3
#define OBJ_VNUM_SILVER_SOME        4
#define OBJ_VNUM_COINS              5

#define OBJ_VNUM_CORPSE_NPC        10
#define OBJ_VNUM_CORPSE_PC         11
#define OBJ_VNUM_SEVERED_HEAD      12
#define OBJ_VNUM_TORN_HEART        13
#define OBJ_VNUM_SLICED_ARM        14
#define OBJ_VNUM_SLICED_LEG        15
#define OBJ_VNUM_GUTS              16
#define OBJ_VNUM_BRAINS            17

#define OBJ_VNUM_MUSHROOM          20
#define OBJ_VNUM_LIGHT_BALL        21
#define OBJ_VNUM_SPRING            22
#define OBJ_VNUM_DISC              23
#define OBJ_VNUM_PORTAL            25

#define OBJ_VNUM_ROSE              1001

#define OBJ_VNUM_PIT               3010

#define OBJ_VNUM_SCHOOL_MACE       3700
#define OBJ_VNUM_SCHOOL_DAGGER     3701
#define OBJ_VNUM_SCHOOL_SWORD      3702
#define OBJ_VNUM_SCHOOL_SPEAR      3717
#define OBJ_VNUM_SCHOOL_STAFF      3718
#define OBJ_VNUM_SCHOOL_AXE        3719
#define OBJ_VNUM_SCHOOL_FLAIL      3720
#define OBJ_VNUM_SCHOOL_WHIP       3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722

#define OBJ_VNUM_SCHOOL_VEST       3703
#define OBJ_VNUM_SCHOOL_SHIELD     3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP               3162

#define OBJ_VNUM_WHISTLE           2116

/* magic numbers. */
#define OBJ_VALUE_MAX       5
#define RESET_VALUE_MAX     5
#define PILL_SKILL_MAX      4
#define SCROLL_SKILL_MAX    4
#define POTION_SKILL_MAX    4

/* Values for object mapping.
 * Used in #OBJECTS. */
#define MAP_INTEGER     0
#define MAP_LOOKUP      1
#define MAP_FLAGS       2
#define MAP_IGNORE      3
#define MAP_BOOLEAN     4

/* Sub-types of MAP_LOOKUP. */
#define MAP_LOOKUP_WEAPON_TYPE  0
#define MAP_LOOKUP_ATTACK_TYPE  1
#define MAP_LOOKUP_LIQUID       2
#define MAP_LOOKUP_SKILL        3

/* Sub-types of MAP_FLAGS. */
#define MAP_FLAGS_WEAPON        0
#define MAP_FLAGS_CONT          1
#define MAP_FLAGS_FURNITURE     3
#define MAP_FLAGS_EXIT          4
#define MAP_FLAGS_GATE          5

/* Well known room virtual numbers.
 * Defined in #ROOMS. */
#define ROOM_VNUM_LIMBO       2
#define ROOM_VNUM_CHAT     1200
#define ROOM_VNUM_TEMPLE   3001
#define ROOM_VNUM_ALTAR    3054
#define ROOM_VNUM_SCHOOL   3700
#define ROOM_VNUM_BALANCE  4500
#define ROOM_VNUM_CIRCLE   4400
#define ROOM_VNUM_DEMISE   4201
#define ROOM_VNUM_HONOR    4300

/* Directions.
 * Used in #ROOMS. */
#define DIR_NONE  -1
#define DIR_NORTH  0
#define DIR_EAST   1
#define DIR_SOUTH  2
#define DIR_WEST   3
#define DIR_UP     4
#define DIR_DOWN   5
#define DIR_MAX    6

/* Conditions. */
#define COND_DRUNK    0
#define COND_FULL     1
#define COND_THIRST   2
#define COND_HUNGER   3
#define COND_MAX      4

/* Types of attacks.
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that. */
#define ATTACK_DEFAULT     -1
#define ATTACK_NONE         0
#define ATTACK_SLASH        3  /* TODO: don't hard-code */
#define ATTACK_POUND        7  /* TODO: don't hard-code */
#define ATTACK_PIERCE       11 /* TODO: don't hard-code */
#define ATTACK_PUNCH        17 /* TODO: don't hard-code */

/* Internal flag used to designate non-skill attacks. */
#define ATTACK_FIGHTING     1000

/* Target types. */
#define SKILL_TARGET_IGNORE          0
#define SKILL_TARGET_CHAR_OFFENSIVE  1
#define SKILL_TARGET_CHAR_DEFENSIVE  2
#define SKILL_TARGET_CHAR_SELF       3
#define SKILL_TARGET_OBJ_INV         4
#define SKILL_TARGET_OBJ_CHAR_DEF    5
#define SKILL_TARGET_OBJ_CHAR_OFF    6
#define SKILL_TARGET_MAX             7

#define TARGET_CHAR         0
#define TARGET_OBJ          1
#define TARGET_ROOM         2
#define TARGET_NONE         3

/* Object defined in limbo.are
 * Used in save.c to load objects that don't exist. */
#define OBJ_VNUM_DUMMY    30

#define FLAG_NONE   0
#define TYPE_NONE   -999

/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */
#define NOTE_DIR    "notes" /* set it to something you like */

#define DEF_NORMAL  0  /* No forced change, but default (any string)  */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */
#define DEF_MAX     3

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
                        /* It should be readable by everyone!           */

#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
                           /* This only applies in the Body of the note */

#define MAX_NOTE_TEXT (4 * MAX_STRING_LENGTH - 1000)

#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Connected state for a channel. */
#define CON_GET_NAME         -14
#define CON_GET_OLD_PASSWORD -13
#define CON_CONFIRM_NEW_NAME -12
#define CON_GET_NEW_PASSWORD -11
#define CON_CONFIRM_PASSWORD -10
#define CON_ANSI              -9
#define CON_GET_TELNETGA      -8
#define CON_GET_NEW_RACE      -7
#define CON_GET_NEW_SEX       -6
#define CON_GET_NEW_CLASS     -5
#define CON_GET_ALIGNMENT     -4
#define CON_DEFAULT_CHOICE    -3
#define CON_GEN_GROUPS        -2
#define CON_PICK_WEAPON       -1
#define CON_PLAYING            0
#define CON_READ_IMOTD         1
#define CON_READ_MOTD          2
#define CON_BREAK_CONNECT      3
#define CON_COPYOVER_RECOVER   4
#define CON_NOTE_TO            5
#define CON_NOTE_SUBJECT       6
#define CON_NOTE_EXPIRE        7
#define CON_NOTE_TEXT          8
#define CON_NOTE_FINISH        9
#define NANNY_MAX              24

/* Character classes. */
#define CLASS_NONE  -1
#define CLASS_MAX    16

/* Limits not defined elsewhere. */
#define CLAN_MAX            3
#define WIZNET_MAX          20
#define RACE_MAX            30
#define PC_RACE_MAX         6
#define PC_RACE_SKILL_MAX   5
#define SOCIAL_MAX          256
#define SKILL_MAX           150
#define SKILL_GROUP_MAX     30
#define SPEC_MAX            22
#define ATTRIBUTE_HIGHEST   25
#define ATTACK_MAX          40
#define BOARD_MAX           5

/* Colour setting codes */
#define COLOUR_TEXT            0
#define COLOUR_AUCTION         1
#define COLOUR_AUCTION_TEXT    2
#define COLOUR_GOSSIP          3
#define COLOUR_GOSSIP_TEXT     4
#define COLOUR_MUSIC           5
#define COLOUR_MUSIC_TEXT      6
#define COLOUR_QUESTION        7
#define COLOUR_QUESTION_TEXT   8
#define COLOUR_ANSWER          9
#define COLOUR_ANSWER_TEXT    10
#define COLOUR_QUOTE          11
#define COLOUR_QUOTE_TEXT     12
#define COLOUR_IMMTALK_TEXT   13
#define COLOUR_IMMTALK_TYPE   14
#define COLOUR_INFO           15
#define COLOUR_SAY            16
#define COLOUR_SAY_TEXT       17
#define COLOUR_TELL           18
#define COLOUR_TELL_TEXT      19
#define COLOUR_REPLY          20
#define COLOUR_REPLY_TEXT     21
#define COLOUR_GTELL_TEXT     22
#define COLOUR_GTELL_TYPE     23
#define COLOUR_WIZNET         24
#define COLOUR_ROOM_TITLE     25
#define COLOUR_ROOM_TEXT      26
#define COLOUR_ROOM_EXITS     27
#define COLOUR_ROOM_THINGS    28
#define COLOUR_PROMPT         29
#define COLOUR_FIGHT_DEATH    30
#define COLOUR_FIGHT_YHIT     31
#define COLOUR_FIGHT_OHIT     32
#define COLOUR_FIGHT_THIT     33
#define COLOUR_FIGHT_SKILL    34
#define COLOUR_MAX            35

/* This file holds the copyover data */
#define COPYOVER_FILE "copyover.data"

/* This is the executable file */
#define EXE_FILE      "bin/rom"

/* Recycleable types. */
#define RECYCLE_BAN_T         0
#define RECYCLE_AREA_T        1
#define RECYCLE_EXTRA_DESCR_T 2
#define RECYCLE_EXIT_T        3
#define RECYCLE_ROOM_INDEX_T  4
#define RECYCLE_OBJ_INDEX_T   5
#define RECYCLE_SHOP_T        6
#define RECYCLE_MOB_INDEX_T   7
#define RECYCLE_RESET_T       8
#define RECYCLE_HELP_T        9
#define RECYCLE_MPROG_CODE_T  10
#define RECYCLE_DESCRIPTOR_T  11
#define RECYCLE_GEN_T         12
#define RECYCLE_AFFECT_T      13
#define RECYCLE_OBJ_T         14
#define RECYCLE_CHAR_T        15
#define RECYCLE_PC_T          16
#define RECYCLE_MEM_T         17
#define RECYCLE_BUFFER_T      18
#define RECYCLE_MPROG_LIST_T  19
#define RECYCLE_HELP_AREA_T   20
#define RECYCLE_NOTE_T        21
#define RECYCLE_SOCIAL_T      22
#define RECYCLE_PORTAL_EXIT_T 23
#define RECYCLE_PORTAL_T      24
#define RECYCLE_MAX           25

/* Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing. */
#define MAX_STRING_SPACE    2097152 /* 2^21 */
#define MAX_PERM_BLOCK      131072
#define MAX_MEM_LIST        11
#define MAX_PERM_BLOCKS     1024

/* Types of tables used for our master reference table. */
#define TABLE_FLAGS     0
#define TABLE_TYPES     1
#define TABLE_UNIQUE    2

#define TABLE_MAX   76

/* Types of portals. */
#define PORTAL_TO_ROOM  0
#define PORTAL_TO_EXIT  1

/* Days of the week. */
#define DAY_MOON        0
#define DAY_BULL        1
#define DAY_DECEPTION   2
#define DAY_THUNDER     3
#define DAY_FREEDOM     4
#define DAY_GREAT_GODS  5
#define DAY_SUN         6
#define DAY_MAX         7

/* Months in the year. */
#define MONTH_WINTER            0
#define MONTH_WINTER_WOLF       1
#define MONTH_FROST_GIANT       2
#define MONTH_OLD_FORCES        3
#define MONTH_GRAND_STRUGGLE    4
#define MONTH_SPRING            5
#define MONTH_NATURE            6
#define MONTH_FUTILITY          7
#define MONTH_DRAGON            8
#define MONTH_SUN               9
#define MONTH_HEAT             10
#define MONTH_BATTLE           11
#define MONTH_DARK_SHADES      12
#define MONTH_SHADOWS          13
#define MONTH_LONG_SHADOWS     14
#define MONTH_ANCIENT_DARKNESS 15
#define MONTH_GREAT_EVIL       16
#define MONTH_MAX              17

/* Additional time information. */
#define DAYS_PER_MONTH  35
#define HOURS_PER_DAY   24

/* Preposition types for standing/resting/sitting. */
#define POS_PREP_NO_OBJECT     -3
#define POS_PREP_NOT_FURNITURE -2
#define POS_PREP_BAD_POSITION  -1
#define POS_PREP_AT             0
#define POS_PREP_ON             1
#define POS_PREP_IN             2
#define POS_PREP_BY             3

/* Parameters for char_exit_string() */
#define EXITS_LONG      0
#define EXITS_AUTO      1
#define EXITS_PROMPT    2

/* Door reset types. */
#define RESET_DOOR_NONE     0
#define RESET_DOOR_CLOSED   1
#define RESET_DOOR_LOCKED   2

/* Types of anum references used for assignment. */
#define ANUM_ROOM   0
#define ANUM_MOB    1
#define ANUM_OBJ    2

/* Special identifiers for exit key cases. */
#define KEY_NOKEYHOLE  -1
#define KEY_NOKEY       0
#define KEY_VALID       1 /* Everything here and higher is a real key. */

/* Command logging types. */
#define LOG_NORMAL  0
#define LOG_ALWAYS  1
#define LOG_NEVER   2

/* Music stuff. */
#define MAX_SONGS       20
#define MAX_SONG_LINES  100 /* this boils down to about 1k per song */
#define MAX_SONG_GLOBAL 10  /* max songs the global jukebox can hold */

/* Types of damage effects. */
#define EFFECT_NONE     0
#define EFFECT_FIRE     1
#define EFFECT_COLD     2
#define EFFECT_SHOCK    3
#define EFFECT_ACID     4
#define EFFECT_POISON   5
#define EFFECT_MAX      6

/* Internal mappings of skills. */
#define SKILL_MAP_BACKSTAB          0
#define SKILL_MAP_DODGE             1
#define SKILL_MAP_ENVENOM           2
#define SKILL_MAP_HIDE              3
#define SKILL_MAP_PEEK              4
#define SKILL_MAP_PICK_LOCK         5
#define SKILL_MAP_SNEAK             6
#define SKILL_MAP_STEAL             7
#define SKILL_MAP_DISARM            8
#define SKILL_MAP_ENHANCED_DAMAGE   9
#define SKILL_MAP_KICK             10
#define SKILL_MAP_PARRY            11
#define SKILL_MAP_RESCUE           12
#define SKILL_MAP_SECOND_ATTACK    13
#define SKILL_MAP_THIRD_ATTACK     14
#define SKILL_MAP_BLINDNESS        15
#define SKILL_MAP_CHARM_PERSON     16
#define SKILL_MAP_CURSE            17
#define SKILL_MAP_INVIS            18
#define SKILL_MAP_MASS_INVIS       19
#define SKILL_MAP_POISON           20
#define SKILL_MAP_PLAGUE           21
#define SKILL_MAP_SLEEP            22
#define SKILL_MAP_SANCTUARY        23
#define SKILL_MAP_FLY              24
#define SKILL_MAP_AXE              25
#define SKILL_MAP_DAGGER           26
#define SKILL_MAP_FLAIL            27
#define SKILL_MAP_MACE             28
#define SKILL_MAP_POLEARM          29
#define SKILL_MAP_SHIELD_BLOCK     30
#define SKILL_MAP_SPEAR            31
#define SKILL_MAP_SWORD            32
#define SKILL_MAP_WHIP             33
#define SKILL_MAP_BASH             34
#define SKILL_MAP_BERSERK          35
#define SKILL_MAP_DIRT             36
#define SKILL_MAP_HAND_TO_HAND     37
#define SKILL_MAP_TRIP             38
#define SKILL_MAP_FAST_HEALING     39
#define SKILL_MAP_HAGGLE           40
#define SKILL_MAP_LORE             41
#define SKILL_MAP_MEDITATION       42
#define SKILL_MAP_SCROLLS          43
#define SKILL_MAP_STAVES           44
#define SKILL_MAP_WANDS            45
#define SKILL_MAP_RECALL           46
#define SKILL_MAP_FRENZY           47
#define SKILL_MAP_MAX              48

#endif
