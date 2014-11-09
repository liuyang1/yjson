#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "yjson.h"

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
int _debug = 0;
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
void disabledebug()
{
    _debug = 0;
}
void enabledebug()
{
    _debug = 1;
}
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
char transMap(char c)
{
    static char Map[2][20] = {"\"\\/bfnrt", "\"\\/\b\f\n\r\t"};
    size_t j;
    char ret;
    for (j = 0; j < sizeof(Map[0]); j++) {
        if (c == Map[0][j]) {
            ret = Map[1][j];
            break;
        }
    }
    return ret;
}
char* yParseString(char* s, size_t* oStep)
{
    char* o = s;
    int trans = 0;
    while (!(*s == sStringBE && trans == 0)) {
        if (*s == sStringTrans && trans == 0) {
            trans = 1;
        } else if (trans == 1) {
            trans = 0;
        }
        s++;
    }
    *oStep = s - o;

    char* ret = malloc(sizeof(*oStep));
    size_t i, f;
    for (i = 0, f = 0; f < *oStep; i++, f++) {
        if (o[f] == sStringTrans) {
            f++;
            ret[i] = transMap(o[f]);
        } else {
            ret[i] = o[f];
        }
    }
    ret[i] = sStringTerm;
    return ret;
}
void* yParse(char* s, size_t* oStep)
{
    char* crs = s;
    size_t step;
    void *pp;
    while (1) {
        if (isspace(*crs)) {
            continue;
        } else if (*crs == sStringTerm) {
            break;
        } else if (*crs == sStringBE) {
            crs++;
            char* str = yParseString(crs, &step);
            yString* p = yAlloc(eString);
            p->s = str;
            pp = p;
            crs += step + 1;
        } else if (*crs == sArrayBegin) {
            crs++;
            yType* ae;
            while (*crs != sArrayEnd) {
                ae = yParse(s, &step);
            }
        } else if (*crs == sObjectBegin) {

        } else {
            double d = yParseNumber(crs, &step);
            yNumber* p = yAlloc(eNumber);
            p->n = d;
            pp = p;
            crs += step;
        }
    }
    if (oStep != NULL) {
        *oStep = crs - s;
    }
    return pp;
}
