#ifndef MUX_H
#define MUX_H

#include <systemc.h>

template <int WIDTH, typename T = sc_uint<WIDTH>>  // T pode ser sc_uint ou sc_int
SC_MODULE(Mux) {
    sc_in<T> input0, input1;  // Entradas genéricas (uint ou int)
    sc_in<bool> sel;
    sc_out<T> output;         // Saída do mesmo tipo

    void process() {
        output.write(sel.read() ? input1.read() : input0.read());
    }

    SC_CTOR(Mux) {
        SC_METHOD(process);
        sensitive << input0 << input1 << sel;
    }
};

#endif // MUX_H