#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "bn.c"

#define init        bignum_init
#define from_int    bignum_from_int
#define to_int      bignum_to_int
#define from_string bignum_from_string
#define to_string   bignum_to_string
#define add         bignum_add
#define sub         bignum_sub
#define mul         bignum_mul
#define div         bignum_div
#define mod         bignum_mod
#define divmod      bignum_divmod
#define and         bignum_and
#define or          bignum_or
#define xor         bignum_xor
#define lshift      bignum_lshift
#define rshift      bignum_rshift
#define cmp         bignum_cmp
#define is_zero     bignum_is_zero
#define inc         bignum_inc
#define dec         bignum_dec
#define pow         bignum_pow
#define isqrt       bignum_isqrt
#define assign      bignum_assign

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
void printnum(struct bn* num) {
    if (is_zero(num)) {
        printf("0");
        return;
    }
    struct bn ten;
    from_int(&ten, 10);

    struct bn original;
    assign(&original, num);
    struct bn tmp;
    struct bn digit;
    init(&tmp);
    init(&digit);

    char *numstr = malloc(1);
    char *renumstr;
    size_t len = 0;
    size_t buf = 1;
    while (!is_zero(&original)) {
        divmod(&original, &ten, &tmp, &digit);
        assign(&original, &tmp);

        if (len + 1 >= buf) {
            buf *= 2;
            renumstr = realloc(numstr, buf);
            if (renumstr == NULL) {
                fprintf(stderr, "Could not reallocate\n");
                free(numstr);
                return;
            }
            numstr = renumstr;
        }
        numstr[len++] = to_int(&digit) + '0';
    }
    numstr[len] = '\0';
    printf("%s", strrev(numstr));
    free(numstr);
}

struct bn zero;
struct bn tmp1;
struct bn tmp2;
struct bn* counters;
size_t csize = 1;
struct bn bn_csize;

struct bn* val1(int value) {
    from_int(&tmp1, value);
    return &tmp1;
}
struct bn* val2(int value) {
    from_int(&tmp2, value);
    return &tmp2;
}

struct bn* c_get(struct bn* index, int deref) { // left side of commands has implicit first dereference
    if (deref == 0) return index;
    struct bn* counter = &zero;
    if (cmp(index, &bn_csize) == SMALLER) counter = &counters[to_int(index)];
    if (counter == NULL) counter = &zero;
    return c_get(counter, --deref);
}
void c_add(struct bn* index, struct bn* amount) {
    int presize = csize;
    while (cmp(index, &bn_csize) != SMALLER) {
        bool move_index  = index  != &zero && index  != &tmp1 && index  != &tmp2;
        bool move_amount = amount != &zero && amount != &tmp1 && amount != &tmp2;
        size_t indexdiff = index - counters;
        size_t amountdiff = amount - counters;
        csize *= 2;
        struct bn* tmp = (struct bn*)realloc(counters, csize * sizeof(struct bn));
        if (tmp == NULL) {
            fprintf(stderr, "FAIL REALLOC\n");
            free(counters);
            exit(1);
        }
        counters = tmp;
        if (move_index)  index  = counters + indexdiff;
        if (move_amount) amount = counters + amountdiff;
        from_int(&bn_csize, csize);
    }
    for (int i = presize; i < csize; i++) {
        init(&counters[i]);
    }
    struct bn* counter = &counters[to_int(index)];
    add(&counters[to_int(index)], amount, &counters[to_int(index)]);
}
void debug() {
    printf("\x1B[H\x1B[2J");
    for (size_t i = 0; i < csize; i++) {
        printf("[%zu] = ", i);
        printnum(&counters[i]);
        printf(";\n");
    }
    getchar();
}

int main(int argc, char* argv[]) {
    init(&zero);
    init(&tmp1);
    init(&tmp2);
    from_int(&bn_csize, csize);
    counters = (struct bn*)malloc(sizeof(struct bn));
    if (counters == NULL) exit(1);
    init(&counters[0]);

    /*
    5+VALUE1 // Value 1
    1+VALUE2 // Value 2
    2+5      // Value tracker
    3+6      // Flag tracker
    4+7      // Result tracker

    *a1<       // Decrement loop
        1*aa2<
            *aa3<
                a4+1
                1&
            >
            a3+1
        >
        2+2
        3+2
        4+2
    >
    */
    c_add(val1(5), val2(9999)); // Value 1
    c_add(val1(1), val2(9998)); // Value 2
    c_add(val1(2), val2(5)); // Value tracker
    c_add(val1(3), val2(6)); // Flag tracker
    c_add(val1(4), val2(7)); // Result tracker
    struct bn i1;
    struct bn l1;
    init(&i1);
    assign(&l1, c_get(val1(1), 1));
    for (; cmp(&i1, &l1) == SMALLER; inc(&i1)) { // Decrement loop
        //debug();
        struct bn i2;
        struct bn l2;
        init(&i2);
        assign(&l2, c_get(val1(2), 2));
        for (; cmp(&i2, &l2) == SMALLER; inc(&i2)) {
            struct bn i3;
            struct bn l3;
            init(&i3);
            assign(&l3, c_get(val1(3), 2));
            for (; cmp(&i3, &l3) == SMALLER; inc(&i3)) {
                c_add(c_get(val1(4), 1), val2(1));
                goto a2;
            }
            c_add(c_get(val1(3), 1), val2(1));
            a2:;
        }
        c_add(val1(2), val2(2));
        c_add(val1(3), val2(2));
        c_add(val1(4), val2(2));
    }
    //debug();

    //printnum(c_get(val1(2), 2));
    

    free(counters);
}
