#include <systemc.h>

SC_MODULE(SignExtend) {
    sc_in<sc_uint<16>> input;
    sc_out<sc_int<32>> output;
    
    void extend() {
        sc_uint<16> imm = input.read();
        sc_int<32> extended;
        
        // Interpreta imm como complemento de 2 e estende
        if (imm[15] == 1)  // Se negativo (MSB = 1)
            extended = (0xFFFF0000 | imm);  // Estende com 1's
        else
            extended = imm;  // Estende com 0's
        
        output.write(extended);
    }
    
    SC_CTOR(SignExtend) {
        SC_METHOD(extend);
        sensitive << input;
    }
};