#include <systemc.h>
#include <vector>

SC_MODULE(DataMemory) {
    sc_in_clk clk;                  // Clock adicionado
    sc_in<sc_int<32>> address;
    sc_in<sc_int<32>> write_data;
    sc_in<bool> mem_read, mem_write;
    sc_out<sc_int<32>> read_data;
    
    std::vector<sc_int<32>> mem;

    void write_process() {
        if (mem_write.read()) {
            sc_uint<32> addr = address.read() & 0xFFFFFFFC;
            
            if ((addr >> 2) >= mem.size()) {
                mem.resize((addr >> 2) + 1, 0);
            }
            
            mem[addr >> 2] = write_data.read();
            cout << "MEM [WRITE] @ " << sc_time_stamp() 
                 << " | Addr: 0x" << hex << addr
                 << " | Data: 0x" << write_data.read() << endl;
        }
    }

    void read_process() {
        if (mem_read.read()) {
            sc_uint<32> addr = address.read() & 0xFFFFFFFC;
            
            if ((addr >> 2) < mem.size()) {
                read_data.write(mem[addr >> 2]);
                cout << "MEM [READ] @ " << sc_time_stamp()
                     << " | Addr: 0x" << hex << addr
                     << " | Data: 0x" << mem[addr >> 2] << endl;
            } else {
                read_data.write(0);
            }
        }
    }

    void dump_memory(sc_int<32>* out_mem) const {
        if (!out_mem) return;
        
        for(int i = 0; i < 32 && i < mem.size(); i++) {
            out_mem[i] = mem[i];
        }
    }

    SC_CTOR(DataMemory) : mem(32, 0) {
        SC_METHOD(write_process);
        sensitive << clk.pos();  // Sincronizado com borda de subida
        dont_initialize();

        SC_METHOD(read_process);
        sensitive << address << mem_read;  // Leitura combinacional
    }
};