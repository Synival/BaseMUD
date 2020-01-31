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

#ifndef __ROM_FLAGS_H
#define __ROM_FLAGS_H

#include "merc.h"

/* RT ASCII conversions -- used so we can have letters in this file */
#define BIT_01       (0x00000001) /* A */
#define BIT_02       (0x00000002) /* B */
#define BIT_03       (0x00000004) /* C */
#define BIT_04       (0x00000008) /* D */
#define BIT_05       (0x00000010) /* E */
#define BIT_06       (0x00000020) /* F */
#define BIT_07       (0x00000040) /* G */
#define BIT_08       (0x00000080) /* H */
#define BIT_09       (0x00000100) /* I */
#define BIT_10       (0x00000200) /* J */
#define BIT_11       (0x00000400) /* K */
#define BIT_12       (0x00000800) /* L */
#define BIT_13       (0x00001000) /* M */
#define BIT_14       (0x00002000) /* N */
#define BIT_15       (0x00004000) /* O */
#define BIT_16       (0x00008000) /* P */
#define BIT_17       (0x00010000) /* Q */
#define BIT_18       (0x00020000) /* R */
#define BIT_19       (0x00040000) /* S */
#define BIT_20       (0x00080000) /* T */
#define BIT_21       (0x00100000) /* U */
#define BIT_22       (0x00200000) /* V */
#define BIT_23       (0x00400000) /* W */
#define BIT_24       (0x00800000) /* X */
#define BIT_25       (0x01000000) /* Y */
#define BIT_26       (0x02000000) /* Z */
#define BIT_27       (0x04000000) /* a */
#define BIT_28       (0x08000000) /* b */
#define BIT_29       (0x10000000) /* c */
#define BIT_30       (0x20000000) /* d */
#define BIT_31       (0x40000000) /* e */

/* Site ban structure.  */
#define BAN_SUFFIX     (BIT_01)
#define BAN_PREFIX     (BIT_02)
#define BAN_NEWBIES    (BIT_03)
#define BAN_ALL        (BIT_04)
#define BAN_PERMIT     (BIT_05)
#define BAN_PERMANENT  (BIT_06)

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK    (BIT_01)
#define OFF_BACKSTAB       (BIT_02)
#define OFF_BASH           (BIT_03)
#define OFF_BERSERK        (BIT_04)
#define OFF_DISARM         (BIT_05)
#define OFF_DODGE          (BIT_06)
#define OFF_FADE           (BIT_07)
#define OFF_FAST           (BIT_08)
#define OFF_KICK           (BIT_09)
#define OFF_KICK_DIRT      (BIT_10)
#define OFF_PARRY          (BIT_11)
#define OFF_RESCUE         (BIT_12)
#define OFF_TAIL           (BIT_13)
#define OFF_TRIP           (BIT_14)
#define OFF_CRUSH          (BIT_15)
#define ASSIST_ALL         (BIT_16)
#define ASSIST_ALIGN       (BIT_17)
#define ASSIST_RACE        (BIT_18)
#define ASSIST_PLAYERS     (BIT_19)
#define ASSIST_GUARD       (BIT_20)
#define ASSIST_VNUM        (BIT_21)

/* IMM/RES/VULN bits for mobs */
#define RES_SUMMON         (BIT_01)
#define RES_CHARM          (BIT_02)
#define RES_MAGIC          (BIT_03)
#define RES_WEAPON         (BIT_04)
#define RES_BASH           (BIT_05)
#define RES_PIERCE         (BIT_06)
#define RES_SLASH          (BIT_07)
#define RES_FIRE           (BIT_08)
#define RES_COLD           (BIT_09)
#define RES_LIGHTNING      (BIT_10)
#define RES_ACID           (BIT_11)
#define RES_POISON         (BIT_12)
#define RES_NEGATIVE       (BIT_13)
#define RES_HOLY           (BIT_14)
#define RES_ENERGY         (BIT_15)
#define RES_MENTAL         (BIT_16)
#define RES_DISEASE        (BIT_17)
#define RES_DROWNING       (BIT_18)
#define RES_LIGHT          (BIT_19)
#define RES_SOUND          (BIT_20)
#define RES_UNUSED_FLAG_1  (BIT_21)
#define RES_UNUSED_FLAG_2  (BIT_22)
#define RES_UNUSED_FLAG_3  (BIT_23)
#define RES_WOOD           (BIT_24)
#define RES_SILVER         (BIT_25)
#define RES_IRON           (BIT_26)

/* body form */
#define FORM_EDIBLE        (BIT_01)
#define FORM_POISON        (BIT_02)
#define FORM_MAGICAL       (BIT_03)
#define FORM_INSTANT_DECAY (BIT_04)
#define FORM_OTHER         (BIT_05) /* defined by material bit */
#define FORM_UNUSED_FLAG_1 (BIT_06)

/* actual form */
#define FORM_ANIMAL        (BIT_07)
#define FORM_SENTIENT      (BIT_08)
#define FORM_UNDEAD        (BIT_09)
#define FORM_CONSTRUCT     (BIT_10)
#define FORM_MIST          (BIT_11)
#define FORM_INTANGIBLE    (BIT_12)

#define FORM_BIPED         (BIT_13)
#define FORM_CENTAUR       (BIT_14)
#define FORM_INSECT        (BIT_15)
#define FORM_SPIDER        (BIT_16)
#define FORM_CRUSTACEAN    (BIT_17)
#define FORM_WORM          (BIT_18)
#define FORM_BLOB          (BIT_19)
#define FORM_UNUSED_FLAG_2 (BIT_20)
#define FORM_UNUSED_FLAG_3 (BIT_21)

#define FORM_MAMMAL        (BIT_22)
#define FORM_BIRD          (BIT_23)
#define FORM_REPTILE       (BIT_24)
#define FORM_SNAKE         (BIT_25)
#define FORM_DRAGON        (BIT_26)
#define FORM_AMPHIBIAN     (BIT_27)
#define FORM_FISH          (BIT_28)
#define FORM_COLD_BLOOD    (BIT_29)

/* body parts */
#define PART_HEAD          (BIT_01)
#define PART_ARMS          (BIT_02)
#define PART_LEGS          (BIT_03)
#define PART_HEART         (BIT_04)
#define PART_BRAINS        (BIT_05)
#define PART_GUTS          (BIT_06)
#define PART_HANDS         (BIT_07)
#define PART_FEET          (BIT_08)
#define PART_FINGERS       (BIT_09)
#define PART_EAR           (BIT_10)
#define PART_EYE           (BIT_11)
#define PART_LONG_TONGUE   (BIT_12)
#define PART_EYESTALKS     (BIT_13)
#define PART_TENTACLES     (BIT_14)
#define PART_FINS          (BIT_15)
#define PART_WINGS         (BIT_16)
#define PART_TAIL          (BIT_17)
#define PART_UNUSED_FLAG_1 (BIT_18)
#define PART_UNUSED_FLAG_2 (BIT_19)
#define PART_UNUSED_FLAG_3 (BIT_20)
#define PART_CLAWS         (BIT_21) /* for combat */
#define PART_FANGS         (BIT_22)
#define PART_HORNS         (BIT_23)
#define PART_SCALES        (BIT_24)
#define PART_TUSKS         (BIT_25)

/* Bits for 'affected_by'.
 * Used in #MOBILES.  */
#define AFF_BLIND          (BIT_01)
#define AFF_INVISIBLE      (BIT_02)
#define AFF_DETECT_EVIL    (BIT_03)
#define AFF_DETECT_INVIS   (BIT_04)
#define AFF_DETECT_MAGIC   (BIT_05)
#define AFF_DETECT_HIDDEN  (BIT_06)
#define AFF_DETECT_GOOD    (BIT_07)
#define AFF_SANCTUARY      (BIT_08)
#define AFF_FAERIE_FIRE    (BIT_09)
#define AFF_INFRARED       (BIT_10)
#define AFF_CURSE          (BIT_11)
#define AFF_UNUSED_FLAG_1  (BIT_12) /* old: flaming */
#define AFF_POISON         (BIT_13)
#define AFF_PROTECT_EVIL   (BIT_14)
#define AFF_PROTECT_GOOD   (BIT_15)
#define AFF_SNEAK          (BIT_16)
#define AFF_HIDE           (BIT_17)
#define AFF_SLEEP          (BIT_18)
#define AFF_CHARM          (BIT_19)
#define AFF_FLYING         (BIT_20)
#define AFF_PASS_DOOR      (BIT_21)
#define AFF_HASTE          (BIT_22)
#define AFF_CALM           (BIT_23)
#define AFF_PLAGUE         (BIT_24)
#define AFF_WEAKEN         (BIT_25)
#define AFF_DARK_VISION    (BIT_26)
#define AFF_BERSERK        (BIT_27)
#define AFF_SWIM           (BIT_28)
#define AFF_REGENERATION   (BIT_29)
#define AFF_SLOW           (BIT_30)

/* Extra flags.
 * Used in #OBJECTS. */
#define ITEM_GLOW          (BIT_01)
#define ITEM_HUM           (BIT_02)
#define ITEM_DARK          (BIT_03)
#define ITEM_LOCK          (BIT_04)
#define ITEM_EVIL          (BIT_05)
#define ITEM_INVIS         (BIT_06)
#define ITEM_MAGIC         (BIT_07)
#define ITEM_NODROP        (BIT_08)
#define ITEM_BLESS         (BIT_09)
#define ITEM_ANTI_GOOD     (BIT_10)
#define ITEM_ANTI_EVIL     (BIT_11)
#define ITEM_ANTI_NEUTRAL  (BIT_12)
#define ITEM_NOREMOVE      (BIT_13)
#define ITEM_INVENTORY     (BIT_14)
#define ITEM_NOPURGE       (BIT_15)
#define ITEM_ROT_DEATH     (BIT_16)
#define ITEM_VIS_DEATH     (BIT_17)
#define ITEM_UNUSED_FLAG_1 (BIT_18)
#define ITEM_NONMETAL      (BIT_19)
#define ITEM_NOLOCATE      (BIT_20)
#define ITEM_MELT_DROP     (BIT_21)
#define ITEM_HAD_TIMER     (BIT_22)
#define ITEM_SELL_EXTRACT  (BIT_23)
#define ITEM_UNUSED_FLAG_2 (BIT_24)
#define ITEM_BURN_PROOF    (BIT_25)
#define ITEM_NOUNCURSE     (BIT_26)
#define ITEM_CORRODED      (BIT_27)

/* Wear flags.
 * Used in #OBJECTS. */
#define ITEM_TAKE         (BIT_01)
#define ITEM_WEAR_FINGER  (BIT_02)
#define ITEM_WEAR_NECK    (BIT_03)
#define ITEM_WEAR_BODY    (BIT_04)
#define ITEM_WEAR_HEAD    (BIT_05)
#define ITEM_WEAR_LEGS    (BIT_06)
#define ITEM_WEAR_FEET    (BIT_07)
#define ITEM_WEAR_HANDS   (BIT_08)
#define ITEM_WEAR_ARMS    (BIT_09)
#define ITEM_WEAR_SHIELD  (BIT_10)
#define ITEM_WEAR_ABOUT   (BIT_11)
#define ITEM_WEAR_WAIST   (BIT_12)
#define ITEM_WEAR_WRIST   (BIT_13)
#define ITEM_WIELD        (BIT_14)
#define ITEM_HOLD         (BIT_15)
#define ITEM_NO_SAC       (BIT_16)
#define ITEM_WEAR_FLOAT   (BIT_17)
#define ITEM_WEAR_LIGHT   (BIT_18)

/* weapon types */
#define WEAPON_FLAMING    (BIT_01)
#define WEAPON_FROST      (BIT_02)
#define WEAPON_VAMPIRIC   (BIT_03)
#define WEAPON_SHARP      (BIT_04)
#define WEAPON_VORPAL     (BIT_05)
#define WEAPON_TWO_HANDS  (BIT_06)
#define WEAPON_SHOCKING   (BIT_07)
#define WEAPON_POISON     (BIT_08)

/* gate flags */
#define GATE_NORMAL_EXIT  (BIT_01)
#define GATE_NOCURSE      (BIT_02)
#define GATE_GOWITH       (BIT_03)
#define GATE_BUGGY        (BIT_04)
#define GATE_RANDOM       (BIT_05)

/* furniture flags */
#define STAND_AT    (BIT_01)
#define STAND_ON    (BIT_02)
#define STAND_IN    (BIT_03)
#define SIT_AT      (BIT_04)
#define SIT_ON      (BIT_05)
#define SIT_IN      (BIT_06)
#define REST_AT     (BIT_07)
#define REST_ON     (BIT_08)
#define REST_IN     (BIT_09)
#define SLEEP_AT    (BIT_10)
#define SLEEP_ON    (BIT_11)
#define SLEEP_IN    (BIT_12)
#define PUT_AT      (BIT_13)
#define PUT_ON      (BIT_14)
#define PUT_IN      (BIT_15)
#define PUT_INSIDE  (BIT_16)

/* bits groups for furniture. */
#define STAND_BITS  (STAND_AT | STAND_ON | STAND_IN)
#define SIT_BITS    (SIT_AT   | SIT_ON   | SIT_IN)
#define REST_BITS   (REST_AT  | REST_ON  | REST_IN)
#define SLEEP_BITS  (SLEEP_AT | SLEEP_ON | SLEEP_IN)

/* Values for containers (value[1]).
 * Used in #OBJECTS. */
#define CONT_CLOSEABLE   (BIT_01)
#define CONT_PICKPROOF   (BIT_02)
#define CONT_CLOSED      (BIT_03)
#define CONT_LOCKED      (BIT_04)
#define CONT_PUT_ON      (BIT_05)

/* Room flags.
 * Used in #ROOMS. */
#define ROOM_DARK           (BIT_01)
#define ROOM_UNUSED_FLAG_1  (BIT_02) /* old: death */
#define ROOM_NO_MOB         (BIT_03)
#define ROOM_INDOORS        (BIT_04)
#define ROOM_UNUSED_FLAG_2  (BIT_05) /* old: lawful */
#define ROOM_UNUSED_FLAG_3  (BIT_06) /* old: neutral */
#define ROOM_UNUSED_FLAG_4  (BIT_07) /* old: chaotic */
#define ROOM_UNUSED_FLAG_5  (BIT_08) /* old: no_magic */
#define ROOM_UNUSED_FLAG_6  (BIT_09) /* old: tunnel */

#define ROOM_PRIVATE        (BIT_10)
#define ROOM_SAFE           (BIT_11)
#define ROOM_SOLITARY       (BIT_12)
#define ROOM_PET_SHOP       (BIT_13)
#define ROOM_NO_RECALL      (BIT_14)
#define ROOM_IMP_ONLY       (BIT_15)
#define ROOM_GODS_ONLY      (BIT_16)
#define ROOM_HEROES_ONLY    (BIT_17)
#define ROOM_NEWBIES_ONLY   (BIT_18)
#define ROOM_LAW            (BIT_19)
#define ROOM_NOWHERE        (BIT_20)

/* Exit flags.
 * Used in #ROOMS. */
#define EX_ISDOOR           (BIT_01)
#define EX_CLOSED           (BIT_02)
#define EX_LOCKED           (BIT_03)
#define EX_UNUSED_FLAG_1    (BIT_04) /* old: rsclosed */
#define EX_UNUSED_FLAG_2    (BIT_05) /* old: rslocked */
#define EX_PICKPROOF        (BIT_06)
#define EX_NOPASS           (BIT_07)
#define EX_EASY             (BIT_08)
#define EX_HARD             (BIT_09)
#define EX_INFURIATING      (BIT_10)
#define EX_NOCLOSE          (BIT_11)
#define EX_NOLOCK           (BIT_12)

/* ACT bits for players. */
#define PLR_IS_NPC        (BIT_01) /* Don't EVER set.    */

/* RT auto flags */
#define PLR_UNUSED_FLAG_1 (BIT_02) /* old: bought_pet */
#define PLR_AUTOASSIST    (BIT_03)
#define PLR_AUTOEXIT      (BIT_04)
#define PLR_AUTOLOOT      (BIT_05)
#define PLR_AUTOSAC       (BIT_06)
#define PLR_AUTOGOLD      (BIT_07)
#define PLR_AUTOSPLIT     (BIT_08)

#define PLR_UNUSED_FLAG_2 (BIT_09)
#define PLR_UNUSED_FLAG_3 (BIT_10)
#define PLR_UNUSED_FLAG_4 (BIT_11)
#define PLR_UNUSED_FLAG_5 (BIT_12)
#define PLR_UNUSED_FLAG_6 (BIT_13)

/* RT personal flags */
#define PLR_HOLYLIGHT     (BIT_14)
#define PLR_UNUSED_FLAG_7 (BIT_15)
#define PLR_CANLOOT       (BIT_16)
#define PLR_NOSUMMON      (BIT_17)
#define PLR_NOFOLLOW      (BIT_18)
#define PLR_UNUSED_FLAG_8 (BIT_19) /* reserved */
#define PLR_COLOUR        (BIT_20)

/* penalty flags */
#define PLR_PERMIT        (BIT_21)
#define PLR_UNUSED_FLAG_9 (BIT_22)
#define PLR_LOG           (BIT_23)
#define PLR_DENY          (BIT_24)
#define PLR_FREEZE        (BIT_25)
#define PLR_THIEF         (BIT_26)
#define PLR_KILLER        (BIT_27)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (BIT_01)
#define COMM_DEAF               (BIT_02)
#define COMM_NOWIZ              (BIT_03)
#define COMM_NOAUCTION          (BIT_04)
#define COMM_NOGOSSIP           (BIT_05)
#define COMM_NOQUESTION         (BIT_06)
#define COMM_NOMUSIC            (BIT_07)
#define COMM_NOCLAN             (BIT_08)
#define COMM_NOQUOTE            (BIT_09)
#define COMM_SHOUTSOFF          (BIT_10)
#define COMM_UNUSED_FLAG_1      (BIT_11)

/* display flags */
#define COMM_COMPACT            (BIT_12)
#define COMM_BRIEF              (BIT_13)
#define COMM_PROMPT             (BIT_14)
#define COMM_COMBINE            (BIT_15)
#define COMM_TELNET_GA          (BIT_16)
#define COMM_SHOW_AFFECTS       (BIT_17)
#define COMM_NOGRATS            (BIT_18)
#define COMM_UNUSED_FLAG_2      (BIT_19)

/* penalties */
#define COMM_NOEMOTE            (BIT_20)
#define COMM_NOSHOUT            (BIT_21)
#define COMM_NOTELL             (BIT_22)
#define COMM_NOCHANNELS         (BIT_23)
#define COMM_UNUSED_FLAG_3      (BIT_24)
#define COMM_SNOOP_PROOF        (BIT_25)
#define COMM_AFK                (BIT_26)

/* BaseMUD flags */
#define COMM_MATERIALS          (BIT_27)

/* WIZnet flags */
#define WIZ_ON                  (BIT_01)
#define WIZ_TICKS               (BIT_02)
#define WIZ_LOGINS              (BIT_03)
#define WIZ_SITES               (BIT_04)
#define WIZ_LINKS               (BIT_05)
#define WIZ_DEATHS              (BIT_06)
#define WIZ_RESETS              (BIT_07)
#define WIZ_MOBDEATHS           (BIT_08)
#define WIZ_FLAGS               (BIT_09)
#define WIZ_PENALTIES           (BIT_10)
#define WIZ_SACCING             (BIT_11)
#define WIZ_LEVELS              (BIT_12)
#define WIZ_SECURE              (BIT_13)
#define WIZ_SWITCHES            (BIT_14)
#define WIZ_SNOOPS              (BIT_15)
#define WIZ_RESTORE             (BIT_16)
#define WIZ_LOAD                (BIT_17)
#define WIZ_NEWBIE              (BIT_18)
#define WIZ_PREFIX              (BIT_19)
#define WIZ_SPAM                (BIT_20)

/* memory settings */
#define MEM_CUSTOMER  (BIT_01)
#define MEM_SELLER    (BIT_02)
#define MEM_HOSTILE   (BIT_03)
#define MEM_AFRAID    (BIT_04)

/* MOBprog definitions */
#define TRIG_ACT      (BIT_01)
#define TRIG_BRIBE    (BIT_02)
#define TRIG_DEATH    (BIT_03)
#define TRIG_ENTRY    (BIT_04)
#define TRIG_FIGHT    (BIT_05)
#define TRIG_GIVE     (BIT_06)
#define TRIG_GREET    (BIT_07)
#define TRIG_GRALL    (BIT_08)
#define TRIG_KILL     (BIT_09)
#define TRIG_HPCNT    (BIT_10)
#define TRIG_RANDOM   (BIT_11)
#define TRIG_SPEECH   (BIT_12)
#define TRIG_EXIT     (BIT_13)
#define TRIG_EXALL    (BIT_14)
#define TRIG_DELAY    (BIT_15)
#define TRIG_SURR     (BIT_16)

/* Area flags. */
#define AREA_CHANGED  (BIT_01) /* Area has been modified. */
#define AREA_ADDED    (BIT_02) /* Area has been added to. */
#define AREA_LOADING  (BIT_03) /* Used for counting in db.c */

/* Damage flags. */
#define DAM_MAGICAL   (BIT_01)

/* TO types for act. */
#define TO_CHAR    (BIT_01) /* 'ch' only */
#define TO_VICT    (BIT_02) /* 'vch' only */
#define TO_OTHERS  (BIT_03) /* everyone else */
#define TO_NOTCHAR (TO_VICT | TO_OTHERS)
#define TO_ALL     (TO_CHAR | TO_VICT | TO_OTHERS)

/* Flag tables. */
extern const FLAG_T plr_flags[];
extern const FLAG_T affect_flags[];
extern const FLAG_T off_flags[];
extern const FLAG_T form_flags[];
extern const FLAG_T part_flags[];
extern const FLAG_T comm_flags[];
extern const FLAG_T mprog_flags[];
extern const FLAG_T area_flags[];
extern const FLAG_T exit_flags[];
extern const FLAG_T room_flags[];
extern const FLAG_T extra_flags[];
extern const FLAG_T wear_flags[];
extern const FLAG_T container_flags[];
extern const FLAG_T weapon_flags[];
extern const FLAG_T res_flags[];
extern const FLAG_T gate_flags[];
extern const FLAG_T furniture_flags[];
extern const FLAG_T dam_flags[];

/* Function prototypes for flag management. */
flag_t flag_lookup (const FLAG_T *flag_table, const char *name);
flag_t flag_lookup_exact (const FLAG_T *flag_table, const char *name);
flag_t flag_lookup_real (const FLAG_T *flag_table, const char *name,
    bool exact);
const FLAG_T *flag_get_by_name (const FLAG_T *flag_table, const char *name);
const FLAG_T *flag_get_by_name_exact (const FLAG_T *flag_table,
    const char *name);
const FLAG_T *flag_get (const FLAG_T *flag_table, flag_t bit);
const char *flag_get_name (const FLAG_T *flag_table, flag_t bit);

flag_t flags_from_string (const FLAG_T *flag_table, const char *name);
flag_t flags_from_string_exact (const FLAG_T *flag_table, const char *name);
flag_t flags_from_string_real (const FLAG_T *flag_table, const char *name,
    bool exact);

const char *flags_to_string (const FLAG_T *flag_table, flag_t bits);
const char *flags_to_string_real (const FLAG_T *flag_table, flag_t bits,
    const char *none_str);

#endif
