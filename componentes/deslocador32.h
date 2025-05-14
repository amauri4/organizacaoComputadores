#ifndef DESLOCADOR32_H
#define DESLOCADOR32_H

#include <systemc.h>

SC_MODULE(Shifter_32b) {
    // Entrada: valor de 32 bits (saída do extensor de sinal)
    sc_in<sc_int<32>> input; 
    sc_out<sc_int<32>> output;

    SC_CTOR(Shifter_32b) {
        SC_METHOD(shift);
        sensitive << input;
    }

    void shift() {
        output.write(input.read() << 2);
    }
};

#endif