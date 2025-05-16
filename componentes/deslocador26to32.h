#ifndef DESLOCADOR_26_TO_32
#define DESLOCADOR_26_TO_32

#include <systemc.h>

SC_MODULE(Shifter_26to32) {
    sc_in<sc_uint<4>> pc_high;           // Bits 31..28 do PC atual
    sc_in<sc_uint<26>> instr_index;      // Campo de 26 bits da instrução J
    sc_out<sc_uint<32>> jump_address;    // Endereço final de jump (PC[31:28] + instr_index << 2)

    SC_CTOR(Shifter_26to32) {
        SC_METHOD(compute_jump_address);
        sensitive << pc_high << instr_index;
    }

    void compute_jump_address() {
        sc_uint<28> shifted = instr_index.read() << 2;       // 26 << 2 = 28 bits
        sc_uint<32> result = (pc_high.read(), shifted);      // Concatenação: 4 bits + 28 bits
        jump_address.write(result);
    }
};

#endif // SHIFTER_26TO32_H
