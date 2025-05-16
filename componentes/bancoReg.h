#include <systemc.h>

SC_MODULE(RegisterFile) {
    sc_in<sc_uint<5>> read_reg1, read_reg2, write_reg;
    sc_in<sc_int<32>> write_data;
    sc_in<bool> reg_write;
    sc_in_clk clk;
    
    sc_out<sc_int<32>> read_data1, read_data2;
    
    sc_int<32> registers[32]; // 32 registradores de 32 bits
    
    void read() {
        read_data1.write(registers[read_reg1.read()]);
        read_data2.write(registers[read_reg2.read()]);
        
        cout << "REG READ @ " << sc_time_stamp() 
        << " | $" << read_reg1.read() << "=0x" << hex << registers[read_reg1.read()]
        << " | $" << read_reg2.read() << "=0x" << hex << registers[read_reg2.read()] << endl;
    }
    
    void write() {
        if (reg_write.read() && write_reg.read() != 0 && clk.posedge()) { // $zero não pode ser escrito
            registers[write_reg.read()] = write_data.read();
            cout << "REG WRITE @ " << sc_time_stamp() 
                 << " | $" << write_reg.read() << "=0x" << hex << write_data.read() << endl;
        }
    }

    void dump_registers(sc_int<32>* out_regs) const {
        for (int i = 0; i < 32; ++i) {
            out_regs[i] = registers[i];
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
        dont_initialize();
    }
};