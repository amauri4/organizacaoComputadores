#include "../componentes/unidadeControle.h"
#include <systemc.h>
#include <iostream>
#include <string>

#define ASSERT(cond, msg) \
    do { if (!(cond)) { std::cerr << "FAIL: " << msg << std::endl; exit(1); } } while (0)

int sc_main(int argc, char* argv[]) {
    // Criação de todos os módulos e sinais ANTES de iniciar a simulação
    ControlUnit cu("cu");
    sc_signal<sc_uint<6>> opcode;
    sc_signal<bool> reg_dst, jump, branch, mem_read, mem_to_reg, mem_write, alu_src, reg_write;
    sc_signal<sc_uint<2>> alu_op;

    // Conexão de todos os sinais
    cu.opcode(opcode);
    cu.reg_dst(reg_dst);
    cu.jump(jump);
    cu.branch(branch);
    cu.mem_read(mem_read);
    cu.mem_to_reg(mem_to_reg);
    cu.mem_write(mem_write);
    cu.alu_src(alu_src);
    cu.reg_write(reg_write);
    cu.alu_op(alu_op);

    std::cout << "Iniciando teste da ControlUnit..." << std::endl;

    // Função auxiliar para testar cada instrução
    auto test_instruction = [&](sc_uint<6> op, const std::string& name, 
                              bool exp_reg_dst, bool exp_jump, bool exp_branch,
                              bool exp_mem_read, bool exp_mem_to_reg, bool exp_mem_write,
                              bool exp_alu_src, bool exp_reg_write, sc_uint<2> exp_alu_op) {
        opcode.write(op);
        sc_start(1, SC_NS);  // Avança a simulação
        
        // Verifica todos os sinais de controle
        ASSERT(reg_dst.read() == exp_reg_dst, name + " - reg_dst");
        ASSERT(jump.read() == exp_jump, name + " - jump");
        ASSERT(branch.read() == exp_branch, name + " - branch");
        ASSERT(mem_read.read() == exp_mem_read, name + " - mem_read");
        ASSERT(mem_to_reg.read() == exp_mem_to_reg, name + " - mem_to_reg");
        ASSERT(mem_write.read() == exp_mem_write, name + " - mem_write");
        ASSERT(alu_src.read() == exp_alu_src, name + " - alu_src");
        ASSERT(reg_write.read() == exp_reg_write, name + " - reg_write");
        ASSERT(alu_op.read() == exp_alu_op, name + " - alu_op");
    };

    // Testa cada tipo de instrução
    test_instruction(0x00, "R-type", 1, 0, 0, 0, 0, 0, 0, 1, 2);
    test_instruction(0x23, "LW",     0, 0, 0, 1, 1, 0, 1, 1, 0);
    test_instruction(0x2B, "SW",     0, 0, 0, 0, 0, 1, 1, 0, 0);
    test_instruction(0x04, "BEQ",    0, 0, 1, 0, 0, 0, 0, 0, 1);
    test_instruction(0x02, "J",      0, 1, 0, 0, 0, 0, 0, 0, 0);

    std::cout << "Todos os testes da ControlUnit passaram!" << std::endl;
    return 0;
}