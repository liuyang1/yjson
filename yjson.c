#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "stdlib.h"
#include "assert.h"
#include "ctype.h"

typedef enum {
    OK = 0,
    Error,
} ErrorCode;
typedef enum {
    eInvalid = 0,
    eObject,
    eArray,
    eString,
    eNumber,
} yMeta;
struct yType {
    yMeta meta;
    struct yType* parent;
};
typedef struct yType yTypeHead;
typedef struct yType yType;

typedef struct yObject {
    yTypeHead head;
    struct yType* firstchild;
} yObject;

typedef struct {
    yTypeHead head;
    yType* next;
    yType* prior;
} yArray;

typedef struct {
    yTypeHead head;
    char* s;
} yString;

typedef struct {
    yTypeHead head;
    double n;
} yNumber;

void yHeadInit(yTypeHead* h, yMeta meta, yType* parent)
{
    h->meta = meta;
    h->parent = parent;
}
size_t type2size[] = {0, sizeof(yObject), sizeof(yArray),
                      sizeof(yString), sizeof(yNumber)};
void* yAlloc(yMeta meta)
{
    size_t len = type2size[meta];
    yTypeHead* p = (yTypeHead*)malloc(len);
    bzero(p, len);
    yHeadInit(p, meta, NULL);
    return p;
}

void yFree(yType* p)
{
    // TODO:
    // need free child-pointer and next-pointer
    if (p != NULL) {
        free(p);
    }
}
int _debug = 1;
int _calllevel = 0;
void debug(const char *fmt, ...)
{
    if (_debug <= 0)
        return;
    static int spacecnt = 2;
    va_list args;
    va_start(args, fmt);
    int i;
    for (i = 0; i < _calllevel * spacecnt; i++) {
        putchar(' ');
    }
    vprintf(fmt, args);
    va_end(args);
}
#define infn            {_calllevel++;debug("%s >>>>\n", __FUNCTION__);}
#define outfn           {debug("%s <<<<\n", __FUNCTION__);_calllevel--;}
void yDump(yType* p)
{
    debug("----dump %p----\n", p);
    if (p == NULL) {
        debug("null pointer\n");
        return;
    }
    if (p->meta == eObject) {
        debug("object");
    } else if (p->meta == eArray) {
        debug("array");
    } else if (p->meta == eString) {
        yString* ps = (yString*)p;
        debug("str: %s\n", ps->s);
    } else if (p->meta == eNumber) {
        yNumber* pn = (yNumber*)p;
        debug("num: %lld\n", pn->n);
    } else {
        debug("unknown!");
    }
    debug("\n");
}
typedef enum {
    sObjectBegin = '{',
    sObjectEnd = '}',
    sArrayBegin = '[',
    sArrayEnd = ']',
    sDelimeter = ',',
    sStringBE = '"',
    sStringTrans = '\\',
    sStringTerm = '\0',
    sPosLogic = 0,
    sNegLogic = -1,
    sDebug = 1,
    sNoDebug = 0,
} yState;
int isNotZeroDigit(char c)
{
    return c >= '1' && c <= '9';
}
char* skipWhitespace(char* s)
{
    while (isspace(*s)) {
        s++;
    }
    return s;
}
ErrorCode yStringSet(yString* p, char* b, char* e)
{
    infn;
    debug("param: %p %p %p\n", p, b, e);
    debug("value: %s\n", b);
    if (p->s) {
        free(p->s);
    }
    char t;
    if (e != NULL) {
        t = *e;
        *e = 0;
    }
    // TODO: Trans char
    p->s = strdup(b);
    debug("%p %s %p %s\n", b, b, p->s, p->s);
    if (e != NULL) {
        *e = t;
    }
    outfn;
    return OK;
}

char* incStr(char *s)
{
    return s+1;
}
char* decStr(char *s)
{
    return s-1;
}
char* stringUntilFind(char* s, int direct, int (*fn)(char*))
{
    char* (*mvfn)(char*) = direct >= sPosLogic ? incStr : decStr;
    do {
        s = mvfn(s);
    } while (!(*fn)(s));
    return s;
}
char* forwardUntilFind(char* s, int (*fn)(char*))
{
    return stringUntilFind(s, sPosLogic, fn);
}
char* backwardUntilFind(char* s, int (*fn)(char*))
{
    return stringUntilFind(s, sNegLogic, fn);
}
int prbStringEndFn(char* s)
{
    return (*s == sStringBE && (*s-1) != sStringTerm);
}
char* probeString(char* s)
{
    return forwardUntilFind(s, prbStringEndFn);
}
int yParseDigit(char* s, size_t* oStep)
{
    char* crs = s;
    int v = 0;
    while (isdigit(*crs)) {
        v = (*crs - '0') + 10 * v;
        crs++;
    }
    *oStep = crs - s;
    return v;
}
int yParseInt(char *s, size_t* oStep)
{
    char* crs = s;
    int v = 0;
    int sym = 1;
    size_t step;
    if (*crs == '-') {
        sym = -1;
        crs++;
    } else if (*crs == '+') {
        sym = 1;
        crs++;
    }
    v = yParseDigit(crs, &step);
    v = sym * v;
    crs += step;
    *oStep = crs - s;
    return v;
}
double yParseNumber(char* s, size_t* oStep)
{
    char* crs = s;
    double v, ret;
    size_t step;
    ret = yParseInt(crs, &step);
    crs += step;
    if (*crs == '.') {
        crs++;
        v = yParseDigit(crs, &step);
        v = v / pow(10, step);
        if (ret >= 0) {
            ret += v;
        } else {
            ret -= v;
        }
        crs += step;
    }
    if (*crs == 'e' || *crs == 'E') {
        crs++;
        v = yParseInt(crs, &step);
        ret *= pow(10, v);
        crs += step;
    }
    *oStep = crs - s;
    return ret;
}
// IN:  char* s
// OUT: foward step
// RET: yType* p
void* yParse(char* s, size_t* oStep)
{
    char* crs = s;
    size_t step;
    void *pp;
    while (1) {
        if (isspace(*crs)) {
            continue;
        } else if (*crs == sStringBE) {
            crs = probeString(s);
            yString* p = yAlloc(eString);
            yStringSet(p, s + 1, crs);
            s = crs + 1;
        } else if (*crs == sArrayBegin) {
            crs++;
            yType* ae;
            while (*crs != sArrayEnd) {
                ae = yParse(s, &step);
            }
        } else if (*crs == sObjectBegin) {

        } else if (*crs == sStringTerm) {
            break;
        } else {
            double d = yParseNumber(crs, &step);
            yNumber* p = yAlloc(eNumber);
            p->n = d;
            crs += step;
            pp = p;
        }
    }
    if (oStep != NULL) {
        *oStep = crs - s;
    }
    return pp;
}
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
    yString *p = (yString*)yAlloc(eString);
    yStringSet(p, "123", NULL);
    assert(strcmp(p->s, "123") == 0);
    char s[] = "123";
    yStringSet(p, s, s+2);
    assert(strcmp(p->s, "12") == 0);
}
void testParse()
{
    char s[] = "\"123\"";
    yType* p = yParse(s);
    yDump(p);
    // XXX: rdata section, cannot write, so yStringSet maybe error;
    // yType* p1 = yParse("\"123\"");
    // yDump(p1);
}
int main()
{
    puts("----begin----");
    testNumber();
    puts("----number done----");
    testString();
    puts("----string done----");
    testParse();
    puts("----parser done----");
    puts("----over ----");
    return 0;
}
