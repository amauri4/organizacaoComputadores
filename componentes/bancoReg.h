#include <systemc.h>

SC_MODULE(RegisterFile) {
    sc_in<sc_uint<5>> read_reg1, read_reg2, write_reg;
    sc_in<sc_uint<32>> write_data;
    sc_in<bool> reg_write;
    sc_in_clk clk;
    
    sc_out<sc_uint<32>> read_data1, read_data2;
    
    sc_uint<32> registers[32]; // 32 registradores de 32 bits
    
    void read() {
        read_data1.write(registers[read_reg1.read()]);
        read_data2.write(registers[read_reg2.read()]);
    }
    
    void write() {
        if (reg_write.read() && write_reg.read() != 0) { // $zero não pode ser escrito
            registers[write_reg.read()] = write_data.read();
        }
    }
    
    SC_CTOR(RegisterFile) {
        for (int i = 0; i < 32; i++) {
            registers[i] = 0;
        }
        
        SC_METHOD(read);
        sensitive << read_reg1 << read_reg2;
        
        SC_METHOD(write);
        sensitive << clk.pos();
    }
};