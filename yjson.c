#include <stdio.h>
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
    long long n;
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
ErrorCode yNumberSet(yNumber* p, char *s)
{
    p->n = atoll(s);
    return OK;
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
void* yParse(char* s)
{
    infn;
    char* crs = s;
    char t;
    yType* p = NULL;
    while (1) {
        if (isspace(*s)) {
            continue;
        } else if (*s == sStringBE) {
            crs = probeString(s);
            p = yAlloc(eString);
            yStringSet(p, s+1, crs);
            s = crs + 1;
        } else if (*s == sArrayBegin) {
            
        } else if (*s == sObjectBegin) {

        } else if (*s == sStringTerm) {
            break;
        }
    }
    outfn;
    return p;
}
void testNumber()
{
    yNumber* p = (yNumber*)yAlloc(eNumber);
    yNumberSet(p, "123");
    assert(p->n == 123);
    yNumberSet(p, "-123");
    assert(p->n == -123);
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
