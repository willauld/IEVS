/* Parse IEVS configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ini/ini.h"

typedef struct
{
    int version;
    const char *name;
    const char *email;
} configuration;

typedef struct
{
    unsigned int operation; /*1.Regrets2.Picture3.RandomTests4.ManualTally*/
    /*operation -- 1.Regrets*/
    unsigned int BROutputMode; /*init to zero*/
    unsigned int seed;
    /* below init to -1 to signify unset */
    const char *outputfile;
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
    /*operation -- 4.ManualTally*/

} ievs_config;

char mylower[200];
/* over writes mylower with lower case of input string */
char *static_lower_str(const char *str)
{
    int i;
    for (i = 0; str[i]; i++)
    {
        mylower[i] = tolower(str[i]);
    }
    mylower[i] = '\0';
    return mylower;
}

static int ievs_handler(void *user, const char *section, const char *name,
                        const char *cvalue)
{
    ievs_config *pconfig = (ievs_config *)user;
    char *value = static_lower_str(cvalue);

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("", "seed"))
    {
        pconfig->seed = atoi(value);
    }
    else if (MATCH("regrets", "outputfile"))
    {
        pconfig->outputfile = strdup(value);
    }
    else if (MATCH("regrets", "honfraclower"))
    {
        pconfig->honfraclower = atoi(value);
    }
    else if (MATCH("regrets", "honfracupper"))
    {
        pconfig->honfracupper = atoi(value);
    }
    else if (MATCH("regrets", "candnumlower"))
    {
        pconfig->candnumlower = atoi(value);
    }
    else if (MATCH("regrets", "candnumupper"))
    {
        pconfig->candnumupper = atoi(value);
    }
    else if (MATCH("regrets", "votnumlower"))
    {
        pconfig->votnumlower = atoi(value);
    }
    else if (MATCH("regrets", "votnumupper"))
    {
        pconfig->votnumupper = atoi(value);
    }
    else if (MATCH("regrets", "numelections2try"))
    {
        pconfig->numelections2try = atoi(value);
    }
    else if (MATCH("regrets", "utilnumlower"))
    {
        pconfig->utilnumlower = atoi(value);
    }
    else if (MATCH("regrets", "utilnumupper"))
    {
        pconfig->utilnumupper = atoi(value);
    }
    else if (MATCH("regrets", "real_world_based_utilities"))
    {
        pconfig->real_world_based_utilities = atoi(value);
    }
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
    else if (MATCH("regrets", "htmlmode"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = HTMLMODE;
        }
    }
    else if (MATCH("regrets", "texmode"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = TEXMODE;
        }
    }
    else if (MATCH("regrets", "normalizeregrets"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = NORMALIZEREGRETS;
        }
    }
    else if (MATCH("regrets", "sortmode"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = SORTMODE;
        }
    }
    else if (MATCH("regrets", "shentrupvsr"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = SHENTRUPVSR;
        }
    }
    else if (MATCH("regrets", "omiterrorbars"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = OMITERRORBARS;
        }
    }
    else if (MATCH("regrets", "vbcondmode"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = VBCONDMODE;
        }
    }
    else if (MATCH("regrets", "doagreetables"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = DOAGREETABLES;
        }
    }
    else if (MATCH("regrets", "allmeths"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = ALLMETHS;
        }
    }
    else if (MATCH("regrets", "top10meths"))
    {
        if (strcmp(value, "true") == 0)
        {
            pconfig->BROutputMode = TOP10METHS;
        }
    }
    else
    {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

static int handler(void *user, const char *section, const char *name,
                   const char *value)
{
    configuration *pconfig = (configuration *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("protocol", "version"))
    {
        pconfig->version = atoi(value);
    }
    else if (MATCH("user", "name"))
    {
        pconfig->name = strdup(value);
    }
    else if (MATCH("user", "email"))
    {
        pconfig->email = strdup(value);
    }
    else
    {
        return 0; /* unknown section/name, error */
    }
    return 1;
}

static int dumper(void *user, const char *section, const char *name,
                  const char *value)
{
    static char prev_section[50] = "";

    if (strcmp(section, prev_section))
    {
        printf("%s[%s]\n", (prev_section[0] ? "\n" : ""), section);
        strncpy(prev_section, section, sizeof(prev_section));
        prev_section[sizeof(prev_section) - 1] = '\0';
    }
    printf("%s = %s\n", name, value);
    return 1;
}

int dump_ini(int argc, char *argv[])
{
    int error;

    error = ini_parse(argv[1], dumper, NULL);
    if (error < 0)
    {
        printf("Can't read '%s'!\n", argv[1]);
        return 2;
    }
    else if (error)
    {
        printf("Bad config file (first error on line %d)!\n", error);
        return 3;
    }
    return 0;
}

int do_ini(int argc, char *argv[])
{
    ievs_config config;
    config.seed = 12345;
    config.outputfile = NULL;
    config.BROutputMode = 0; /*init to zero*/
    /* below init to -1 to signify unset */
    config.honfraclower = -1;
    config.honfracupper = -1;
    config.candnumlower = -1;
    config.candnumupper = -1;
    config.votnumlower = -1;
    config.votnumupper = -1;
    config.numelections2try = -1;
    config.utilnumlower = -1;
    config.utilnumupper = -1;
    config.real_world_based_utilities = -1; /*LoadEldataFiles();RWBRDriver();*/

    if (ini_parse(argv[1], ievs_handler, &config) < 0)
    {
        printf("Can't load '%s'\n", argv[1]);
        return 1;
    }
    printf("-=-=-=-=-\n");
    printf("seed: %d, outputfile: %s\n", config.seed, config.outputfile);
    printf("honfrac lower: %d, upper: %d\n", config.honfraclower, config.honfracupper);
    printf("candnum lower: %d, upper: %d\n", config.candnumlower, config.candnumupper);
    printf("votnum lower: %d, upper: %d\n", config.votnumlower, config.votnumupper);
    printf("utilnum lower: %d, upper: %d\n", config.utilnumlower, config.utilnumupper);
    printf("numelections2try: %d, real_world_utils: %d\n", config.numelections2try, config.real_world_based_utilities);
    printf("BROutputMode: 0x%X\n", config.BROutputMode);
    if (config.outputfile)
        free((void *)config.outputfile);
    return 0;
}

int do_test_ini(int argc, char *argv[])
{
    configuration config;
    config.version = 0; /* set defa:wults */
    config.name = NULL;
    config.email = NULL;

    if (ini_parse("test.ini", handler, &config) < 0)
    {
        printf("Can't load 'test.ini'\n");
        return 1;
    }
    printf("Config loaded from 'test.ini': version=%d, name=%s, email=%s\n",
           config.version, config.name, config.email);

    if (config.name)
        free((void *)config.name);
    if (config.email)
        free((void *)config.email);

    return 0;
}
