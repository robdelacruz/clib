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
    char *s;
    u32 len;
} String;
#define STRING_LIT(sz) ((String){sz, countof(sz)-1})
#define STRING(sz) ((String){sz, strlen(sz)})

typedef struct {
    String *base;
    u16 len;
    u16 cap;
} StringArray;
#define STRINGARRAY(p, cap) ((StringArray){p, 0, cap})

void *PushBytes(Arena *a, u64 size)
{
    if (a->pos + size > a->cap)
        panic("PushBytes() hit cap limit"); 

    char *p = a->base + a->pos;
    a->pos += size;
    return p;
}

String PushString(Arena *a, char *sz)
{
    String str;
    u32 sz_len = strlen(sz);
    str.s = PushBytes(a, sz_len+1);
    str.len = sz_len;
    memcpy(str.s, sz, sz_len+1);
    return str;
}

String PushDupString(Arena *a, String src)
{
    return PushString(a, src.s);
}

void StringArrayAppend(StringArray *ss, String str)
{
    if (ss->len >= ss->cap)
        return;
    ss->base[ss->len] = str;
    ss->len++;
}

StringArray StringSplit(Arena *a, String str, char *sep)
{
    String *strs = PushStructs(a, String, 16); // Maximum of 16 string segments can be returned.
    StringArray ss = STRINGARRAY(strs, 16);

    String tmpstr;
    char *p = str.s;
    int seplen = strlen(sep);

    while (*p != 0) {
        char *psep = strstr(p, sep);
        if (psep == NULL) {
            tmpstr = PushString(a, p);
            StringArrayAppend(&ss, tmpstr);
            break;
        }

        int slen = psep - p;
        tmpstr.s = PushBytes(a, slen+1);
        memcpy(tmpstr.s, p, slen);
        tmpstr.s[slen] = 0;
        StringArrayAppend(&ss, tmpstr);

        p = psep + seplen;
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
    printf("str1: '%s' len: %d\n", str1.s, str1.len);
    printf("str2: '%s' len: %d\n", str2.s, str2.len);

    printf("scratch: pos: %ld\n", scratch.pos);
    PushBytes(&scratch, 10);
    printf("scratch: pos: %ld\n", scratch.pos);
    PushBytes(&scratch, 120);
    printf("scratch: pos: %ld\n", scratch.pos);
    PushStruct(&scratch, String);
    printf("scratch: pos: %ld\n", scratch.pos);

    String newstr1 = PushString(&scratch, "new string");
    String newstr2 = PushString(&scratch, "another new string");
    String newstr3 = PushDupString(&scratch, newstr2);
    newstr3.s[0] = '1';
    printf("scratch: pos: %ld\n", scratch.pos);

    foo(newstr1, newstr2, newstr3);

    StringArray ss = StringSplit(&scratch, STRING_LIT("abc;def; ghi,abc;1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16"), ";");
    for (int i=0; i < ss.len; i++) {
        printf("ss[%d]: '%s'\n", i, ss.base[i].s);

    }
}

void foo(String s1, String s2, String s3)
{
    printf("s1: '%s' len: %ld\n", s1.s, s1.len);
    printf("s2: '%s' len: %ld\n", s2.s, s2.len);
    printf("s3: '%s' len: %ld\n", s3.s, s3.len);
}


