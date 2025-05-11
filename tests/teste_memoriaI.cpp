#include <systemc.h>
#include "../componentes/memoriaInstrucoes.h"
#include <cassert>

int sc_main(int argc, char* argv[]) {
    // 1. Inicialização dos Sinais
    sc_signal<sc_uint<32>> addr_signal;
    sc_signal<sc_uint<32>> instr_signal;
    sc_clock clock("clock", 10, SC_NS);

    // 2. Instanciação e Inicialização da Memória
    InstructionMemory mem("instruction_memory");
    
    // Programa de teste MIPS válido
    const uint32_t programa[] = {
        0x20010005,  // addi $1, $0, 5
        0x2002000A,  // addi $2, $0, 10
        0x00221820   // add $3, $1, $2
    };
    
    for (auto instr : programa) {
        mem.add_instruction(instr);
    }

    // 3. Conexões
    mem.address(addr_signal);
    mem.instruction(instr_signal);

    // 4. Verificação de inicialização
    assert(mem.mem.size() > 0 && "Memória não inicializada!");

    cout << "=== INÍCIO DA SIMULAÇÃO ===" << endl;
    mem.print_memory();

    try {
        // 5. Simulação controlada
        for (int i = 0; i <= 3; i++) {  // Testa 4 endereços (0,4,8,12)
            addr_signal.write(i * 4);
            
            // Executa um ciclo de clock
            sc_start(10, SC_NS);
            
            // Verificação robusta do valor lido
            sc_uint<32> instr = instr_signal.read();
            cout << "Ciclo " << dec << i 
                 << " | PC: 0x" << hex << addr_signal.read()
                 << " | Instrução: 0x" << instr << endl;
            
            // Verificação adicional para endereço válido
            if (i < 3) {  // Para as 3 primeiras instruções
                if (instr != programa[i]) {
                    cerr << "ERRO: Leitura incorreta em 0x" 
                         << hex << addr_signal.read() << endl;
                    return 1;
                }
            } else {  // Para o quarto endereço (deveria ser NOP)
                if (instr != 0x00000000) {
                    cerr << "ERRO: Leitura inválida em endereço não mapeado" << endl;
                    return 1;
                }
            }
        }
    } 
    catch (const sc_report& e) {
        cerr << "Erro durante simulação: " << e.what() << endl;
        return 1;
    }

    cout << "=== SIMULAÇÃO CONCLUÍDA COM SUCESSO ===" << endl;
    return 0;
}