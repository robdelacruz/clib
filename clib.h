#ifndef CLIB_H
#define CLIB_H

#define SIZE_MB (1024*1024)
#define SIZE_GB (1024*1024*1024)

#define countof(v) (sizeof(v) / sizeof((v)[0]))
#define memzero(p, v) (memset(p, 0, sizeof(v)))

typedef char i8;
typedef short i16;
typedef long i32;
typedef long long i64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef struct {
    char *base;
    u64 pos;
    u64 cap;
} Arena;

typedef struct {
    char *bs;
    u16 len;
} String;

typedef struct {
    String *base;
    u16 len;
    u16 cap;
} StringList;

#define MAKE_ARENA(name, cap) \
    char _buf_ ## name[cap]; \
    Arena name = (Arena){_buf_ name, 0, cap}
#define PushStruct(a, v) PushBytes(a, sizeof(v), NULL)
#define PushStructs(a, v, n) PushBytes(a, sizeof(v)*n, NULL)
#define ResetArena(a) {a->pos = 0;}

void *PushBytes(Arena *a, u64 size, void *src);
String PushString(Arena *a, char *sz);

#define LSTR(sz) ((String){sz, countof(sz)-1})
#define STR(sz) ((String){sz, strlen(sz)})
#define MAKE_STR(name, sz) \
    char _buf_ ## name [] = sz; \
    String name = (String){_buf_ ## name, strlen(_buf_ ## name)}
#define MAKE_LSTR(name, sz) \
    char _buf_ ## name [] = sz; \
    String name = (String){_buf_ ## name, sizeof(_buf_ ## name)-1}

char *CString(Arena *a, String str);
String StringDup(Arena *a, String src);
String StringFormat(Arena *a, const char *fmt, ...);
double StringToFloat(String str);
int StringSearch(String str, int startpos, String searchstr);

#define STRINGLIST(p, cap) ((StringList){p, 0, cap})
void StringListAppend(StringList *ss, String str);
StringList StringSplit(Arena *a, String str, String sep);

#endif

