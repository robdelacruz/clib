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
#define PushStruct(a, v) PushBytes(a, sizeof(v), NULL)
#define PushStructs(a, v, n) PushBytes(a, sizeof(v)*n, NULL)
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

void *PushBytes(Arena *a, u64 size, void *src)
{
    if (a->pos + size > a->cap)
        panic("PushBytes() hit cap limit"); 

    char *p = a->base + a->pos;
    if (src)
        memcpy(p, src, size);
    a->pos += size;
    return p;
}

String PushCString(Arena *a, char *sz)
{
    String str;
    u32 sz_len = strlen(sz);
    str.bs = PushBytes(a, sz_len, sz);
    str.len = sz_len;
    return str;
}

char *CString(Arena *a, String str)
{
    char *sz = PushBytes(a, str.len+1, NULL);
    memcpy(sz, str.bs, str.len);
    sz[str.len] = 0;
    return sz;
}

char *CStaticString(String str)
{
    static char g_sz[1024];

    // Copy sz bytes into static storage (not thread-safe)
    int ncopy = str.len;
    if (ncopy > sizeof(g_sz)-1)
        ncopy = sizeof(g_sz)-1;
    memcpy(g_sz, str.bs, ncopy);
    g_sz[ncopy] = 0;

    return g_sz;
}

String StringDup(Arena *a, String src)
{
    String str;
    str.len = src.len;
    str.bs = PushBytes(a, src.len, src.bs);
    return str;
}

double StringToFloat(Arena a, String str)
{
    return atof(CString(&a, str));
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
    String tmpstr;
    int itokstart=0;
    int toklen=0;

    // ntoks = number of tokens after splitting string
    int ntoks=1;
    while (1) {
        int isep = StringSearch(str, itokstart, sep);
        if (isep == -1)
            break;

        ntoks++;
        itokstart = isep + sep.len;
    }

    // Create stringlist ss to store string tokens.
    String *strs = PushStructs(a, String, ntoks);
    StringList ss = STRINGLIST(strs, ntoks);

    itokstart = 0;
    while (1) {
        int isep = StringSearch(str, itokstart, sep);
        if (isep == -1)
            toklen = str.len - itokstart;
        else
            toklen = isep - itokstart;

        tmpstr.bs = PushBytes(a, toklen, str.bs + itokstart);
        tmpstr.len = toklen;
        StringListAppend(&ss, tmpstr);

        if (isep == -1)
            break;

        itokstart = isep + sep.len;
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

    printf("scratch: pos: %lld\n", scratch.pos);
    PushBytes(&scratch, 10, NULL);
    printf("scratch: pos: %lld\n", scratch.pos);
    PushBytes(&scratch, 120, NULL);
    printf("scratch: pos: %lld\n", scratch.pos);
    PushStruct(&scratch, String);
    printf("scratch: pos: %lld\n", scratch.pos);

    String newstr1 = PushCString(&scratch, "new string abcdefghijklmnop");
    String newstr2 = PushCString(&scratch, "another new string + more characters");
    String newstr3 = StringDup(&scratch, newstr2);
    newstr3.bs[0] = '1';
    printf("scratch: pos: %lld\n", scratch.pos);

    foo(newstr1, newstr2, newstr3);

    char *sz1 = CString(&scratch, newstr1);
    printf("sz1: '%s'\n", sz1);
    char *sz2 = CString(&scratch, newstr2);
    printf("sz2: '%s'\n", sz2);
    char *sz2a = CStaticString(newstr2);
    printf("sz2a: '%s'\n", sz2a);
    newstr3.bs[0] = '2';
    char *sz3 = CStaticString(newstr3);
    printf("sz3: '%s'\n", sz3);

    String str3 = LSTRING("abc;def;ghi;jkl;mno;pqrstuv;xyz;123;456; ;789");
    String sep = LSTRING(";");
    StringList ss = StringSplit(&scratch, str3, sep);
    for (int i=0; i < ss.len; i++) {
        String str = ss.base[i];
        printf("ss[%d]: '%.*s'\n", i, str.len, str.bs);
    }

    str1 = PushCString(&scratch, "123.45");
    double f1 = StringToFloat(scratch, str1);
    str2 = PushCString(&scratch, "2025.01");
    double f2 = StringToFloat(scratch, str2);
    printf("StringToFloat '%s' ==> %.2f\n", CStaticString(str1), f1);
    printf("StringToFloat '%s' ==> %.2f\n", CStaticString(str2), f2);
}

void foo(String s1, String s2, String s3)
{
    printf("s1: '%.*s' len: %d\n", s1.len, s1.bs, s1.len);
    printf("s2: '%.*s' len: %d\n", s1.len, s2.bs, s2.len);
    printf("s3: '%.*s' len: %d\n", s1.len, s3.bs, s3.len);
}


