#ifndef MIPS_SIMPLIFICADO_H
#define MIPS_SIMPLIFICADO_H

#include <systemc.h>
#include "./componentes/pc.h"
#include "./componentes/memoriaInstrucoes.h"
#include "./componentes/decoder.h"
#include "./componentes/mux.h"
#include "./componentes/somador.h"
#include "./componentes/bancoReg.h"
#include "./componentes/extensorSinal.h"
#include "./componentes/ula.h"
#include "./componentes/controleUla.h"
#include "./componentes/deslocador26to32.h"
#include "./componentes/deslocador32.h"
#include "./componentes/somadorEnderecos.h"

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
    sc_signal<sc_uint<5>> write_reg;
    sc_signal<sc_uint<26>> jump_addr_raw;
    sc_signal<sc_uint<28>> jump_addr_shifted;
    sc_signal<sc_uint<32>> jump_target;
    
    sc_signal<sc_int<32>> extended_signal;

    sc_signal<sc_int<32>> extended_signal_add;

    sc_signal<sc_int<32>> pc_add_offset;

    sc_signal<sc_uint<32>> write_data_reg;
    sc_signal<sc_uint<32>> read_data_1;
    sc_signal<sc_uint<32>> read_data_2;

    sc_signal<sc_uint<4>> alu_control;

    // Sinais de controle
    sc_signal<bool> reg_dst;
    sc_signal<bool> reg_write;
    sc_signal<sc_uint<2>> alu_op;

    // Componentes
    PC* pc;
    InstructionMemory* imem;
    Decodificador* decod;
    Mux<5>* mux_regdst;
    Adder* adder_pc;
    AdderAddress* adder_endereco;
    RegisterFile* regs;
    SignExtend* signE;
    //ALU* ula;
    ALUControl* ulaControle;
    //Mux<32>* mux_ula;
    Shifter_26to28* deslocador26to32;
    Shifter_32b* deslocador32;

    SC_CTOR(MIPS_Simplificado) {
        // Instanciando componentes
        pc = new PC("PC");
        imem = new InstructionMemory("IMEM");
        decod = new Decodificador("DECOD");
        mux_regdst = new Mux<5>("MUX_REGDST");
        adder_pc = new Adder("ADDER_PC");
        adder_endereco = new AdderAddress("ADDER_ENDERECO");
        regs = new RegisterFile("BANCO_REGISTRADORES");
        signE = new SignExtend("EXTENSOR_SINAL");
        //ula = new ALU("ULA");
        ulaControle = new ALUControl("CONTROLE_ULA");
        //mux_ula = new Mux<32>("MUX_ULA");
        deslocador26to32 = new Shifter_26to28("DESLOCADOR_26_TO_32");
        deslocador32 = new Shifter_32b("DESLOCADOR_32");

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
        decod->jump_address(jump_addr_raw);

        // Adição de PC + 4
        adder_pc->a(pc_out);
        adder_pc->b(const_four);
        adder_pc->sum(pc_plus4);

        // Deslocador para cálculo do Jump
        deslocador26to32->input(jump_addr_raw);
        deslocador26to32->output(jump_addr_shifted);

        // Cálculo do endereço final (PC+4[31..28] || jump_addr_shifted)
        jump_target.write((pc_plus4.read().range(31, 28), jump_addr_shifted.read()));

        // Extensor de sinal
        signE->input(imm);
        signE->output(extended_signal);

        // Deslocador de 32 bits
        deslocador32->input(extended_signal);
        deslocador32->output(extended_signal_add);

        // Somador endereços
        adder_endereco->a(pc_plus4);
        adder_endereco->b(extended_signal_add);
        adder_endereco->sum(pc_add_offset);

        // Mux offset com PC

        // ULA controle
        ulaControle->alu_op(alu_op);
        ulaControle->funct(funct);
        ulaControle->alu_control(alu_control);

        // Multiplexador para registrador destino
        mux_regdst->input0(rt);
        mux_regdst->input1(rd);
        mux_regdst->sel(reg_dst);
        mux_regdst->output(write_reg);

        // Inicializa a entrada com valor padrão para testes
        write_data_reg.write(0xDEADBEEF);

        // Banco de registradores
        regs->clk(clk);
        regs->read_reg1(rs);
        regs->read_reg2(rt);
        regs->write_reg(write_reg);
        regs->write_data(write_data_reg);
        regs->reg_write(reg_write);

        regs->read_data1(read_data_1);
        regs->read_data2(read_data_2);

        // Atualização de sinais derivados
        SC_METHOD(update_signals);
        sensitive << clk.pos();

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
        delete regs;
    }

private:
    void update_signals() {
        reg_dst.write(opcode.read() == 0);
        pc_next.write(pc_plus4.read());
        // Ativa RegWrite para: R-type (opcode=0), lw (0x23), addi (0x08), etc.
        reg_write.write(
            opcode.read() == 0 ||    // R-type
            opcode.read() == 0x23 || // lw
            opcode.read() == 0x08    // addi
        );
    }

    void show_debug() {
        debug_pc.write(pc_out.read());
        debug_instruction.write(instruction.read());
        
        cout << "---------------------------" << endl;
        cout << "Ciclo @ " << sc_time_stamp() << endl;
        cout << "PC: 0x" << hex << pc_out.read() << endl;
        cout << "Instr: 0x" << hex << instruction.read() << endl;
        cout << "Opcode: 0x" << hex << opcode.read() << endl;
        cout << "rs: $" << dec << rs.read() << "\trt: $" << rt.read() << "\trd: $" << rd.read() << endl;
        cout << "RegWrite: " << reg_write.read() << " WriteReg: $" << dec << write_reg.read() << endl;
        cout << "WriteData: 0x" << hex << write_data_reg.read() << endl;
    }
    
public: 
    sc_uint<32> registers[32]; 

    void load_program(const std::vector<uint32_t>& instructions) {
        for (auto instr : instructions) {
            imem->add_instruction(instr);
        }
    }

    void load_instruction(uint32_t instruction) {
        imem->add_instruction(instruction);
    }

    void visualization_registers(){
        regs->dump_registers(registers);
    }
};

#endif // MIPS_SIMPLIFICADO_H
