#include "clib.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void panic(char *s)
{
    if (s)
        fprintf(stderr, "%s\n", s);
    abort();
}

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

String PushString(Arena *a, char *sz)
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

String StringDup(Arena *a, String src)
{
    String str;
    str.len = src.len;
    str.bs = PushBytes(a, src.len, src.bs);
    return str;
}

String StringFormat(Arena *a, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    int slen = vsnprintf(NULL, 0, fmt, args);
    char *psz = PushBytes(a, slen+1, NULL);
    va_end(args);

    va_start(args, fmt);
    vsnprintf(psz, slen+1, fmt, args);
    va_end(args);

    return (String){psz, slen};
}

double StringToFloat(String str)
{
    char sz[50];
    if (str.len > sizeof(sz)-1)
        return 0.0;

    memcpy(sz, str.bs, str.len);
    sz[str.len] = 0;
    return atof(sz);
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

void StringListAppend(StringList *ss, String str)
{
    if (ss->len >= ss->cap)
        return;
    ss->base[ss->len] = str;
    ss->len++;
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

