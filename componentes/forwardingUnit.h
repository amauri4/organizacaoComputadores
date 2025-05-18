#ifndef FORWARDING_UNIT_H
#define FORWARDING_UNIT_H

#include <systemc.h>

SC_MODULE(ForwardingUnit) {
    // Entradas
    sc_in<bool> ex_mem_reg_write;    // Se vai escrever no registrador no estágio MEM
    sc_in<bool> mem_wb_reg_write;    // Se vai escrever no registrador no estágio WB
    sc_in<sc_uint<5>> ex_mem_rd;     // Registrador destino no estágio MEM
    sc_in<sc_uint<5>> mem_wb_rd;     // Registrador destino no estágio WB
    sc_in<sc_uint<5>> id_ex_rs;      // Registrador fonte rs no estágio EX
    sc_in<sc_uint<5>> id_ex_rt;      // Registrador fonte rt no estágio EX
    
    // Saídas
    sc_out<sc_uint<2>> forwardA;     // Controle para forwarding da entrada A da ULA
    sc_out<sc_uint<2>> forwardB;     // Controle para forwarding da entrada B da ULA
    
    SC_CTOR(ForwardingUnit) {
        SC_METHOD(forward_signals);
        sensitive << ex_mem_reg_write << mem_wb_reg_write 
                 << ex_mem_rd << mem_wb_rd 
                 << id_ex_rs << id_ex_rt;
    }
    
    void forward_signals() {
        // Forwarding para entrada A (rs)
        if (ex_mem_reg_write.read() && ex_mem_rd.read() != 0 && 
            ex_mem_rd.read() == id_ex_rs.read()) {
            forwardA.write(0b10); // Forward do estágio EX/MEM
        }
        else if (mem_wb_reg_write.read() && mem_wb_rd.read() != 0 && 
                mem_wb_rd.read() == id_ex_rs.read()) {
            forwardA.write(0b01); // Forward do estágio MEM/WB
        }
        else {
            forwardA.write(0b00); // Sem forwarding
        }
        
        // Forwarding para entrada B (rt)
        if (ex_mem_reg_write.read() && ex_mem_rd.read() != 0 && 
            ex_mem_rd.read() == id_ex_rt.read()) {
            forwardB.write(0b10); // Forward do estágio EX/MEM
        }
        else if (mem_wb_reg_write.read() && mem_wb_rd.read() != 0 && 
                mem_wb_rd.read() == id_ex_rt.read()) {
            forwardB.write(0b01); // Forward do estágio MEM/WB
        }
        else {
            forwardB.write(0b00); // Sem forwarding
        }
    }
};

#endif // FORWARDING_UNIT_H