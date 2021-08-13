
GDB_FLAGS := -g
PG_FLAGS := -pg
INI_FLAGS := -DINCLUDE_INI_FILE
MPI_FLAGS := -DDO_MPI -DDO_MPI_RAND
MPI_FLAGS_NO_RANDOM := -DDO_MPI
OPENMP_FLAGS = -fopenmp
LD_FLAGS := -lm
C_FLAGS := -Wall
MPI_SPRNG := -L./sprng2.0/include ./sprng2.0/lib/libsprng.a -DDO_MPI_SPRNG
SPRNG := -L./sprng2.0/include ./sprng2.0/lib/libsprng.a -DDO_S_SPRNG

ifeq ($(OS),Windows_NT)
	RM := del
	CP := copy
else
	RM := rm
	CP := cp
endif

ievs.exe: IEVS.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $^ $(LD_FLAGS) -o $@ 

all: ievs.exe ievs_ini.exe ievs_mpi.exe a.exe original.exe #ievs_omp.exe

test_mp_all: ievs_mpi_sprng.exe ievs_mpi_blc32_mpi_rand.exe ievs_mpi_blc32.exe ievs_ini_sprng.exe ievs_ini.exe

a.exe: ievs_ini.exe
	$(CP) ievs_ini.exe a.exe

handleini.c: handleini.h

ini/ini.c: ini/ini.h

ievs_mpi_sprng.exe: IEVS.c handleini.c ini/ini.c
	mpicc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $(MPI_FLAGS)  $^ $(LD_FLAGS) $(MPI_SPRNG) -o $@ 

ievs_mpi_blc32_mpi_rand.exe: IEVS.c handleini.c ini/ini.c
	mpicc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $(MPI_FLAGS) $^ $(LD_FLAGS) -o $@ 

ievs_mpi_blc32.exe: IEVS.c handleini.c ini/ini.c
	mpicc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $(MPI_FLAGS_NO_RANDOM) $^ $(LD_FLAGS) -o $@ 

ievs_omp.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $(OPENMP_FLAGS) $^ $(LD_FLAGS) -o $@ 

ievs_ini_sprng.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $^ $(LD_FLAGS) $(SPRNG) -o $@ 

ievs_ini.exe: IEVS.c handleini.c ini/ini.c
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $(INI_FLAGS) $^ $(LD_FLAGS) -o $@ 

original.exe: original.c
	#gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $^ $(LD_FLAGS) "-Wl,--stack,4194304" -o $@ 
	gcc $(C_FLAGS) $(GDB_FLAGS) $(PG_FLAGS) $^ $(LD_FLAGS) -o $@ 

version:
	gcc --version

clean: 
	-$(RM) -f ievs.exe ievs_ini.exe a.exe ievs_omp.exe ievs_mpi.exe


