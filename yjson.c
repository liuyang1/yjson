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

int _debug = 1;
void debug(const char *fmt, ...)
{
    if (_debug <= 0)
        return;
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}
#define infn            {debug("%s enter\n", __FUNCTION__);}
#define outfn           {debug("%s leave\n", __FUNCTION__);}
void yDump(yType* p)
{
    if (p == NULL) {
        printf("null pointer\n");
        return;
    }
    if (p->meta == eObject) {
        printf("object");
    } else if (p->meta == eArray) {
        printf("array");
    } else if (p->meta == eString) {
        yString* ps = (yString*)p;
        printf("str: %s\n", ps->s);
    } else if (p->meta == eNumber) {
        yNumber* pn = (yNumber*)p;
        printf("num: %lld\n", pn->n);
    } else {
        printf("unknown!");
    }
    printf("\n");
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
    if (p->s) {
        free(p->s);
    }
    char t;
    if (e != NULL) {
        t = *e;
        infn;
        *e = 0;
        infn;
    }
    infn;
    p->s = strdup(b);
    printf("%p %s %p %s\n", b, b, p->s, p->s);
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
    char* crs = s;
    yType* p = NULL;
    while (1) {
        if (isspace(*s)) {
            continue;
        } else if (*s == sStringBE) {
            crs = probeString(s);
            debug("%p %c %p %c\n", s, *s, crs, *crs);
            p = yAlloc(eString);
            yStringSet(p, s+1, crs);
        } else if (*s == sArrayBegin) {
            
        } else if (*s == sObjectBegin) {

        } else if (*s == sStringTerm) {
            break;
        }
    }
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
    yType* p = yParse("\"123\"");
    yDump(p);
}
int main()
{
    testNumber();
    // testString();
    testParse();
    printf("test over!\n");
    return 0;
}
