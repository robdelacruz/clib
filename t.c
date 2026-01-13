#include <stdio.h>
#include "clib.h"

MAKE_ARENA(scratch, SIZE_MB);
MAKE_ARENA(main_arena, SIZE_MB);

MAKE_LSTR(str0, "123");

void foo(String s1, String s2, String s3);

int main(int argc, char *argv[])
{
    MAKE_LSTR(str1, "abc");
    MAKE_LSTR(str2, "1234567");

    printf("str1: '%.*s' len: %d\n", str1.len, str1.bs, str1.len);
    printf("str2: '%.*s' len: %d\n", str2.len, str2.bs, str2.len);

    printf("scratch: pos: %lld\n", scratch.pos);
    PushBytes(&main_arena, 10, NULL);
    printf("scratch: pos: %lld\n", scratch.pos);
    PushBytes(&scratch, 120, NULL);
    printf("scratch: pos: %lld\n", scratch.pos);
    PushStruct(&scratch, String);
    printf("scratch: pos: %lld\n", scratch.pos);

    String newstr1 = PushString(&scratch, "new string abcdefghijklmnop");
    String newstr2 = PushString(&scratch, "another new string + more characters");
    String newstr3 = StringDup(&scratch, newstr2);
    newstr3.bs[0] = '1';
    printf("scratch: pos: %lld\n", scratch.pos);

    foo(newstr1, newstr2, newstr3);

    char *sz1 = CString(&scratch, newstr1);
    printf("sz1: '%s'\n", sz1);
    char *sz2 = CString(&scratch, newstr2);
    printf("sz2: '%s'\n", sz2);
    newstr3.bs[0] = '2';
    char *sz3 = CString(&scratch, newstr3);
    printf("sz3: '%s'\n", sz3);

    String testsplit = PushString(&scratch, "abc;def;ghi;jkl;mno;pqrstuv;xyz;123;456; ;789");
    StringList ss = StringSplit(&scratch, testsplit, LSTR(";"));
    for (int i=0; i < ss.len; i++) {
        String str = ss.base[i];
        printf("ss[%d]: '%.*s'\n", i, str.len, str.bs);
    }

    str1 = PushString(&scratch, "123.45");
    double f1 = StringToFloat(str1);
    str2 = PushString(&scratch, "2025.01");
    double f2 = StringToFloat(str2);
    printf("StringToFloat '%s' ==> %.2f\n", CString(&scratch, str1), f1);
    printf("StringToFloat '%s' ==> %.2f\n", CString(&scratch, str2), f2);

    str1 = StringFormat(&scratch, "StringFormat test: '%s', n: %d, f: %.2f", "clib test", 123, 123.45678);
    printf("str1: '%.*s'\n", str1.len, str1.bs);
    printf("str1.len: %d\n", str1.len);

    str2 = StringFormat(&scratch, "abc: %d", 45);
    printf("str2: '%.*s'\n", str2.len, str2.bs);
    printf("str2.len: %d\n", str2.len);
}

void foo(String s1, String s2, String s3)
{
    printf("s1: '%.*s' len: %d\n", s1.len, s1.bs, s1.len);
    printf("s2: '%.*s' len: %d\n", s1.len, s2.bs, s2.len);
    printf("s3: '%.*s' len: %d\n", s1.len, s3.bs, s3.len);
}


