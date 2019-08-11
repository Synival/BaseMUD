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

/* Site ban structure.  */
#define BAN_SUFFIX     (A)
#define BAN_PREFIX     (B)
#define BAN_NEWBIES    (C)
#define BAN_ALL        (D)
#define BAN_PERMIT     (E)
#define BAN_PERMANENT  (F)

/* RT ASCII conversions -- used so we can have letters in this file */
#define A            1
#define B            2
#define C            4
#define D            8
#define E           16
#define F           32
#define G           64
#define H          128

#define I          256
#define J          512
#define K         1024
#define L         2048
#define M         4096
#define N         8192
#define O        16384
#define P        32768

#define Q        65536
#define R       131072
#define S       262144
#define T       524288
#define U      1048576
#define V      2097152
#define W      4194304
#define X      8388608

#define Y     16777216
#define Z     33554432
#define aa    67108864  /* doubled due to conflicts */
#define bb   134217728
#define cc   268435456
#define dd   536870912
#define ee  1073741824

/* ACT bits for mobs.
 * Used in #MOBILES. */
#define MOB_IS_NPC         (A)  /* Auto set for mobs  */
#define MOB_SENTINEL       (B)  /* Stays in one room  */
#define MOB_SCAVENGER      (C)  /* Picks up objects   */
#define MOB_UNUSED_FLAG_1  (D)  /* old: isnpc */
#define MOB_UNUSED_FLAG_2  (E)  /* old: nice_thief */
#define MOB_AGGRESSIVE     (F)  /* Attacks PC's       */
#define MOB_STAY_AREA      (G)  /* Won't leave area   */
#define MOB_WIMPY          (H)
#define MOB_PET            (I)  /* Auto set for pets  */
#define MOB_TRAIN          (J)  /* Can train PC's     */
#define MOB_PRACTICE       (K)  /* Can practice PC's  */
#define MOB_UNUSED_FLAG_3  (L)
#define MOB_UNUSED_FLAG_4  (M)
#define MOB_UNUSED_FLAG_5  (N)
#define MOB_UNDEAD         (O)
#define MOB_UNUSED_FLAG_6  (P)
#define MOB_CLERIC         (Q)
#define MOB_MAGE           (R)
#define MOB_THIEF          (S)
#define MOB_WARRIOR        (T)
#define MOB_NOALIGN        (U)
#define MOB_NOPURGE        (V)
#define MOB_OUTDOORS       (W)
#define MOB_UNUSED_FLAG_7  (X)
#define MOB_INDOORS        (Y)
#define MOB_UNUSED_FLAG_8  (Z)
#define MOB_IS_HEALER      (aa)
#define MOB_GAIN           (bb)
#define MOB_UPDATE_ALWAYS  (cc)
#define MOB_IS_CHANGER     (dd)

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK    (A)
#define OFF_BACKSTAB       (B)
#define OFF_BASH           (C)
#define OFF_BERSERK        (D)
#define OFF_DISARM         (E)
#define OFF_DODGE          (F)
#define OFF_FADE           (G)
#define OFF_FAST           (H)
#define OFF_KICK           (I)
#define OFF_KICK_DIRT      (J)
#define OFF_PARRY          (K)
#define OFF_RESCUE         (L)
#define OFF_TAIL           (M)
#define OFF_TRIP           (N)
#define OFF_CRUSH          (O)
#define ASSIST_ALL         (P)
#define ASSIST_ALIGN       (Q)
#define ASSIST_RACE        (R)
#define ASSIST_PLAYERS     (S)
#define ASSIST_GUARD       (T)
#define ASSIST_VNUM        (U)

/* IMM/RES/VULN bits for mobs */
#define RES_SUMMON         (A)
#define RES_CHARM          (B)
#define RES_MAGIC          (C)
#define RES_WEAPON         (D)
#define RES_BASH           (E)
#define RES_PIERCE         (F)
#define RES_SLASH          (G)
#define RES_FIRE           (H)
#define RES_COLD           (I)
#define RES_LIGHTNING      (J)
#define RES_ACID           (K)
#define RES_POISON         (L)
#define RES_NEGATIVE       (M)
#define RES_HOLY           (N)
#define RES_ENERGY         (O)
#define RES_MENTAL         (P)
#define RES_DISEASE        (Q)
#define RES_DROWNING       (R)
#define RES_LIGHT          (S)
#define RES_SOUND          (T)
#define RES_UNUSED_FLAG_1  (U)
#define RES_UNUSED_FLAG_2  (V)
#define RES_UNUSED_FLAG_3  (W)
#define RES_WOOD           (X)
#define RES_SILVER         (Y)
#define RES_IRON           (Z)

/* body form */
#define FORM_EDIBLE        (A)
#define FORM_POISON        (B)
#define FORM_MAGICAL       (C)
#define FORM_INSTANT_DECAY (D)
#define FORM_OTHER         (E) /* defined by material bit */
#define FORM_UNUSED_FLAG_1 (F)

/* actual form */
#define FORM_ANIMAL        (G)
#define FORM_SENTIENT      (H)
#define FORM_UNDEAD        (I)
#define FORM_CONSTRUCT     (J)
#define FORM_MIST          (K)
#define FORM_INTANGIBLE    (L)

#define FORM_BIPED         (M)
#define FORM_CENTAUR       (N)
#define FORM_INSECT        (O)
#define FORM_SPIDER        (P)
#define FORM_CRUSTACEAN    (Q)
#define FORM_WORM          (R)
#define FORM_BLOB          (S)
#define FORM_UNUSED_FLAG_2 (T)
#define FORM_UNUSED_FLAG_3 (U)

#define FORM_MAMMAL        (V)
#define FORM_BIRD          (W)
#define FORM_REPTILE       (X)
#define FORM_SNAKE         (Y)
#define FORM_DRAGON        (Z)
#define FORM_AMPHIBIAN     (aa)
#define FORM_FISH          (bb)
#define FORM_COLD_BLOOD    (cc)

/* body parts */
#define PART_HEAD          (A)
#define PART_ARMS          (B)
#define PART_LEGS          (C)
#define PART_HEART         (D)
#define PART_BRAINS        (E)
#define PART_GUTS          (F)
#define PART_HANDS         (G)
#define PART_FEET          (H)
#define PART_FINGERS       (I)
#define PART_EAR           (J)
#define PART_EYE           (K)
#define PART_LONG_TONGUE   (L)
#define PART_EYESTALKS     (M)
#define PART_TENTACLES     (N)
#define PART_FINS          (O)
#define PART_WINGS         (P)
#define PART_TAIL          (Q)
#define PART_UNUSED_FLAG_1 (R)
#define PART_UNUSED_FLAG_2 (S)
#define PART_UNUSED_FLAG_3 (T)
#define PART_CLAWS         (U) /* for combat */
#define PART_FANGS         (V)
#define PART_HORNS         (W)
#define PART_SCALES        (X)
#define PART_TUSKS         (Y)

/* Bits for 'affected_by'.
 * Used in #MOBILES.  */
#define AFF_BLIND          (A)
#define AFF_INVISIBLE      (B)
#define AFF_DETECT_EVIL    (C)
#define AFF_DETECT_INVIS   (D)
#define AFF_DETECT_MAGIC   (E)
#define AFF_DETECT_HIDDEN  (F)
#define AFF_DETECT_GOOD    (G)
#define AFF_SANCTUARY      (H)
#define AFF_FAERIE_FIRE    (I)
#define AFF_INFRARED       (J)
#define AFF_CURSE          (K)
#define AFF_UNUSED_FLAG_1  (L) /* old: flaming */
#define AFF_POISON         (M)
#define AFF_PROTECT_EVIL   (N)
#define AFF_PROTECT_GOOD   (O)
#define AFF_SNEAK          (P)
#define AFF_HIDE           (Q)
#define AFF_SLEEP          (R)
#define AFF_CHARM          (S)
#define AFF_FLYING         (T)
#define AFF_PASS_DOOR      (U)
#define AFF_HASTE          (V)
#define AFF_CALM           (W)
#define AFF_PLAGUE         (X)
#define AFF_WEAKEN         (Y)
#define AFF_DARK_VISION    (Z)
#define AFF_BERSERK        (aa)
#define AFF_SWIM           (bb)
#define AFF_REGENERATION   (cc)
#define AFF_SLOW           (dd)

/* Extra flags.
 * Used in #OBJECTS. */
#define ITEM_GLOW          (A)
#define ITEM_HUM           (B)
#define ITEM_DARK          (C)
#define ITEM_LOCK          (D)
#define ITEM_EVIL          (E)
#define ITEM_INVIS         (F)
#define ITEM_MAGIC         (G)
#define ITEM_NODROP        (H)
#define ITEM_BLESS         (I)
#define ITEM_ANTI_GOOD     (J)
#define ITEM_ANTI_EVIL     (K)
#define ITEM_ANTI_NEUTRAL  (L)
#define ITEM_NOREMOVE      (M)
#define ITEM_INVENTORY     (N)
#define ITEM_NOPURGE       (O)
#define ITEM_ROT_DEATH     (P)
#define ITEM_VIS_DEATH     (Q)
#define ITEM_UNUSED_FLAG_1 (R)
#define ITEM_NONMETAL      (S)
#define ITEM_NOLOCATE      (T)
#define ITEM_MELT_DROP     (U)
#define ITEM_HAD_TIMER     (V)
#define ITEM_SELL_EXTRACT  (W)
#define ITEM_UNUSED_FLAG_2 (X)
#define ITEM_BURN_PROOF    (Y)
#define ITEM_NOUNCURSE     (Z)
#define ITEM_CORRODED      (aa)

/* Wear flags.
 * Used in #OBJECTS. */
#define ITEM_TAKE         (A)
#define ITEM_WEAR_FINGER  (B)
#define ITEM_WEAR_NECK    (C)
#define ITEM_WEAR_BODY    (D)
#define ITEM_WEAR_HEAD    (E)
#define ITEM_WEAR_LEGS    (F)
#define ITEM_WEAR_FEET    (G)
#define ITEM_WEAR_HANDS   (H)
#define ITEM_WEAR_ARMS    (I)
#define ITEM_WEAR_SHIELD  (J)
#define ITEM_WEAR_ABOUT   (K)
#define ITEM_WEAR_WAIST   (L)
#define ITEM_WEAR_WRIST   (M)
#define ITEM_WIELD        (N)
#define ITEM_HOLD         (O)
#define ITEM_NO_SAC       (P)
#define ITEM_WEAR_FLOAT   (Q)

/* weapon types */
#define WEAPON_FLAMING    (A)
#define WEAPON_FROST      (B)
#define WEAPON_VAMPIRIC   (C)
#define WEAPON_SHARP      (D)
#define WEAPON_VORPAL     (E)
#define WEAPON_TWO_HANDS  (F)
#define WEAPON_SHOCKING   (G)
#define WEAPON_POISON     (H)

/* gate flags */
#define GATE_NORMAL_EXIT  (A)
#define GATE_NOCURSE      (B)
#define GATE_GOWITH       (C)
#define GATE_BUGGY        (D)
#define GATE_RANDOM       (E)

/* furniture flags */
#define STAND_AT    (A)
#define STAND_ON    (B)
#define STAND_IN    (C)
#define SIT_AT      (D)
#define SIT_ON      (E)
#define SIT_IN      (F)
#define REST_AT     (G)
#define REST_ON     (H)
#define REST_IN     (I)
#define SLEEP_AT    (J)
#define SLEEP_ON    (K)
#define SLEEP_IN    (L)
#define PUT_AT      (M)
#define PUT_ON      (N)
#define PUT_IN      (O)
#define PUT_INSIDE  (P)

/* Values for containers (value[1]).
 * Used in #OBJECTS. */
#define CONT_CLOSEABLE   (A)
#define CONT_PICKPROOF   (B)
#define CONT_CLOSED      (C)
#define CONT_LOCKED      (D)
#define CONT_PUT_ON      (E)

/* Room flags.
 * Used in #ROOMS. */
#define ROOM_DARK           (A)
#define ROOM_UNUSED_FLAG_1  (B) /* old: death */
#define ROOM_NO_MOB         (C)
#define ROOM_INDOORS        (D)
#define ROOM_UNUSED_FLAG_2  (E) /* old: lawful */
#define ROOM_UNUSED_FLAG_3  (F) /* old: neutral */
#define ROOM_UNUSED_FLAG_4  (G) /* old: chaotic */
#define ROOM_UNUSED_FLAG_5  (H) /* old: no_magic */
#define ROOM_UNUSED_FLAG_6  (I) /* old: tunnel */

#define ROOM_PRIVATE        (J)
#define ROOM_SAFE           (K)
#define ROOM_SOLITARY       (L)
#define ROOM_PET_SHOP       (M)
#define ROOM_NO_RECALL      (N)
#define ROOM_IMP_ONLY       (O)
#define ROOM_GODS_ONLY      (P)
#define ROOM_HEROES_ONLY    (Q)
#define ROOM_NEWBIES_ONLY   (R)
#define ROOM_LAW            (S)
#define ROOM_NOWHERE        (T)

/* Exit flags.
 * Used in #ROOMS. */
#define EX_ISDOOR           (A)
#define EX_CLOSED           (B)
#define EX_LOCKED           (C)
#define EX_UNUSED_FLAG_1    (D) /* old: rsclosed */
#define EX_UNUSED_FLAG_2    (E) /* old: rslocked */
#define EX_PICKPROOF        (F)
#define EX_NOPASS           (G)
#define EX_EASY             (H)
#define EX_HARD             (I)
#define EX_INFURIATING      (J)
#define EX_NOCLOSE          (K)
#define EX_NOLOCK           (L)

/* ACT bits for players. */
#define PLR_IS_NPC        (A) /* Don't EVER set.    */

/* RT auto flags */
#define PLR_UNUSED_FLAG_1 (B) /* old: bought_pet */
#define PLR_AUTOASSIST    (C)
#define PLR_AUTOEXIT      (D)
#define PLR_AUTOLOOT      (E)
#define PLR_AUTOSAC       (F)
#define PLR_AUTOGOLD      (G)
#define PLR_AUTOSPLIT     (H)

#define PLR_UNUSED_FLAG_2 (I)
#define PLR_UNUSED_FLAG_3 (J)
#define PLR_UNUSED_FLAG_4 (K)
#define PLR_UNUSED_FLAG_5 (L)
#define PLR_UNUSED_FLAG_6 (M)

/* RT personal flags */
#define PLR_HOLYLIGHT     (N)
#define PLR_UNUSED_FLAG_7 (O)
#define PLR_CANLOOT       (P)
#define PLR_NOSUMMON      (Q)
#define PLR_NOFOLLOW      (R)
#define PLR_UNUSED_FLAG_8 (S) /* reserved */
#define PLR_COLOUR        (T)

/* penalty flags */
#define PLR_PERMIT        (U)
#define PLR_UNUSED_FLAG_9 (V)
#define PLR_LOG           (W)
#define PLR_DENY          (X)
#define PLR_FREEZE        (Y)
#define PLR_THIEF         (Z)
#define PLR_KILLER        (aa)

/* RT comm flags -- may be used on both mobs and chars */
#define COMM_QUIET              (A)
#define COMM_DEAF               (B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
#define COMM_NOGOSSIP           (E)
#define COMM_NOQUESTION         (F)
#define COMM_NOMUSIC            (G)
#define COMM_NOCLAN             (H)
#define COMM_NOQUOTE            (I)
#define COMM_SHOUTSOFF          (J)
#define COMM_UNUSED_FLAG_1      (K)

/* display flags */
#define COMM_COMPACT            (L)
#define COMM_BRIEF              (M)
#define COMM_PROMPT             (N)
#define COMM_COMBINE            (O)
#define COMM_TELNET_GA          (P)
#define COMM_SHOW_AFFECTS       (Q)
#define COMM_NOGRATS            (R)
#define COMM_UNUSED_FLAG_2      (S)

/* penalties */
#define COMM_NOEMOTE            (T)
#define COMM_NOSHOUT            (U)
#define COMM_NOTELL             (V)
#define COMM_NOCHANNELS         (W)
#define COMM_UNUSED_FLAG_3      (X)
#define COMM_SNOOP_PROOF        (Y)
#define COMM_AFK                (Z)

/* BaseMUD flags */
#define COMM_MATERIALS          (aa)

/* WIZnet flags */
#define WIZ_ON                  (A)
#define WIZ_TICKS               (B)
#define WIZ_LOGINS              (C)
#define WIZ_SITES               (D)
#define WIZ_LINKS               (E)
#define WIZ_DEATHS              (F)
#define WIZ_RESETS              (G)
#define WIZ_MOBDEATHS           (H)
#define WIZ_FLAGS               (I)
#define WIZ_PENALTIES           (J)
#define WIZ_SACCING             (K)
#define WIZ_LEVELS              (L)
#define WIZ_SECURE              (M)
#define WIZ_SWITCHES            (N)
#define WIZ_SNOOPS              (O)
#define WIZ_RESTORE             (P)
#define WIZ_LOAD                (Q)
#define WIZ_NEWBIE              (R)
#define WIZ_PREFIX              (S)
#define WIZ_SPAM                (T)

/* memory settings */
#define MEM_CUSTOMER  (A)
#define MEM_SELLER    (B)
#define MEM_HOSTILE   (C)
#define MEM_AFRAID    (D)

/* MOBprog definitions */
#define TRIG_ACT      (A)
#define TRIG_BRIBE    (B)
#define TRIG_DEATH    (C)
#define TRIG_ENTRY    (D)
#define TRIG_FIGHT    (E)
#define TRIG_GIVE     (F)
#define TRIG_GREET    (G)
#define TRIG_GRALL    (H)
#define TRIG_KILL     (I)
#define TRIG_HPCNT    (J)
#define TRIG_RANDOM   (K)
#define TRIG_SPEECH   (L)
#define TRIG_EXIT     (M)
#define TRIG_EXALL    (N)
#define TRIG_DELAY    (O)
#define TRIG_SURR     (P)

/* Area flags. */
#define AREA_CHANGED  (A) /* Area has been modified. */
#define AREA_ADDED    (B) /* Area has been added to. */
#define AREA_LOADING  (C) /* Used for counting in db.c */

/* Flag tables. */
extern const FLAG_TYPE mob_flags[];
extern const FLAG_TYPE plr_flags[];
extern const FLAG_TYPE affect_flags[];
extern const FLAG_TYPE off_flags[];
extern const FLAG_TYPE form_flags[];
extern const FLAG_TYPE part_flags[];
extern const FLAG_TYPE comm_flags[];
extern const FLAG_TYPE mprog_flags[];
extern const FLAG_TYPE area_flags[];
extern const FLAG_TYPE exit_flags[];
extern const FLAG_TYPE door_resets[];
extern const FLAG_TYPE room_flags[];
extern const FLAG_TYPE extra_flags[];
extern const FLAG_TYPE wear_flags[];
extern const FLAG_TYPE container_flags[];
extern const FLAG_TYPE weapon_flags[];
extern const FLAG_TYPE res_flags[];
extern const FLAG_TYPE portal_flags[];
extern const FLAG_TYPE furniture_flags[];

#endif
