#ifndef SOMADORENDERECOS_H
#define SOMADORENDERECOS_H

#include <systemc.h>

SC_MODULE(AdderAddress) {
    sc_in<sc_uint<32>> a;     // PC+4 (sc_uint)
    sc_in<sc_int<32>> b;      // Offset estendido (sc_int)
    sc_out<sc_int<32>> sum;   // Saída (sc_int)

    SC_CTOR(AdderAddress) {
        SC_METHOD(add);
        sum.write(sc_int<32>(a.read()) + b.read());
    }

    void add() {
        sum.write(a.read() + b.read());
    }
};

#endif 