#include <stdio.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "yjson.h"

char ySymbolStr[][10] = {"null", "true", "false"};

int _debug = 0;
int _calllevel = 0;
void debug(const char *fmt, ...)
{
    if (_debug <= 0) {
        return;
    }
    static int spacecnt = 2;
    va_list args;
    va_start(args, fmt);
    int i;
    for (i = 0; i < _calllevel * spacecnt; i++) {
        putchar(' ');
    }
    vprintf(fmt, args);
    // auto add newline char
    size_t len = strlen(fmt);
    if (len != 0 && fmt[len - 1] != '\n') {
        putchar('\n');
    }
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

void yHeadInit(yTypeHead *h, yMeta meta)
{
    h->meta = meta;
}

enum {
    sObjectBegin = '{',
    sObjectEnd = '}',
    sObjectDelt = ':',
    sArrayBegin = '[',
    sArrayEnd = ']',
    sDelimeter = ',',
    sStringBE = '"',
    sStringTrans = '\\',
    sStringTerm = '\0',
};

typedef void   (*yTypeFn)       (yType *);
typedef yType * (*yTypeParseFn)  (char *, int *);
typedef struct {
    int size;
    yTypeFn init;
    yTypeFn display;
} yTypeMap;

void yDisplayRecur(yType *p);

void yStringDisp(yType *p)
{
    printf("\"%s\"", ((yString *)p)->s);
}

void yObjectDisp(yType *p)
{
    yObjectNode *pp = ((yObject *)p)->node;
    putchar(sObjectBegin);
    while (pp != NULL) {
        yStringDisp((yType *)pp->key);
        putchar(sObjectDelt);
        yDisplayRecur(pp->value);
        pp = (yObjectNode *)pp->next;
        if (pp == NULL) {
            break;
        }
        printf("%c ", sDelimeter);
    }
    putchar(sObjectEnd);
}

void yArrayDisp(yType *p)
{
    putchar(sArrayBegin);
    yArrayNode *pp = ((yArray *)p)->node;
    while (pp != NULL) {
        yDisplayRecur(pp->elm);
        pp = (yArrayNode *)pp->next;
        if (pp == NULL) {
            break;
        }
        printf("%c ", sDelimeter);
    }
    putchar(sArrayEnd);
}

void yNumberDisp(yType *p)
{
    printf("%.3f", ((yNumber *)p)->n);
}

void ySymbolDisp(yType *p)
{
    printf("%s", ySymbolStr[((ySymbol *)p)->val]);
}

yTypeMap map[] = {
    [eInvalid]      = {0, NULL, NULL},
    [eObject]       = {sizeof(yObject), NULL, yObjectDisp},
    [eArray] = {sizeof(yArray), NULL, yArrayDisp},
    [eString] = {sizeof(yString), NULL, yStringDisp},
    [eNumber] = {sizeof(yNumber), NULL, yNumberDisp},
    [eArrayNode] = {sizeof(yArrayNode), NULL, NULL},
    [eSymbol] = {sizeof(ySymbol), NULL, ySymbolDisp},
    [eObjectNode] = {sizeof(yObjectNode), NULL, NULL},
};

void *yAlloc(yMeta meta)
{
    size_t len = map[meta].size;
    yTypeHead *p = (yTypeHead *)malloc(len);
    bzero(p, len);
    if (meta != eArrayNode || meta != eObjectNode) {
        yHeadInit(p, meta);
    }
    return p;
}

void yFree(yType *p)
{
    // TODO:
    // need free child-pointer and next-pointer
    if (p != NULL) {
        free(p);
    }
}

void yDisplayRecur(yType *p)
{
    yTypeFn fn = map[p->meta].display;
    if (fn != NULL) {
        fn(p);
    } else {
        printf("cannot display %p", p);
    }
}

void yDisplay(void *p)
{
    if (p == NULL) {
        printf("nil");
        return;
    }
    yType *pType = (yType *)p;
    yDisplayRecur(pType);
    putchar('\n');
}

int yParseDigit(char *s, size_t *oStep)
{
    char *crs = s;
    int v = 0;
    while (isdigit(*crs)) {
        v = (*crs - '0') + 10 * v;
        crs++;
    }
    *oStep = crs - s;
    return v;
}

int yParseInt(char *s, size_t *oStep)
{
    char *crs = s;
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

double yParseNumber(char *s, size_t *oStep)
{
    char *crs = s;
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

char *yParseString(char *s, size_t *oStep)
{
    char *o = s;
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

    char *ret = (char *)malloc(sizeof(*oStep));
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

ySymbolVal yParseSymbol(char *s, size_t *oStep)
{
    ySymbolVal v = ysInvalid;
    size_t i;
    for (i = 0; i < sizeof(ySymbolStr) / sizeof(ySymbolStr[0]); i++) {
        if (strncasecmp(s, ySymbolStr[i], strlen(ySymbolStr[i])) == 0) {
            v = i;
            *oStep = strlen(ySymbolStr[i]);
            break;
        }
    }
    return v;
}

yNumber *ykParseNumber(char *s, size_t *oStep)
{
    double d = yParseNumber(s, oStep);
    yNumber *p = (yNumber *)yAlloc(eNumber);
    p->n = d;
    return p;
}

yString *ykParseString(char *crs, size_t *oStep)
{
    crs++;
    char *str = yParseString(crs, oStep);
    yString *p = yAlloc(eString);
    p->s = str;
    *oStep += 2;
    return p;
}

void forwardSpace(char **s)
{
    while (isspace(**s)) {
        (*s)++;
    }
}

yArray *ykParseArray(char *crs, size_t *oStep)
{
    char *o = crs;
    crs++;
    size_t step = 0;
    yArray *pa = yAlloc(eArray);
    yArrayNode **tail = &(pa->node);
    while (*crs != sArrayEnd) {
        yArrayNode *pn = yAlloc(eArrayNode);
        pn->elm = yParse(crs, &step);
        crs += step;
        *tail = pn;
        tail = (yArrayNode **)&(pn->next);
        forwardSpace(&crs);
        if (*crs == sDelimeter) {
            crs++;
            continue;
        }
    }
    *oStep = crs - o + 1;
    tail = NULL;
    return pa;
}

yObject *ykParseObject(char *crs, size_t *oStep)
{
    char *o = crs;
    crs++;
    yObject *po = yAlloc(eObject);
    size_t step;
    yObjectNode **tail = &(po->node);
    while (*crs != sObjectEnd) {
        yObjectNode *pn = yAlloc(eObjectNode);
        pn->key = yParse(crs, &step);
        crs += step;
        forwardSpace(&crs);
        if (*crs == sObjectDelt) {
            crs++;
        } else {
            debug("error: cannot find sObjectDelt");
            return NULL;
        }
        pn->value = yParse(crs, &step);
        crs += step;
        *tail = pn;
        tail = (yObjectNode **)&(pn->next);
        forwardSpace(&crs);
        if (*crs == sDelimeter) {
            crs++;
            continue;
        }
    }
    crs++;
    *oStep = crs - o;
    tail = NULL;
    return po;
}

ySymbol *ykParseSymbol(char *crs, size_t *oStep)
{
    size_t step;
    ySymbolVal v = yParseSymbol(crs, &step);
    if (v != ysInvalid) {
        ySymbol *p = yAlloc(eSymbol);
        p->val = v;
        *oStep = step;
        return p;
    } else {
        printf("error: unknown %p %s", crs, crs);
        return NULL;
    }
}

void *yParse(char *s, size_t *oStep)
{
    char *crs = s;
    size_t step;
    void *p;
    while (1) {
        forwardSpace(&crs);
        if (*crs == sStringTerm) {
            break;
        } else if (*crs == sObjectBegin) {
            p = ykParseObject(crs, &step);
            break;
        } else if (*crs == sArrayBegin) {
            p = ykParseArray(crs, &step);
            break;
        } else if (*crs == sStringBE) {
            p = ykParseString(crs, &step);
            break;
        } else if (isdigit(*crs) || *crs == '+' || *crs == '-') {
            p = ykParseNumber(crs, &step);
            break;
        } else {
            p = ykParseSymbol(crs, &step);
            break;
        }
    }
    crs += step;
    if (oStep != NULL) {
        *oStep = crs - s;
    }
    return p;
}
