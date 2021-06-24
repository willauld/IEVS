gcc .\IEVS.c
gcc -pg -DINCLUDE_INI_FILE .\handleini.c .\ini\ini.c .\IEVS.c -o ievs_ini.exe
# old gcc "-Wl,--stack,4194304" .\IEVS.c
