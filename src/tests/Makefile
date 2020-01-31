CC      = gcc
PROF    = -g -O
NOCRYPT =
C_FLAGS =  -Wall -Wno-trigraphs -Wno-unused-function $(PROF) $(NOCRYPT)
L_FLAGS =  $(PROF)
LIBS    = -lcrypt
EXE     = ./tests

O_FILES = \
    main.o utils.o ext_tests.o

rom: $(O_FILES)
	$(CC) $(L_FLAGS) -o $(EXE) $(O_FILES) $(LIBS)

depend:
	makedepend -Y. *.c 2>/dev/null

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

clean:
	rm *.o
