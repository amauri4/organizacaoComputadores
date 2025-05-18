#include <systemc.h>
#include <vector>

SC_MODULE(DataMemory) {
    // Portas de entrada/saída
    sc_in_clk clk;                  // Clock para sincronização do pipeline
    sc_in<sc_int<32>> address;      // Endereço de memória
    sc_in<sc_int<32>> write_data;   // Dado a ser escrito
    sc_in<bool> mem_read;           // Sinal de leitura
    sc_in<bool> mem_write;          // Sinal de escrita
    sc_out<sc_int<32>> read_data;   // Dado lido
    
    // Memória principal
    std::vector<sc_int<32>> mem;
    
    // Registros de pipeline
    struct {
        sc_int<32> addr;
        sc_int<32> data;
        bool read_en;
        bool write_en;
    } pipe_stage;

    void read_process() {
        // Estágio 1: Registra os sinais de controle (borda de subida)
        if (clk.posedge()) {
            pipe_stage.addr = address.read();
            pipe_stage.read_en = mem_read.read();
            pipe_stage.write_en = mem_write.read();
            pipe_stage.data = write_data.read();
        }
        
        // Estágio 2: Executa a operação (borda de descida)
        if (clk.negedge()) {
            if (pipe_stage.read_en) {
                sc_uint<32> addr = pipe_stage.addr & 0xFFFFFFFC;
                
                if ((addr >> 2) < mem.size()) {
                    read_data.write(mem[addr >> 2]);
                    cout << "MEM [READ] @ " << sc_time_stamp()
                         << " | Addr: 0x" << hex << addr
                         << " | Data: 0x" << mem[addr >> 2] << endl;
                } else {
                    read_data.write(0);
                }
            }
            
            if (pipe_stage.write_en) {
                sc_uint<32> addr = pipe_stage.addr & 0xFFFFFFFC;
                
                if ((addr >> 2) >= mem.size()) {
                    mem.resize((addr >> 2) + 1, 0);
                }
                
                mem[addr >> 2] = pipe_stage.data;
                cout << "MEM [WRITE] @ " << sc_time_stamp() 
                     << " | Addr: 0x" << hex << addr
                     << " | Data: 0x" << pipe_stage.data << endl;
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
        // Inicializa estágio de pipeline
        pipe_stage.addr = 0;
        pipe_stage.data = 0;
        pipe_stage.read_en = false;
        pipe_stage.write_en = false;
        
        // Configura método sensível ao clock
        SC_METHOD(read_process);
        sensitive << clk;
        dont_initialize();
    }
};