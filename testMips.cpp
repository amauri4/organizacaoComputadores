#include <systemc.h>
#include "organizacaoMips.h"
#include <iostream>

using namespace std;

int sc_main(int argc, char* argv[]) {
    // 1. Configuração inicial
    sc_clock clk("clk", 10, SC_NS);  // Clock de 10 ns
    sc_signal<bool> reset;
    sc_signal<sc_uint<32>> debug_pc;
    sc_signal<sc_uint<32>> debug_instruction;

    // 2. Instanciação do MIPS
    MIPS_Simplificado mips("MIPS");
    mips.clk(clk);
    mips.reset(reset);
    mips.debug_pc(debug_pc);
    mips.debug_instruction(debug_instruction);

    // 3. Carregamento do programa na memória
    cout << "Carregando instruções na memória..." << endl;
    
    // Programa de teste básico (MIPS)
    std::vector<uint32_t> programa_teste = {
        0x20010005,  // addi $1, $0, 5
        0x2002000A,  // addi $2, $0, 10
        0x00221820   // add $3, $1, $2
    };
    
    mips.load_program(programa_teste);

    // 4. Verificação do carregamento
    cout << "Conteúdo da memória:" << endl;
    for (size_t i = 0; i < mips.imem->mem.size(); ++i) {
        cout << "Mem[" << i*4 << "]: 0x" << hex << mips.imem->mem[i] << endl;
    }

    // 5. Simulação passo a passo
    cout << "\nIniciando simulação..." << endl;
    
    // Reset inicial
    reset.write(true);
    sc_start(15, SC_NS);
    reset.write(false);
    cout << "Reset concluído @ " << sc_time_stamp() << endl;

    // Execução de ciclos com monitoramento
    for (int i = 0; i < 4; ++i) {
        cout << "\n--- Ciclo " << i << " ---" << endl;
        cout << "Estado pré-clk @ " << sc_time_stamp() << ":" << endl;
        cout << "PC: 0x" << hex << debug_pc.read() << endl;
        cout << "Instrução: 0x" << debug_instruction.read() << endl;

        sc_start(10, SC_NS);  // Avança um ciclo de clock

        cout << "Estado pós-clk @ " << sc_time_stamp() << ":" << endl;
        cout << "PC: 0x" << debug_pc.read() << endl;
        cout << "Próximo PC: 0x" << mips.pc_next.read() << endl;
    }

    cout << "\nSimulação concluída com sucesso!" << endl;
    return 0;
}


