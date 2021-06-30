
GDB_FLAGS := -g
#PG_FLAGS := -pg
INI_FLAGS := -DINCLUDE_INI_FILE
LD_FLAGS := -lm

ifeq ($(OS),Windows_NT)
	RM := del
	CP := copy
else
	RM := rm
	CP := cp
endif

ievs.exe: IEVS.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $^ $(LD_FLAGS) -o $@ 

all: ievs.exe ievs_ini.exe a.exe

a.exe: ievs.exe
	$(CP) ievs.exe a.exe

handleini.c: handleini.h

ini/ini.c: ini/ini.h

ievs_ini.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $^ $(LD_FLAGS) -o $@ 

version:
	gcc --version

clean: 
	-$(RM) *.exe


