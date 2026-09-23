#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "bn.c"

FILE *primes;

// https://stackoverflow.com/a/8534275
char *strrev(char *str) {
    char *p1, *p2;

    if (! str || ! *str) return str;
    for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2) {
        *p1 ^= *p2;
        *p2 ^= *p1;
        *p1 ^= *p2;
    }
    return str;
}

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
        if (dchar < '0' || dchar > '9') {
            return false;
        }
        bignum_mul(num, &ten, &tmp);
        bignum_from_int(&digit, dchar - '0');
        bignum_add(&tmp, &digit, num);
    }
    return true;
}
void printnum(struct bn* num) {
    if (bignum_is_zero(num)) {
        printf("0");
        return;
    }
    struct bn ten;
    bignum_from_int(&ten, 10);

    struct bn original;
    bignum_assign(&original, num);
    struct bn tmp;
    struct bn digit;
    bignum_init(&tmp);
    bignum_init(&digit);

    char *numstr = malloc(1);
    char *renumstr;
    size_t len = 0;
    size_t buf = 1;
    while (!bignum_is_zero(&original)) {
        bignum_divmod(&original, &ten, &tmp, &digit);
        bignum_assign(&original, &tmp);

        if (len + 1 >= buf) {
            buf *= 2;
            renumstr = realloc(numstr, buf);
            if (renumstr == NULL) {
                fprintf(stderr, "Could not reallocate");
                exit(1);
            }
            numstr = renumstr;
        }
        numstr[len++] = bignum_to_int(&digit) + '0';
    }
    numstr[len] = '\0';
    printf("%s", strrev(numstr));
    free(numstr);
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

    if (strcmp(sbase, "primes") != 0) { // aAME
        struct bn base;
        bool validbase = base10num(sbase, &base);
        int basecomp = validbase ? bignum_cmp(&base, &one) : SMALLER;
        if (basecomp != LARGER) {
            fprintf(stderr, "Argument 2 must be an integer greater than 1");
            return 1;
        }

        if (strcmp(smode, "from") == 0) {
            char *sarray = argv[3];
            
            struct bn array;
            bool validarray = base10num(sarray, &array);
            if (!validarray) {
                fprintf(stderr, "Argument 3 must be a non-negative integer");
                return 1;
            }

            struct bn tmp;
            struct bn digit;
            bool iszero = bignum_is_zero(&array);
            while (!iszero) {
                bignum_divmod(&array, &base, &tmp, &digit);
                bignum_assign(&array, &tmp);
                iszero = bignum_is_zero(&array);
                printnum(&digit);
                if (!iszero) printf(" ");
            }
        } else if (strcmp(smode, "to") == 0) {
            struct bn array;
            bignum_init(&array);
            
            struct bn tmp;
            struct bn digit;
            bignum_init(&tmp);
            bignum_init(&digit);
            bool validdigit = true;
            int digitcomp = SMALLER;
            for (int i = argc-1; i >= 3; i--) {
                validdigit = base10num(argv[i], &digit);
                digitcomp = validdigit ? bignum_cmp(&digit, &base) : LARGER;
                if (digitcomp != SMALLER) {
                    fprintf(stderr, "Argument %d must be a non-negative integer less than the given base", i);
                    return 1;
                }
                bignum_mul(&array, &base, &tmp);
                bignum_add(&tmp, &digit, &array);
            }
            printnum(&array);
        }
    } else { // mAME
        primes = fopen("smallprimes.txt", "r");
        if (primes == NULL) {
            fprintf(stderr, "smallprimes.txt is missing");
            return 1;
        }
        
        if (strcmp(smode, "from") == 0) {
            char *sarray = argv[3];
            
            struct bn array;
            bool validarray = base10num(sarray, &array);
            if (!validarray) {
                fprintf(stderr, "Argument 3 must be a valid integer greater than or equal to zero");
                return 1;
            }

            int prime;
            struct bn primebn;
            if (fscanf(primes, "%d", &prime) == 0) {
                fprintf(stderr, "Ran out of primes");
                return 1;
            }
            bignum_from_int(&primebn, prime);
            struct bn tmp;
            struct bn mod;
            struct bn digit;
            bool first = true;
            bignum_init(&digit);
            bool arraycomp = bignum_cmp(&array, &one);
            while (arraycomp == LARGER) {
                bignum_divmod(&array, &primebn, &tmp, &mod);
                if (bignum_is_zero(&mod)) {
                    bignum_inc(&digit);
                    bignum_assign(&array, &tmp);
                } else {
                    if (!first) printf(" ");
                    printnum(&digit);
                    bignum_init(&digit);
                    first = false;
                    arraycomp = bignum_cmp(&array, &one);
                    if (arraycomp == LARGER && fscanf(primes, "%d", &prime) != 1) {
                        fprintf(stderr, "Ran out of primes");
                        return 1;
                    }
                    bignum_from_int(&primebn, prime);
                }
            }
        } else {
            struct bn array;
            bignum_assign(&array, &one);

            int prime;
            struct bn primebn;
            struct bn tmp;
            struct bn digit;
            bignum_init(&tmp);
            bignum_init(&digit);
            bool validdigit = true;
            for (int i = 3; i < argc; i++) {
                validdigit = base10num(argv[i], &digit);
                if (!validdigit) {
                    fprintf(stderr, "Argument %d must be a non-negative integer less than the given base", i);
                    return 1;
                }
                
                if (fscanf(primes, "%d", &prime) == 0) {
                    fprintf(stderr, "Ran out of primes");
                    return 1;
                }
                bignum_from_int(&primebn, prime);

                while (!bignum_is_zero(&digit)) {
                    bignum_dec(&digit);
                    bignum_mul(&array, &primebn, &tmp);
                    bignum_assign(&array, &tmp);
                }
            }
            printnum(&array);
        }
    }

    return 0;
}
