#include <systemc.h>

SC_MODULE(SignExtend) {
    sc_in<sc_uint<16>> input;
    sc_out<sc_uint<32>> output;
    
    void extend_process() {
        sc_uint<16> in = input.read();
        sc_uint<32> out;
        
        if (in[15] == 1) {
            out = 0xFFFF0000 | in;
        } else {
            out = in;
        }
        
        output.write(out);
    }
    
    SC_CTOR(SignExtend) {
        SC_METHOD(extend_process);
        sensitive << input;
    }
};