#rm ./test01.out
./a.exe > ./test01.out <<!
12345
1
SimulationResults
2
1
1
2
1
1
3
1
1
!
diff -s ./test01.out ./test01.good

# PS C:\home\auld\src\IEVS> .\a.exe
# IEVS (Warren D. Smith's infinitely extendible voting system comparator) at your service!
# Version=3.250000  Year=2021  Month=5
# 
# Constants:
# sizeof(uint)=4 bytes    sizeof(uint32)=4        sizeof(uint64)=8        sizeof(real)=8
# sizeof(edata)=2261000   MaxNumCands=32  MaxNumVoters=2048       MaxNumIssues=8
# NumMethods=34   NumCoreMethods=12       TRUE=1  FALSE=0
# ArtinPrime=196549
# BROutputMode=0
# 
# Please enter random seed (0 causes machine to auto-generate from TimeOfDay)
# 12345 <<-- line 3 above
# Random generator initialized with seed=12345:
# 0.837376 0.566012 0.455470 0.753659 0.665476 0.541795 0.724432
# 
# Computing sequence LCM(1,2,...N) for N=1..22:
# LCMfact[23]=0
# What do you want to do?
# 1=BayesianRegrets
# 2=YeePicture
# 3=Test RandGen (and other self-tests)
# 4=Tally an election with votes you enter
# 1   <<<<-- line 4 above
# Answer a sequence of questions indicating what output format you want for
# the regret tables:
# 0. Simulation output file name? enter name or <return> is not desired.
# SimulationResults <<<-- line 5 above
# Simulatin output will be written to: SimulationResults.out
# I. voting methods (1) sorted-by-regret or (2) by voting-method-number?
# [The latter, while arbitrary, has the advantage of invariance throughout the run.]
# 2   <<<-- line 6 above
# sorting by voting method number.
# II. output (1) plain ASCII (2) TeX table formatting (3) HTML table formatting?
# 1   <<<<--- line 7 above
# plain ASCII.
# III. BRs (1) plain (2) normalized so SociallyBest=0, RandomWinner=1
#      (3) normalized so SociallyBest=100, RandomWinner=0, WorseThanRandom<0?
# 1   <<<<--- line 8 above
# plain.
# IV. Error bars (1) on every BR value (2) omit & only compute for RandomWinner
# 2   <<<<--- line 9 above
# omit error bars.
# V. Print Agreement counts with (1) true-utility(undistorted) Condorcet Winners, (2) vote-based CWs    
# 1   <<<<--- line 10 above
# true-utility CWs.
# VI. Print out intermethod winner-agreement-count tables (1) no, (2) yes
# 1  <<<<--- line 11 above
# NO agree-count tables.
# VII. Print out regrets for (1) no, (2) only best 10, (3) all methods
# 3  <<<--- line 12 above
# All regrets printed (maximum verbosity).
# VIII. (1) All parameter knob-settings, or (2) restricted ranges?
# 1  <<<--- line 13 above
# All settings.
# IX. (1) Machine or (2) Real-world-based utilities?
# 1  <<<--- line 14 above
# Machine.
# PS C:\home\auld\src\IEVS>