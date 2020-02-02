CC      = gcc
PROF    = -g -O
NOCRYPT =
C_FLAGS =  -Wall -Wno-trigraphs -Werror $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
LIBS    = -lcrypt
EXE     = ../bin/rom

O_FILES = \
    act_board.o act_comm.o act_conf.o act_fight.o act_group.o act_info.o \
    act_move.o act_obj.o act_olc.o act_player.o act_shop.o act_skills.o \
    affects.o ban.o board.o boot.o chars.o colour.o comm.o db_old.o db.o \
    descs.o do_sub.o effects.o ext_flags.o fight.o find.o flags.o fread.o fwrite.o \
    globals.o groups.o interp.o items.o json.o json_import.o json_objr.o \
    json_objw.o json_read.o json_write.o json_tblr.o json_tblw.o lookup.o \
    magic.o materials.o memory.o mob_cmds.o mob_prog.o mobiles.o music.o \
    nanny.o objs.o olc.o olc_aedit.o olc_hedit.o olc_medit.o olc_mpedit.o \
    olc_oedit.o olc_redit.o olc_save.o players.o portals.o quickmud.o \
    recycle.o rooms.o save.o signal.o skills.o special.o spell_aff.o \
    spell_create.o spell_cure.o spell_info.o spell_misc.o spell_move.o \
    spell_npc.o spell_off.o string.o tables.o types.o update.o utils.o \
    wiz_im.o wiz_l1.o wiz_l2.o wiz_l3.o wiz_l4.o wiz_l5.o wiz_l6.o wiz_l7.o \
    wiz_l8.o wiz_ml.o

rom: $(O_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(O_FILES) $(LIBS)

depend:
	makedepend -Y. *.c 2>/dev/null

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o
# DO NOT DELETE

act_board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_board.o: json.h structs.h types.h flags.h tables.h ext_flags.h board.h
act_board.o: comm.h recycle.h utils.h lookup.h interp.h act_info.h chars.h
act_board.o: memory.h globals.h act_board.h
act_comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_comm.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
act_comm.o: recycle.h lookup.h comm.h utils.h mob_prog.h db.h do_sub.h
act_comm.o: chars.h find.h globals.h players.h memory.h act_comm.h
act_conf.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_conf.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
act_conf.o: interp.h colour.h groups.h comm.h utils.h lookup.h do_sub.h
act_conf.o: chars.h memory.h act_conf.h
act_fight.o: affects.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_fight.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
act_fight.o: comm.h lookup.h interp.h groups.h mob_prog.h recycle.h fight.h
act_fight.o: act_comm.h chars.h find.h players.h act_fight.h
act_group.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_group.o: json.h structs.h types.h flags.h tables.h ext_flags.h groups.h
act_group.o: utils.h comm.h db.h chars.h find.h globals.h act_group.h
act_info.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_info.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
act_info.o: magic.h recycle.h lookup.h utils.h groups.h db.h fight.h update.h
act_info.o: comm.h save.h do_sub.h act_comm.h act_obj.h chars.h rooms.h
act_info.o: objs.h find.h spell_info.h globals.h memory.h items.h players.h
act_info.o: act_info.h
act_move.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_move.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
act_move.o: lookup.h utils.h comm.h mob_prog.h affects.h db.h fight.h
act_move.o: groups.h act_info.h chars.h rooms.h objs.h find.h globals.h
act_move.o: items.h players.h act_move.h
act_obj.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_obj.o: json.h structs.h types.h flags.h tables.h ext_flags.h affects.h
act_obj.o: utils.h comm.h db.h fight.h groups.h mob_prog.h save.h magic.h
act_obj.o: act_group.h act_comm.h act_move.h recycle.h chars.h objs.h rooms.h
act_obj.o: find.h globals.h lookup.h items.h players.h act_obj.h
act_olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_olc.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
act_olc.o: lookup.h interp.h utils.h db.h comm.h olc_save.h act_info.h
act_olc.o: chars.h globals.h items.h mobiles.h rooms.h objs.h olc_aedit.h
act_olc.o: olc_hedit.h olc_medit.h olc_mpedit.h olc_oedit.h olc_redit.h olc.h
act_olc.o: act_olc.h
act_player.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_player.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
act_player.o: comm.h save.h utils.h fight.h interp.h recycle.h chars.h
act_player.o: descs.h memory.h globals.h act_player.h
act_shop.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_shop.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
act_shop.o: comm.h interp.h utils.h db.h groups.h chars.h objs.h find.h
act_shop.o: act_comm.h spell_cure.h materials.h globals.h memory.h items.h
act_shop.o: mobiles.h players.h rooms.h act_shop.h
act_skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
act_skills.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
act_skills.o: utils.h comm.h interp.h magic.h fight.h lookup.h recycle.h
act_skills.o: act_comm.h chars.h find.h players.h memory.h act_skills.h
affects.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
affects.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
affects.o: utils.h lookup.h chars.h affects.h
ban.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
ban.o: structs.h types.h flags.h tables.h ext_flags.h recycle.h utils.h
ban.o: fread.h interp.h comm.h chars.h globals.h memory.h fwrite.h ban.h
board.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
board.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
board.o: fread.h utils.h comm.h recycle.h lookup.h chars.h descs.h memory.h
board.o: globals.h board.h
boot.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
boot.o: json.h structs.h types.h flags.h tables.h ext_flags.h signal.h
boot.o: globals.h utils.h interp.h db.h descs.h recycle.h memory.h save.h
boot.o: chars.h olc.h nanny.h update.h comm.h rooms.h quickmud.h act_info.h
boot.o: boot.h
chars.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
chars.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h comm.h
chars.o: objs.h affects.h lookup.h magic.h db.h groups.h fight.h interp.h
chars.o: recycle.h rooms.h mob_prog.h act_move.h act_info.h act_group.h
chars.o: wiz_l6.h materials.h globals.h memory.h board.h update.h items.h
chars.o: players.h mobiles.h chars.h
colour.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
colour.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
colour.o: chars.h colour.h
comm.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
comm.o: json.h structs.h types.h flags.h tables.h ext_flags.h colour.h
comm.o: recycle.h utils.h fight.h interp.h db.h olc.h save.h mob_prog.h
comm.o: lookup.h act_info.h chars.h rooms.h objs.h descs.h globals.h
comm.o: players.h comm.h
db.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
db.o: structs.h types.h flags.h tables.h ext_flags.h utils.h recycle.h
db.o: affects.h lookup.h json_objw.h json_write.h json_read.h json_import.h
db.o: music.h ban.h board.h olc.h portals.h chars.h rooms.h objs.h db_old.h
db.o: update.h globals.h memory.h items.h mobiles.h skills.h fread.h db.h
db_old.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
db_old.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h
db_old.o: recycle.h utils.h interp.h lookup.h olc.h affects.h globals.h
db_old.o: memory.h items.h fread.h mobiles.h objs.h rooms.h db_old.h
descs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
descs.o: json.h structs.h types.h flags.h tables.h ext_flags.h ban.h chars.h
descs.o: colour.h comm.h db.h recycle.h utils.h interp.h lookup.h globals.h
descs.o: memory.h descs.h
do_sub.o: comm.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
do_sub.o: structs.h types.h flags.h tables.h ext_flags.h chars.h do_sub.h
effects.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
effects.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
effects.o: db.h utils.h comm.h magic.h update.h affects.h objs.h chars.h
effects.o: items.h effects.h
ext_flags.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
ext_flags.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
ext_flags.o: lookup.h utils.h
fight.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
fight.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
fight.o: lookup.h update.h utils.h db.h effects.h mob_prog.h comm.h save.h
fight.o: groups.h magic.h affects.h act_fight.h act_obj.h act_comm.h
fight.o: act_move.h chars.h objs.h find.h globals.h memory.h items.h
fight.o: players.h mobiles.h fight.h
find.o: interp.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
find.o: structs.h types.h flags.h tables.h ext_flags.h utils.h chars.h db.h
find.o: globals.h rooms.h find.h
flags.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
flags.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
flags.o: lookup.h utils.h
fread.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
fread.o: structs.h types.h flags.h tables.h ext_flags.h memory.h globals.h
fread.o: fread.h
fwrite.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
fwrite.o: json.h structs.h types.h flags.h tables.h ext_flags.h save.h
fwrite.o: fwrite.h
globals.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
globals.o: json.h structs.h types.h flags.h tables.h ext_flags.h
groups.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
groups.o: structs.h types.h flags.h tables.h ext_flags.h affects.h comm.h
groups.o: db.h chars.h globals.h groups.h
interp.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
interp.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
interp.o: comm.h db.h recycle.h chars.h find.h descs.h globals.h memory.h
interp.o: lookup.h act_board.h act_comm.h act_conf.h act_fight.h act_group.h
interp.o: act_info.h act_move.h act_obj.h act_player.h act_shop.h
interp.o: act_skills.h act_olc.h wiz_im.h wiz_l1.h wiz_l2.h wiz_l3.h wiz_l4.h
interp.o: wiz_l5.h wiz_l6.h wiz_l7.h wiz_l8.h wiz_ml.h olc.h mob_cmds.h
interp.o: interp.h
items.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
items.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h comm.h
items.o: interp.h objs.h lookup.h chars.h affects.h magic.h db.h recycle.h
items.o: rooms.h mob_prog.h groups.h globals.h music.h players.h fread.h
items.o: fwrite.h act_info.h act_move.h act_group.h items.h
json.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
json.o: memory.h
json_import.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h
json_import.o: macros.h json.h structs.h types.h flags.h tables.h ext_flags.h
json_import.o: utils.h recycle.h db.h lookup.h portals.h globals.h memory.h
json_import.o: json_objr.h json_import.h
json_objr.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_objr.o: json.h structs.h types.h flags.h tables.h ext_flags.h
json_objr.o: json_import.h recycle.h memory.h lookup.h portals.h db.h
json_objr.o: globals.h json_objr.h
json_objw.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_objw.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
json_objw.o: utils.h json_objw.h
json_read.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_read.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
json_read.o: json_read.h
json_tblr.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_tblr.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
json_tblr.o: colour.h memory.h json_import.h json_tblr.h
json_tblw.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_tblw.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
json_tblw.o: colour.h json_tblw.h
json_write.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
json_write.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
json_write.o: json_write.h
lookup.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
lookup.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
lookup.o: interp.h db.h recycle.h globals.h lookup.h
magic.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
magic.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
magic.o: affects.h comm.h fight.h utils.h chars.h magic.h
materials.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
materials.o: json.h structs.h types.h flags.h tables.h ext_flags.h
materials.o: materials.h
memory.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
memory.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
memory.o: globals.h recycle.h memory.h
mob_cmds.o: lookup.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
mob_cmds.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
mob_cmds.o: groups.h utils.h comm.h fight.h mob_prog.h db.h chars.h objs.h
mob_cmds.o: rooms.h find.h act_info.h globals.h mobiles.h mob_cmds.h
mob_prog.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
mob_prog.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
mob_prog.o: utils.h groups.h interp.h mob_cmds.h db.h chars.h objs.h find.h
mob_prog.o: globals.h mob_prog.h
mobiles.o: globals.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
mobiles.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
mobiles.o: recycle.h memory.h chars.h affects.h lookup.h fight.h interp.h
mobiles.o: act_fight.h magic.h mobiles.h
music.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
music.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h comm.h
music.o: interp.h globals.h lookup.h chars.h objs.h memory.h fread.h music.h
nanny.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
nanny.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
nanny.o: recycle.h lookup.h utils.h comm.h db.h save.h ban.h fight.h
nanny.o: act_info.h act_skills.h act_board.h act_obj.h chars.h objs.h descs.h
nanny.o: globals.h memory.h magic.h players.h rooms.h nanny.h
objs.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
objs.o: json.h structs.h types.h flags.h tables.h ext_flags.h affects.h
objs.o: utils.h chars.h db.h recycle.h comm.h lookup.h materials.h globals.h
objs.o: memory.h items.h objs.h
olc.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
olc.o: structs.h types.h flags.h tables.h ext_flags.h utils.h interp.h comm.h
olc.o: db.h recycle.h lookup.h magic.h chars.h memory.h act_olc.h olc_aedit.h
olc.o: olc_hedit.h olc_medit.h olc_mpedit.h olc_oedit.h olc_redit.h olc.h
olc_aedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_aedit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_aedit.o: lookup.h db.h recycle.h interp.h globals.h olc.h memory.h
olc_aedit.o: olc_aedit.h
olc_hedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_hedit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_hedit.o: lookup.h db.h recycle.h utils.h interp.h globals.h olc.h
olc_hedit.o: memory.h olc_hedit.h
olc_medit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_medit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_medit.o: lookup.h db.h recycle.h utils.h interp.h mob_cmds.h chars.h
olc_medit.o: globals.h olc.h memory.h mobiles.h olc_medit.h
olc_mpedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_mpedit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_mpedit.o: db.h recycle.h utils.h lookup.h chars.h globals.h olc.h
olc_mpedit.o: memory.h olc_mpedit.h
olc_oedit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_oedit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_oedit.o: lookup.h db.h recycle.h utils.h interp.h affects.h act_info.h
olc_oedit.o: chars.h globals.h olc.h memory.h items.h objs.h olc_oedit.h
olc_redit.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_redit.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
olc_redit.o: lookup.h db.h recycle.h utils.h interp.h act_info.h chars.h
olc_redit.o: objs.h find.h globals.h olc.h olc_medit.h olc_oedit.h memory.h
olc_redit.o: mobiles.h rooms.h olc_redit.h
olc_save.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
olc_save.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
olc_save.o: utils.h db.h mob_cmds.h comm.h interp.h globals.h olc.h items.h
olc_save.o: fread.h fwrite.h objs.h mobiles.h rooms.h olc_save.h
players.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
players.o: json.h structs.h types.h flags.h tables.h ext_flags.h chars.h
players.o: lookup.h objs.h affects.h memory.h utils.h globals.h comm.h save.h
players.o: magic.h players.h
portals.o: lookup.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
portals.o: json.h structs.h types.h flags.h tables.h ext_flags.h recycle.h
portals.o: utils.h memory.h portals.h
quickmud.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
quickmud.o: json.h structs.h types.h flags.h tables.h ext_flags.h fread.h
quickmud.o: quickmud.h
recycle.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
recycle.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
recycle.o: lookup.h db.h affects.h objs.h chars.h globals.h memory.h
recycle.o: recycle.h
rooms.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
rooms.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h utils.h
rooms.o: objs.h interp.h chars.h globals.h rooms.h
save.o: recycle.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
save.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h colour.h db.h
save.o: utils.h board.h chars.h objs.h globals.h memory.h items.h mobiles.h
save.o: players.h fread.h fwrite.h rooms.h save.h
sha256.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
sha256.o: json.h structs.h types.h flags.h tables.h ext_flags.h sha256.h
signal.o: signal.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
signal.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
signal.o: globals.h
skills.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
skills.o: json.h structs.h types.h flags.h tables.h ext_flags.h lookup.h
skills.o: utils.h skills.h
special.o: utils.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
special.o: json.h structs.h types.h flags.h tables.h ext_flags.h fight.h
special.o: comm.h interp.h magic.h lookup.h db.h act_fight.h act_comm.h
special.o: act_move.h chars.h objs.h spell_aff.h globals.h rooms.h special.h
spell_aff.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_aff.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
spell_aff.o: affects.h comm.h chars.h fight.h groups.h objs.h recycle.h
spell_aff.o: lookup.h items.h spell_aff.h
spell_create.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_create.o: json.h structs.h types.h flags.h tables.h ext_flags.h find.h
spell_create.o: comm.h db.h objs.h utils.h chars.h globals.h memory.h items.h
spell_create.o: spell_create.h
spell_cure.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_cure.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
spell_cure.o: utils.h lookup.h fight.h chars.h objs.h spell_cure.h
spell_info.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_info.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
spell_info.o: utils.h lookup.h chars.h recycle.h db.h interp.h act_info.h
spell_info.o: affects.h objs.h globals.h items.h memory.h spell_info.h
spell_misc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_misc.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
spell_misc.o: utils.h lookup.h db.h affects.h objs.h interp.h chars.h
spell_misc.o: globals.h items.h spell_misc.h
spell_move.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_move.o: json.h structs.h types.h flags.h tables.h ext_flags.h find.h
spell_move.o: chars.h comm.h interp.h act_info.h db.h fight.h objs.h items.h
spell_move.o: players.h rooms.h spell_move.h
spell_npc.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_npc.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
spell_npc.o: lookup.h effects.h fight.h comm.h chars.h spell_npc.h
spell_off.o: magic.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
spell_off.o: json.h structs.h types.h flags.h tables.h ext_flags.h utils.h
spell_off.o: fight.h comm.h affects.h lookup.h db.h spell_aff.h chars.h
spell_off.o: objs.h globals.h items.h players.h spell_off.h
string.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
string.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h
string.o: utils.h interp.h olc.h db.h olc_mpedit.h globals.h memory.h
tables.o: nanny.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
tables.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h recycle.h
tables.o: colour.h board.h special.h effects.h json_tblr.h json_tblw.h
tables.o: magic.h memory.h utils.h spell_aff.h spell_create.h spell_cure.h
tables.o: spell_info.h spell_misc.h spell_move.h spell_npc.h spell_off.h
types.o: merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
types.o: structs.h types.h flags.h tables.h ext_flags.h lookup.h utils.h
update.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
update.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h
update.o: music.h comm.h affects.h utils.h mob_prog.h fight.h save.h magic.h
update.o: db.h act_player.h chars.h objs.h lookup.h globals.h items.h
update.o: players.h rooms.h update.h
utils.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
utils.o: json.h structs.h types.h flags.h tables.h ext_flags.h comm.h db.h
utils.o: interp.h chars.h globals.h utils.h
wiz_im.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_im.o: json.h structs.h types.h flags.h tables.h ext_flags.h interp.h db.h
wiz_im.o: utils.h comm.h lookup.h recycle.h do_sub.h mob_cmds.h act_info.h
wiz_im.o: chars.h objs.h rooms.h find.h affects.h globals.h memory.h wiz_im.h
wiz_l1.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l1.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_l1.o: interp.h save.h fight.h ban.h utils.h act_player.h wiz_l4.h chars.h
wiz_l1.o: find.h descs.h globals.h wiz_l1.h
wiz_l2.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l2.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_l2.o: interp.h ban.h utils.h recycle.h lookup.h chars.h rooms.h find.h
wiz_l2.o: globals.h wiz_l2.h
wiz_l3.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l3.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
wiz_l3.o: fight.h utils.h chars.h find.h descs.h globals.h wiz_l3.h
wiz_l4.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l4.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_l4.o: interp.h save.h fight.h utils.h lookup.h affects.h chars.h objs.h
wiz_l4.o: find.h recycle.h descs.h globals.h mobiles.h wiz_l4.h
wiz_l5.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l5.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_l5.o: interp.h save.h fight.h utils.h lookup.h affects.h do_sub.h
wiz_l5.o: recycle.h act_info.h chars.h objs.h rooms.h find.h memory.h
wiz_l5.o: globals.h mobiles.h players.h wiz_l5.h
wiz_l6.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l6.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
wiz_l6.o: chars.h rooms.h find.h globals.h memory.h wiz_l6.h
wiz_l7.o: db.h merc.h basemud.h compat.h defs.h typedefs.h macros.h json.h
wiz_l7.o: structs.h types.h flags.h tables.h ext_flags.h comm.h interp.h
wiz_l7.o: utils.h chars.h rooms.h find.h globals.h wiz_l7.h
wiz_l8.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_l8.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_l8.o: interp.h fight.h utils.h act_info.h chars.h rooms.h find.h memory.h
wiz_l8.o: wiz_l8.h
wiz_ml.o: ./string.h merc.h basemud.h compat.h defs.h typedefs.h macros.h
wiz_ml.o: json.h structs.h types.h flags.h tables.h ext_flags.h db.h comm.h
wiz_ml.o: interp.h save.h fight.h utils.h act_info.h chars.h rooms.h find.h
wiz_ml.o: descs.h boot.h memory.h globals.h players.h mobiles.h objs.h
wiz_ml.o: quickmud.h wiz_ml.h
