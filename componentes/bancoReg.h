#include <systemc.h>

SC_MODULE(RegisterFile) {
    // Portas de entrada/saída
    sc_in_clk clk;  // Clock para sincronização do pipeline
    sc_in<sc_uint<5>> read_reg1, read_reg2;
    sc_in<sc_uint<5>> write_reg;
    sc_in<sc_int<32>> write_data;
    sc_in<bool> reg_write;
    
    sc_out<sc_int<32>> read_data1, read_data2;
    
    // Registradores do banco
    sc_int<32> registers[32];
    
    // Estágios de pipeline
    struct {
        sc_uint<5> reg1_addr, reg2_addr;
        sc_int<32> reg1_data, reg2_data;
    } pipe_stage;

    void read() {
        // Estágio 1: Busca dos endereços (borda de subida) - invertido antes
        if (clk.posedge()) {
            pipe_stage.reg1_addr = read_reg1.read();
            pipe_stage.reg2_addr = read_reg2.read();
            
            // Debug do estágio 1
            cout << "REGISTER FILE [BUSCANDO] @ " << sc_time_stamp() << ":\n"
                 << "  Fetch addresses:\n"
                 << "    read_reg1 = " << pipe_stage.reg1_addr 
                 << " (hex: 0x" << hex << pipe_stage.reg1_addr << ")\n"
                 << "    read_reg2 = " << pipe_stage.reg2_addr 
                 << " (hex: 0x" << hex << pipe_stage.reg2_addr << ")" << endl;
        }
        
        // Estágio 2: Leitura dos dados (borda de descida)
        if (clk.negedge()) {
            pipe_stage.reg1_data = registers[pipe_stage.reg1_addr];
            pipe_stage.reg2_data = registers[pipe_stage.reg2_addr];
            
            read_data1.write(pipe_stage.reg1_data);
            read_data2.write(pipe_stage.reg2_data);
            
            // Debug do estágio 2
            cout << "REGISTER FILE [LENDO OS ENDERECOS] @ " << sc_time_stamp() << ":\n"
                 << "  Read values:\n"
                 << "    $" << pipe_stage.reg1_addr << " = 0x" 
                 << hex << pipe_stage.reg1_data << "\n"
                 << "    $" << pipe_stage.reg2_addr << " = 0x" 
                 << hex << pipe_stage.reg2_data << endl;
        }
    }
    
    void write() {
        // Escrita ocorre na borda de descida?
        if (clk.negedge() && reg_write.read()) { // pos ANTES
            if (reg_write.read() && write_reg.read() != 0) { // $zero não pode ser escrito
                registers[write_reg.read()] = write_data.read();
                cout << "REG WRITE @ " << sc_time_stamp() 
                     << " | $" << write_reg.read() 
                     << "=0x" << hex << write_data.read() << endl;
            }
        }
    }

    void dump_registers(sc_int<32>* out_regs) const {
        for (int i = 0; i < 32; ++i) {
            out_regs[i] = registers[i];
        }
    }
    
    SC_CTOR(RegisterFile) {
        // Inicializa registradores
        for (int i = 0; i < 32; i++) {
            registers[i] = 0;
        }
        
        // Inicializa estágio de pipeline
        pipe_stage.reg1_addr = 0;
        pipe_stage.reg2_addr = 0;
        pipe_stage.reg1_data = 0;
        pipe_stage.reg2_data = 0;
        
        // Configura métodos sensíveis
        SC_METHOD(read);
        sensitive << clk; // neg
        dont_initialize();
        
        SC_METHOD(write);
        sensitive << clk.neg(); //neg // << reg_write << write_reg << write_data
        dont_initialize();
    }
};