#ifndef SOMADOR_H
#define SOMADOR_H

#include <systemc.h>

SC_MODULE(Adder) {
    sc_in<sc_uint<32>> a;
    sc_in<sc_uint<32>> b;
    sc_out<sc_uint<32>> sum;

    SC_CTOR(Adder) {
        SC_METHOD(add);
        sensitive << a << b;
    }

    void add() {
        sum.write(a.read() + b.read());
    }
};

#endif // ADDER_H