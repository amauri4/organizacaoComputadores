#include <systemc.h>

SC_MODULE(ALU) {
    // Portas de entrada/saída (mantidas iguais)
    sc_in<sc_int<32>> a, b;         
    sc_in<sc_uint<4>> alu_control;   
    sc_out<sc_int<32>> result; 
    sc_out<bool> zero;
    
    void alu_process() {
        sc_int<32> a_val = a.read();
        sc_int<32> b_val = b.read();
        sc_uint<4> control = alu_control.read();
        sc_int<32> res = 0;
        bool z = false;
        
        switch(control) {
            // Operações Aritméticas
            case 0b0010: // ADD (para add, addi, lw, sw)
                res = a_val + b_val;
                break;
                
            case 0b0110: // SUB (para sub, beq)
                res = a_val - b_val;
                break;
                
            // Operações Lógicas
            case 0b0000: // AND
                res = a_val & b_val;
                break;
                
            case 0b0001: // OR
                res = a_val | b_val;
                break;
                
            case 0b1100: // NOR
                res = ~(a_val | b_val);
                break;
                
            // Operações de Comparação
            case 0b0111: // SLT (set less than)
                res = (a_val < b_val) ? 1 : 0;
                break;
                
            case 0b1000: // SLL (shift left logical)
                res = b_val << a_val;
                break;
                
            default:
                res = 0;
                cerr << "Operação ALU não implementada: " << hex << control << endl;
        }
        
        zero.write(res == 0);
        result.write(res);
    }
    
    SC_CTOR(ALU) {
        SC_METHOD(alu_process);
        sensitive << a << b << alu_control;
        //SC_METHOD(debug);
        //sensitive << result << zero;
        dont_initialize();
    }
    
    void debug() {
        cout << "ALU: a=" << hex << a.read() 
             << " b=" << b.read() 
             << " ctrl=" << alu_control.read() 
             << " res=" << result.read() 
             << " zero=" << zero.read() << endl;
    }
};