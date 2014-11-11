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
} yMeta;

typedef struct yType {
    yMeta meta;
    struct yType* parent;
}yType, yTypeHead;

typedef struct yObject {
    yTypeHead head;
    struct yType* firstchild;
} yObject;

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
    yTypeHead head;
    double n;
} yNumber;
// IN:  char* s
// OUT: foward step
// RET: yType* p
void* yParse(char* s, size_t* oStep);
char* yParseString(char* s, size_t* oStep);
void* yAlloc(yMeta meta);
void disabledebug();
void enabledebug();
void yDump(yType* p);
int yParseDigit(char* s, size_t* oStep);
int yParseInt(char *s, size_t* oStep);
double yParseNumber(char* s, size_t* oStep);
#endif /* end of include guard: YJSON_H_BJTDHYQ0 */
