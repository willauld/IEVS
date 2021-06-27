
GDB_FLAGS := -g
PG_FLAGS := -pg
M_FLAGS := -m64 #-g -pg
GNU_FLAGS := -D__GNUC__ # to define gcc internal HUGE_VAL
INI_FLAGS := -DINCLUDE_INI_FILE

ievs.exe: IEVS.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(GNU_FLAGS) $(INI_FLAGS) $^  -o $@ 

all: ievs.exe ievs_ini.exe a.exe

a.exe: ievs.exe
	cp ievs.exe a.exe

handleini.c: handleini.h

ini/ini.c: ini/ini.h

ievs_ini.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(GNU_FLAGS) $(INI_FLAGS) $^  -o $@ 

version:
	gcc --version


