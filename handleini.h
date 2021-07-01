

/* should be from an include file for here and IEVS.c */
#define HTMLMODE 1         /*output adding "HTML table" formatting commands*/
#define TEXMODE 2          /*output adding "TeX table" formatting commands*/
#define NORMALIZEREGRETS 4 /*Output "normalized" Regrets so RandomWinner=1, SociallyBest=0. */
#define SORTMODE 8         /*Output with voting methods in sorted order by increasing regrets.*/
#define SHENTRUPVSR 16     /*Output "Shentrup normalized" Regrets so RandomWinner=0%, SociallyBest=100%. */
#define OMITERRORBARS 32
#define VBCONDMODE 64 /*vote-based condorcet winner agreement counts; versus true utility based CWs*/
#define DOAGREETABLES 128
#define ALLMETHS 256
#define TOP10METHS 512

typedef struct
{
    unsigned int operation; /*1.Regrets2.Picture3.RandomTests4.ManualTally*/
    /*operation -- 1.Regrets*/
    unsigned int BROutputMode; /*init to zero*/
    unsigned int seed;
    /* below init to -1 to signify unset */
    char *outputfile;
    int honfraclower;
    int honfracupper;
    int candnumlower;
    int candnumupper;
    int votnumlower;
    int votnumupper;
    int numelections2try;
    int utilnumlower;
    int utilnumupper;
    int real_world_based_utilities; /*LoadEldataFiles();RWBRDriver();*/

    /*operation -- 2.Picture*/
    /*operation -- 3.RandomTests*/
    // do = now
    /*operation -- 4.ManualTally*/

} ievs_config;

int dump_ini(int argc, char *argv[]);
ievs_config *do_ini(int argc, char *argv[]);
