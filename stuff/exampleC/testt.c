#include "testt.h"



int main(void) {
    int u;
    struct Tmp x = {5, NULL};
    struct Tmp y = Tmp(x);
    printf("%d\n", y.a);
    TmpConstructorArg1(2);
    return 0;
};