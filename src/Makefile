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
    affects.o ban.o board.o boot.o chars.o colour.o comm.o db_old.o db.o descs.o do_sub.o \
    effects.o fight.o find.o flags.o groups.o interp.o json.o json_obj.o \
    json_tbl.o lookup.o magic.o materials.o mob_cmds.o mob_prog.o music.o nanny.o \
    objs.o olc.o olc_aedit.o olc_hedit.o olc_medit.o olc_mpedit.o olc_oedit.o \
    olc_redit.o olc_save.o portals.o recycle.o rooms.o save.o signal.o \
    skills.o special.o spell_aff.o spell_create.o spell_cure.o spell_info.o \
    spell_misc.o spell_move.o spell_npc.o spell_off.o string.o tables.o \
    types.o update.o utils.o wiz_im.o wiz_l1.o wiz_l2.o wiz_l3.o wiz_l4.o \
    wiz_l5.o wiz_l6.o wiz_l7.o wiz_l8.o wiz_ml.o

rom: $(O_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(O_FILES) $(LIBS)

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o
