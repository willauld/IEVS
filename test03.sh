rm ./test03.out
rm ./testSim03.out
./a.exe > ./test03.out <<!
12345
1
testSim03
2
1
3
2
1
1
1
2
50 100
2 7
1024 2048
59
1
4 10
!

diff -s ./test03.out ./test03.good
diff -s ./testSim03.out ./testSim03.good
diff -s ./testSim03.err ./testSimErr03.good

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
# 12345
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
# 1
# Answer a sequence of questions indicating what output format you want for
# the regret tables:
# 0. Simulation output file name? enter name or <return> is not desired.
# testSaturday22
# Simulatin output will be written to: testSaturday22.out
# 
# I. voting methods (1) sorted-by-regret or (2) by voting-method-number?
# [The latter, while arbitrary, has the advantage of invariance throughout the run.]
# 2
# sorting by voting method number.
# II. output (1) plain ASCII (2) TeX table formatting (3) HTML table formatting?
# 1
# plain ASCII.
# III. BRs (1) plain (2) normalized so SociallyBest=0, RandomWinner=1
#      (3) normalized so SociallyBest=100, RandomWinner=0, WorseThanRandom<0?
# 3
# Best=100, Random=0.
# IV. Error bars (1) on every BR value (2) omit & only compute for RandomWinner
# 2
# omit error bars.
# V. Print Agreement counts with (1) true-utility(undistorted) Condorcet Winners, (2) vote-based CWs
# 1
# true-utility CWs.
# VI. Print out intermethod winner-agreement-count tables (1) no, (2) yes
# 1
# NO agree-count tables.
# VII. Print out regrets for (1) no, (2) only best 10, (3) all methods
# 1
# No regrets printed (minimum verbosity).
# VIII. (1) All parameter knob-settings, or (2) restricted ranges?
# 2
# Restricted Ranges...
# Honesty fraction range - default is 0 100:
# 50 100
# Honesty fraction range [50, 100] chosen.
# Candidate Number range - default is 2 7 [but this range ignored if real-world dataset]:
# 2 7
# Candidate number range [2, 7] chosen.
# Voter Number range - default is 2 2048 [but this range ignored if real-world dataset:
# 1024 2048
# Voter number range [1024, 2048] chosen.
# Number of elections to try per scenario - default is 59
# 59
# Trying 59 elections per scenario.
# IX. (1) Machine or (2) Real-world-based utilities?
# 1
# Machine.
# Select which utility-generators you want (default 0 thru 15):
#  0: RandomNormalUtils
#  1: IssueDotProd[1]
#  2: IssueDotProd[2]
#  3: IssueDotProd[3]
#  4: IssueDotProd[4]
#  5: IssueDotProd[5]
#  6: IssueDistance[1]
#  7: IssueDistance[2]
#  8: IssueDistance[3]
#  9: IssueDistance[4]
# 10: IssueDistance[5]
# 11: IssueDistance[-1]
# 12: IssueDistance[-2]
# 13: IssueDistance[-3]
# 14: IssueDistance[-4]
# 15: IssueDistance[-5]
# 4 10
# Utility gens  [4, 10] chosen.
# PS C:\home\auld\src\IEVS>