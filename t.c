#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void panic(char *s)
{
    if (s)
        fprintf(stderr, "%s\n", s);
    abort();
}

typedef struct {
    char *base;
    u64 pos;
    u64 cap;
} Arena;
#define ARENA(p, cap) ((Arena){p, 0, cap})
#define PushStruct(a, v) PushBytes(a, sizeof(v))
#define PushStructs(a, v, n) PushBytes(a, sizeof(v)*n)
#define ResetArena(a) {a->pos = 0;}

typedef struct {
    char *bs;
    u16 len;
} String;
#define LSTRING(sz) ((String){sz, countof(sz)-1})
#define STRING(sz) ((String){sz, strlen(sz)})

typedef struct {
    String *base;
    u16 len;
    u16 cap;
} StringList;
#define STRINGLIST(p, cap) ((StringList){p, 0, cap})

void *PushBytes(Arena *a, u64 size)
{
    if (a->pos + size > a->cap)
        panic("PushBytes() hit cap limit"); 

    char *p = a->base + a->pos;
    a->pos += size;
    return p;
}

String PushCString(Arena *a, char *sz)
{
    String str;
    u32 sz_len = strlen(sz);
    str.bs = PushBytes(a, sz_len);
    str.len = sz_len;
    memcpy(str.bs, sz, sz_len);
    return str;
}

String PushDupString(Arena *a, String src)
{
    String str;
    str.len = src.len;
    str.bs = PushBytes(a, src.len);
    memcpy(str.bs, src.bs, src.len);
    return str;
}

void StringListAppend(StringList *ss, String str)
{
    if (ss->len >= ss->cap)
        return;
    ss->base[ss->len] = str;
    ss->len++;
}

int StringSearch(String str, int startpos, String searchstr)
{
    for (int i=startpos; i < str.len; i++) {
        for (int isearch=0, istr=i; isearch < searchstr.len && istr < str.len; isearch++, istr++) {
            if (str.bs[istr] != searchstr.bs[isearch])
                break;
            if (isearch == searchstr.len-1) // Match found
                return i;
        }
    }
    return -1;
}

StringList StringSplit(Arena *a, String str, String sep)
{
    int MAX_PARTS = 16;
    String *strs = PushStructs(a, String, MAX_PARTS); // Maximum of 16 string segments can be returned.
    StringList ss = STRINGLIST(strs, MAX_PARTS);

    String tmpstr;
    int istart=0;
    int slen=0;

    while (1) {
        int isep = StringSearch(str, istart, sep);
        if (isep == -1)
            slen = str.len - istart;
        else
            slen = isep - istart;

        tmpstr.bs = PushBytes(a, slen);
        memcpy(tmpstr.bs, str.bs + istart, slen);
        tmpstr.len = slen;
        StringListAppend(&ss, tmpstr);

        if (isep == -1)
            break;

        istart = isep + sep.len;
    }

    return ss;
}

char scratchblk[SIZE_MB];
Arena scratch = ARENA(scratchblk, sizeof(scratchblk));

void foo(String s1, String s2, String s3);

int main(int argc, char *argv[])
{
    char *s1 = "abc";
    char s2[] = "1234567";

    String str1 = STRING(s1);
    String str2 = STRING(s2);
    printf("str1: '%.*s' len: %d\n", str1.len, str1.bs, str1.len);
    printf("str2: '%.*s' len: %d\n", str2.len, str2.bs, str2.len);

    printf("scratch: pos: %ld\n", scratch.pos);
    PushBytes(&scratch, 10);
    printf("scratch: pos: %ld\n", scratch.pos);
    PushBytes(&scratch, 120);
    printf("scratch: pos: %ld\n", scratch.pos);
    PushStruct(&scratch, String);
    printf("scratch: pos: %ld\n", scratch.pos);

    String newstr1 = PushCString(&scratch, "new string");
    String newstr2 = PushCString(&scratch, "another new string");
    String newstr3 = PushDupString(&scratch, newstr2);
    newstr3.bs[0] = '1';
    printf("scratch: pos: %ld\n", scratch.pos);

    foo(newstr1, newstr2, newstr3);

    String str3 = LSTRING("abc; 123;;789;");
    String sep = LSTRING(";");
    StringList ss = StringSplit(&scratch, str3, sep);
    for (int i=0; i < ss.len; i++) {
        String str = ss.base[i];
        printf("ss[%d]: '%.*s'\n", i, str.len, str.bs);

    }
}

void foo(String s1, String s2, String s3)
{
    printf("s1: '%.*s' len: %ld\n", s1.len, s1.bs, s1.len);
    printf("s2: '%.*s' len: %ld\n", s1.len, s2.bs, s2.len);
    printf("s3: '%.*s' len: %ld\n", s1.len, s3.bs, s3.len);
}


