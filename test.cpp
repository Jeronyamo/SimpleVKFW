#include <iostream>


// Decorators (kinda)

struct A {
    int v = 1;
    float h = 1.2;

    A() {}
    A(const A &_a) : v{_a.v}, h{_a.h} {}
    A(int _v) : v{_v} {}
    A(int _v, float _h) : v{_v}, h{_h} {}
    virtual ~A() {}

    virtual int get() { return v; }
    virtual float getf() { return h; }
};

struct B {
    int f = 4;
    float g = 2.3;

    B() {}
    B(const B &_b) : f{_b.f}, g{_b.g} {}
    B(int _f) : f{_f} {}
    B(int _f, float _g) : f{_f}, g{_g} {}
    virtual ~B() {}

    virtual int get() { return f; }
    virtual float getf() { return g; }
};


void f1() {
    return;
}

void f2() {
    return f1();
}


int main() {
    // A a;
    // B b;
    // C{a};
    // C<A> ca{{2, 3.4}};
    // C<B> cb{{3, 4.5}};
    // std::cout << ca.getf() << ", " << cb.getf() << std::endl;
    return 0;
}