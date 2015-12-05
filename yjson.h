#if !defined(YJSON_H_BJTDHYQ0)
#define YJSON_H_BJTDHYQ0

void debug(const char *fmt, ...);
extern int _calllevel;
#define infn            {_calllevel++;debug("%s >>>>\n", __FUNCTION__);}
#define outfn           {debug("%s <<<<\n", __FUNCTION__);_calllevel--;}

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
    eArrayNode,
    eSymbol,
    eObjectNode,
} yMeta;

typedef struct yType {
    yMeta meta;
}yType, yTypeHead;

typedef struct {
    yType* elm;
    yType* next;
} yArrayNode;
typedef struct {
    yTypeHead head;
    yArrayNode* node;
} yArray;

typedef struct {
    yTypeHead head;
    char* s;
} yString;

typedef struct {
    yString* key;
    yType* value;
    yType* next;
} yObjectNode;
typedef struct yObject {
    yTypeHead head;
    yObjectNode* node;
} yObject;

typedef struct {
    yTypeHead head;
    double n;
} yNumber;

typedef enum {
    ysInvalid = -1,
    ysNull = 0,
    ysTrue = 1,
    ysFalse = 2,
} ySymbolVal;
typedef struct {
    yTypeHead head;
    ySymbolVal val;
} ySymbol;
// IN:  char* s
// OUT: foward step
// RET: yType* p
void* yParse(char* s, size_t* oStep);
char* yParseString(char* s, size_t* oStep);
void* yAlloc(yMeta meta);
void disabledebug();
void enabledebug();
void yDisplay(void* p);
int yParseDigit(char* s, size_t* oStep);
int yParseInt(char *s, size_t* oStep);
double yParseNumber(char* s, size_t* oStep);
#endif /* end of include guard: YJSON_H_BJTDHYQ0 */
