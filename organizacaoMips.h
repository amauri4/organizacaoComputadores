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
#include "./componentes/muxBranch.h"
#include "./componentes/muxJump.h"
#include "./componentes/unidadeControle.h"
#include "./componentes/memoriaDados.h"

#include <iomanip>
#include <sstream>


struct IF_ID_Register {
    sc_signal<sc_uint<32>> pc_plus4;
    sc_signal<sc_uint<32>> instruction;
    
    void reset() {
        pc_plus4.write(0);
        instruction.write(0);
    }
    
    void update(sc_uint<32> new_pc_plus4, sc_uint<32> new_instruction) {
        pc_plus4.write(new_pc_plus4);
        instruction.write(new_instruction);
    }
};

SC_MODULE(MIPS_Simplificado) {
    // Portas principais
    sc_in_clk clk;
    sc_in<bool> reset;
    sc_signal<bool> escrita;

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
    sc_signal<sc_uint<32>> jump_target;
    sc_signal<sc_uint<4>>pc_bits;
    sc_signal<sc_int<32>> extended_signal;

    sc_signal<sc_int<32>> extended_signal_add;

    sc_signal<sc_int<32>> pc_add_offset;

    sc_signal<sc_int<32>> write_data_reg;
    sc_signal<sc_int<32>> read_data_1;
    sc_signal<sc_int<32>> read_data_2;

    sc_signal<sc_int<32>> output_mux_ula;
    sc_signal<sc_int<32>> output_mux_offset_pc;

    sc_signal<sc_uint<32>> fetched_instruction;

    // DEBUG JUMP
    sc_signal<sc_uint<4>>pc_bits_debug;
    sc_signal<sc_uint<32>>pc_upper_32_debug;

    sc_signal<sc_uint<4>> alu_control;
    sc_signal<sc_int<32>> ula_result;
    sc_signal<sc_int<32>> read_data;

    // Sinais de controle
    sc_signal<bool> reg_dst;
    sc_signal<bool> reg_write;
    sc_signal<sc_uint<2>> alu_op;
    sc_signal<bool> alu_src;
    sc_signal<bool> jump;
    sc_signal<bool> branch;
    sc_signal<bool> mem_read;
    sc_signal<bool> mem_to_reg;
    sc_signal<bool> mem_write;
    sc_signal<bool> zero;

    sc_signal<bool> branch_taken;

    // Componentes
    PC* pc;
    InstructionMemory* imem;
    Decodificador* decod;
    Mux<5>* mux_regdst;
    Adder* adder_pc;
    AdderAddress* adder_endereco;
    RegisterFile* regs;
    SignExtend* signE;
    ALU* ula;
    ALUControl* ulaControle;
    ControlUnit* parteOperativa;
    MuxBranch<32>* mux_somador_endereco;
    MuxJump<32>* mux_jump_endereco;
    Mux<32, sc_int<32>>* mux_ula;
    Mux<32, sc_int<32>>* mux_saida_ula;

    Shifter_26to32* deslocador26to32;
    Shifter_32b* deslocador32;
    DataMemory* memoriaDados;

    IF_ID_Register if_id;  
    
    // Sinais adicionais
    sc_signal<bool> flush_pipeline;
    sc_signal<bool> stall_pipeline;

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
        ula = new ALU("ULA");
        ulaControle = new ALUControl("CONTROLE_ULA");
        mux_somador_endereco = new MuxBranch<32>("MUX_SOMA_ENDERECO");
        mux_ula = new Mux<32, sc_int<32>>("MUX_ULA");
        mux_saida_ula = new Mux<32, sc_int<32>>("MUX_SAIDA_ULA");
        mux_jump_endereco = new MuxJump<32>("MUX_JUMP");
        deslocador26to32 = new Shifter_26to32("DESLOCADOR_26_TO_32");
        deslocador32 = new Shifter_32b("DESLOCADOR_32");
        parteOperativa = new ControlUnit("PARTE_OPERATIVA");
        memoriaDados = new DataMemory("MEMORIA_DE_DADOS");

        // Constante 4
        const_four.write(4);

        // Conectando PC
        pc->reset(reset);
        pc->clk(clk);
        pc->next_addr(pc_next);
        pc->escrita_instrucao(escrita);
        pc->current_addr(pc_out);

        // Memória de instruções
        imem->address(pc_out);
        imem->escrita(escrita);
        imem->instruction(instruction);

        // Decodificador
        decod->instruction(instruction);
        decod->escrita(escrita);
        decod->opcode(opcode);
        decod->rs(rs);
        decod->rt(rt);
        decod->rd(rd);
        decod->imm(imm);
        decod->funct(funct);
        decod->jump_address(jump_addr_raw);

        // Parte operativa 
        parteOperativa->opcode(opcode);
        parteOperativa->reg_dst(reg_dst);
        parteOperativa->reg_write(reg_write);
        parteOperativa->alu_op(alu_op);
        parteOperativa->alu_src(alu_src);
        parteOperativa->mem_write(mem_write);
        parteOperativa->jump(jump);
        parteOperativa->mem_read(mem_read);
        parteOperativa->mem_to_reg(mem_to_reg);
        parteOperativa->branch(branch);

        // Adição de PC + 4
        adder_pc->a(pc_out);
        adder_pc->b(const_four);
        adder_pc->sum(pc_plus4);

        // Deslocador para cálculo do Jump
        deslocador26to32->pc_high(pc_bits);
        deslocador26to32->instr_index(jump_addr_raw);
        deslocador26to32->jump_address(jump_target); 

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

        // ULA controle
        ulaControle->alu_op(alu_op);
        ulaControle->funct(funct);
        ulaControle->alu_control(alu_control);

        // Multiplexador para registrador destino
        mux_regdst->input0(rt);
        mux_regdst->input1(rd);
        mux_regdst->sel(reg_dst);
        mux_regdst->output(write_reg);

        // Banco de registradores
        regs->read_reg1(rs);
        regs->read_reg2(rt);
        regs->write_reg(write_reg);
        regs->write_data(write_data_reg);
        regs->reg_write(reg_write);
        // Saida do banco
        regs->read_data1(read_data_1);
        regs->read_data2(read_data_2);

        // Mux ULA
        mux_ula->input0(read_data_2);
        mux_ula->input1(extended_signal);
        mux_ula->sel(alu_src);
        mux_ula->output(output_mux_ula);

        // ULA
        ula->a(read_data_1);
        ula->b(output_mux_ula);
        ula->alu_control(alu_control);
        ula->zero(zero);
        ula->result(ula_result);

        // Mux offset com PC
        mux_somador_endereco->pc_plus4(pc_plus4);
        mux_somador_endereco->branch_target(pc_add_offset);
        mux_somador_endereco->branch_taken(branch_taken); // decisao de seletor de branch
        mux_somador_endereco->out(output_mux_offset_pc);

        // Mux jump 
        mux_jump_endereco->branch_mux_out(output_mux_offset_pc);
        mux_jump_endereco->jump_target(jump_target);
        mux_jump_endereco->jump_taken(jump);
        mux_jump_endereco->pc_next(pc_next);

        // Memoria de dados
        memoriaDados->address(ula_result);
        memoriaDados->write_data(read_data_2);
        memoriaDados->mem_read(mem_read);
        memoriaDados->mem_write(mem_write);
        memoriaDados->read_data(read_data);

        //Mux saida dados
        mux_saida_ula->input0(ula_result);
        mux_saida_ula->input1(read_data);
        mux_saida_ula->sel(mem_to_reg);
        mux_saida_ula->output(write_data_reg);

        // Atualização de sinais derivados
        SC_METHOD(update_branch_decision);
        sensitive << branch << zero;
        dont_initialize();

        SC_METHOD(control_pc_update);
        sensitive << clk.pos();  
        dont_initialize();

        SC_METHOD(update_pc_bits);
        sensitive << pc_out;
        dont_initialize();
    }

    ~MIPS_Simplificado() {
        delete adder_endereco;
        delete signE;
        delete ula;
        delete ulaControle;
        delete parteOperativa;
        delete mux_somador_endereco;
        delete mux_ula;
        delete mux_saida_ula;
        delete mux_jump_endereco;
        delete deslocador26to32;
        delete deslocador32;
        delete memoriaDados;
    }

private:

    void control_pc_update() {

        using std::hex;
        using std::dec;
        using std::setw;
        using std::setfill;
        using std::cout;
        using std::endl;

        cout << "\n═══════════════════════════════════════" << endl;
        cout << " Ciclo @ " << sc_time_stamp() << endl;
        cout << " PC: 0x" << hex << setw(8) << setfill('0') << pc_out.read() << endl;
        
        // Decodificação da instrução
        cout << "\n[INSTRUÇÃO]" << endl;
        cout << " 0x" << setw(8) << setfill('0') << instruction.read() << " | ";
        switch(opcode.read()) {
            case 0x00: // R-type
                cout << "R-type: ";
                switch(funct.read()) {
                    case 0x20: cout << "add"; break;
                    case 0x22: cout << "sub"; break;
                    case 0x24: cout << "and"; break;
                    case 0x25: cout << "or"; break;
                    case 0x2A: cout << "slt"; break;
                    case 0x08: cout << "jr"; break;
                    default: cout << "funct 0x" << hex << funct.read();
                }
                cout << " $" << dec << rd.read() << ", $" << rs.read() << ", $" << rt.read();
                break;
            case 0x08: cout << "addi $" << dec << rt.read() << ", $" << rs.read() << ", 0x" << hex << imm.read(); break;
            case 0x04: cout << "beq $" << dec << rs.read() << ", $" << rt.read() << ", 0x" << hex << imm.read(); break;
            case 0x05: cout << "bne $" << dec << rs.read() << ", $" << rt.read() << ", 0x" << hex << imm.read(); break;
            case 0x02: cout << "j 0x" << hex << jump_addr_raw.read(); break;
            case 0x03: cout << "jal 0x" << hex << jump_addr_raw.read(); break;
            case 0x23: cout << "lw $" << dec << rt.read() << ", 0x" << hex << imm.read() << "($" << rs.read() << ")"; break;
            case 0x2B: cout << "sw $" << dec << rt.read() << ", 0x" << hex << imm.read() << "($" << rs.read() << ")"; break;
            default:   cout << "Instrução não identificada";
        }
        
        // Sinais de controle
        cout << "\n\n[CONTROLE]" << endl;
        cout << " RegDst=" << reg_dst.read() << " RegWrite=" << reg_write.read()
            << " ALUSrc=" << alu_src.read() << " ALUOp=" << alu_op.read() << endl;
        cout << " Branch=" << branch.read() << " Jump=" << jump.read()
            << " MemRead=" << mem_read.read() << " MemWrite=" << mem_write.read() << endl;

        // Informações de desvio
        if (branch.read() || jump.read()) {
            cout << "\n[CONTROLE DE FLUXO]" << endl;
            if (branch.read()) {
                cout << " Branch: " << (branch_taken.read() ? "TOMADO" : "NÃO TOMADO") << endl;
                cout << "  Offset: 0x" << hex << extended_signal.read() << endl;
                cout << "  Alvo: 0x" << pc_add_offset.read() << endl;
            }
            if (jump.read()) {
                cout << " Jump: TOMADO" << endl;
                cout << "  Alvo: 0x" << jump_target.read() << endl;
                cout << "Jump 26 bits:  0x" << jump_addr_raw << endl;
                cout << "Jump 32 bits:  0x" << jump_target << endl;
                cout << "PC next:  0x" << pc_next << endl;
                cout << "PC out: 0x" << pc_out << endl;
                // DEBUG OBRIGATÓRIO
                cout << "DEBUG JUMP CALCULATION:" << endl;
                cout << "PC: 0x" << hex << pc_out.read() << endl;
                cout << "PC upper (4 bits): 0x" << hex << pc_bits_debug << endl;
                cout << "PC upper (32 bits): 0x" << hex << pc_upper_32_debug << endl;
                cout << "Jump shifted: 0x" << hex << jump_target.read() << endl;

                if (opcode.read() == 0x03) { // JAL
                    cout << "  Endereço de retorno: 0x" << (pc_out.read() + 8) << endl;
                }
            }
        }

        // Estado da ULA
        cout << "\n[ULA]" << endl;
        cout << " A=0x" << setw(8) << setfill('0') << read_data_1.read() 
            << " B=0x" << setw(8) << output_mux_ula.read() 
            << " Ctrl=0x" << alu_control.read() << endl;
        cout << " Result=0x" << setw(8) << ula_result.read() 
            << " Zero=" << zero.read() << endl;

        // Estado dos registradores
        cout << "\n[REGISTRADORES]" << endl;
        cout << " Clock: " << clk.read() << endl;
        cout << " Destino: $" << dec << write_reg.read() 
            << " Valor: 0x" << hex << setw(8) << setfill('0') << write_data_reg.read() << endl;
        
        // Mostra registradores importantes
        cout << " $1=0x" << setw(8) << registers[0] 
            << " $2=0x" << setw(8) << registers[1] 
            << " $5=0x" << setw(8) << registers[5] 
            << " $6=0x" << setw(8) << registers[6] 
            << " $31=0x" << setw(8) << registers[31] << endl;

        // Estado da memória (apenas endereços relevantes)
        if (mem_read.read() || mem_write.read()) {
            cout << "\n[MEMÓRIA]" << endl;
            uint32_t addr = ula_result.read() & 0xFFFFFFFC;
            cout << " Acesso a 0x" << hex << setw(8) << setfill('0') << addr;
            if (mem_read.read()) {
                cout << " | Lendo: 0x" << read_data.read() << endl;
            }
            if (mem_write.read()) {
                cout << " | Escrevendo: 0x" << read_data_2.read() << " | Endereço: " << addr<< endl;
            }
        }

    }

    void debug_sync() {
        cout << "SYNC DEBUG @ " << sc_time_stamp() << ":\n"
            << "  PC: 0x" << hex << pc_out.read() << "\n"
            << "  Next PC: 0x" << pc_next.read() << "\n"
            << "  Mem input: 0x" << imem->address.read() << "\n"
            << "  Mem output: 0x" << instruction.read() << "\n"
            << "  Clock phase: " << clk.read() << endl;
    }

    void update_branch_decision() {
        branch_taken.write(branch.read() && zero.read());
    }

    void update_pc_bits() {
        cout << "\n" << "Endereço atual atualizado" << sc_time_stamp() << "\n";
        pc_bits.write(pc_out.read().range(31, 28));
    }
    
public: 
    sc_int<32> registers[32]; 
    sc_int<32> mem[32]; 

    void load_program(const std::vector<uint32_t>& instructions) {
        for (auto instr : instructions) {
            imem->add_instruction(instr);
        }
    }

    void load_instruction(uint32_t instruction) {
        // Verificação de valor máximo
        if (instruction > 0xFFFFFFFF) {
            std::cerr << "ERRO: Instrução excede 32 bits: 0x"
                    << std::hex << instruction << std::endl;
            return;
        }
        imem->add_instruction(instruction);
    }

    void visualization_registers(){
        regs->dump_registers(registers);
    }

    void visualization_memory(){
        memoriaDados->dump_memory(mem);
    }

};

#endif // MIPS_SIMPLIFICADO_H
