#include <stdio.h>
#include <string.h>
#include "yjson.h"
void testNumber()
{
    infn;
    char s[] = "12";
    int v1;
    size_t step;
    v1 = yParseDigit(s, &step);
    debug("v %ld step %d\n", v1, step);
    char s2[] = "-123";
    v1 = yParseInt(s2, &step);
    debug("v %d step %d\n", v1, step);
    debug("----number basic over----\n");
    char sarray[][20] = {"123", "-123", "-123.1", "-123.1e1", "-123.1E-1", "0123"};
    unsigned int i;
    double d;
    for (i = 0; i < sizeof(sarray) / sizeof(sarray[0]); i++) {
        d = yParseNumber(sarray[i], &step);
        debug("s %10s -> d %10.3f step %d\n", sarray[i], d, step);
    }
    debug("----number over----\n");
    yNumber* p;
    for (i = 0; i < sizeof(sarray) / sizeof(sarray[0]); i++) {
        p = yParse(sarray[i], &step);
        debug("s %10s -> d %10.3f step %d\n", sarray[i], p->n, step);
    }
    outfn;
}

void testString()
{
    infn;
    yString* p;
    char sa[][20] = {"\"0123\"", "\"0123\\t56789\"",
    "\"\\\"\"", // only one quote
    "\"\\\\\"", // only one trans
    "\"\\/\"", // only one backslash /
    "\"\\\n\"", // only new line
    "\"\u27af\"", // unicode char test
    };
    size_t i, step;
    for (i = 0; i < sizeof(sa) / sizeof(sa[0]); i++) {
        debug("test case %d s %s\n", i, sa[i]);
        p = yParse(sa[i], &step);
        debug("s %20s -> %20s\n", sa[i], p->s);
    }
    outfn;
}
void testParse()
{
    char s[] = "\"123\"";
    yType* p = yParse(s, NULL);
    yDump(p);
    // XXX: rdata section, cannot write, so yStringSet maybe error;
    // yType* p1 = yParse("\"123\"");
    // yDump(p1);
    char s1[] = "[1,2,3]";
    yType* p1 = yParse(s1, NULL);
    yDump(p1);
}
int main()
{
    enabledebug();
    puts("----begin----");
    testNumber();
    puts("----number done----");
    testString();
    puts("----string done----");
    // testParse();
    puts("----parser done----");
    puts("----over ----");
    return 0;
}
