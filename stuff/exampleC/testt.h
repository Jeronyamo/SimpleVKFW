//#include <stddef.h>
#include <stdio.h>

struct TRUE_S {};
struct FALSE_S {};

#define True struct TRUE_S
#define False struct FALSE_S
#define TrueExpr (struct TRUE_S){}
#define FalseExpr (struct FALSE_S){}

// Type comparator
#define is_compatible(x, T) _Generic((x), T: TrueExpr, default: FalseExpr)

//FOR CLANG/GCC
/*
#define is_same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define is_ptr_or_array(p) (__builtin_classify_type(p) == 5)
#define decay(a)           (&*__builtin_choose_expr(is_ptr_or_array(a), a, NULL))
#define is_pointer(p)      is_same_type(p, decay(p))
//#define Tmp(x) _Generic(__builtin_choose_expr(is_pointer(x), (struct True){},(struct False){}), struct True : (), struct False : ()) 
*/

static void _blank_func(int x, ...) { return; }

typedef int p(int q, int r);
int f(int q, int r) { return 0; }
struct Tmp { int a; p *g; };


static struct Tmp TmpConstructorCopy(struct Tmp _v) {
    return _v;
}
static struct Tmp TmpConstructorArg1(int _a) {
    return (struct Tmp){_a, NULL};
}
static struct Tmp TmpConstructorArg2(p* _func) {
    return (struct Tmp){0, _func};
}

// In simplest cases "constructors" don't require function calls, but result in justified warnings
//#define Tmp(x) _Generic(is_compatible(x, p*), True : (struct Tmp) {4, x}, False : (struct Tmp) {x, NULL})

#define Tmp(x) _Generic(is_compatible(x,        int), True : TmpConstructorArg1, False:\
               _Generic(is_compatible(x,         p*), True : TmpConstructorArg2, False:\
               _Generic(is_compatible(x, struct Tmp), True : TmpConstructorCopy, False : _blank_func)))(x)
