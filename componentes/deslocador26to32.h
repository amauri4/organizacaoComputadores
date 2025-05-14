#ifndef DESLOCADOR26TO28_H
#define DESLOCADOR26TO28_H

#include <systemc.h>

SC_MODULE(Shifter_26to28) {
    sc_in<sc_uint<26>> input;  
    sc_out<sc_uint<28>> output; 

    SC_CTOR(Shifter_26to28) {
        SC_METHOD(shift);
        sensitive << input;
    }

    void shift() {
        output.write(input.read() << 2); // Desloca 2 bits para a esquerda
    }
};

#endif 