CC      = gcc
PROF    = -g -O
NOCRYPT =
C_FLAGS =  -Wall $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
LIBS    = -lcrypt
EXE     = ../bin/rom

O_FILES = \
    act_board.o act_comm.o act_conf.o act_fight.o act_group.o act_info.o \
    act_move.o act_obj.o act_olc.o act_player.o act_shop.o act_skills.o \
    affects.o ban.o board.o boot.o chars.o colour.o comm.o db_old.o db.o \
    descs.o do_sub.o effects.o fight.o find.o flags.o globals.o groups.o interp.o \
    json.o json_import.o json_obj.o json_read.o json_write.o json_tbl.o \
    lookup.o magic.o materials.o memory.o mob_cmds.o mob_prog.o music.o nanny.o objs.o \
    olc.o olc_aedit.o olc_hedit.o olc_medit.o olc_mpedit.o olc_oedit.o \
    olc_redit.o olc_save.o portals.o recycle.o rooms.o save.o signal.o \
    skills.o special.o spell_aff.o spell_create.o spell_cure.o spell_info.o \
    spell_misc.o spell_move.o spell_npc.o spell_off.o string.o tables.o \
    types.o update.o utils.o wiz_im.o wiz_l1.o wiz_l2.o wiz_l3.o wiz_l4.o \
    wiz_l5.o wiz_l6.o wiz_l7.o wiz_l8.o wiz_ml.o

rom: $(O_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(O_FILES) $(LIBS)

depend:
	makedepend -Y. *.c

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o
# DO NOT DELETE

act_board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_board.o: json.h structs.h types.h flags.h tables.h board.h comm.h
act_board.o: recycle.h utils.h lookup.h interp.h act_info.h chars.h memory.h
act_board.o: globals.h act_board.h
act_comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_comm.o: json.h structs.h types.h flags.h tables.h interp.h recycle.h
act_comm.o: lookup.h comm.h utils.h mob_prog.h db.h do_sub.h chars.h find.h
act_comm.o: globals.h act_comm.h
act_conf.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_conf.o: json.h structs.h types.h flags.h tables.h db.h interp.h colour.h
act_conf.o: groups.h comm.h utils.h lookup.h do_sub.h chars.h memory.h
act_conf.o: act_conf.h
act_fight.o: skills.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_fight.o: json.h structs.h types.h flags.h tables.h affects.h utils.h
act_fight.o: comm.h lookup.h interp.h groups.h mob_prog.h recycle.h fight.h
act_fight.o: act_comm.h chars.h find.h act_fight.h
act_group.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_group.o: json.h structs.h types.h flags.h tables.h groups.h utils.h
act_group.o: comm.h db.h chars.h find.h globals.h act_group.h
act_info.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_info.o: json.h structs.h types.h flags.h tables.h interp.h magic.h
act_info.o: recycle.h lookup.h utils.h skills.h groups.h db.h fight.h
act_info.o: update.h comm.h save.h do_sub.h act_comm.h act_obj.h chars.h
act_info.o: rooms.h objs.h find.h spell_info.h globals.h memory.h act_info.h
act_move.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_move.o: json.h structs.h types.h flags.h tables.h interp.h lookup.h
act_move.o: utils.h comm.h mob_prog.h affects.h skills.h db.h fight.h
act_move.o: groups.h act_info.h chars.h rooms.h objs.h find.h globals.h
act_move.o: act_move.h
act_obj.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_obj.o: json.h structs.h types.h flags.h tables.h affects.h utils.h comm.h
act_obj.o: db.h skills.h fight.h groups.h mob_prog.h save.h magic.h update.h
act_obj.o: act_group.h act_comm.h act_move.h recycle.h music.h chars.h objs.h
act_obj.o: rooms.h find.h globals.h act_obj.h
act_olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_olc.o: json.h structs.h types.h flags.h tables.h recycle.h lookup.h
act_olc.o: interp.h utils.h db.h comm.h olc_save.h act_info.h chars.h
act_olc.o: globals.h olc_aedit.h olc_hedit.h olc_medit.h olc_mpedit.h
act_olc.o: olc_oedit.h olc_redit.h olc.h act_olc.h
act_player.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_player.o: json.h structs.h types.h flags.h tables.h db.h comm.h save.h
act_player.o: utils.h fight.h interp.h recycle.h chars.h descs.h memory.h
act_player.o: globals.h act_player.h
act_shop.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_shop.o: json.h structs.h types.h flags.h tables.h lookup.h comm.h
act_shop.o: interp.h utils.h db.h skills.h groups.h chars.h objs.h find.h
act_shop.o: act_comm.h spell_cure.h materials.h globals.h memory.h act_shop.h
act_skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_skills.o: json.h structs.h types.h flags.h tables.h db.h utils.h comm.h
act_skills.o: interp.h magic.h fight.h lookup.h skills.h recycle.h act_comm.h
act_skills.o: chars.h find.h act_skills.h
affects.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
affects.o: json.h structs.h types.h flags.h tables.h recycle.h utils.h
affects.o: lookup.h chars.h affects.h
ban.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
ban.o: structs.h types.h flags.h tables.h recycle.h utils.h db.h save.h
ban.o: interp.h comm.h chars.h globals.h memory.h ban.h
board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
board.o: json.h structs.h types.h flags.h tables.h interp.h db.h utils.h
board.o: comm.h recycle.h lookup.h chars.h descs.h memory.h globals.h board.h
boot.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
boot.o: json.h structs.h types.h flags.h tables.h interp.h comm.h db.h
boot.o: utils.h signal.h descs.h save.h olc.h nanny.h update.h chars.h
boot.o: recycle.h act_info.h globals.h memory.h boot.h
chars.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
chars.o: json.h structs.h types.h flags.h tables.h utils.h skills.h comm.h
chars.o: objs.h affects.h lookup.h magic.h db.h groups.h fight.h interp.h
chars.o: recycle.h rooms.h mob_prog.h act_move.h act_info.h act_group.h
chars.o: wiz_l6.h materials.h globals.h memory.h chars.h
colour.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
colour.o: json.h structs.h types.h flags.h tables.h lookup.h chars.h colour.h
comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
comm.o: json.h structs.h types.h flags.h tables.h colour.h recycle.h utils.h
comm.o: fight.h interp.h db.h olc.h save.h mob_prog.h lookup.h act_info.h
comm.o: chars.h rooms.h objs.h descs.h globals.h comm.h
db.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
db.o: structs.h types.h flags.h tables.h utils.h recycle.h affects.h lookup.h
db.o: json_obj.h json_write.h json_read.h json_import.h music.h ban.h board.h
db.o: olc.h portals.h chars.h rooms.h objs.h db_old.h update.h globals.h
db.o: memory.h db.h
db_old.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
db_old.o: json.h structs.h types.h flags.h tables.h db.h recycle.h utils.h
db_old.o: interp.h lookup.h olc.h affects.h globals.h memory.h db_old.h
descs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
descs.o: json.h structs.h types.h flags.h tables.h ban.h chars.h colour.h
descs.o: comm.h db.h recycle.h utils.h interp.h lookup.h globals.h memory.h
descs.o: descs.h
do_sub.o: comm.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
do_sub.o: structs.h types.h flags.h tables.h chars.h do_sub.h
effects.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
effects.o: json.h structs.h types.h flags.h tables.h lookup.h db.h utils.h
effects.o: comm.h skills.h magic.h update.h affects.h objs.h chars.h
effects.o: effects.h
fight.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
fight.o: json.h structs.h types.h flags.h tables.h interp.h lookup.h update.h
fight.o: utils.h db.h effects.h mob_prog.h skills.h comm.h save.h groups.h
fight.o: magic.h affects.h act_fight.h act_obj.h act_comm.h act_move.h
fight.o: chars.h objs.h find.h globals.h memory.h fight.h
find.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
find.o: structs.h types.h flags.h tables.h utils.h chars.h db.h globals.h
find.o: find.h
flags.o: merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
flags.o: structs.h types.h flags.h tables.h
globals.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
globals.o: json.h structs.h types.h flags.h tables.h
groups.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
groups.o: structs.h types.h flags.h tables.h affects.h comm.h skills.h db.h
groups.o: chars.h globals.h groups.h
interp.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
interp.o: json.h structs.h types.h flags.h tables.h utils.h comm.h db.h
interp.o: recycle.h chars.h find.h descs.h globals.h memory.h act_board.h
interp.o: act_comm.h act_conf.h act_fight.h act_group.h act_info.h act_move.h
interp.o: act_obj.h act_player.h act_shop.h act_skills.h act_olc.h wiz_im.h
interp.o: wiz_l1.h wiz_l2.h wiz_l3.h wiz_l4.h wiz_l5.h wiz_l6.h wiz_l7.h
interp.o: wiz_l8.h wiz_ml.h olc.h mob_cmds.h interp.h
json.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json.o: json.h structs.h types.h flags.h tables.h
json_import.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
json_import.o: macros.h json.h structs.h types.h flags.h tables.h utils.h
json_import.o: recycle.h db.h lookup.h portals.h globals.h memory.h
json_import.o: json_import.h
json_obj.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_obj.o: json.h structs.h types.h flags.h tables.h db.h lookup.h utils.h
json_obj.o: json_obj.h
json_read.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_read.o: json.h structs.h types.h flags.h tables.h utils.h json_read.h
json_tbl.o: json_obj.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_tbl.o: json.h structs.h types.h flags.h tables.h json_tbl.h
json_write.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_write.o: json.h structs.h types.h flags.h tables.h utils.h json_write.h
lookup.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
lookup.o: json.h structs.h types.h flags.h tables.h utils.h interp.h db.h
lookup.o: recycle.h globals.h lookup.h
magic.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
magic.o: json.h structs.h types.h flags.h tables.h lookup.h affects.h comm.h
magic.o: skills.h fight.h utils.h chars.h magic.h
materials.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
materials.o: json.h structs.h types.h flags.h tables.h materials.h
memory.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
memory.o: json.h structs.h types.h flags.h tables.h utils.h globals.h
memory.o: memory.h
mob_cmds.o: lookup.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
mob_cmds.o: json.h structs.h types.h flags.h tables.h interp.h groups.h
mob_cmds.o: utils.h comm.h fight.h mob_prog.h db.h chars.h objs.h rooms.h
mob_cmds.o: find.h act_info.h globals.h mob_cmds.h
mob_prog.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
mob_prog.o: json.h structs.h types.h flags.h tables.h lookup.h utils.h
mob_prog.o: groups.h interp.h mob_cmds.h db.h chars.h objs.h find.h globals.h
mob_prog.o: mob_prog.h
music.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
music.o: json.h structs.h types.h flags.h tables.h utils.h comm.h db.h
music.o: interp.h globals.h music.h
nanny.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
nanny.o: json.h structs.h types.h flags.h tables.h interp.h recycle.h
nanny.o: lookup.h skills.h utils.h comm.h db.h save.h ban.h fight.h
nanny.o: act_info.h act_skills.h act_board.h act_obj.h chars.h objs.h descs.h
nanny.o: globals.h memory.h nanny.h
objs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
objs.o: json.h structs.h types.h flags.h tables.h affects.h utils.h chars.h
objs.o: db.h recycle.h comm.h lookup.h materials.h globals.h memory.h objs.h
olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
olc.o: structs.h types.h flags.h tables.h utils.h interp.h comm.h db.h
olc.o: recycle.h lookup.h magic.h chars.h memory.h act_olc.h olc_aedit.h
olc.o: olc_hedit.h olc_medit.h olc_mpedit.h olc_oedit.h olc_redit.h olc.h
olc_aedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_aedit.o: json.h structs.h types.h flags.h tables.h comm.h lookup.h db.h
olc_aedit.o: recycle.h interp.h globals.h olc.h memory.h olc_aedit.h
olc_hedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_hedit.o: json.h structs.h types.h flags.h tables.h comm.h lookup.h db.h
olc_hedit.o: recycle.h utils.h interp.h globals.h olc.h memory.h olc_hedit.h
olc_medit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_medit.o: json.h structs.h types.h flags.h tables.h comm.h lookup.h db.h
olc_medit.o: recycle.h utils.h interp.h mob_cmds.h chars.h globals.h olc.h
olc_medit.o: memory.h olc_medit.h
olc_mpedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_mpedit.o: json.h structs.h types.h flags.h tables.h comm.h db.h recycle.h
olc_mpedit.o: utils.h lookup.h chars.h globals.h olc.h olc_mpedit.h
olc_oedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_oedit.o: json.h structs.h types.h flags.h tables.h comm.h lookup.h db.h
olc_oedit.o: recycle.h utils.h interp.h affects.h act_info.h chars.h
olc_oedit.o: globals.h olc.h memory.h olc_oedit.h
olc_redit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_redit.o: json.h structs.h types.h flags.h tables.h comm.h lookup.h db.h
olc_redit.o: recycle.h utils.h interp.h act_info.h chars.h objs.h find.h
olc_redit.o: globals.h olc.h olc_medit.h olc_oedit.h memory.h olc_redit.h
olc_save.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_save.o: json.h structs.h types.h flags.h tables.h lookup.h utils.h db.h
olc_save.o: mob_cmds.h comm.h interp.h globals.h olc.h olc_save.h
portals.o: lookup.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
portals.o: json.h structs.h types.h flags.h tables.h recycle.h utils.h
portals.o: memory.h portals.h
recycle.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
recycle.o: json.h structs.h types.h flags.h tables.h utils.h lookup.h db.h
recycle.o: affects.h objs.h chars.h globals.h memory.h recycle.h
rooms.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
rooms.o: structs.h types.h flags.h tables.h lookup.h utils.h objs.h interp.h
rooms.o: rooms.h
save.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
save.o: structs.h types.h flags.h tables.h lookup.h colour.h db.h utils.h
save.o: skills.h board.h chars.h objs.h globals.h memory.h save.h
sha256.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
sha256.o: json.h structs.h types.h flags.h tables.h sha256.h
signal.o: signal.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
signal.o: json.h structs.h types.h flags.h tables.h
skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
skills.o: json.h structs.h types.h flags.h tables.h interp.h magic.h
skills.o: recycle.h lookup.h affects.h comm.h utils.h fight.h act_info.h
skills.o: act_skills.h chars.h skills.h
special.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
special.o: json.h structs.h types.h flags.h tables.h fight.h comm.h interp.h
special.o: magic.h lookup.h skills.h db.h act_fight.h act_comm.h act_move.h
special.o: chars.h objs.h spell_aff.h globals.h special.h
spell_aff.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_aff.o: json.h structs.h types.h flags.h tables.h utils.h affects.h
spell_aff.o: comm.h chars.h fight.h groups.h skills.h objs.h recycle.h
spell_aff.o: lookup.h spell_aff.h
spell_create.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_create.o: json.h structs.h types.h flags.h tables.h find.h comm.h db.h
spell_create.o: objs.h utils.h chars.h globals.h memory.h spell_create.h
spell_cure.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_cure.o: json.h structs.h types.h flags.h tables.h skills.h comm.h
spell_cure.o: utils.h lookup.h fight.h chars.h objs.h spell_cure.h
spell_info.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_info.o: json.h structs.h types.h flags.h tables.h comm.h utils.h
spell_info.o: lookup.h chars.h recycle.h db.h interp.h act_info.h affects.h
spell_info.o: objs.h globals.h spell_info.h
spell_misc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_misc.o: json.h structs.h types.h flags.h tables.h comm.h utils.h
spell_misc.o: lookup.h db.h affects.h objs.h interp.h chars.h globals.h
spell_misc.o: spell_misc.h
spell_move.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_move.o: json.h structs.h types.h flags.h tables.h find.h chars.h comm.h
spell_move.o: interp.h act_info.h db.h fight.h objs.h spell_move.h
spell_npc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_npc.o: json.h structs.h types.h flags.h tables.h utils.h lookup.h
spell_npc.o: effects.h fight.h comm.h chars.h spell_npc.h
spell_off.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_off.o: json.h structs.h types.h flags.h tables.h utils.h fight.h comm.h
spell_off.o: affects.h lookup.h db.h spell_aff.h skills.h chars.h objs.h
spell_off.o: globals.h spell_off.h
string.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
string.o: json.h structs.h types.h flags.h tables.h comm.h utils.h interp.h
string.o: olc.h db.h olc_mpedit.h globals.h memory.h
tables.o: skills.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
tables.o: json.h structs.h types.h flags.h tables.h nanny.h lookup.h
tables.o: recycle.h colour.h board.h special.h effects.h json_tbl.h magic.h
tables.o: spell_aff.h spell_create.h spell_cure.h spell_info.h spell_misc.h
tables.o: spell_move.h spell_npc.h spell_off.h
types.o: merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
types.o: structs.h types.h flags.h tables.h
update.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
update.o: json.h structs.h types.h flags.h tables.h interp.h music.h comm.h
update.o: affects.h utils.h skills.h mob_prog.h fight.h save.h magic.h db.h
update.o: act_player.h chars.h objs.h lookup.h globals.h update.h
utils.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
utils.o: json.h structs.h types.h flags.h tables.h comm.h db.h interp.h
utils.o: chars.h globals.h utils.h
wiz_im.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_im.o: json.h structs.h types.h flags.h tables.h interp.h db.h utils.h
wiz_im.o: comm.h lookup.h recycle.h do_sub.h mob_cmds.h act_info.h chars.h
wiz_im.o: objs.h rooms.h find.h affects.h globals.h memory.h wiz_im.h
wiz_l1.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l1.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_l1.o: save.h fight.h ban.h utils.h act_player.h wiz_l4.h chars.h find.h
wiz_l1.o: descs.h globals.h wiz_l1.h
wiz_l2.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l2.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_l2.o: ban.h utils.h recycle.h lookup.h chars.h rooms.h find.h globals.h
wiz_l2.o: wiz_l2.h
wiz_l3.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l3.o: structs.h types.h flags.h tables.h comm.h interp.h fight.h utils.h
wiz_l3.o: chars.h find.h descs.h globals.h wiz_l3.h
wiz_l4.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l4.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_l4.o: save.h fight.h utils.h lookup.h skills.h affects.h chars.h objs.h
wiz_l4.o: find.h recycle.h descs.h globals.h wiz_l4.h
wiz_l5.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l5.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_l5.o: save.h fight.h utils.h lookup.h skills.h affects.h do_sub.h
wiz_l5.o: recycle.h act_info.h chars.h objs.h rooms.h find.h memory.h
wiz_l5.o: globals.h wiz_l5.h
wiz_l6.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l6.o: structs.h types.h flags.h tables.h comm.h interp.h chars.h rooms.h
wiz_l6.o: find.h globals.h memory.h wiz_l6.h
wiz_l7.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l7.o: structs.h types.h flags.h tables.h comm.h interp.h utils.h chars.h
wiz_l7.o: rooms.h find.h globals.h wiz_l7.h
wiz_l8.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l8.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_l8.o: fight.h utils.h act_info.h chars.h rooms.h find.h memory.h wiz_l8.h
wiz_ml.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_ml.o: json.h structs.h types.h flags.h tables.h db.h comm.h interp.h
wiz_ml.o: save.h fight.h utils.h act_info.h chars.h rooms.h find.h descs.h
wiz_ml.o: boot.h memory.h globals.h wiz_ml.h
