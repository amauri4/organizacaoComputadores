#ifndef MEMORIA_INSTRUCOES_H
#define MEMORIA_INSTRUCOES_H

#include <systemc.h>
#include <vector>
#include <iomanip>
#include <iostream>

SC_MODULE(InstructionMemory) {
    sc_in<sc_uint<32>> address;
    sc_in_clk clk;
    sc_out<sc_uint<32>> instruction;
    std::vector<sc_uint<32>> mem;

    void add_instruction(uint32_t instr) {
        // Garante que o valor é truncado para 32 bits
        uint32_t masked_instr = instr & 0xFFFFFFFF;
        
        std::cout << "Armazenando: 0x" 
                << std::hex << std::setw(8) << std::setfill('0') 
                << masked_instr << std::endl;

        mem.push_back(static_cast<sc_uint<32>>(masked_instr));

        // Verificação rigorosa
        uint32_t stored_value = mem.back().to_uint();
        if (stored_value != masked_instr) {
            std::cerr << "ERRO: Valor armazenado (0x" << std::hex << stored_value
                    << ") difere do input (0x" << masked_instr << ")" << std::endl;
        }
    }

    void read_process() {
        sc_uint<32> addr = address.read();
        size_t word_addr = addr >> 2; 
        
        if ((word_addr < mem.size()) && clk.posedge()) {
            instruction.write(mem[word_addr]);
            std::cout << "\nLENDO INSTRUÇÃO -> " << instruction.read() << "\n";
        } else {
            instruction.write(0x00000000); 
            if (addr != 0) {
                std::cerr << "Aviso: Endereço inválido 0x" 
                          << std::hex << std::setw(8) << std::setfill('0') 
                          << addr << " em " << sc_time_stamp() << std::endl;
            }
        }
    }

    void print_memory() const {
        std::cout << "=== Conteúdo da Memória ===" << std::endl;

        for (size_t i = 0; i < mem.size(); ++i) {
            std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') 
                      << (mem[i].to_uint() & 0xFFFFFFFF) << '\n';
        }

    }

    SC_CTOR(InstructionMemory) {
        SC_METHOD(read_process);
        sensitive << clk.pos();
    }
};

#endif // MEMORIA_INSTRUCOES_H