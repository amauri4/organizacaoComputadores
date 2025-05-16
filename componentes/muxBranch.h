#ifndef MUXBRANCH_H
#define MUXBRANCH_H

#include <systemc.h>

template <int WIDTH>
SC_MODULE(MuxBranch) {
    sc_in<sc_uint<32>> pc_plus4;       // Entrada 0: PC+4 (sem sinal)
    sc_in<sc_int<32>> branch_target;   // Entrada 1: Branch Target (com sinal)
    sc_in<bool> branch_taken;          // Controle (1 = branch, 0 = PC+4)
    sc_out<sc_int<32>> out;            // Saída (com sinal)

    SC_CTOR(MuxBranch) {
        SC_METHOD(process);
        sensitive << pc_plus4 << branch_target << branch_taken;
    }

    void process() {
        if (branch_taken.read()) {
            out.write(branch_target.read());  // Usa branch_target se branch for tomado
        } else {
            // Converte pc_plus4 (sc_uint) para sc_int (sem alterar o valor)
            out.write(sc_int<32>(pc_plus4.read()));
        }
    }
};

#endif // MUX_H