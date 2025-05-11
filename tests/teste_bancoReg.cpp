#include "../componentes/bancoReg.h"
#include <systemc.h>

int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);
    sc_signal<sc_uint<5>> read_reg1, read_reg2, write_reg;
    sc_signal<sc_uint<32>> write_data, read_data1, read_data2;
    sc_signal<bool> reg_write;
    
    RegisterFile rf("RegisterFile");
    rf.clk(clk);
    rf.read_reg1(read_reg1);
    rf.read_reg2(read_reg2);
    rf.write_reg(write_reg);
    rf.write_data(write_data);
    rf.reg_write(reg_write);
    rf.read_data1(read_data1);
    rf.read_data2(read_data2);
    
    sc_trace_file *tf = sc_create_vcd_trace_file("register_file_trace");
    sc_trace(tf, clk, "clk");
    sc_trace(tf, read_reg1, "read_reg1");
    sc_trace(tf, read_reg2, "read_reg2");
    sc_trace(tf, write_reg, "write_reg");
    sc_trace(tf, write_data, "write_data");
    sc_trace(tf, reg_write, "reg_write");
    sc_trace(tf, read_data1, "read_data1");
    sc_trace(tf, read_data2, "read_data2");
    
    cout << "Iniciando teste do banco de registradores..." << endl;
    
    // Escrever no registrador 1
    write_reg.write(1);
    write_data.write(0x12345678);
    reg_write.write(1);
    sc_start(10, SC_NS);
    
    // Ler do registrador 1
    reg_write.write(0);
    read_reg1.write(1);
    sc_start(10, SC_NS);
    assert(read_data1.read() == 0x12345678);
    
    // Escrever no registrador 2
    write_reg.write(2);
    write_data.write(0x87654321);
    reg_write.write(1);
    sc_start(10, SC_NS);
    
    // Ler dos registradores 1 e 2
    reg_write.write(0);
    read_reg1.write(1);
    read_reg2.write(2);
    sc_start(10, SC_NS);
    assert(read_data1.read() == 0x12345678);
    assert(read_data2.read() == 0x87654321);
    
    // Tentar escrever no registrador 0 (não deve funcionar)
    write_reg.write(0);
    write_data.write(0x11111111);
    reg_write.write(1);
    sc_start(10, SC_NS);
    
    read_reg1.write(0);
    sc_start(10, SC_NS);
    assert(read_data1.read() == 0); 
    
    cout << "Teste do banco de registradores concluído com sucesso!" << endl;
    
    sc_close_vcd_trace_file(tf);
    return 0;
}