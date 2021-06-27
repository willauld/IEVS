
C_FLAGS := -m64 #-g -pg
INI_FLAGS := -DINCLUDE_INI_FILE

ievs.exe: IEVS.c
	gcc $(C_FLAGS) $^  -o $@ 

all: ievs.exe ievs_ini.exe a.exe

a.exe: ievs.exe
	cp ievs.exe a.exe

handleini.c: handleini.h

ini/ini.c: ini/ini.h

ievs_ini.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(INI_FLAGS) $^  -o $@ 