#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bn.c"

bool base10num(char *str, struct bn* num) {
    struct bn ten;
    bignum_from_int(&ten, 10);

    struct bn tmp;
    struct bn digit;
    bignum_init(num);
    bignum_init(&tmp);
    bignum_init(&digit);
    for (int i = 0; str[i] != 0; i++) {
        int dchar = str[i];
        if (dchar < 48 || dchar > 57) {
            return false;
        }
        bignum_mul(num, &ten, &tmp);
        bignum_from_int(&digit, dchar - 48);
        bignum_add(&tmp, &digit, num);
    }
    return true;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: from <base> <array>, to <base> <num1> [num2] ...");
        return 1;
    }

    struct bn one;
    bignum_from_int(&one, 1);
    
    char *smode = argv[1];
    char *sbase = argv[2];

    struct bn base;
    bool validbase = base10num(sbase, &base);
    char nil[] = "";
    char *basecomp = validbase ? bignum_cmp(&base, &one) : nil;
    if (!strcmp(basecomp, "LARGER")) {
        fprintf(stderr, "Argument 2 must be a valid integer greater than one");
        return false;
    }

    if (strcmp(smode, "from")) {
        char *sarray = argv[3];
        if (!validarray) {
            fprintf(stderr, "Argument 2 must be a valid integer greater than zero");
            return false;
        }


        /*struct bn result;
        bignum_add(&base, &array, &result);
        bignum_to_string(&result, buf, sizeof(buf));
        printf("add = %s\n", buf);
    }*/
    }

    return 0;
}