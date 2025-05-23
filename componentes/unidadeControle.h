#include <systemc.h>

SC_MODULE(ControlUnit)
{
    sc_in<sc_uint<6>> opcode;
    sc_out<bool> reg_dst, jump, branch, mem_read, mem_to_reg, mem_write, alu_src, reg_write;
    sc_out<sc_uint<2>> alu_op;

    void control_process()
    {

        if (opcode.read() == 0x08)
        {                        // ADDI
            reg_dst.write(0);    // Escreve no rt (não no rd)
            alu_src.write(1);    // Usa o imediato
            mem_to_reg.write(0); // Resultado vem da ULA
            reg_write.write(1);  // Habilita escrita
            mem_read.write(0);
            mem_write.write(0);
            branch.write(0);
            alu_op.write(0b00); // Operação de add
            jump.write(0);
        }
        else
            switch (opcode.read())
            {
            case 0x00: // R-type
                reg_dst.write(1);
                alu_src.write(0);
                mem_to_reg.write(0);
                reg_write.write(1);
                mem_read.write(0);
                mem_write.write(0);
                branch.write(0);
                alu_op.write(0b10); // 2 para olhar o funct
                jump.write(0);
                break;

            case 0x23: // LW
                reg_dst.write(0);
                alu_src.write(1);
                mem_to_reg.write(1);
                reg_write.write(1);
                mem_read.write(1);
                mem_write.write(0);
                branch.write(0);
                alu_op.write(0b00); // ADD para cálculo de endereço
                jump.write(0);
                break;

            case 0x2B:            // SW
                reg_dst.write(0); // realmente não importa
                alu_src.write(1);
                mem_to_reg.write(0); // não importa
                reg_write.write(0);
                mem_read.write(0);
                mem_write.write(1);
                branch.write(0);
                alu_op.write(0b00); // ADD para cálculo de endereço
                jump.write(0);
                break;

            case 0x04:            // BEQ
                reg_dst.write(0); // não importa
                alu_src.write(0);
                mem_to_reg.write(0); // não importa
                reg_write.write(0);
                mem_read.write(0);
                mem_write.write(0);
                branch.write(1);
                alu_op.write(0b01); // SUB para comparação
                jump.write(0);
                break;

            case 0x02:               // J
                reg_dst.write(0);    // não importa
                alu_src.write(0);    // não importa
                mem_to_reg.write(0); // não importa
                reg_write.write(0);
                mem_read.write(0);
                mem_write.write(0);
                branch.write(0);
                alu_op.write(0b00); // não importa
                jump.write(1);
                break;

            default: // NOP ou instrução não implementada
                reg_dst.write(0);
                alu_src.write(0);
                mem_to_reg.write(0);
                reg_write.write(0);
                mem_read.write(0);
                mem_write.write(0);
                branch.write(0);
                alu_op.write(0b00);
                jump.write(0);
            }
    }

    SC_CTOR(ControlUnit)
    {
        SC_METHOD(control_process);
        sensitive << opcode;
        dont_initialize();
    }
};