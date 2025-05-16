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
    sc_int<32> memory_data[MEM_SIZE] = {0};  // Inicializa com zeros

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

    // Programa de teste completo
    vector<uint32_t> programa_teste = {
        // Inicialização de registradores
        0x20010005, // addi $1, $0, 5       | $1 = 5
        0x2002000A, // addi $2, $0, 10      | $2 = 10
        0x20030000, // addi $3, $0, 0       | $3 = 0 (contador)
        
        // Teste de BEQ (não deve pular)
        0x10220002, // beq $1, $2, 2        | if ($1 == $2) jump +2 (não vai pular)
        0x20630001, // addi $3, $3, 1       | $3++ (deve executar)
        0x20630001, // addi $3, $3, 1       | $3++ (deve executar)
        
        // Teste de BEQ (deve pular)
        0x10210002, // beq $1, $1, 2        | if ($1 == $1) jump +2 (deve pular)
        0x20630001, // addi $3, $3, 1       | $3++ (não deve executar)
        0x20630001, // addi $3, $3, 1       | $3++ (não deve executar)
        
        // Teste de BNE (deve pular)
        0x14220002, // bne $1, $2, 2        | if ($1 != $2) jump +2 (deve pular)
        0x20630001, // addi $3, $3, 1       | $3++ (não deve executar)
        0x20630001, // addi $3, $3, 1       | $3++ (não deve executar)
        
        // Teste de J (jump absoluto)
        0x08000011, // j 0x00000011         | jump para a instrução no endereço 0x44
        
        // Instruções que não devem executar (puladas pelo jump)
        0x200400FF, // addi $4, $0, 0xFF    | não deve executar
        0x200500FF, // addi $5, $0, 0xFF    | não deve executar
        
        // Destino do jump (endereço 0x44)
        0x20060011, // addi $6, $0, 0x11    | $6 = 0x11 (deve executar)
        
        // Teste de JAL (jump and link)
        0x0C000015, // jal 0x00000015       | salta para subrotina em 0x54, armazena endereço em $31
        
        // Continuação após retorno
        0x20070022, // addi $7, $0, 0x22    | $7 = 0x22
        
        // Fim do programa
        0x08000019, // j 0x00000019         | loop infinito
        
        // Subrotina (endereço 0x54)
        0x23E80004, // addi $8, $31, 4      | $8 = endereço de retorno + 4
        0x20090033, // addi $9, $0, 0x33    | $9 = 0x33
        0x03E00008, // jr $31               | retorna da subrotina
        
        // Loop infinito (endereço 0x64)
        0x08000019  // j 0x00000019         | loop infinito
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

    int num_ciclos = 10;
    // Execução passo a passo com verificações detalhada
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