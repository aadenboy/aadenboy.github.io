#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "bn.c"

/*
#define init        bignum_init
#define from_int    bignum_from_int
#define add         bignum_add
#define cmp         bignum_cmp
#define is_zero     bignum_is_zero
#define inc         bignum_inc
#define dec         bignum_dec
#define assign      bignum_assign
*/
struct bni { // bignum + infinity
    bool inf;
    struct bn* num;
};
void init(struct bni* n) {
    n->inf = false;
    bignum_init(n->num);
}
void from_int(struct bni* n, int i) {
    n->inf = false;
    bignum_from_int(n->num, i);
}
void add(struct bni* a, struct bni* b, struct bni* c) {
    if (a->inf || b->inf) {
        c->inf = true;
        return;
    }
    bignum_add(a->num, b->num, c->num);
}
int cmp(struct bni* a, struct bni* b) {
    if (a->inf && !b->inf) return LARGER;
    if (a->inf && b->inf) return EQUAL;
    if (!a->inf && b->inf) return SMALLER;
    return bignum_cmp(a->num, b->num);
}
int is_zero(struct bni* n) {
    if (n->inf) return 0;
    return bignum_is_zero(n->num);
}
int is_infinite(struct bni* n) {
    return n->inf;
}
void inc(struct bni* n) {
    if (!n->inf) bignum_inc(n->num);
}
void dec(struct bni* n) {
    if (!n->inf) bignum_dec(n->num);
}
void assign(struct bni* a, struct bni* b) {
    a->inf = b->inf;
    if (!a->inf) bignum_assign(a->num, b->num);
}

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
void printnum(struct bni* num) {
    if (is_infinite(num)) {
        printf("∞");
        return;
    }
    if (is_zero(num)) {
        printf("0");
        return;
    }
    struct bn ten;
    from_int(&ten, 10);

    struct bn original;
    assign(&original, num->num);
    struct bn tmp;
    struct bn digit;
    init(&tmp);
    init(&digit);

    char *numstr = malloc(1);
    char *renumstr;
    size_t len = 0;
    size_t buf = 1;
    while (!is_zero(&original)) {
        bignum_divmod(&original, &ten, &tmp, &digit);
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

struct bni zero;
struct bni infinity;
struct bni tmp1;
struct bni tmp2;
struct bni* counters;
struct bni inf_counter;
size_t csize = 1;
struct bni bn_csize;

struct bni* val1(int value) {
    from_int(&tmp1, value);
    return &tmp1;
}
struct bni* val2(int value) {
    from_int(&tmp2, value);
    return &tmp2;
}

struct bni* c_get(struct bni* index, int deref) { // left side of commands has implicit first dereference
    struct bni* counter = index;
    for (int i = 0; i < deref; i++) {
        if (is_infinite(counter)) counter = &inf_counter;
        else if (cmp(counter, &bni_csize) == SMALLER) counter = &counters[to_int(counter)];
        else counter = &zero;
    }
    return counter;
}
void c_add(struct bn* index, struct bn* amount) {
    int presize = csize;
    while (!is_infinite(index) && cmp(index, &bn_csize) != SMALLER) {
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
    if (is_infinite(index)) {
        add(&inf_counter, amount, &inf_counter);
    } else {
        struct bn* counter = &counters[to_int(index)];
        add(&counters[to_int(index)], amount, &counters[to_int(index)]);
    }
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
    init(&infinity);
    infinity.inf = true;
    init(&tmp1);
    init(&tmp2);
    from_int(&bn_csize, csize);
    counters = (struct bn*)malloc(sizeof(struct bn));
    if (counters == NULL) exit(1);
    init(&counters[0]);

    /*
    1+VALUE1 // Value 1
    2+VALUE2 // Value 2
    3+a2     // Temporary variable

    a1*1<    // Same equality check as demonstrated later
        *a1<   // Increment the result by value 2
            a3&  // This will jump to the outer loop when the temp variable equals value 1
            3+1  // Since the temp variable already equals value 2, this will be true after Value 1 - Value 2 incremnets
            4+1  // And we just track every increment
        >
    >
    */
    c_add(val1(1), val2(99999)); // Value 1
    c_add(val1(2), val2(99998)); // Value 2
    c_add(val1(3), c_get(val2(2), 1)); // Temporary variable

    struct bn j1;
    assign(&j1, c_get(val2(1), 1));
    struct bn i1;
    assign(&i1, val2(1));
    for (; !is_zero(&i1); dec(&i1)) { // Decrement loop
        struct bn i2;
        assign(&i2, c_get(val2(1), 1));
        for (; !is_zero(&i2); dec(&i2)) {
            if (cmp(c_get(val1(3), 1), &j1) == EQUAL)
                goto l1;
            c_add(val1(3), val2(1));
            c_add(val1(4), val2(1));
        }
        l1:;
    }
    //debug();

    //printnum(c_get(val1(2), 2));
    

    free(counters);
}
