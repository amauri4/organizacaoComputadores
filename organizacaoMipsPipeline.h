#ifndef MIPS_PIPELINED_H
#define MIPS_PIPELINED_H

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
#include "./componentes/hazardUnit.h"
#include "./componentes/forwardingUnit.h"

SC_MODULE(MIPS_Pipelined)
{
    // Clock e reset
    sc_in_clk clk;
    sc_in<bool> reset;

    // Sinais de debug
    // sc_out<sc_uint<32>> debug_pc;
    // sc_out<sc_uint<32>> debug_instruction;
    sc_signal<sc_uint<32>> const_four;
    sc_signal<sc_int<32>> alu_b_before_forward, alu_b_after_forward;
    ;
    // SINAIS DE CONTROLE

    sc_signal<bool> ctrl_reg_dst;
    sc_signal<bool> ctrl_jump;
    sc_signal<bool> ctrl_branch;
    sc_signal<bool> ctrl_mem_read;
    sc_signal<bool> ctrl_mem_to_reg;
    sc_signal<sc_uint<2>> ctrl_alu_op;
    sc_signal<bool> ctrl_mem_write;
    sc_signal<bool> ctrl_alu_src;
    sc_signal<bool> ctrl_reg_write;
    sc_signal<bool> stall_store_signal;
    sc_signal<bool> mem_write_stall;

    //---------------------------
    // Registradores de Pipeline
    //---------------------------

    // IF/ID Register
    struct IF_ID_Reg
    {
        sc_signal<sc_uint<32>> pc_plus4;
        sc_signal<sc_uint<32>> instruction;
        sc_signal<bool> if_id_write;

        void reset()
        {
            pc_plus4.write(0);
            instruction.write(0);
        }
    } if_id;

    // ID/EX Register
    struct ID_EX_Reg
    {
        // Controle
        sc_signal<bool> reg_write, mem_to_reg, branch, mem_read, mem_write, alu_src, reg_dst;
        sc_signal<sc_uint<2>> alu_op;
        sc_signal<sc_uint<6>> funct;

        // Dados
        sc_signal<sc_uint<32>> pc_plus4;
        sc_signal<sc_int<32>> read_data1, read_data2;
        sc_signal<sc_int<32>> imm_extended;
        sc_signal<sc_uint<5>> rs, rt, rd;

        void reset()
        {
            funct.write(0);
            reg_write.write(0);
            mem_to_reg.write(0);
            branch.write(0);
            mem_read.write(0);
            mem_write.write(0);
            alu_src.write(0);
            reg_dst.write(0);
            alu_op.write(0);
            pc_plus4.write(0);
            read_data1.write(0);
            read_data2.write(0);
            imm_extended.write(0);
            rs.write(0);
            rt.write(0);
            rd.write(0);
        }
    } id_ex;

    // EX/MEM Register
    struct EX_MEM_Reg
    {
        // Controle
        sc_signal<bool> reg_write, mem_to_reg, branch, mem_read, mem_write;

        // Dados
        sc_signal<sc_int<32>> branch_target;
        sc_signal<bool> zero;
        sc_signal<sc_int<32>> alu_result;
        sc_signal<sc_int<32>> write_data;
        sc_signal<sc_uint<5>> write_reg;

        void reset()
        {
            reg_write.write(0);
            mem_to_reg.write(0);
            branch.write(0);
            mem_read.write(0);
            mem_write.write(0);
            branch_target.write(0);
            zero.write(0);
            alu_result.write(0);
            write_data.write(0);
            write_reg.write(0);
        }
    } ex_mem;

    // MEM/WB Register
    struct MEM_WB_Reg
    {
        // Controle
        sc_signal<bool> reg_write, mem_to_reg;

        // Dados
        sc_signal<sc_int<32>> read_data;
        sc_signal<sc_int<32>> alu_result;
        sc_signal<sc_uint<5>> write_reg;

        void reset()
        {
            reg_write.write(0);
            mem_to_reg.write(0);
            read_data.write(0);
            alu_result.write(0);
            write_reg.write(0);
        }
    } mem_wb;

    //---------------------------
    // Sinais Internos
    //---------------------------
    sc_signal<sc_uint<32>> pc_in, pc_out, pc_plus4, pc_out_intermediate;
    sc_signal<sc_uint<32>> instruction;
    sc_signal<sc_uint<6>> opcode;
    sc_signal<sc_uint<5>> rs, rt, rd;
    sc_signal<sc_uint<16>> imm;
    sc_signal<sc_uint<6>> funct;
    sc_signal<sc_uint<32>> jump_target;
    sc_signal<sc_uint<26>> jump_addr_raw;
    sc_signal<sc_uint<4>> pc_upper_bits;

    sc_signal<sc_int<32>> extended_imm;
    sc_signal<sc_int<32>> extended_imm_shifted;
    sc_signal<sc_int<32>> branch_target;

    sc_signal<sc_uint<5>> write_reg;
    sc_signal<sc_int<32>> write_data;

    sc_signal<sc_int<32>> read_data1, read_data2;
    sc_signal<sc_int<32>> alu_b;
    sc_signal<sc_uint<4>> alu_control;
    sc_signal<sc_int<32>> alu_result;
    sc_signal<bool> alu_zero;

    sc_signal<bool> jump;
    sc_signal<bool> branch_taken;
    sc_signal<sc_int<32>> next_pc;

    sc_signal<sc_int<32>> read_data_mem;

    // Sinais de forwarding
    sc_signal<sc_uint<2>> forwardA, forwardB;

    // Sinais de hazard
    sc_signal<bool> pc_write, if_id_write, control_mux;

    //---------------------------
    // Componentes
    //---------------------------
    PC *pc;
    InstructionMemory *imem;
    Decodificador *decod;
    RegisterFile *regs;
    SignExtend *sign_ext;
    ALU *ula;
    ALUControl *alu_ctrl;
    ControlUnit *ctrl;
    DataMemory *dmem;
    Mux<5> *mux_regdst;
    Mux<32, sc_int<32>> *mux_alu_src;
    Mux<32, sc_int<32>> *mux_mem_to_reg;
    MuxBranch<32> *mux_pc_branch;
    MuxJump<32> *mux_pc_jump;
    Shifter_26to32 *jump_shifter;
    Shifter_32b *branch_shifter;
    Adder *pc_adder;
    AdderAddress *branch_adder;
    HazardUnit *hazard_unit;
    ForwardingUnit *forwarding_unit;

    SC_CTOR(MIPS_Pipelined)
    {
        // Inicialização dos registradores de pipeline
        if_id.reset();
        id_ex.reset();
        ex_mem.reset();
        mem_wb.reset();

        // Instanciação dos componentes
        pc = new PC("PC");
        imem = new InstructionMemory("IMEM");
        decod = new Decodificador("DECOD");
        regs = new RegisterFile("REGS");
        sign_ext = new SignExtend("SIGN_EXT");
        ula = new ALU("ULA");
        alu_ctrl = new ALUControl("ALU_CTRL");
        ctrl = new ControlUnit("CTRL");
        dmem = new DataMemory("DMEM");
        mux_regdst = new Mux<5>("MUX_REGDST");
        mux_alu_src = new Mux<32, sc_int<32>>("MUX_ALU_SRC");
        mux_mem_to_reg = new Mux<32, sc_int<32>>("MUX_MEM_TO_REG");
        mux_pc_branch = new MuxBranch<32>("MUX_PC_BRANCH");
        mux_pc_jump = new MuxJump<32>("MUX_PC_JUMP");
        jump_shifter = new Shifter_26to32("JUMP_SHIFTER");
        branch_shifter = new Shifter_32b("BRANCH_SHIFTER");
        pc_adder = new Adder("PC_ADDER");
        branch_adder = new AdderAddress("BRANCH_ADDER");
        hazard_unit = new HazardUnit("HAZARD_UNIT");
        forwarding_unit = new ForwardingUnit("FORWARDING_UNIT");

        const_four.write(4);
        // Conexões do estágio IF
        pc->reset(reset);
        pc->next_addr(pc_in);
        pc->clk(clk);
        pc->enable(pc_write);
        pc->current_addr(pc_out);

        imem->address(pc_out);
        imem->clk(clk);
        imem->instruction(instruction);

        pc_adder->a(pc_out);
        pc_adder->b(const_four);
        pc_adder->sum(pc_plus4);

        // Conexões do estágio ID
        decod->instruction(if_id.instruction);
        decod->opcode(opcode);
        decod->rs(rs);
        decod->rt(rt);
        decod->rd(rd);
        decod->imm(imm);
        decod->funct(funct);
        decod->jump_address(jump_addr_raw);

        ctrl->opcode(opcode);
        ctrl->reg_dst(ctrl_reg_dst);
        ctrl->jump(jump);
        ctrl->branch(ctrl_branch);
        ctrl->mem_read(ctrl_mem_read);
        ctrl->mem_to_reg(ctrl_mem_to_reg);
        ctrl->alu_op(ctrl_alu_op);
        ctrl->mem_write(ctrl_mem_write);
        ctrl->alu_src(ctrl_alu_src);
        ctrl->reg_write(ctrl_reg_write);

        regs->clk(clk);
        regs->read_reg1(rs);
        regs->read_reg2(rt);
        regs->write_reg(mem_wb.write_reg);
        regs->write_data(write_data);
        regs->reg_write(mem_wb.reg_write);
        regs->read_data1(read_data1);
        regs->read_data2(read_data2);

        sign_ext->input(imm);
        sign_ext->output(extended_imm);

        branch_shifter->input(extended_imm);
        branch_shifter->output(extended_imm_shifted);

        branch_adder->a(id_ex.pc_plus4);
        branch_adder->b(extended_imm_shifted);
        branch_adder->sum(branch_target);

        branch_taken = ex_mem.branch.read() && ex_mem.zero.read();

        mux_pc_branch->pc_plus4(pc_plus4);
        mux_pc_branch->branch_target(ex_mem.branch_target);
        mux_pc_branch->branch_taken(branch_taken);
        mux_pc_branch->out(next_pc);

        // SALVAR O PC_BITS E O JUMP_ADDR
        jump_shifter->pc_high(pc_upper_bits);
        jump_shifter->instr_index(jump_addr_raw);
        jump_shifter->jump_address(jump_target);

        mux_pc_jump->branch_mux_out(next_pc);
        mux_pc_jump->jump_target(jump_target);
        mux_pc_jump->jump_taken(jump);
        mux_pc_jump->pc_next(pc_in);

        // Conexões do estágio EX
        mux_regdst->input0(id_ex.rt);
        mux_regdst->input1(id_ex.rd);
        mux_regdst->sel(id_ex.reg_dst);
        mux_regdst->output(write_reg);

        mux_alu_src->input0(id_ex.read_data2);
        mux_alu_src->input1(id_ex.imm_extended);
        mux_alu_src->sel(id_ex.alu_src);
        mux_alu_src->output(alu_b_before_forward);

        alu_ctrl->alu_op(id_ex.alu_op);
        alu_ctrl->funct(id_ex.funct);
        alu_ctrl->alu_control(alu_control);

        ula->a(alu_a_forward);
        ula->b(alu_b_after_forward); /// PROBLEMA COM A FOWARD UNIT
        ula->alu_control(alu_control);
        ula->zero(alu_zero);
        ula->result(alu_result);

        // Conexões do estágio MEM
        dmem->clk(clk);
        dmem->address(ex_mem.alu_result);
        dmem->write_data(ex_mem.write_data);
        dmem->mem_read(ex_mem.mem_read);
        dmem->mem_write(ex_mem.mem_write);
        dmem->read_data(read_data_mem);

        // Conexões do estágio WB
        mux_mem_to_reg->input0(mem_wb.alu_result);
        mux_mem_to_reg->input1(mem_wb.read_data);
        mux_mem_to_reg->sel(mem_wb.mem_to_reg);
        mux_mem_to_reg->output(write_data);

        // Unidade de forwarding
        forwarding_unit->ex_mem_reg_write(ex_mem.reg_write);
        forwarding_unit->mem_wb_reg_write(mem_wb.reg_write);
        forwarding_unit->ex_mem_rd(ex_mem.write_reg);
        forwarding_unit->mem_wb_rd(mem_wb.write_reg);
        forwarding_unit->id_ex_rs(id_ex.rs);
        forwarding_unit->id_ex_rt(id_ex.rt);
        // Saidas
        forwarding_unit->forwardA(forwardA);
        forwarding_unit->forwardB(forwardB);

        // Unidade de hazard
        hazard_unit->id_ex_mem_read(id_ex.mem_read);
        hazard_unit->id_ex_rt(id_ex.rt);
        hazard_unit->if_id_rs(rs);
        hazard_unit->if_id_rt(rt);
        hazard_unit->id_ex_mem_write(id_ex.mem_write);
        // Saidas
        hazard_unit->pc_write(pc_write);
        hazard_unit->if_id_write(if_id_write);
        hazard_unit->control_mux(control_mux);

        // Atualização dos registradores de pipeline
        SC_METHOD(update_if_id);
        sensitive << clk.pos();

        SC_METHOD(update_id_ex);
        sensitive << clk.pos();

        SC_METHOD(update_ex_mem);
        sensitive << clk.pos();

        SC_METHOD(update_mem_wb);
        sensitive << clk.pos();

        SC_METHOD(check_memory_write);
        sensitive << clk.pos();

        // Forwarding muxes
        SC_METHOD(set_alu_inputs);
        sensitive << forwardA << forwardB << id_ex.read_data1 << id_ex.read_data2
                  << ex_mem.alu_result << write_data << alu_b_before_forward;
    }

    ~MIPS_Pipelined()
    {
        delete pc;
        delete imem;
        delete decod;
        delete regs;
        delete sign_ext;
        delete ula;
        delete alu_ctrl;
        delete ctrl;
        delete dmem;
        delete mux_regdst;
        delete mux_alu_src;
        delete mux_mem_to_reg;
        delete mux_pc_branch;
        delete mux_pc_jump;
        delete jump_shifter;
        delete branch_shifter;
        delete pc_adder;
        delete branch_adder;
        delete hazard_unit;
        delete forwarding_unit;
    }

private:
    sc_signal<sc_int<32>> alu_a_forward;

    void check_memory_write()
    {
        if (ex_mem.mem_write.read() && !reset.read())
        {
            cout << "Attempting to write to memory @ " << sc_time_stamp()
                 << " | Addr: 0x" << hex << ex_mem.alu_result.read()
                 << " | Data: 0x" << ex_mem.write_data.read() << endl;
        }
    }

    void set_alu_inputs()
    {

        cout << "\n\nFORWARDING DEBUG @ " << sc_time_stamp() << ":\n";
        cout << "  forwardA: " << forwardA.read() << " | forwardB: " << forwardB.read() << "\n";
        cout << "  alu_a_forward: 0x" << hex << alu_a_forward.read() << "\n";
        cout << "  alu_b_before_forward: 0x" << alu_b_before_forward.read() << "\n";
        cout << "  alu_b_after_forward: 0x" << alu_b_after_forward.read() << "\n\n";
        // Forwarding para entrada A da ULA
        switch (forwardA.read())
        {
        case 0:
            alu_a_forward.write(id_ex.read_data1.read());
            break;
        case 1:
            alu_a_forward.write(write_data.read()); // write_data.read()
            break;
        case 2:
            alu_a_forward.write(ex_mem.alu_result.read());
            break;
        }

        // Forwarding para entrada B da ULA
        switch (forwardB.read())
        {
        case 0:
            alu_b_after_forward.write(alu_b_before_forward.read());
            break;
        case 1:
            alu_b_after_forward.write(write_data.read()); // write_data.read()
            break;
        case 2:
            alu_b_after_forward.write(ex_mem.alu_result.read());
            break;
        default:
            alu_b_after_forward.write(alu_b_before_forward.read());
            cout << "\n\n PROBLEMA NO SELETOR B DA ULA.\n\n";
        }

        if (id_ex.mem_write.read())
        {
            cout << "STORE DATA FLOW @ " << sc_time_stamp() << endl;
            cout << "  RT register: $" << id_ex.rt.read() << endl;
            cout << "  Value from ID/EX: 0x" << hex << id_ex.read_data2.read() << endl;
            cout << "  ForwardB selector: " << forwardB.read() << endl;
            cout << "  Forwarded value: 0x" << alu_b_after_forward.read() << endl;
        }

        if (ex_mem.mem_write.read())
        {

            cout << "STORE OPERATION @ " << sc_time_stamp()
                 << " | Data to store: 0x" << hex << alu_b_after_forward.read()
                 << " | Address: 0x" << alu_result.read() << endl;
        }
    }

    void update_if_id()
    {
        cout << "\n\nSTAGE DEBUG (IF/ID) @" << sc_time_stamp() << ":\n"
             << "  IF/ID: " << hex << if_id.instruction.read() << "\n"
             << "  ID/EX: " << hex << id_ex.alu_op.read() << "\n\n";
        if (reset.read())
        {
            if_id.reset();
        }
        else if (if_id_write)
        {
            if_id.pc_plus4.write(pc_plus4.read());
            if_id.instruction.write(instruction.read());
        }
    }

    void update_id_ex()
    {

        cout << "\n\nSTAGE DEBUG (ID/EX) @" << sc_time_stamp() << ":\n"
             << "  IF/ID: " << hex << if_id.instruction.read() << "\n"
             << "  ID/EX: " << hex << id_ex.alu_op.read() << "\n\n";

        cout << "\n\nEX STAGE @ " << sc_time_stamp()
             << " | ALU Result: 0x" << hex << alu_result.read()
             << " | Base: 0x" << id_ex.read_data1.read()
             << " | Offset: 0x" << id_ex.imm_extended.read() << endl
             << " | IF ID WRITE: " << if_id_write << endl
             << endl;

        if (reset.read())
        {
            id_ex.reset();
        }
        else if (!control_mux)
        {
            // Passa os sinais de controle
            id_ex.reg_write.write(ctrl_reg_write.read());
            id_ex.mem_to_reg.write(ctrl_mem_to_reg.read());
            id_ex.branch.write(ctrl_branch.read());
            id_ex.mem_read.write(ctrl_mem_read.read());
            id_ex.mem_write.write(ctrl_mem_write.read());
            id_ex.alu_src.write(ctrl_alu_src.read());
            id_ex.reg_dst.write(ctrl_reg_dst.read());
            id_ex.alu_op.write(ctrl_alu_op.read());
            id_ex.funct.write(funct.read());
            // Passa os dados
        }
        else
        {
            id_ex.reg_write.write(0);
            id_ex.mem_to_reg.write(0);
            id_ex.branch.write(0);
            id_ex.mem_read.write(0);
            id_ex.mem_write.write(0);
            id_ex.alu_src.write(0);
            id_ex.reg_dst.write(0);
            id_ex.alu_op.write(0);
            id_ex.funct.write(0);
        }

        id_ex.pc_plus4.write(if_id.pc_plus4.read());
        id_ex.read_data1.write(read_data1.read());
        id_ex.read_data2.write(read_data2.read());
        id_ex.imm_extended.write(extended_imm.read());
        id_ex.rs.write(rs.read());
        id_ex.rt.write(rt.read());
        id_ex.rd.write(rd.read());

        if (control_mux.read())
        {
            cout << "\n\nHAZARD HANDLING:" << endl;
            cout << "  rs: $" << rs.read() << " rt: $" << rt.read() << " rd: $" << rd.read() << endl;
            cout << "  Data1: 0x" << hex << read_data1.read() << " Data2: 0x" << read_data2.read() << endl
                 << endl;
        }
    }

    void update_ex_mem()
    {
        cout << "BRANCH DEBUG | Target: " << branch_target.read()
             << " | Taken: " << (ex_mem.branch.read() && ex_mem.zero.read())
             << " | PC: " << pc_in.read();

        if (reset.read())
        {
            ex_mem.reset();
        }
        else
        {
            if (id_ex.mem_write.read())
            {
                cout << "STORE instruction detected in EX/MEM @ " << sc_time_stamp()
                     << " | Data: 0x" << hex << alu_b_after_forward.read()
                     << " | Address: 0x" << alu_result.read() << " VALOR ANTIGO " << id_ex.read_data2.read() << endl;
            }

            cout << "\n\nEX/MEM UPDATE @ " << sc_time_stamp() << endl;
            cout << "  ALU Result: 0x" << hex << alu_result.read() << endl;
            cout << "  ALU B Forwarded: 0x" << alu_b_after_forward.read() << endl;
            cout << "  ID/EX Data2: 0x" << id_ex.read_data2.read() << endl;
            cout << "  Write Data Selected: 0x"
                 << (id_ex.mem_write.read() ? id_ex.read_data2.read() : alu_b_after_forward.read()) << endl
                 << endl;

            ex_mem.reg_write.write(id_ex.reg_write.read());
            ex_mem.mem_to_reg.write(id_ex.mem_to_reg.read());
            ex_mem.branch.write(id_ex.branch.read());
            ex_mem.mem_read.write(id_ex.mem_read.read());
            ex_mem.mem_write.write(id_ex.mem_write.read());
            ex_mem.zero.write(alu_zero.read());
            // ex_mem.alu_result.write(alu_result.read());
            ex_mem.write_data.write(alu_b_after_forward.read());

            if (id_ex.mem_write.read())
            { // Se for uma store
                ex_mem.alu_result.write(id_ex.read_data2.read() + id_ex.imm_extended.read());
            }
            else
            {
                ex_mem.alu_result.write(alu_result.read()); // Caso contrário, use a ULA normal
            }

            ex_mem.write_reg.write(write_reg.read());

            sc_int<32> branch_target_int = branch_target.read();
            if (branch_target_int >= 0)
            {
                ex_mem.branch_target.write(branch_target_int);
            }
            else
            {
                SC_REPORT_WARNING("Conversion", "Negative address detected");
                ex_mem.branch_target.write(0);
            }
        }
    }

    void update_mem_wb()
    {

        if (ex_mem.mem_read.read())
        {
            cout << "LOAD DEBUG @ " << sc_time_stamp()
                 << " | Addr: 0x" << hex << ex_mem.alu_result.read()
                 << " | Data: 0x" << read_data_mem.read() << endl;
        }
        if (ex_mem.mem_write.read())
        {
            cout << "MEM STAGE VERIFICATION @ " << sc_time_stamp() << ":\n";
            cout << "  Address: 0x" << hex << ex_mem.alu_result.read() << "\n";
            cout << "  Data: 0x" << ex_mem.write_data.read() << "\n";
            cout << "  MemWrite signal: " << ex_mem.mem_write.read() << "\n";
        }
        if (reset.read())
        {
            mem_wb.reset();
        }
        else
        {
            mem_wb.reg_write.write(ex_mem.reg_write.read());
            mem_wb.mem_to_reg.write(ex_mem.mem_to_reg.read());

            mem_wb.read_data.write(read_data_mem);
            cout << "\n\nDADO A SER ESCRITO NO BANCO" << write_data << "\n\n";
            mem_wb.alu_result.write(ex_mem.alu_result.read());
            mem_wb.write_reg.write(ex_mem.write_reg.read());
        }
    }

public:
    sc_int<32> registers[32];
    sc_int<32> mem[32];

    // Métodos para carregar programa e debug (similares ao original)
    void load_program(const std::vector<uint32_t> &instructions)
    {
        for (auto instr : instructions)
        {
            imem->add_instruction(instr);
        }
    }

    void visualization_registers()
    {
        regs->dump_registers(registers);
    }

    void visualization_memory()
    {
        dmem->dump_memory(mem);
    }
};

#endif // MIPS_PIPELINED_H