
C_FLAGS := -g -pg -DINCLUDE_INI_FILE
GNU_FLAGS := -D__GNUC__ # to define gcc internal HUGE_VAL


ievs.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $^  -o $@ 

version:
	gcc --version