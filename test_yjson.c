#include <stdio.h>
#include <string.h>
#include "yjson.h"
void testNumber()
{
    char s[] = "12";
    int v1;
    size_t step;
    v1 = yParseDigit(s, &step);
    debug("v %ld step %d\n", v1, step);
    char s2[] = "-123";
    v1 = yParseInt(s2, &step);
    debug("v %d step %d\n", v1, step);
    debug("----number basic over----\n");
    char sarray[][20] = {"123", "-123", "-123.1", "-123.1e1", "-123.1E-1",
        "0123"};
    size_t i;
    double d;
    for (i = 0; i < sizeof(sarray) / sizeof(sarray[0]); i++) {
        d = yParseNumber(sarray[i], &step);
        debug("s %10s %d -> d %10.3f step %d\n",
                sarray[i], strlen(sarray[i]), d, step);
    }
    debug("----number yParseNumber over----\n");
    yNumber* p;
    for (i = 0; i < sizeof(sarray) / sizeof(sarray[0]); i++) {
        p = yParse(sarray[i], &step);
        debug("s %10s %d -> d %10.3f step %d\n",
                sarray[i], strlen(sarray[i]), p->n, step);
    }
    debug("----number yParse passed----\n");
}

void testString()
{
    yString* p;
    char sa[][20] = {"\"0123\"", "\"0123\\t56789\"",
    "\"\\\"\"", // only one quote
    "\"\\\\\"", // only one trans
    "\"\\/\"", // only one backslash /
    "\"a\\n\"", // only new line
    "\"\u27af\"", // unicode char test
    };
    size_t step;
    for (size_t i = 0; i < sizeof(sa) / sizeof(sa[0]); i++) {
        p = yParse(sa[i], &step);
        debug("s %12s -> %12s\n", sa[i], p->s);
        // for (size_t j = 0; j < strlen(p->s); j++) {
        //     debug("%x ", p->s[j]);
        // }
        // debug("\n");
    }
    debug("----string yParse passed----\n");
}
void testArray()
{
    yArray* p;
    char sa[][50] = {
        "[1,2,3]",
        "[11,2e1,-4]",
        "[11,\"123\",-4]",
        "[   11  ,  \"123\"  ,  -4  ]",
        "[   11  ,  \"a  12  3\"  ,  -4  ]",
        "[   11  ,  [ 1, 2 ],  -4  ]",
        "[   true, false, null]",
        "[   true, false, null, [TRUE, false]]",
    };
    size_t i, step;
    for (i = 0; i < sizeof(sa) / sizeof(sa[0]); i++) {
        debug("s %s ->\t", sa[i]);
        p = yParse(sa[i], &step);
        yDisplay(p);
    }
    debug("----array yParse passed----\n");
}
void testSymbol()
{
    ySymbol* p;
    char sa[][50] = {
        "true",
        "True",
        "TRUE",
        "false",
        "null",
        "invalid"
    };
    size_t step;
    for (size_t i = 0; i < sizeof(sa) / sizeof(sa[0]); i++) {
        p = yParse(sa[i], &step);
        if (p != NULL) {
            debug("s %s -> ", sa[i]);
            yDisplay(p);
        } else {
            debug("s %s -> %p\n", sa[i], p);
        }
    }
    debug("----symbol yParse passed----\n");
}
void testObject()
{
    yObject* p;
    char sa[][50] = {
        "{\"a\":1}",
        "{\"a\":\"abc\", \"abc\":true}",
        "{\"a\":\"abc\", \"abc\":true, \"def\":[\"abc\"]}",
        "[123, \"abc\", true, {\"a\":1}]",
        "{ \"elm\": {\"a\": 1} }",
        "{ \"e[]{}m\": {\"a\": 1} }",
        "{    \"e[]{}m\"   :    {   \"a\"  :   1  }   }  ",
    };
    size_t step;
    for (size_t i = 0; i < sizeof(sa) / sizeof(sa[0]); i++) {
        p = yParse(sa[i], &step);
        if (p != NULL) {
            debug("s %s -> ", sa[i]);
            yDisplay(p);
        } else {
            debug("s %s -> %p\n", sa[i], p);
        }
    }
    debug("----ojbect yParse passed----\n");
}
int main()
{
    enabledebug();
    puts("----begin----");
    testNumber();
    testString();
    testArray();
    testSymbol();
    testObject();
    // testParse();
    puts("----parser done----");
    puts("----over ----");
    return 0;
}
