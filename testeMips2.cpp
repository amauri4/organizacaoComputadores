#include <systemc.h>
#include "organizacaoMips.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

// Função para imprimir registradores com mais detalhes
void print_registradores(MIPS_Simplificado& mips) {
    mips.visualization_registers();
    cout << "Banco de Registradores:" << endl;
    for(int i = 0; i < 32; ++i) {
        cout << "$" << dec << setw(2) << i << " = 0x" << hex << setw(8) << setfill('0') 
             << mips.registers[i] << " (" << dec << setfill(' ') << setw(10) 
             << mips.registers[i] << ")" << endl;
    }
}

void print_memory(MIPS_Simplificado& mips, bool show_all = false) {
    // Obter dados da memória
    sc_int<32> memory_data[32];  // Assumindo memória de 1KB (1024 palavras)
    mips.memoriaDados->dump_memory(memory_data);

    cout << "Memória de Dados:" << endl;
    
    // Contador para linhas com dados
    int count = 0;
    
    // Mostrar cabeçalho a cada 8 linhas
    const int HEADER_INTERVAL = 8;
    
    for(int addr = 0; addr < 1024*4; addr += 4) {
        int word_index = addr / 4;
        sc_int<32> word = memory_data[word_index];
        
        // Mostrar apenas palavras não-zero ou se show_all for true
        if(show_all || word != 0) {
            // Mostrar cabeçalho a cada HEADER_INTERVAL linhas
            if(count % HEADER_INTERVAL == 0) {
                cout << "╔═══════════╦═════════════════════════╗" << endl;
                cout << "║ Endereço   ║ Dado                    ║" << endl;
                cout << "╠═══════════╬═════════════════════════╣" << endl;
            }
            
            cout << "║ 0x" << hex << setw(8) << setfill('0') << addr << " ║ "
                 << "0x" << setw(8) << word << " (" 
                 << dec << setfill(' ') << setw(10) << word << ") ║" << endl;
            
            count++;
            
            // Fechar bloco a cada HEADER_INTERVAL linhas
            if(count % HEADER_INTERVAL == 0) {
                cout << "╚═══════════╩═════════════════════════╝" << endl << endl;
            }
        }
    }
    
    // Fechar último bloco se não estiver completo
    if(count % HEADER_INTERVAL != 0) {
        cout << "╚═══════════╩═════════════════════════╝" << endl << endl;
    }
    
    cout << "Total de palavras exibidas: " << dec << count << endl;
}

// Função para verificar valores com mais detalhes
void verifica_valor(const string& nome, uint32_t valor, uint32_t esperado) {
    cout << "  " << nome << ": 0x" << hex << setw(8) << setfill('0') << valor;
    if(valor == esperado) {
        cout << " \033[32m[OK]\033[0m";
    } else {
        cout << " \033[31m[ERRO] Esperado: 0x" << hex << setw(8) << esperado << "\033[0m";
    }
    cout << endl;
}

int sc_main(int argc, char* argv[]) {
    // Configuração inicial
    sc_clock clk("clk", 10, SC_NS, 0.5);
    sc_signal<bool> reset;
    sc_signal<sc_uint<32>> debug_pc;
    sc_signal<sc_uint<32>> debug_instruction;
    MIPS_Simplificado mips("MIPS");
    mips.clk(clk);
    mips.reset(reset);
    mips.debug_pc(debug_pc);
    mips.debug_instruction(debug_instruction);

    // Programa de teste completo
    vector<uint32_t> programa_teste = {
        0x20010005, // addi $1, $0, 5       | $1 = 5
        0x2002000A, // addi $2, $0, 10      | $2 = 10
        0x00221820, // add  $3, $1, $2      | $3 = 15 (R-type)
        0x00222022, // sub  $4, $1, $2      | $4 = -5 (R-type)
        0x2005FFFB, // addi $5, $0, -5      | $5 = -5 (I-type)
        0x00253020, // add  $6, $1, $5      | $6 = 0 (teste com negativo)
        0x00223825, // or   $7, $1, $2     | $7 = 15 (5 | 10)
        0x00224024, // and  $8, $1, $2     | $8 = 0 (5 & 10)
        //0x0022482A, // slt  $9, $1, $2     | $9 = 1 (5 < 10)
        0x202A0003, // addi $10, $1, 3     | $10 = 8
        //0x0800000B, // j    0x0000000B      | Jump para a última instrução
        0x200B00BB, // addi $11, $0, 0xBB   | Não deve executar (pulado pelo jump)
        0xAC010020  // sw   $1, 32($0)     | Mem[32] = 5
    };

    mips.load_program(programa_teste);

    // Arquivo de trace para GTKWave
    sc_trace_file *tf = sc_create_vcd_trace_file("mips_trace");
    sc_trace(tf, clk, "clk");
    sc_trace(tf, reset, "reset");
    sc_trace(tf, mips.pc_out, "pc");
    sc_trace(tf, mips.instruction, "instruction");
    sc_trace(tf, mips.read_data_1, "read_data1");
    sc_trace(tf, mips.read_data_2, "read_data2");
    sc_trace(tf, mips.output_mux_ula, "ula_operand2");
    sc_trace(tf, mips.alu_control, "alu_control");
    sc_trace(tf, mips.ula_result, "ula_result");
    sc_trace(tf, mips.zero, "zero");
    sc_trace(tf, mips.reg_write, "reg_write");
    sc_trace(tf, mips.reg_dst, "reg_dst");
    sc_trace(tf, mips.alu_src, "alu_src");

    // Reset inicial
    reset.write(true);
    sc_start(30, SC_NS);
    reset.write(false);
    cout << "Reset concluído @ " << sc_time_stamp() << endl;

    int num_ciclos = 8;
    // Execução passo a passo com verificações detalhadas
    for (int ciclo = 0; ciclo < num_ciclos; ++ciclo) {

        cout << "\n────────── INÍCIO CICLO " << dec << ciclo << " ──────────" << endl;

        mips.debug_execution();
        sc_start(10, SC_NS); // Avança um ciclo de clock

        cout << "────────── FIM CICLO " << ciclo << " ────────────" << endl;

    }

    cout << "\n=== Estado Final ===" << endl;
    print_registradores(mips);
    print_memory(mips);
    sc_close_vcd_trace_file(tf);
    return 0;
}