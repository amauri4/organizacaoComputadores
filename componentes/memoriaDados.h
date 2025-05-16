#include <systemc.h>
#include <vector>

SC_MODULE(DataMemory) {
    sc_in<sc_int<32>> address;
    sc_in<sc_int<32>> write_data;
    sc_in<bool> mem_read, mem_write;
    sc_out<sc_int<32>> read_data;
    
    std::vector<sc_int<32>> mem;
    
    void memory_process() {
        sc_int<32> addr = address.read();
        
        if (mem_write.read()) {
            if (addr/4 >= mem.size()) {
                mem.resize(addr/4 + 1, 0);
            }
            mem[addr/4] = write_data.read();
        }
        
        if (mem_read.read()) {
            if (addr/4 < mem.size()) {
                read_data.write(mem[addr/4]);
            } else {
                read_data.write(0);
            }
        }
    }

    void dump_memory(sc_int<32>* out_mem) const {
        for (size_t i = 0; i < mem.size(); ++i) {
            out_mem[i] = mem[i];
        }
    }
    
    SC_CTOR(DataMemory) {
        SC_METHOD(memory_process);
        sensitive << address << write_data << mem_read << mem_write;
    }
};