#include <systemc.h>
#include "organizacaoMips.h"
#include <iostream>
#include <iomanip>

using namespace std;

// Função para imprimir registradores (deve ser implementada no seu MIPS)
void print_registradores(MIPS_Simplificado& mips) {
    mips.visualization_registers();
    cout << "Banco de Registradores:" << endl;
    for(int i = 0; i < 4; ++i) {  // Mostra apenas os primeiros 4 para exemplo
        cout << "$" << dec << i << " = 0x" << hex << setw(8) 
             << mips.registers[i] << endl;
    }
}

// Função para verificar valores nos registradores
void verifica_registrador(MIPS_Simplificado& mips, int reg, int esperado) {
    int valor = mips.registers[reg];
    cout << "Verificando $" << dec << reg << " = 0x" << hex << valor;
    
    if(valor == esperado) {
        cout << " \033[32m[OK]\033[0m" << endl;
    } else {
        cout << " \033[31m[ERRO] Esperado: 0x" << hex << esperado 
             << "\033[0m" << endl;
    }
}

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
    
    // Programa de teste com operações no banco de registradores
    vector<uint32_t> programa_teste = {
        0x08100000,  // addi $1, $0, 5     | $1 = 5
        //0x2002000A,  // addi $2, $0, 10    | $2 = 10
        //0x00221820,  // add $3, $1, $2     | $3 = 15
        //0xAC030000   // sw $3, 0($0)       | Mem[0] = 15
    };
    
    mips.load_program(programa_teste);

    // 4. Verificação do carregamento
    cout << "Conteúdo da memória:" << endl;
    for (size_t i = 0; i < mips.imem->mem.size(); ++i) {
        cout << "Mem[0x" << hex << setw(4) << setfill('0') << (i*4) 
             << "] = 0x" << setw(8) << mips.imem->mem[i] << endl;
    }

    // 5. Simulação passo a passo com verificação de registradores
    cout << "\nIniciando simulação..." << endl;
    
    // Reset inicial
    reset.write(true);
    sc_start(15, SC_NS);
    reset.write(false);
    cout << "Reset concluído @ " << sc_time_stamp() << endl;

    // Execução de ciclos com monitoramento detalhado
    for (int ciclo = 0; ciclo < 5; ++ciclo) {
        cout << "\n--- Ciclo " << dec << ciclo << " ---" << endl;
        
        // Estado pré-clock
        //cout << "Estado pré-clk @ " << sc_time_stamp() << ":" << endl;
        //cout << "PC: 0x" << hex << setw(8) << debug_pc.read() << endl;
        //cout << "Instrução: 0x" << setw(8) << debug_instruction.read() << endl;
        //print_registradores(mips);  // Função personalizada

        // Avança um ciclo
        sc_start(10, SC_NS);

        // Estado pós-clock
        //cout << "\nEstado pós-clk @ " << sc_time_stamp() << ":" << endl;
        //cout << "Próximo PC: 0x" << setw(8) << mips.pc_next.read() << endl;
        
        // Verificações específicas
        switch(ciclo) {
            case 1: verifica_registrador(mips, 1, 5); break;   // $1 = 5
            case 2: verifica_registrador(mips, 2, 10); break;  // $2 = 10
            case 3: verifica_registrador(mips, 3, 15); break;  // $3 = 15
        }
    }

    cout << "\nSimulação concluída!" << endl;
    return 0;
}