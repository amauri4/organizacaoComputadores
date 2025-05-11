#ifndef MIPS_SIMPLIFICADO_H
#define MIPS_SIMPLIFICADO_H

#include <systemc.h>
#include "./componentes/pc.h"
#include "./componentes/memoriaInstrucoes.h"
#include "./componentes/decoder.h"
#include "./componentes/mux.h"
#include "./componentes/somador.h"

SC_MODULE(MIPS_Simplificado) {
    // Portas principais
    sc_in_clk clk;
    sc_in<bool> reset;

    // Sinais de depuração
    sc_out<sc_uint<32>> debug_pc;
    sc_out<sc_uint<32>> debug_instruction;

    // Sinais internos
    sc_signal<sc_uint<32>> pc_out, pc_next, pc_plus4;
    sc_signal<sc_uint<32>> instruction;
    sc_signal<sc_uint<6>> opcode;
    sc_signal<sc_uint<5>> rs, rt, rd;
    sc_signal<sc_uint<16>> imm;
    sc_signal<sc_uint<6>> funct;
    sc_signal<sc_uint<32>> const_four;
    sc_signal<bool> is_r_type_signal;
    sc_signal<sc_uint<5>> write_reg;

    // Componentes
    PC* pc;
    InstructionMemory* imem;
    Decodificador* decod;
    Mux<5>* mux_regdst;
    Adder* adder_pc;

    SC_CTOR(MIPS_Simplificado) {
        // Instanciando componentes
        pc = new PC("PC");
        imem = new InstructionMemory("IMEM");
        decod = new Decodificador("DECOD");
        mux_regdst = new Mux<5>("MUX_REGDST");
        adder_pc = new Adder("ADDER_PC");

        // Constante 4
        const_four.write(4);

        // Conectando PC
        pc->clk(clk);
        pc->reset(reset);
        pc->next_addr(pc_next);
        pc->current_addr(pc_out);

        // Memória de instruções
        imem->address(pc_out);
        imem->instruction(instruction);

        // Decodificador
        decod->instruction(instruction);
        decod->opcode(opcode);
        decod->rs(rs);
        decod->rt(rt);
        decod->rd(rd);
        decod->imm(imm);
        decod->funct(funct);

        // Adição de PC + 4
        adder_pc->a(pc_out);
        adder_pc->b(const_four);
        adder_pc->sum(pc_plus4);

        // Multiplexador para registrador destino
        mux_regdst->input0(rt);
        mux_regdst->input1(rd);
        mux_regdst->sel(is_r_type_signal);
        mux_regdst->output(write_reg);

        // Atualização de sinais derivados
        SC_METHOD(update_signals);
        sensitive << opcode << rt << rd << pc_plus4;

        // Depuração
        SC_METHOD(show_debug);
        sensitive << clk.pos();
    }

    ~MIPS_Simplificado() {
        delete pc;
        delete imem;
        delete decod;
        delete mux_regdst;
        delete adder_pc;
    }

private:
    void update_signals() {
        is_r_type_signal.write(opcode.read() == 0);
        pc_next.write(pc_plus4.read());
    }

    void show_debug() {
        debug_pc.write(pc_out.read());  // Atualiza a saída debug
        debug_instruction.write(instruction.read());  // Atualiza a saída debug

        // Debug na saída
        cout << "---------------------------" << endl;
        cout << "Ciclo @ " << sc_time_stamp() << endl;
        cout << "PC: 0x" << hex << pc_out.read() << endl;
        cout << "Instr: 0x" << hex << instruction.read() << endl;
        cout << "Opcode: 0x" << hex << opcode.read() << endl;
        cout << "rs: $" << dec << rs.read() << "\trt: $" << rt.read() << "\trd: $" << rd.read() << endl;
        cout << "imm: 0x" << hex << imm.read() << "\tfunct: 0x" << funct.read() << endl;
        cout << "DestReg (mux): $" << dec << write_reg.read() << endl;
        cout << "PC+4: 0x" << hex << pc_plus4.read() << endl;
    }
public: 

    void load_program(const std::vector<uint32_t>& instructions) {
        for (auto instr : instructions) {
            imem->add_instruction(instr);
        }
    }

    void load_instruction(uint32_t instruction) {
        imem->add_instruction(instruction);
    }
};

#endif // MIPS_SIMPLIFICADO_H
