#include <systemc.h>
#include "organizacaoMips.h"

int sc_main(int argc, char* argv[]) {
    // Sinais principais
    sc_clock clk("clk", 10, SC_NS);  // Clock com período de 10 ns
    sc_signal<bool> reset;

    // Sinais de debug
    sc_signal<sc_uint<32>> debug_pc;
    sc_signal<sc_uint<32>> debug_instruction;

    // Instancia o módulo MIPS
    MIPS_Simplificado mips("MIPS");
    mips.clk(clk);
    mips.reset(reset);
    mips.debug_pc(debug_pc);
    mips.debug_instruction(debug_instruction);

    // Inicia simulação
    reset.write(true);
    sc_start(10, SC_NS);  // ciclo inicial com reset ativado
    reset.write(false);

    // Roda por alguns ciclos
    for (int i = 0; i < 5; ++i) {
        sc_start(10, SC_NS);
    }

    return 0;
}
