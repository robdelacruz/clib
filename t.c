#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "clib.h"

MAKE_ARENA(ar0, SIZE_MB);
MAKE_ARENA(ar1, 5*SIZE_MB);
MAKE_ARENA(ar2, 5*SIZE_MB);

typedef struct {
    time_t date;
    String time;
    String desc;
    float amt;
    u16 catid;
} Exp;

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        printf("Usage: %s <expfile>\n", argv[0]);
        exit(0);
    }

    String expfile = PushString(&ar0, argv[1]);
    printf("expfile: '%.*s'\n", expfile.len, expfile.bs);
}

