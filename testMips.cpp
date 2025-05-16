#include <systemc.h>
#include "organizacaoMips.h"
#include <iostream>
#include <iomanip>

using namespace std;

// Função para imprimir registradores
void print_registradores(MIPS_Simplificado& mips) {
    mips.visualization_registers();
    cout << "Banco de Registradores:" << endl;
    for(int i = 0; i < 32; ++i) {
        if(mips.registers[i] != 0) { // Mostra apenas registradores não nulos
            cout << "$" << dec << i << " = 0x" << hex << setw(8) 
                 << mips.registers[i] << " (" << dec << mips.registers[i] << ")" << endl;
        }
    }
}

// Função para verificar valores
void verifica_registrador(MIPS_Simplificado& mips, int reg, uint32_t esperado_hex, string teste) {
    //uint32_t valor = mips.registers[reg];
    cout << "Teste " << teste << ": $" << dec << reg << " = 0x" << hex << setw(8) << mips.ula_result;
    // if(valor == esperado_hex) {
    //     cout << " \033[32m[OK]\033[0m" << endl;
    // } else {
    //     cout << " \033[31m[ERRO] Esperado: 0x" << hex << setw(8) << esperado_hex 
    //          << "\033[0m" << endl;
    // }
}

int sc_main(int argc, char* argv[]) {
    // Configuração inicial
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<bool> reset;
    sc_signal<sc_uint<32>> debug_pc;
    sc_signal<sc_uint<32>> debug_instruction;
    MIPS_Simplificado mips("MIPS");
    mips.clk(clk);
    mips.reset(reset);
    mips.debug_pc(debug_pc);
    mips.debug_instruction(debug_instruction);

    // Programa de teste com operações diversificadas
    vector<uint32_t> programa_teste = {
        0x20010005, // addi $1, $0, 5       | $1 = 5
        //0x2002000A, // addi $2, $0, 10      | $2 = 10
        //0x00221820, // add  $3, $1, $2      | $3 = 15 (R-type)
        //0x00222022, // sub  $4, $1, $2      | $4 = -5 (R-type)
        //0x2005FFFB, // addi $5, $0, -5      | $5 = -5 (I-type)
        //0x8C060000, // lw   $6, 0($0)       | Carrega Mem[0] em $6
        //0xAC070000, // sw   $7, 0($0)       | Armazena $7 em Mem[0]
       // 0x10220002, // beq  $1, $2, 2       | Branch se $1 == $2 (não tomado)
        //0x0800000A, // j    0x0000000A      | Jump absoluto
        //0x20080064  // addi $8, $0, 100     | $8 = 100 (não executado por causa do jump)
    };

    mips.load_program(programa_teste);

    // Reset inicial
    reset.write(true);
    sc_start(15, SC_NS);
    reset.write(false);
    cout << "Reset concluído @ " << sc_time_stamp() << endl;

    // Execução passo a passo com verificações
    for (int ciclo = 0; ciclo < 2; ++ciclo) { //10
        cout << "\n--- Ciclo " << dec << ciclo << " ---" << endl;
        cout << "PC: 0x" << hex << setw(8) << mips.pc_out.read() << endl;
        cout << "Instrução: 0x" << setw(8) << mips.instruction.read() << endl;

        // Monitoramento dos sinais de controle
        cout << "Sinais de Controle:" << endl;
        cout << "  ALUSrc: " << mips.alu_src.read() << " | RegWrite: " << mips.reg_write.read() 
             << " | ALUOp: " << mips.alu_op.read() << "| Branch:" << mips.branch.read()
             << " | Jump: " << mips.jump.read()  << " | MemRead:" <<mips.mem_read << " | MemToReg: " <<mips.mem_to_reg 
             << " | MemWrite" << mips.mem_write << " | RegDest"<< mips.reg_dst << endl;
        cout << "SAÍDA ULA ->" << mips.ula_result << '\n';
        // Executa um ciclo
        sc_start(10, SC_NS);

        // Verificações específicas
        switch(ciclo) {
            case 1: verifica_registrador(mips, 1, 0x00000005, "addi $1, 5"); break;
            case 2: verifica_registrador(mips, 2, 0x0000000A, "addi $2, 10"); break;
            case 3: verifica_registrador(mips, 3, 0x0000000F, "add $3, $1+$2"); break;
            case 4: verifica_registrador(mips, 4, 0xFFFFFFFB, "sub $4, $1-$2"); break;
            case 5: verifica_registrador(mips, 5, 0xFFFFFFFB, "addi $5, -5"); break;
            case 6: cout << "Operação: lw $6, 0($0)" << endl; break;
            case 7: cout << "Operação: sw $7, 0($0)" << endl; break;
            case 8: cout << "Operação: beq (não tomado)" << endl; break;
            case 9: cout << "Operação: jump" << endl; break;
        }
    }

    cout << "\n=== Estado Final ===" << endl;
    print_registradores(mips);
    return 0;
}