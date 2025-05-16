#ifndef MUXJUMP_H
#define MUXJUMP_H

#include <systemc.h>

template <int WIDTH>
SC_MODULE(MuxJump) {
    sc_in<sc_int<32>> branch_mux_out;  // Entrada 0: Saída do Branch MUX (com sinal)
    sc_in<sc_uint<32>> jump_target;    // Entrada 1: Jump Target (sem sinal)
    sc_in<bool> jump_taken;            // Controle (1 = jump, 0 = mantém branch/PC+4)
    sc_out<sc_uint<32>> pc_next;       // Saída final para o PC (sem sinal)

    void process() {
        if (jump_taken.read()) {
            pc_next.write(jump_target.read());  // Usa jump_target se jump for tomado
        } else {
            // Converte branch_mux_out (sc_int) para sc_uint (endereço bruto)
            pc_next.write(sc_uint<32>(branch_mux_out.read()));
        }
    }

    SC_CTOR(MuxJump) {
        SC_METHOD(process);
        sensitive << branch_mux_out << jump_target << jump_taken;
    }
};

#endif // MUX_H