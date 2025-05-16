#include <systemc.h>
#include "organizacaoMips.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

// Função para imprimir registradores com mais detalhe
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
    using std::cout;
    using std::endl;
    using std::hex;
    using std::dec;
    using std::setw;
    using std::setfill;
    
    // Verificação de segurança
    if (!mips.memoriaDados) {
        cout << "Erro: Memória de dados não inicializada!" << endl;
        return;
    }

    // Tamanho seguro para a memória
    const int MEM_SIZE = 32;
    sc_int<32> memory_data[MEM_SIZE] = {0};  // Inicializa com zero

    // Copia segura (evita overflow)
    try {
        mips.memoriaDados->dump_memory(memory_data);
    } catch (...) {
        cout << "Erro ao acessar memória!" << endl;
        return;
    }

    cout << "Memória de Dados (32 palavras):" << endl;
    cout << "╔════════════╦═════════════════════════╗" << endl;
    cout << "║ Endereço   ║ Dado                    ║" << endl;
    cout << "╠════════════╬═════════════════════════╣" << endl;
    
    int count = 0;
    
    for(int i = 0; i < 32; i++) {
        sc_int<32> word = memory_data[i];
        
        if(show_all || word != 0) {
            uint32_t unsigned_word = static_cast<uint32_t>(word);
            int addr = i * 4;  // Cada palavra ocupa 4 bytes
            
            cout << "║ 0x" << hex << setw(8) << setfill('0') << addr << " ║ "
                 << "0x" << setw(8) << unsigned_word << " (" 
                 << dec << setfill(' ') << setw(10) << word << ") ║" << endl;
            
            count++;
        }
    }
    
    cout << "╚════════════╩═════════════════════════╝" << endl;
    cout << "Palavras não-zero: " << dec << count << "/32" << endl << endl;
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

    vector<uint32_t> teste_aritmetica = {
        /*0x00*/ 0x20010005, // addi $1, $0, 5       | $1 = 5
        /*0x04*/ 0x2002000A, // addi $2, $0, 10      | $2 = 10
        /*0x08*/ 0x00221820, // add $3, $1, $2       | $3 = 15 (5+10)
        /*0x0C*/ 0x2064000F, // addi $4, $3, 15      | $4 = 30 (15+15)
        /*0x10*/ 0x08000004  // j 0x00000004         | Loop (PC=0x10)
    };

    vector<uint32_t> teste_branch = {
        /*0x00*/ 0x20010005, // addi $1, $0, 5       | $1 = 5
        /*0x04*/ 0x2002000A, // addi $2, $0, 10      | $2 = 10
        /*0x08*/ 0x10220002, // beq $1, $2, 2        | Não pula (5 != 10)
        /*0x0C*/ 0x20630001, // addi $3, $3, 1       | $3 = 1 (EXECUTA)
        /*0x10*/ 0x10210001, // beq $1, $1, 1       | Pula próxima instrução
        /*0x14*/ 0x20630001, // addi $3, $3, 1       | PULADO
        /*0x18*/ 0x08000006  // j 0x00000006         | Loop (PC=0x18)
    };

    vector<uint32_t> teste_memoria = {
        /*0x00*/ 0x20060011, // addi $6, $0, 0x11    | $6 = 0x11
        /*0x04*/ 0xAC060020, // sw $6, 0x20($0)      | Mem[0x20] = 0x11
        /*0x08*/ 0x8C080020, // lw $8, 0x20($0)      | $8 = 0x11
        /*0x0C*/ 0x08000003  // j 0x00000003         | Loop (PC=0x0C)
    };

    vector<uint32_t> teste_logico = {
        /*0x00*/ 0x2001000F, // addi $1, $0, 0x0F    | $1 = 0x0F
        /*0x04*/ 0x2002003C, // addi $2, $0, 0x3C    | $2 = 0x3C
        /*0x08*/ 0x00222024, // and $4, $1, $2       | $4 = 0x0C (0x0F & 0x3C)
        /*0x0C*/ 0x00222825, // or $5, $1, $2        | $5 = 0x3F (0x0F | 0x3C)
        /*0x10*/ 0x00223027, // nor $6, $1, $2       | $6 = ~(0x0F | 0x3C)
        /*0x14*/ 0x08000005  // j 0x00000005         | Loop (PC=0x14)
    };

    vector<uint32_t> teste_jump = {
        /*0x00*/ 0x20010001, // addi $1, $0, 1       | $1 = 1
        /*0x04*/ 0x08000008, // j 0x00000008         | Pula para 0x20
        /*0x08*/ 0x20010002, // addi $1, $0, 2       | PULADO
        /*0x0C*/ 0x20010003, // addi $1, $0, 3       | PULADO
        /*0x10*/ 0x20010004, // addi $1, $0, 4       | PULADO
        /*0x14*/ 0x20010005, // addi $1, $0, 5       | PULADO
        /*0x18*/ 0x20010006, // addi $1, $0, 6       | PULADO
        /*0x1C*/ 0x20010007, // addi $1, $0, 7       | PULADO
        /*0x20*/ 0x2002000A, // addi $2, $0, 10      | $2 = 10 (DESTINO)
        /*0x24*/ 0x08000009  // j 0x00000009         | Loop (PC=0x24)
    };

    vector<uint32_t> programa_final = {
        // Inicialização
        /*0x00*/ 0x20010005, // addi $1, $0, 5       | $1 = 5
        /*0x04*/ 0x2002000A, // addi $2, $0, 10      | $2 = 10
        
        // Teste aritmético
        /*0x08*/ 0x00221820, // add $3, $1, $2       | $3 = 15
        
        // Teste memory
        /*0x0C*/ 0xAC030020, // sw $3, 0x20($0)      | Mem[0x20] = 15
        /*0x10*/ 0x8C040020, // lw $4, 0x20($0)      | $4 = 15
        
        // Teste lógico
        /*0x14*/ 0x00222824, // and $5, $1, $2       | $5 = 5 & 10 = 0
        
        // Teste branch
        /*0x18*/ 0x10210002, // beq $1, $1, 2        | Pula para 0x20
        /*0x1C*/ 0x20060001, // addi $6, $0, 1       | PULADO
        
        // Destino do branch
        /*0x20*/ 0x08000008  // j 0x00000008         | Loop (PC=0x20)
    };

    mips.load_program(teste_memoria);

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

    int num_ciclos = 5;
    // Execução passo a passo com verificações detalhada
    for (int ciclo = 0; ciclo < num_ciclos; ++ciclo) {

        cout << "\n────────── INÍCIO CICLO " << dec << ciclo << " ──────────" << endl;

        mips.debug_execution();
        sc_start(10, SC_NS); // Avança um ciclo de clock

        cout << "────────── FIM CICLO " << ciclo << " ────────────" << endl;
        print_registradores(mips);
    }

    cout << "\n=== Estado Final ===" << endl;
    print_registradores(mips);
    print_memory(mips, true);
    sc_close_vcd_trace_file(tf);
    return 0;
}