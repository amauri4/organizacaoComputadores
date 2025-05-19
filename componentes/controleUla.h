#include <systemc.h>

SC_MODULE(ALUControl) {
    sc_in<sc_uint<2>> alu_op;
    sc_in<sc_uint<6>> funct;
    sc_out<sc_uint<4>> alu_control;
    
    void alu_control_process() {
        sc_uint<2> op = alu_op.read();
        sc_uint<6> func = funct.read();
        sc_uint<4> control = 0;
        
        switch(op) {
            case 0b00: // LW/SW/ADDI
                control = 0b0010; // ADD
                break;
                
            case 0b01: // BEQ/BNE
                control = 0b0110; // SUB
                break;
                
            case 0b10: // R-type
                switch(func) {
                    case 0x20: control = 0b0010; break; // ADD
                    case 0x22: control = 0b0110; break; // SUB
                    case 0x24: control = 0b0000; break; // AND
                    case 0x25: control = 0b0001; break; // OR
                    case 0x26: control = 0b1100; break; // XOR
                    case 0x27: control = 0b1101; break; // NOR
                    case 0x2A: control = 0b0111; break; // SLT
                    case 0x00: control = 0b1000; break; // SLL
                    default:   control = 0b0000;
                }
                break;
                
            case 0b11: // SLTI
                control = 0b0111; // SLT
                break;
                
            default:
                control = 0b0000;
        }
        
        alu_control.write(control);
    }
    
    SC_CTOR(ALUControl) {
        SC_METHOD(alu_control_process);
        sensitive << alu_op << funct;
    }
};