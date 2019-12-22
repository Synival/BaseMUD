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

#include "merc.h"

const FLAG_T sex_types[SEX_MAX + 1] = {
    {"neutral", SEX_NEUTRAL, TRUE},
    {"male",    SEX_MALE,    TRUE},
    {"female",  SEX_FEMALE,  TRUE},
    {"random",  SEX_EITHER,  TRUE},
    {0}
};

const FLAG_T affect_apply_types[APPLY_MAX + 1] = {
    {"none",                  APPLY_NONE,          TRUE},
    {"strength",              APPLY_STR,           TRUE},
    {"dexterity",             APPLY_DEX,           TRUE},
    {"intelligence",          APPLY_INT,           TRUE},
    {"wisdom",                APPLY_WIS,           TRUE},
    {"constitution",          APPLY_CON,           TRUE},
    {"sex",                   APPLY_SEX,           TRUE},
    {"class",                 APPLY_CLASS,         TRUE},
    {"level",                 APPLY_LEVEL,         TRUE},
    {"age",                   APPLY_AGE,           TRUE},
    {"height",                APPLY_HEIGHT,        TRUE},
    {"weight",                APPLY_WEIGHT,        TRUE},
    {"mana",                  APPLY_MANA,          TRUE},
    {"hp",                    APPLY_HIT,           TRUE},
    {"moves",                 APPLY_MOVE,          TRUE},
    {"gold",                  APPLY_GOLD,          TRUE},
    {"experience",            APPLY_EXP,           TRUE},
    {"armor class",           APPLY_AC,            TRUE},
    {"hit roll",              APPLY_HITROLL,       TRUE},
    {"damage roll",           APPLY_DAMROLL,       TRUE},
    {"saves",                 APPLY_SAVES,         TRUE},
    {"save vs rod",           APPLY_SAVING_ROD,    TRUE},
    {"save vs petrification", APPLY_SAVING_PETRI,  TRUE},
    {"save vs breath",        APPLY_SAVING_BREATH, TRUE},
    {"save vs spell",         APPLY_SAVING_SPELL,  TRUE},
    {"spell affect",          APPLY_SPELL_AFFECT,  FALSE},
    {0}
};

const FLAG_T ac_types[AC_MAX + 1] = {
    {"pierce", AC_PIERCE, TRUE},
    {"bash",   AC_BASH,   TRUE},
    {"slash",  AC_SLASH,  TRUE},
    {"magic",  AC_EXOTIC, TRUE},
    {0}
};

const FLAG_T size_types[SIZE_MAX_R + 1] = {
    {"tiny",   SIZE_TINY,   TRUE},
    {"small",  SIZE_SMALL,  TRUE},
    {"medium", SIZE_MEDIUM, TRUE},
    {"large",  SIZE_LARGE,  TRUE},
    {"huge",   SIZE_HUGE,   TRUE},
    {"giant",  SIZE_GIANT,  TRUE},
    {0}
};

const FLAG_T weapon_types[WEAPON_MAX + 1] = {
    {"exotic",  WEAPON_EXOTIC,  TRUE},
    {"sword",   WEAPON_SWORD,   TRUE},
    {"dagger",  WEAPON_DAGGER,  TRUE},
    {"spear",   WEAPON_SPEAR,   TRUE},
    {"mace",    WEAPON_MACE,    TRUE},
    {"axe",     WEAPON_AXE,     TRUE},
    {"flail",   WEAPON_FLAIL,   TRUE},
    {"whip",    WEAPON_WHIP,    TRUE},
    {"polearm", WEAPON_POLEARM, TRUE},
    {0}
};

const FLAG_T position_types[POS_MAX + 1] = {
    {"dead",     POS_DEAD,     FALSE},
    {"mortal",   POS_MORTAL,   FALSE},
    {"incap",    POS_INCAP,    FALSE},
    {"stunned",  POS_STUNNED,  FALSE},
    {"sleeping", POS_SLEEPING, TRUE},
    {"resting",  POS_RESTING,  TRUE},
    {"sitting",  POS_SITTING,  TRUE},
    {"fighting", POS_FIGHTING, FALSE},
    {"standing", POS_STANDING, TRUE},
    {0}
};

const FLAG_T sector_types[SECT_MAX + 1] = {
    {"inside",   SECT_INSIDE,       TRUE},
    {"city",     SECT_CITY,         TRUE},
    {"field",    SECT_FIELD,        TRUE},
    {"forest",   SECT_FOREST,       TRUE},
    {"hills",    SECT_HILLS,        TRUE},
    {"mountain", SECT_MOUNTAIN,     TRUE},
    {"swim",     SECT_WATER_SWIM,   TRUE},
    {"noswim",   SECT_WATER_NOSWIM, TRUE},
    {"unused",   SECT_UNUSED,       FALSE},
    {"air",      SECT_AIR,          TRUE},
    {"desert",   SECT_DESERT,       TRUE},
    {0}
};

const FLAG_T item_types[ITEM_MAX + 1] = {
    {"none",       ITEM_NONE,       FALSE},
    {"light",      ITEM_LIGHT,      TRUE},
    {"scroll",     ITEM_SCROLL,     TRUE},
    {"wand",       ITEM_WAND,       TRUE},
    {"staff",      ITEM_STAFF,      TRUE},
    {"weapon",     ITEM_WEAPON,     TRUE},
    {"unused_item_1",ITEM_UNUSED_1, FALSE},
    {"unused_item_2",ITEM_UNUSED_2, FALSE},
    {"treasure",   ITEM_TREASURE,   TRUE},
    {"armor",      ITEM_ARMOR,      TRUE},
    {"potion",     ITEM_POTION,     TRUE},
    {"clothing",   ITEM_CLOTHING,   TRUE},
    {"furniture",  ITEM_FURNITURE,  TRUE},
    {"trash",      ITEM_TRASH,      TRUE},
    {"unused_item_3",ITEM_UNUSED_3, FALSE},
    {"container",  ITEM_CONTAINER,  TRUE},
    {"unused_item_4",ITEM_UNUSED_4, FALSE},
    {"drink",      ITEM_DRINK_CON,  TRUE},
    {"key",        ITEM_KEY,        TRUE},
    {"food",       ITEM_FOOD,       TRUE},
    {"money",      ITEM_MONEY,      TRUE},
    {"unused_item_5",ITEM_UNUSED_5, FALSE},
    {"boat",       ITEM_BOAT,       TRUE},
    {"npc_corpse", ITEM_CORPSE_NPC, TRUE},
    {"pc_corpse",  ITEM_CORPSE_PC,  TRUE},
    {"fountain",   ITEM_FOUNTAIN,   TRUE},
    {"pill",       ITEM_PILL,       TRUE},
    {"protect",    ITEM_PROTECT,    TRUE},
    {"map",        ITEM_MAP,        TRUE},
    {"portal",     ITEM_PORTAL,     TRUE},
    {"warp_stone", ITEM_WARP_STONE, TRUE},
    {"room_key",   ITEM_ROOM_KEY,   TRUE},
    {"gem",        ITEM_GEM,        TRUE},
    {"jewelry",    ITEM_JEWELRY,    TRUE},
    {"jukebox",    ITEM_JUKEBOX,    TRUE},
    {0}
};

const FLAG_T door_resets[RESET_MAX + 1] = {
    {"open and unlocked",   RESET_OPEN,   TRUE},
    {"closed and unlocked", RESET_CLOSED, TRUE},
    {"closed and locked",   RESET_LOCKED, TRUE},
    {0}
};

const FLAG_T stat_types[STAT_MAX + 1] = {
    {"str", STAT_STR, TRUE},
    {"int", STAT_INT, TRUE},
    {"wis", STAT_WIS, TRUE},
    {"dex", STAT_DEX, TRUE},
    {"con", STAT_CON, TRUE},
    {0}
};

const FLAG_T cond_types[COND_MAX + 1] = {
    {"drunk",  COND_DRUNK,  TRUE},
    {"full",   COND_FULL,   TRUE},
    {"thirst", COND_THIRST, TRUE},
    {"hunger", COND_HUNGER, TRUE},
    {0}
};

const FLAG_T skill_target_types[SKILL_TARGET_MAX + 1] = {
    {"ignore",             SKILL_TARGET_IGNORE,         TRUE},
    {"char_offensive",     SKILL_TARGET_CHAR_OFFENSIVE, TRUE},
    {"char_defensive",     SKILL_TARGET_CHAR_DEFENSIVE, TRUE},
    {"char_self",          SKILL_TARGET_CHAR_SELF,      TRUE},
    {"obj_inventory",      SKILL_TARGET_OBJ_INV,        TRUE},
    {"obj_char_defensive", SKILL_TARGET_OBJ_CHAR_DEF,   TRUE},
    {"obj_char_offensive", SKILL_TARGET_OBJ_CHAR_OFF,   TRUE},
    {0}
};

const FLAG_T board_def_types[DEF_MAX + 1] = {
    {"normal",  DEF_NORMAL,  TRUE},
    {"include", DEF_INCLUDE, TRUE},
    {"exclude", DEF_EXCLUDE, TRUE},
    {0}
};
