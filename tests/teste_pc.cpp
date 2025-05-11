#include "../componentes/pc.h"
#include <systemc.h>
#include <iostream>

#define ASSERT(cond, msg) \
    do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; exit(1); } } while (0)

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<sc_uint<32>> next_addr, current_addr;
    sc_signal<bool> reset;
    
    PC pc("pc");
    pc.clk(clk);
    pc.next_addr(next_addr);
    pc.reset(reset);
    pc.current_addr(current_addr);
    
    std::cout << "Iniciando teste do PC..." << std::endl;
    
    // Teste 1: Reset
    reset.write(1);
    sc_start(1, SC_NS);
    ASSERT(current_addr.read() == 0, "Reset falhou");
    
    // Teste 2: Atualização normal
    reset.write(0);
    next_addr.write(0x00000004);
    sc_start(10, SC_NS); // Espera um ciclo de clock
    ASSERT(current_addr.read() == 0x00000004, "Atualização do PC falhou");
    
    // Teste 3: Reset assíncrono
    next_addr.write(0x00000008);
    reset.write(1);
    sc_start(10, SC_NS);
    ASSERT(current_addr.read() == 0, "Reset assíncrono falhou");
    
    std::cout << "Todos os testes do PC passaram!" << std::endl;
    return 0;
}