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

#include "recycle.h"
#include "lookup.h"
#include "colour.h"
#include "db.h"
#include "utils.h"
#include "recycle.h"
#include "skills.h"
#include "board.h"
#include "chars.h"
#include "objs.h"
#include "globals.h"
#include "memory.h"

#include "save.h"

#if !defined(macintosh)
    extern int _filbuf args ((FILE *));
#endif

/* int rename(const char *oldfname, const char *newfname); viene en stdio.h */

char *print_flags (flag_t flags) {
    int count, pos = 0;
    static char buf[52];

    for (count = 0; count < 32; count++) {
        if (IS_SET (flags, 1 << count)) {
            if (count < 26)
                buf[pos] = 'A' + count;
            else
                buf[pos] = 'a' + (count - 26);
            pos++;
        }
    }

    if (pos == 0) {
        buf[pos] = '0';
        pos++;
    }
    buf[pos] = '\0';

    return buf;
}

/* Array of containers read for proper re-nesting of objects. */
#define MAX_NEST    100
static OBJ_T *obj_nest[MAX_NEST];

/* Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided. */
void save_char_obj (CHAR_T *ch) {
    char strsave[MAX_INPUT_LENGTH];
    FILE *fp;

    if (IS_NPC (ch))
        return;

    /* Fix by Edwin. JR -- 10/15/00
     *
     * Don't save if the character is invalidated.
     * This might happen during the auto-logoff of players.
     * (or other places not yet found out) */
    BAIL_IF_BUG (!IS_VALID (ch),
        "save_char_obj: Trying to save an invalidated character.\n", 0);

    if (ch->desc != NULL && ch->desc->original != NULL)
        ch = ch->desc->original;

#if defined(unix)
    /* create god log */
    if (IS_IMMORTAL (ch) || ch->level >= LEVEL_IMMORTAL) {
        fclose (reserve_file);
        sprintf (strsave, "%s%s", GOD_DIR, capitalize (ch->name));
        if ((fp = fopen (strsave, "w")) == NULL) {
            bug ("save_char_obj: fopen", 0);
            perror (strsave);
        }

        fprintf (fp, "Lev %2d Trust %2d  %s%s\n",
            ch->level, char_get_trust (ch), ch->name, ch->pcdata->title);
        fclose (fp);
        reserve_file = fopen (NULL_FILE, "r");
    }
#endif

    fclose (reserve_file);
    sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (ch->name));
    if ((fp = fopen (TEMP_FILE, "w")) == NULL) {
        bug ("save_char_obj: fopen", 0);
        perror (strsave);
    }
    else {
        fwrite_char (ch, fp);
        if (ch->carrying != NULL)
            fwrite_obj (ch, ch->carrying, fp, 0);
        /* save the pets */
        if (ch->pet != NULL && ch->pet->in_room == ch->in_room)
            fwrite_pet (ch->pet, fp);
        fprintf (fp, "#END\n");
    }
    fclose (fp);
    rename (TEMP_FILE, strsave);
    reserve_file = fopen (NULL_FILE, "r");
}

/* Write the char. */
void fwrite_char (CHAR_T *ch, FILE *fp) {
    AFFECT_T *paf;
    int sn, gn, pos, i;

    fprintf (fp, "#%s\n", IS_NPC (ch) ? "MOB" : "PLAYER");
    fprintf (fp, "Name %s~\n", ch->name);
    fprintf (fp, "Id   %ld\n", ch->id);
    fprintf (fp, "LogO %ld\n", current_time);
    fprintf (fp, "Vers %d\n", 5);
    if (ch->short_descr[0] != '\0')
        fprintf (fp, "ShD  %s~\n", ch->short_descr);
    if (ch->long_descr[0] != '\0')
        fprintf (fp, "LnD  %s~\n", ch->long_descr);
    if (ch->description[0] != '\0')
        fprintf (fp, "Desc %s~\n", ch->description);
    if (ch->prompt != NULL || !str_cmp (ch->prompt, "<%hhp %mm %vmv> ")
        || !str_cmp (ch->prompt, "{c<%hhp %mm %vmv>{x "))
        fprintf (fp, "Prom %s~\n", ch->prompt);
    fprintf (fp, "Race %s~\n", pc_race_table[ch->race].name);
    if (ch->clan)
        fprintf (fp, "Clan %s~\n", clan_table[ch->clan].name);
    fprintf (fp, "Sex  %d\n", ch->sex);
    fprintf (fp, "Cla  %d\n", ch->class);
    fprintf (fp, "Levl %d\n", ch->level);
    if (ch->trust != 0)
        fprintf (fp, "Tru  %d\n", ch->trust);
    fprintf (fp, "Sec  %d\n", ch->pcdata->security);    /* OLC */
    fprintf (fp, "Plyd %d\n", ch->played + (int) (current_time - ch->logon));
    fprintf (fp, "Scro %d\n", ch->lines);
    fprintf (fp, "Room %d\n", (ch->in_room == get_room_index (ROOM_VNUM_LIMBO)
                               && ch->was_in_room != NULL)
             ? ch->was_in_room->vnum
             : ch->in_room == NULL ? ROOM_VNUM_TEMPLE : ch->in_room->vnum);

    fprintf (fp, "HMV  %d %d %d %d %d %d\n",
             ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move,
             ch->max_move);

    if (ch->gold > 0)
        fprintf (fp, "Gold %ld\n", ch->gold);
    else
        fprintf (fp, "Gold 0\n");

    if (ch->silver > 0)
        fprintf (fp, "Silv %ld\n", ch->silver);
    else
        fprintf (fp, "Silv 0\n");

    fprintf (fp, "Exp  %d\n", ch->exp);
    if (ch->mob != 0)
        fprintf (fp, "Mob  %s\n", print_flags (ch->mob));
    if (ch->plr != 0)
        fprintf (fp, "Plr  %s\n", print_flags (ch->plr));
    if (ch->affected_by != 0)
        fprintf (fp, "AfBy %s\n", print_flags (ch->affected_by));
    fprintf (fp, "Comm %s\n", print_flags (ch->comm));
    if (ch->wiznet)
        fprintf (fp, "Wizn %s\n", print_flags (ch->wiznet));
    if (ch->invis_level)
        fprintf (fp, "Invi %d\n", ch->invis_level);
    if (ch->incog_level)
        fprintf (fp, "Inco %d\n", ch->incog_level);
    fprintf (fp, "Pos  %d\n",
             ch->position == POS_FIGHTING ? POS_STANDING : ch->position);
    if (ch->practice != 0)
        fprintf (fp, "Prac %d\n", ch->practice);
    if (ch->train != 0)
        fprintf (fp, "Trai %d\n", ch->train);
    if (ch->saving_throw != 0)
        fprintf (fp, "Save  %d\n", ch->saving_throw);
    fprintf (fp, "Alig  %d\n", ch->alignment);
    if (ch->hitroll != 0)
        fprintf (fp, "Hit   %d\n", ch->hitroll);
    if (ch->damroll != 0)
        fprintf (fp, "Dam   %d\n", ch->damroll);
    fprintf (fp, "ACs %d %d %d %d\n",
             ch->armor[0], ch->armor[1], ch->armor[2], ch->armor[3]);
    if (ch->wimpy != 0)
        fprintf (fp, "Wimp  %d\n", ch->wimpy);
    fprintf (fp, "Attr %d %d %d %d %d\n",
             ch->perm_stat[STAT_STR],
             ch->perm_stat[STAT_INT],
             ch->perm_stat[STAT_WIS],
             ch->perm_stat[STAT_DEX], ch->perm_stat[STAT_CON]);

    fprintf (fp, "AMod %d %d %d %d %d\n",
             ch->mod_stat[STAT_STR],
             ch->mod_stat[STAT_INT],
             ch->mod_stat[STAT_WIS],
             ch->mod_stat[STAT_DEX], ch->mod_stat[STAT_CON]);

    if (IS_NPC (ch))
        fprintf (fp, "Vnum %d\n", ch->index_data->vnum);
    else {
        fprintf (fp, "Pass %s~\n", ch->pcdata->pwd);
        if (ch->pcdata->bamfin[0] != '\0')
            fprintf (fp, "Bin  %s~\n", ch->pcdata->bamfin);
        if (ch->pcdata->bamfout[0] != '\0')
            fprintf (fp, "Bout %s~\n", ch->pcdata->bamfout);
        fprintf (fp, "Titl %s~\n", ch->pcdata->title);
        fprintf (fp, "Pnts %d\n", ch->pcdata->points);
        fprintf (fp, "TSex %d\n", ch->pcdata->true_sex);
        fprintf (fp, "LLev %d\n", ch->pcdata->last_level);
        fprintf (fp, "HMVP %d %d %d\n", ch->pcdata->perm_hit,
                 ch->pcdata->perm_mana, ch->pcdata->perm_move);
        fprintf (fp, "Cnd  %d %d %d %d\n",
                 ch->pcdata->condition[0],
                 ch->pcdata->condition[1],
                 ch->pcdata->condition[2], ch->pcdata->condition[3]);
        for (i = 0; i < COLOUR_MAX; i++)
            fprintf (fp, "Colour %d %ld\n", i, ch->pcdata->colour[i]);

        /* write alias */
        for (pos = 0; pos < MAX_ALIAS; pos++) {
            if (ch->pcdata->alias[pos] == NULL
                || ch->pcdata->alias_sub[pos] == NULL)
                break;

            fprintf (fp, "Alias %s %s~\n", ch->pcdata->alias[pos],
                     ch->pcdata->alias_sub[pos]);
        }

        /* Save note board status */
        /* Save number of boards in case that number changes */
        fprintf (fp, "Boards       %d ", BOARD_MAX);
        for (i = 0; i < BOARD_MAX; i++)
            fprintf (fp, "%s %ld ", board_table[i].name, ch->pcdata->last_note[i]);
        fprintf (fp, "\n");

        for (sn = 0; sn < SKILL_MAX; sn++) {
            if (skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0) {
                fprintf (fp, "Sk %d '%s'\n",
                         ch->pcdata->learned[sn], skill_table[sn].name);
            }
        }

        for (gn = 0; gn < GROUP_MAX; gn++)
            if (group_table[gn].name != NULL && ch->pcdata->group_known[gn])
                fprintf (fp, "Gr '%s'\n", group_table[gn].name);
    }

    for (paf = ch->affected; paf != NULL; paf = paf->next) {
        if (paf->type < 0 || paf->type >= SKILL_MAX)
            continue;
        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10ld\n",
                 skill_table[paf->type].name,
                 paf->bit_type, paf->level, paf->duration, paf->modifier,
                 paf->apply, paf->bits);
    }

#ifdef IMC
    imc_savechar( ch, fp );
#endif
    fprintf (fp, "End\n\n");
}

/* write a pet */
void fwrite_pet (CHAR_T *pet, FILE *fp) {
    AFFECT_T *paf;

    fprintf (fp, "#PET\n");

    fprintf (fp, "Vnum %d\n", pet->index_data->vnum);

    fprintf (fp, "Name %s~\n", pet->name);
    fprintf (fp, "LogO %ld\n", current_time);
    if (pet->short_descr != pet->index_data->short_descr)
        fprintf (fp, "ShD  %s~\n", pet->short_descr);
    if (pet->long_descr != pet->index_data->long_descr)
        fprintf (fp, "LnD  %s~\n", pet->long_descr);
    if (pet->description != pet->index_data->description)
        fprintf (fp, "Desc %s~\n", pet->description);
    if (pet->race != pet->index_data->race)
        fprintf (fp, "Race %s~\n", race_table[pet->race].name);
    if (pet->clan)
        fprintf (fp, "Clan %s~\n", clan_table[pet->clan].name);
    fprintf (fp, "Sex  %d\n", pet->sex);
    if (pet->level != pet->index_data->level)
        fprintf (fp, "Levl %d\n", pet->level);
    fprintf (fp, "HMV  %d %d %d %d %d %d\n",
             pet->hit, pet->max_hit, pet->mana, pet->max_mana, pet->move,
             pet->max_move);
    if (pet->gold > 0)
        fprintf (fp, "Gold %ld\n", pet->gold);
    if (pet->silver > 0)
        fprintf (fp, "Silv %ld\n", pet->silver);
    if (pet->exp > 0)
        fprintf (fp, "Exp  %d\n", pet->exp);
    if (pet->mob != pet->index_data->mob_final)
        fprintf (fp, "Mob  %s\n", print_flags (pet->mob));
    if (pet->plr != 0)
        fprintf (fp, "Plr  %s\n", print_flags (pet->plr));
    if (pet->affected_by != pet->index_data->affected_by_final)
        fprintf (fp, "AfBy %s\n", print_flags (pet->affected_by));
    if (pet->comm != 0)
        fprintf (fp, "Comm %s\n", print_flags (pet->comm));
    fprintf (fp, "Pos  %d\n", pet->position =
             POS_FIGHTING ? POS_STANDING : pet->position);
    if (pet->saving_throw != 0)
        fprintf (fp, "Save %d\n", pet->saving_throw);
    if (pet->alignment != pet->index_data->alignment)
        fprintf (fp, "Alig %d\n", pet->alignment);
    if (pet->hitroll != pet->index_data->hitroll)
        fprintf (fp, "Hit  %d\n", pet->hitroll);
    if (pet->damroll != pet->index_data->damage.bonus)
        fprintf (fp, "Dam  %d\n", pet->damroll);
    fprintf (fp, "ACs  %d %d %d %d\n",
             pet->armor[0], pet->armor[1], pet->armor[2], pet->armor[3]);
    fprintf (fp, "Attr %d %d %d %d %d\n",
             pet->perm_stat[STAT_STR], pet->perm_stat[STAT_INT],
             pet->perm_stat[STAT_WIS], pet->perm_stat[STAT_DEX],
             pet->perm_stat[STAT_CON]);
    fprintf (fp, "AMod %d %d %d %d %d\n",
             pet->mod_stat[STAT_STR], pet->mod_stat[STAT_INT],
             pet->mod_stat[STAT_WIS], pet->mod_stat[STAT_DEX],
             pet->mod_stat[STAT_CON]);

    for (paf = pet->affected; paf != NULL; paf = paf->next) {
        if (paf->type < 0 || paf->type >= SKILL_MAX)
            continue;
        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10ld\n",
                 skill_table[paf->type].name,
                 paf->bit_type, paf->level, paf->duration, paf->modifier,
                 paf->apply, paf->bits);
    }
    fprintf (fp, "End\n");
}

/* Write an object and its contents. */
void fwrite_obj (CHAR_T *ch, OBJ_T *obj, FILE *fp, int nest) {
    EXTRA_DESCR_T *ed;
    AFFECT_T *paf;

    /* Slick recursion to write lists backwards,
     * so loading them will load in forwards order. */
    if (obj->next_content != NULL)
        fwrite_obj (ch, obj->next_content, fp, nest);

    /* Castrate storage characters. */
    if (ch->level < obj->level - 2 && obj->item_type != ITEM_CONTAINER)
        return;

    /* Don't store non-persistant items. */
    if (obj->item_type == ITEM_KEY)
        return;
    if (obj->item_type == ITEM_MAP && !obj->v.map.persist)
        return;

    fprintf (fp, "#O\n");
    fprintf (fp, "Vnum %d\n", obj->index_data->vnum);
    if (!obj->index_data->new_format)
        fprintf (fp, "Oldstyle\n");
    if (obj->enchanted)
        fprintf (fp, "Enchanted\n");
    fprintf (fp, "Nest %d\n", nest);

    /* These data are only used if they do not match the defaults */
    if (obj->name != obj->index_data->name)
        fprintf (fp, "Name %s~\n", obj->name);
    if (obj->short_descr != obj->index_data->short_descr)
        fprintf (fp, "ShD  %s~\n", obj->short_descr);
    if (obj->description != obj->index_data->description)
        fprintf (fp, "Desc %s~\n", obj->description);
    if (obj->extra_flags != obj->index_data->extra_flags)
        fprintf (fp, "ExtF %ld\n", obj->extra_flags);
    if (obj->wear_flags != obj->index_data->wear_flags)
        fprintf (fp, "WeaF %ld\n", obj->wear_flags);
    if (obj->item_type != obj->index_data->item_type)
        fprintf (fp, "Ityp %d\n", obj->item_type);
    if (obj->weight != obj->index_data->weight)
        fprintf (fp, "Wt   %d\n", obj->weight);
    if (obj->condition != obj->index_data->condition)
        fprintf (fp, "Cond %d\n", obj->condition);

    /* Variable data */
    fprintf (fp, "Wear %d\n", obj->wear_loc);
    if (obj->level != obj->index_data->level)
        fprintf (fp, "Lev  %d\n", obj->level);
    if (obj->timer != 0)
        fprintf (fp, "Time %d\n", obj->timer);
    fprintf (fp, "Cost %d\n", obj->cost);
    if (obj->v.value[0] != obj->index_data->v.value[0] ||
        obj->v.value[1] != obj->index_data->v.value[1] ||
        obj->v.value[2] != obj->index_data->v.value[2] ||
        obj->v.value[3] != obj->index_data->v.value[3] ||
        obj->v.value[4] != obj->index_data->v.value[4])
    {
        fprintf (fp, "Val  %ld %ld %ld %ld %ld\n",
                 obj->v.value[0], obj->v.value[1], obj->v.value[2],
                 obj->v.value[3], obj->v.value[4]);
    }

    switch (obj->item_type) {
        case ITEM_POTION: {
            int i;
            for (i = 0; i < POTION_SKILL_MAX; i++) {
                if (obj->v.potion.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.potion.skill[i]].name);
            }
            break;
        }

        case ITEM_SCROLL: {
            int i;
            for (i = 0; i < SCROLL_SKILL_MAX; i++) {
                if (obj->v.scroll.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.scroll.skill[i]].name);
            }
            break;
        }

        case ITEM_PILL: {
            int i;
            for (i = 0; i < PILL_SKILL_MAX; i++) {
                if (obj->v.pill.skill[i] < 0)
                    continue;
                fprintf (fp, "Spell %d '%s'\n", i + 1,
                    skill_table[obj->v.pill.skill[i]].name);
            }
            break;
        }

        case ITEM_STAFF:
            if (obj->v.staff.skill > 0)
                fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->v.staff.skill].name);
            break;

        case ITEM_WAND:
            if (obj->v.wand.skill > 0)
                fprintf (fp, "Spell 3 '%s'\n", skill_table[obj->v.wand.skill].name);
            break;
    }

    for (paf = obj->affected; paf != NULL; paf = paf->next) {
        if (paf->type < 0 || paf->type >= SKILL_MAX)
            continue;
        fprintf (fp, "Affc '%s' %3d %3d %3d %3d %3d %10ld\n",
                 skill_table[paf->type].name,
                 paf->bit_type, paf->level, paf->duration, paf->modifier,
                 paf->apply, paf->bits);
    }

    for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
        fprintf (fp, "ExDe %s~ %s~\n", ed->keyword, ed->description);
    fprintf (fp, "End\n\n");

    if (obj->contains != NULL)
        fwrite_obj (ch, obj->contains, fp, nest + 1);
}

/* Load a char and inventory into a new ch structure. */
bool load_char_obj (DESCRIPTOR_T *d, char *name) {
    char strsave[MAX_INPUT_LENGTH];
    char buf[100];
    CHAR_T *ch;
    FILE *fp;
    bool found;
    int stat;

    ch = char_new ();
    ch->pcdata = pcdata_new ();

    d->character = ch;
    ch->desc = d;
    ch->name = str_dup (name);
    ch->id = get_pc_id ();
    ch->race = race_lookup ("human");
    ch->plr = PLR_NOSUMMON;
    ch->comm = COMM_COMBINE | COMM_PROMPT;
    ch->prompt = str_dup ("<%hhp %mm %vmv> ");
    ch->pcdata->confirm_delete = FALSE;
    ch->pcdata->board = &board_table[DEFAULT_BOARD];
    ch->pcdata->pwd = str_dup ("");
    ch->pcdata->bamfin = str_dup ("");
    ch->pcdata->bamfout = str_dup ("");
    ch->pcdata->title = str_dup ("");
    for (stat = 0; stat < STAT_MAX; stat++)
        ch->perm_stat[stat] = 13;
    ch->pcdata->condition[COND_THIRST] = 48;
    ch->pcdata->condition[COND_FULL] = 48;
    ch->pcdata->condition[COND_HUNGER] = 48;
    ch->pcdata->security = 0; /* OLC */
    char_reset_colour (ch);

#ifdef IMC
    imc_initchar( ch );
#endif
    found = FALSE;
    fclose (reserve_file);

#if defined(unix)
    /* decompress if .gz file exists */
    sprintf (strsave, "%s%s%s", PLAYER_DIR, capitalize (name), ".gz");
    if ((fp = fopen (strsave, "r")) != NULL) {
        fclose (fp);
        sprintf (buf, "gzip -dfq %s", strsave);
        system (buf);
    }
#endif

    sprintf (strsave, "%s%s", PLAYER_DIR, capitalize (name));
    if ((fp = fopen (strsave, "r")) != NULL) {
        int nest;
        for (nest = 0; nest < MAX_NEST; nest++)
            obj_nest[nest] = NULL;

        found = TRUE;
        while (1) {
            char letter;
            char *word;

            letter = fread_letter (fp);
            if (letter == '*') {
                fread_to_eol (fp);
                continue;
            }
            if (letter != '#') {
                bug ("load_char_obj: # not found.", 0);
                break;
            }

            word = fread_word (fp);
                 if (!str_cmp (word, "PLAYER")) fread_char (ch, fp);
            else if (!str_cmp (word, "OBJECT")) fread_obj (ch, fp);
            else if (!str_cmp (word, "O"))      fread_obj (ch, fp);
            else if (!str_cmp (word, "PET"))    fread_pet (ch, fp);
            else if (!str_cmp (word, "END"))    break;
            else {
                bug ("load_char_obj: bad section.", 0);
                break;
            }
        }
        fclose (fp);
    }

    reserve_file = fopen (NULL_FILE, "r");

    /* initialize race */
    if (found) {
        int i;
        if (ch->race == 0)
            ch->race = race_lookup ("human");

        ch->size = pc_race_table[ch->race].size;
        ch->attack_type = ATTACK_PUNCH;

        for (i = 0; i < PC_RACE_SKILL_MAX; i++) {
            if (pc_race_table[ch->race].skills[i] == NULL)
                break;
            group_add (ch, pc_race_table[ch->race].skills[i], FALSE);
        }
        ch->affected_by = ch->affected_by | race_table[ch->race].aff;
        ch->imm_flags   = ch->imm_flags | race_table[ch->race].imm;
        ch->res_flags   = ch->res_flags | race_table[ch->race].res;
        ch->vuln_flags  = ch->vuln_flags | race_table[ch->race].vuln;
        ch->form        = race_table[ch->race].form;
        ch->parts       = race_table[ch->race].parts;
    }

    /* RT initialize skills */
    if (found && ch->version < 2) { /* need to add the new skills */
        group_add (ch, "rom basics", FALSE);
        group_add (ch, class_table[ch->class].base_group, FALSE);
        group_add (ch, class_table[ch->class].default_group, TRUE);
        ch->pcdata->learned[gsn_recall] = 50;
    }

    /* fix levels */
    if (found && ch->version < 3 && (ch->level > 35 || ch->trust > 35)) {
        switch (ch->level) {
            case (40): ch->level = 60; break; /* imp -> imp */
            case (39): ch->level = 58; break; /* god -> supreme */
            case (38): ch->level = 56; break; /* deity -> god */
            case (37): ch->level = 53; break; /* angel -> demigod */
        }

        switch (ch->trust) {
            case (40): ch->trust = 60; break; /* imp -> imp */
            case (39): ch->trust = 58; break; /* god -> supreme */
            case (38): ch->trust = 56; break; /* deity -> god */
            case (37): ch->trust = 53; break; /* angel -> demigod */
            case (36): ch->trust = 51; break; /* hero -> hero */
        }
    }

    /* ream gold */
    if (found && ch->version < 4)
        ch->gold /= 100;
    return found;
}

void load_old_colour (CHAR_T *ch, FILE *fp, char *name) {
    const COLOUR_SETTING_T *setting;
    int number;
    flag_t flag;

    /* Always read the number */
    number = fread_number (fp);
    setting = colour_setting_get_by_name_exact (name);
    BAIL_IF_BUGF (setting == NULL,
        "load_old_colour: unknown color setting '%s'", name);

    /* Convert Lope's saved codes to a bitmask */
    /* NOTE: LOAD_COLOUR was *broken* and used > comparisons
     *       instead of >= comparisons. As a result, color code
     *       100 and 10 would potentially occur for black, and
     *       dark grey wouldn't load at all. */
    flag = 0;
    if (number >= 100) {
        number -= 100;
        flag |= CB_BEEP;
    }
    if (number >= 10) {
        number -= 10;
        flag |= CB_BRIGHT;
    }
    flag |= (number & 0x07);

    /* Number should now be >= 0x00 and <= 0x0f. If it's not, it's broken :(
     * Default to white. */
    if (number > 0x0f)
        flag = CC_WHITE;

    /* Default backcolor, always. */
    flag |= CC_BACK_DEFAULT;

    /* Store color! */
    ch->pcdata->colour[setting->index] = flag;
}

void fread_char (CHAR_T *ch, FILE *fp) {
    char buf[MAX_STRING_LENGTH];
    char *word;
    bool match;
    int count = 0;
    int lastlogoff = current_time;
    int percent;

    log_f ("Loading %s.", ch->name);

    while (1) {
        word = feof (fp) ? "End" : fread_word (fp);
        match = FALSE;

        switch (UPPER (word[0])) {
            case '*':
                match = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                if (!str_cmp (word, "Act")) {
                    match = TRUE;
                    flag_t flags = fread_flag (fp);
                    if (IS_SET (flags, MOB_IS_NPC))
                        ch->mob = flags;
                    else
                        ch->plr = flags;
                }

                KEY ("AffectedBy", ch->affected_by, fread_flag (fp));
                KEY ("AfBy",       ch->affected_by, fread_flag (fp));
                KEY ("Alignment",  ch->alignment, fread_flag (fp));
                KEY ("Alig",       ch->alignment, fread_number (fp));

                if (!str_cmp (word, "Alia")) {
                    if (count >= MAX_ALIAS) {
                        fread_to_eol (fp);
                        match = TRUE;
                        break;
                    }

                    ch->pcdata->alias[count] = str_dup (fread_word (fp));
                    ch->pcdata->alias_sub[count] = str_dup (fread_word (fp));
                    count++;
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Alias")) {
                    if (count >= MAX_ALIAS) {
                        fread_to_eol (fp);
                        match = TRUE;
                        break;
                    }

                    ch->pcdata->alias[count] = str_dup (fread_word (fp));
                    ch->pcdata->alias_sub[count] = fread_string (fp);
                    count++;
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AC") || !str_cmp (word, "Armor")) {
                    fread_to_eol (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "ACs")) {
                    int i;
                    for (i = 0; i < 4; i++)
                        ch->armor[i] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AffD")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);
                    LIST_BACK (paf, next, ch->affected, AFFECT_T);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Affc")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->bit_type = fread_number (fp);
                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);
                    LIST_BACK (paf, next, ch->affected, AFFECT_T);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AttrMod") || !str_cmp (word, "AMod")) {
                    int stat;
                    for (stat = 0; stat < STAT_MAX; stat++)
                        ch->mod_stat[stat] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AttrPerm") || !str_cmp (word, "Attr")) {
                    int stat;

                    for (stat = 0; stat < STAT_MAX; stat++)
                        ch->perm_stat[stat] = fread_number (fp);
                    match = TRUE;
                    break;
                }
                break;

            case 'B':
                KEY ("Bamfin",  ch->pcdata->bamfin,  fread_string (fp));
                KEY ("Bamfout", ch->pcdata->bamfout, fread_string (fp));
                KEY ("Bin",     ch->pcdata->bamfin,  fread_string (fp));
                KEY ("Bout",    ch->pcdata->bamfout, fread_string (fp));

                /* Read in board status */
                if (!str_cmp(word, "Boards" )) {
                    int i,num = fread_number (fp); /* number of boards saved */
                    char *boardname;

                    for (; num ; num-- ) { /* for each of the board saved */
                        boardname = fread_word (fp);
                        i = board_lookup (boardname); /* find board number */

                        if (i == BOARD_NOTFOUND) { /* Does board still exist ? */
                            log_f ("fread_char: %s had unknown board name: %s. Skipped.",
                                ch->name, boardname);
                            fread_number (fp); /* read last_note and skip info */
                        }
                        else /* Save it */
                            ch->pcdata->last_note[i] = fread_number (fp);
                    } /* for */

                    match = TRUE;
                } /* Boards */
                break;

            case 'C':
                KEY ("Class", ch->class, fread_number (fp));
                KEY ("Cla",   ch->class, fread_number (fp));
                KEY ("Clan",  ch->clan, clan_lookup (fread_string (fp)));
                KEY ("Comm",  ch->comm, fread_flag (fp));

                if (!str_cmp (word, "Condition") || !str_cmp (word, "Cond")) {
                    ch->pcdata->condition[0] = fread_number (fp);
                    ch->pcdata->condition[1] = fread_number (fp);
                    ch->pcdata->condition[2] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Cnd")) {
                    ch->pcdata->condition[0] = fread_number (fp);
                    ch->pcdata->condition[1] = fread_number (fp);
                    ch->pcdata->condition[2] = fread_number (fp);
                    ch->pcdata->condition[3] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Coloura")) {
                    load_old_colour (ch, fp, "text");
                    load_old_colour (ch, fp, "auction");
                    load_old_colour (ch, fp, "gossip");
                    load_old_colour (ch, fp, "music");
                    load_old_colour (ch, fp, "question");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colourb")) {
                    load_old_colour (ch, fp, "answer");
                    load_old_colour (ch, fp, "quote");
                    load_old_colour (ch, fp, "quote_text");
                    load_old_colour (ch, fp, "immtalk_text");
                    load_old_colour (ch, fp, "immtalk_type");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colourc")) {
                    load_old_colour (ch, fp, "info");
                    load_old_colour (ch, fp, "tell");
                    load_old_colour (ch, fp, "reply");
                    load_old_colour (ch, fp, "gtell_text");
                    load_old_colour (ch, fp, "gtell_type");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colourd")) {
                    load_old_colour (ch, fp, "room_title");
                    load_old_colour (ch, fp, "room_text");
                    load_old_colour (ch, fp, "room_exits");
                    load_old_colour (ch, fp, "room_things");
                    load_old_colour (ch, fp, "prompt");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Coloure")) {
                    load_old_colour (ch, fp, "fight_death");
                    load_old_colour (ch, fp, "fight_yhit");
                    load_old_colour (ch, fp, "fight_ohit");
                    load_old_colour (ch, fp, "fight_thit");
                    load_old_colour (ch, fp, "fight_skill");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colourf")) {
                    load_old_colour (ch, fp, "wiznet");
                    load_old_colour (ch, fp, "say");
                    load_old_colour (ch, fp, "say_text");
                    load_old_colour (ch, fp, "tell_text");
                    load_old_colour (ch, fp, "reply_text");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colourg")) {
                    load_old_colour (ch, fp, "auction_text");
                    load_old_colour (ch, fp, "gossip_text");
                    load_old_colour (ch, fp, "music_text");
                    load_old_colour (ch, fp, "question_text");
                    load_old_colour (ch, fp, "answer_text");
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Colour")) {
                    ch->pcdata->colour[fread_number(fp)] = fread_flag (fp);
                    match = TRUE;
                }
                break;

            case 'D':
                KEY ("Damroll", ch->damroll, fread_number (fp));
                KEY ("Dam", ch->damroll, fread_number (fp));
                KEY ("Description", ch->description, fread_string (fp));
                KEY ("Desc", ch->description, fread_string (fp));
                break;

            case 'E':
                if (!str_cmp (word, "End")) {
                    /* adjust hp mana move up  -- here for speed's sake */
                    percent =
                        (current_time - lastlogoff) * 25 / (2 * 60 * 60);

                    percent = UMIN (percent, 100);

                    if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
                        && !IS_AFFECTED (ch, AFF_PLAGUE))
                    {
                        ch->hit += (ch->max_hit - ch->hit) * percent / 100;
                        ch->mana += (ch->max_mana - ch->mana) * percent / 100;
                        ch->move += (ch->max_move - ch->move) * percent / 100;
                    }
                    return;
                }
                KEY ("Exp", ch->exp, fread_number (fp));
                break;

            case 'G':
                KEY ("Gold", ch->gold, fread_number (fp));
                if (!str_cmp (word, "Group") || !str_cmp (word, "Gr")) {
                    int gn;
                    char *temp;

                    temp = fread_word (fp);
                    gn = group_lookup (temp);
                 /* gn = group_lookup (fread_word (fp)); */
                    if (gn < 0) {
                        fprintf (stderr, "%s", temp);
                        bug ("fread_char: unknown group. ", 0);
                    }
                    else
                        gn_add (ch, gn);
                    match = TRUE;
                }
                break;

            case 'H':
                KEY ("Hitroll", ch->hitroll, fread_number (fp));
                KEY ("Hit", ch->hitroll, fread_number (fp));

                if (!str_cmp (word, "HpManaMove") || !str_cmp (word, "HMV")) {
                    ch->hit = fread_number (fp);
                    ch->max_hit = fread_number (fp);
                    ch->mana = fread_number (fp);
                    ch->max_mana = fread_number (fp);
                    ch->move = fread_number (fp);
                    ch->max_move = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "HpManaMovePerm")
                    || !str_cmp (word, "HMVP"))
                {
                    ch->pcdata->perm_hit = fread_number (fp);
                    ch->pcdata->perm_mana = fread_number (fp);
                    ch->pcdata->perm_move = fread_number (fp);
                    match = TRUE;
                    break;
                }

                break;

            case 'I':
                KEY ("Id", ch->id, fread_number (fp));
                KEY ("InvisLevel", ch->invis_level, fread_number (fp));
                KEY ("Inco", ch->incog_level, fread_number (fp));
                KEY ("Invi", ch->invis_level, fread_number (fp));
#ifdef IMC
                if( ( match = imc_loadchar( ch, fp, word ) ) )
                    break;
#endif
                break;

            case 'L':
                KEY ("LastLevel", ch->pcdata->last_level, fread_number (fp));
                KEY ("LLev", ch->pcdata->last_level, fread_number (fp));
                KEY ("Level", ch->level, fread_number (fp));
                KEY ("Lev", ch->level, fread_number (fp));
                KEY ("Levl", ch->level, fread_number (fp));
                KEY ("LogO", lastlogoff, fread_number (fp));
                KEY ("LongDescr", ch->long_descr, fread_string (fp));
                KEY ("LnD", ch->long_descr, fread_string (fp));
                break;

            case 'M':
                KEY ("Mob", ch->mob, fread_flag (fp));
                break;

            case 'N':
                KEYS ("Name", ch->name, fread_string (fp));
                break;

            case 'P':
                KEY ("Password", ch->pcdata->pwd, fread_string (fp));
                KEY ("Pass", ch->pcdata->pwd, fread_string (fp));
                KEY ("Played", ch->played, fread_number (fp));
                KEY ("Plyd", ch->played, fread_number (fp));
                KEY ("Plr", ch->plr, fread_flag (fp));
                KEY ("Points", ch->pcdata->points, fread_number (fp));
                KEY ("Pnts", ch->pcdata->points, fread_number (fp));
                KEY ("Position", ch->position, fread_number (fp));
                KEY ("Pos", ch->position, fread_number (fp));
                KEY ("Practice", ch->practice, fread_number (fp));
                KEY ("Prac", ch->practice, fread_number (fp));
                KEYS ("Prompt", ch->prompt, fread_string (fp));
                KEY ("Prom", ch->prompt, fread_string (fp));
                break;

            case 'R':
                KEY ("Race", ch->race, race_lookup (fread_string (fp)));

                if (!str_cmp (word, "Room")) {
                    ch->in_room = get_room_index (fread_number (fp));
                    if (ch->in_room == NULL)
                        ch->in_room = get_room_index (ROOM_VNUM_LIMBO);
                    match = TRUE;
                    break;
                }

                break;

            case 'S':
                KEY ("SavingThrow", ch->saving_throw, fread_number (fp));
                KEY ("Save", ch->saving_throw, fread_number (fp));
                KEY ("Scro", ch->lines, fread_number (fp));
                KEY ("Sex", ch->sex, fread_number (fp));
                KEY ("ShortDescr", ch->short_descr, fread_string (fp));
                KEY ("ShD", ch->short_descr, fread_string (fp));
                KEY ("Sec", ch->pcdata->security, fread_number (fp));    /* OLC */
                KEY ("Silv", ch->silver, fread_number (fp));

                if (!str_cmp (word, "Skill") || !str_cmp (word, "Sk")) {
                    int sn;
                    int value;
                    char *temp;

                    value = fread_number (fp);
                    temp = fread_word (fp);
                    sn = skill_lookup (temp);
                 /* sn = skill_lookup (fread_word (fp)); */
                    if (sn < 0) {
                        fprintf (stderr, "%s", temp);
                        bug ("fread_char: unknown skill. ", 0);
                    }
                    else
                        ch->pcdata->learned[sn] = value;
                    match = TRUE;
                }
                break;

            case 'T':
                KEY ("TrueSex", ch->pcdata->true_sex, fread_number (fp));
                KEY ("TSex", ch->pcdata->true_sex, fread_number (fp));
                KEY ("Trai", ch->train, fread_number (fp));
                KEY ("Trust", ch->trust, fread_number (fp));
                KEY ("Tru", ch->trust, fread_number (fp));

                if (!str_cmp (word, "Title") || !str_cmp (word, "Titl")) {
                    ch->pcdata->title = fread_string (fp);
                    if (ch->pcdata->title[0] != '.'
                        && ch->pcdata->title[0] != ','
                        && ch->pcdata->title[0] != '!'
                        && ch->pcdata->title[0] != '?')
                    {
                        sprintf (buf, " %s", ch->pcdata->title);
                        str_replace_dup (&(ch->pcdata->title), buf);
                    }
                    match = TRUE;
                    break;
                }

                break;

            case 'V':
                KEY ("Version", ch->version, fread_number (fp));
                KEY ("Vers", ch->version, fread_number (fp));
                if (!str_cmp (word, "Vnum")) {
                    ch->index_data = get_mob_index (fread_number (fp));
                    match = TRUE;
                    break;
                }
                break;

            case 'W':
                KEY ("Wimpy", ch->wimpy, fread_number (fp));
                KEY ("Wimp", ch->wimpy, fread_number (fp));
                KEY ("Wizn", ch->wiznet, fread_flag (fp));
                break;
        }

        if (!match) {
            bug ("fread_char: no match.", 0);
            bug (word, 0);
            fread_to_eol (fp);
        }
    }
}

/* load a pet from the forgotten reaches */
void fread_pet (CHAR_T *ch, FILE *fp) {
    char *word;
    CHAR_T *pet;
    bool match;
    int lastlogoff = current_time;
    int percent;
    int vnum = 0;

    /* first entry had BETTER be the vnum or we barf */
    word = feof (fp) ? "END" : fread_word (fp);
    if (!str_cmp (word, "Vnum")) {
        vnum = fread_number (fp);
        if (get_mob_index (vnum) == NULL) {
            bug ("fread_pet: bad vnum %d.", vnum);
            pet = char_create_mobile (get_mob_index (MOB_VNUM_FIDO));
        }
        else
            pet = char_create_mobile (get_mob_index (vnum));
    }
    else {
        bug ("fread_pet: no vnum in file.", 0);
        pet = char_create_mobile (get_mob_index (MOB_VNUM_FIDO));
    }

    while (1) {
        word = feof (fp) ? "END" : fread_word (fp);
        match = FALSE;

        switch (UPPER (word[0])) {
            case '*':
                match = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                if (!str_cmp (word, "Act")) {
                    match = TRUE;
                    flag_t flags = fread_flag (fp);
                    if (IS_SET (flags, MOB_IS_NPC))
                        pet->mob = flags;
                    else
                        pet->plr = flags;
                }

                KEY ("AfBy", pet->affected_by, fread_flag (fp));
                KEY ("Alig", pet->alignment,   fread_number (fp));

                if (!str_cmp (word, "ACs")) {
                    int i;
                    for (i = 0; i < 4; i++)
                        pet->armor[i] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AffD")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);
                    LIST_BACK (paf, next, pet->affected, AFFECT_T);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Affc")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_char: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->bit_type = fread_number (fp);
                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);

                    /* Added here after Chris Litchfield (The Mage's Lair)
                     * pointed out a bug with duplicating affects in saved
                     * pets. -- JR 2002/01/31 */
                    if (!check_pet_affected (vnum, paf))
                        LIST_BACK (paf, next, pet->affected, AFFECT_T);
                    else
                        affect_free (paf);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "AMod")) {
                    int stat;
                    for (stat = 0; stat < STAT_MAX; stat++)
                        pet->mod_stat[stat] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Attr")) {
                    int stat;
                    for (stat = 0; stat < STAT_MAX; stat++)
                        pet->perm_stat[stat] = fread_number (fp);
                    match = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY ("Clan", pet->clan, clan_lookup (fread_string (fp)));
                KEY ("Comm", pet->comm, fread_flag (fp));
                break;

            case 'D':
                KEY ("Dam",  pet->damroll, fread_number (fp));
                KEY ("Desc", pet->description, fread_string (fp));
                break;

            case 'E':
                if (!str_cmp (word, "End")) {
                    pet->leader = ch;
                    pet->master = ch;
                    ch->pet = pet;
                    /* adjust hp mana move up  -- here for speed's sake */
                    percent =
                        (current_time - lastlogoff) * 25 / (2 * 60 * 60);

                    if (percent > 0 && !IS_AFFECTED (ch, AFF_POISON)
                        && !IS_AFFECTED (ch, AFF_PLAGUE))
                    {
                        percent = UMIN (percent, 100);
                        pet->hit += (pet->max_hit - pet->hit) * percent / 100;
                        pet->mana +=
                            (pet->max_mana - pet->mana) * percent / 100;
                        pet->move +=
                            (pet->max_move - pet->move) * percent / 100;
                    }
                    return;
                }
                KEY ("Exp", pet->exp, fread_number (fp));
                break;

            case 'G':
                KEY ("Gold", pet->gold, fread_number (fp));
                break;

            case 'H':
                KEY ("Hit", pet->hitroll, fread_number (fp));
                if (!str_cmp (word, "HMV")) {
                    pet->hit      = fread_number (fp);
                    pet->max_hit  = fread_number (fp);
                    pet->mana     = fread_number (fp);
                    pet->max_mana = fread_number (fp);
                    pet->move     = fread_number (fp);
                    pet->max_move = fread_number (fp);
                    match = TRUE;
                    break;
                }
                break;

            case 'L':
                KEY ("Levl", pet->level, fread_number (fp));
                KEY ("LnD",  pet->long_descr, fread_string (fp));
                KEY ("LogO", lastlogoff, fread_number (fp));
                break;

            case 'M':
                KEY ("Mob", pet->mob, fread_flag (fp));
                break;

            case 'N':
                KEY ("Name", pet->name, fread_string (fp));
                break;

            case 'P':
                KEY ("Plr", pet->plr,      fread_flag (fp));
                KEY ("Pos", pet->position, fread_number (fp));
                break;

            case 'R':
                KEY ("Race", pet->race, race_lookup (fread_string (fp)));
                break;

            case 'S':
                KEY ("Save", pet->saving_throw, fread_number (fp));
                KEY ("Sex", pet->sex, fread_number (fp));
                KEY ("ShD", pet->short_descr, fread_string (fp));
                KEY ("Silv", pet->silver, fread_number (fp));
                break;
        }

        if (!match) {
            bug ("fread_pet: no match.", 0);
            fread_to_eol (fp);
        }
    }
}

void fread_obj (CHAR_T *ch, FILE *fp) {
    OBJ_T *obj;
    char *word;
    int nest;
    bool match;
    bool nested;
    bool vnum;
    bool first;
    bool new_format; /* to prevent errors */
    bool make_new; /* update object */

    vnum = FALSE;
    obj = NULL;
    first = TRUE; /* used to counter fp offset */
    new_format = FALSE;
    make_new = FALSE;

    word = feof (fp) ? "End" : fread_word (fp);
    if (!str_cmp (word, "Vnum")) {
        int vnum;
        first = FALSE; /* fp will be in right place */

        vnum = fread_number (fp);
        if (get_obj_index (vnum) == NULL)
            bug ("fread_obj: bad vnum %d.", vnum);
        else {
            obj = obj_create (get_obj_index (vnum), -1);
            new_format = TRUE;
        }
    }

    if (obj == NULL) { /* either not found or old style */
        obj = obj_new ();
        obj->name = str_dup ("");
        obj->short_descr = str_dup ("");
        obj->description = str_dup ("");
    }

    nested = FALSE;
    vnum = TRUE;
    nest = 0;

    while (1) {
        if (first)
            first = FALSE;
        else
            word = feof (fp) ? "End" : fread_word (fp);
        match = FALSE;

        switch (UPPER (word[0])) {
            case '*':
                match = TRUE;
                fread_to_eol (fp);
                break;

            case 'A':
                if (!str_cmp (word, "AffD")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_obj: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);
                    LIST_BACK (paf, next, obj->affected, AFFECT_T);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Affc")) {
                    AFFECT_T *paf;
                    int sn;

                    paf = affect_new ();
                    sn = skill_lookup (fread_word (fp));
                    if (sn < 0)
                        bug ("fread_obj: unknown skill.", 0);
                    else
                        paf->type = sn;

                    paf->bit_type = fread_number (fp);
                    paf->level    = fread_number (fp);
                    paf->duration = fread_number (fp);
                    paf->modifier = fread_number (fp);
                    paf->apply    = fread_number (fp);
                    paf->bits     = fread_number (fp);
                    LIST_BACK (paf, next, obj->affected, AFFECT_T);
                    match = TRUE;
                    break;
                }
                break;

            case 'C':
                KEY ("Cond", obj->condition, fread_number (fp));
                KEY ("Cost", obj->cost, fread_number (fp));
                break;

            case 'D':
                KEY ("Description", obj->description, fread_string (fp));
                KEY ("Desc", obj->description, fread_string (fp));
                break;

            case 'E':
                if (!str_cmp (word, "Enchanted")) {
                    obj->enchanted = TRUE;
                    match = TRUE;
                    break;
                }

                KEY ("ExtraFlags", obj->extra_flags, fread_flag (fp));
                KEY ("ExtF", obj->extra_flags, fread_flag (fp));

                if (!str_cmp (word, "ExtraDescr") || !str_cmp (word, "ExDe")) {
                    EXTRA_DESCR_T *ed;

                    ed = extra_descr_new ();
                    ed->keyword = fread_string (fp);
                    ed->description = fread_string (fp);
                    LIST_BACK (ed, next, obj->extra_descr, EXTRA_DESCR_T);
                    match = TRUE;
                }

                if (!str_cmp (word, "End")) {
                    if (!nested || (vnum && obj->index_data == NULL)) {
                        bug ("fread_obj: incomplete object.", 0);
                        obj_free (obj);
                        return;
                    }
                    else {
                        if (!vnum) {
                            obj_free (obj);
                            obj = obj_create (get_obj_index (OBJ_VNUM_DUMMY), 0);
                        }
                        if (!new_format) {
                            LIST_FRONT (obj, next, object_list);
                            obj->index_data->count++;
                        }

                        if (!obj->index_data->new_format
                            && obj->item_type == ITEM_ARMOR
                            && obj->v.armor.vs_bash == 0)
                        {
                            obj->v.armor.vs_bash  = obj->v.armor.vs_pierce;
                            obj->v.armor.vs_slash = obj->v.armor.vs_pierce;
                        }
                        if (make_new) {
                            int wear;

                            wear = obj->wear_loc;
                            obj_extract (obj);

                            obj = obj_create (obj->index_data, 0);
                            obj->wear_loc = wear;
                        }
                        if (nest == 0 || obj_nest[nest] == NULL)
                            obj_give_to_char (obj, ch);
                        else
                            obj_give_to_obj (obj, obj_nest[nest - 1]);
                        return;
                    }
                }
                break;

            case 'I':
                KEY ("ItemType", obj->item_type, fread_number (fp));
                KEY ("Ityp", obj->item_type, fread_number (fp));
                break;

            case 'L':
                KEY ("Level", obj->level, fread_number (fp));
                KEY ("Lev", obj->level, fread_number (fp));
                break;

            case 'N':
                KEY ("Name", obj->name, fread_string (fp));

                if (!str_cmp (word, "Nest")) {
                    nest = fread_number (fp);
                    if (nest < 0 || nest >= MAX_NEST)
                        bug ("fread_obj: bad nest %d.", nest);
                    else {
                        obj_nest[nest] = obj;
                        nested = TRUE;
                    }
                    match = TRUE;
                }
                break;

            case 'O':
                if (!str_cmp (word, "Oldstyle")) {
                    if (obj->index_data != NULL
                        && obj->index_data->new_format)
                        make_new = TRUE;
                    match = TRUE;
                }
                break;

            case 'S':
                KEY ("ShortDescr", obj->short_descr, fread_string (fp));
                KEY ("ShD", obj->short_descr, fread_string (fp));

                if (!str_cmp (word, "Spell")) {
                    int value, sn;
                    value = fread_number (fp);
                    sn = skill_lookup (fread_word (fp));

                    if (value < 0 || value >= OBJ_VALUE_MAX)
                        bug ("fread_obj: bad value %d.", value);
                    else if (sn < 0)
                        bug ("fread_obj: unknown skill.", 0);
                    else
                        obj->v.value[value] = sn;
                    match = TRUE;
                }
                break;

            case 'T':
                KEY ("Timer", obj->timer, fread_number (fp));
                KEY ("Time", obj->timer, fread_number (fp));
                break;

            case 'V':
                if (!str_cmp (word, "Values") || !str_cmp (word, "Vals")) {
                    obj->v.value[0] = fread_number (fp);
                    obj->v.value[1] = fread_number (fp);
                    obj->v.value[2] = fread_number (fp);
                    obj->v.value[3] = fread_number (fp);
                    if (obj->item_type == ITEM_WEAPON &&
                        obj->v.weapon.weapon_type == 0)
                    {
                        obj->v.weapon.weapon_type = obj->index_data->
                            v.weapon.weapon_type;
                    }
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Val")) {
                    obj->v.value[0] = fread_number (fp);
                    obj->v.value[1] = fread_number (fp);
                    obj->v.value[2] = fread_number (fp);
                    obj->v.value[3] = fread_number (fp);
                    obj->v.value[4] = fread_number (fp);
                    match = TRUE;
                    break;
                }

                if (!str_cmp (word, "Vnum")) {
                    int vnum;

                    vnum = fread_number (fp);
                    if ((obj->index_data = get_obj_index (vnum)) == NULL)
                        bug ("fread_obj: bad vnum %d.", vnum);
                    else
                        vnum = TRUE;
                    match = TRUE;
                    break;
                }
                break;

            case 'W':
                KEY ("WearFlags", obj->wear_flags, fread_flag (fp));
                KEY ("WeaF", obj->wear_flags, fread_flag (fp));
                KEY ("WearLoc", obj->wear_loc, fread_number (fp));
                KEY ("Wear", obj->wear_loc, fread_number (fp));
                KEY ("Weight", obj->weight, fread_number (fp));
                KEY ("Wt", obj->weight, fread_number (fp));
                break;
        }

        if (!match) {
            bug ("fread_obj: no match.", 0);
            fread_to_eol (fp);
        }
    }
}
