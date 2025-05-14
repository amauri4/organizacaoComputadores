#ifndef MEMORIA_INSTRUCOES_H
#define MEMORIA_INSTRUCOES_H

#include <systemc.h>
#include <vector>
#include <iomanip>
#include <iostream>

SC_MODULE(InstructionMemory) {
    sc_in<sc_uint<32>> address;
    sc_out<sc_uint<32>> instruction;
    
    std::vector<sc_uint<32>> mem;

    // Corrigido: Formatação correta de 32 bits
    void add_instruction(uint32_t instr) {
        std::cout << "Armazenando: 0x" << std::hex << std::setw(8) << instr << std::endl;

        mem.push_back(static_cast<sc_uint<32>>(instr));

        std::cout << "Armazenado: 0x" << std::hex << std::setw(8) << mem.back().to_uint() << std::endl;
    }

    void read_process() {
        sc_uint<32> addr = address.read();
        size_t word_addr = addr >> 2; 
        
        if (word_addr < mem.size()) {
            instruction.write(mem[word_addr]);
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
        sensitive << address;
    }
};

#endif // MEMORIA_INSTRUCOES_H