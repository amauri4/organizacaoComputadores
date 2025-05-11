#ifndef MUX_H
#define MUX_H

#include <systemc.h>

template <int WIDTH>
SC_MODULE(Mux) {
    sc_in<sc_uint<WIDTH>> input0;
    sc_in<sc_uint<WIDTH>> input1;
    sc_in<bool> sel;
    sc_out<sc_uint<WIDTH>> output;

    SC_CTOR(Mux) {
        SC_METHOD(process);
        sensitive << input0 << input1 << sel;
    }

    void process() {
        output.write(sel.read() ? input1.read() : input0.read());
    }
};

#endif // MUX_H