
C_FLAGS := -g -pg -DINCLUDE_INI_FILE

ievs.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $^  -o $@ 