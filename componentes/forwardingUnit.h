#ifndef FORWARDING_UNIT_H
#define FORWARDING_UNIT_H

#include <systemc.h>

SC_MODULE(ForwardingUnit) {
    // Entradas (mantidas conforme seu código)
    sc_in<bool> ex_mem_reg_write;    // Indica escrita no estágio MEM
    sc_in<bool> mem_wb_reg_write;    // Indica escrita no estágio WB
    sc_in<sc_uint<5>> ex_mem_rd;     // Registrador destino MEM
    sc_in<sc_uint<5>> mem_wb_rd;     // Registrador destino WB
    sc_in<sc_uint<5>> id_ex_rs;      // Registrador fonte rs EX
    sc_in<sc_uint<5>> id_ex_rt;      // Registrador fonte rt EX
    
    // Saídas (mantidas conforme seu código)
    sc_out<sc_uint<2>> forwardA;     // Controle para entrada A da ULA
    sc_out<sc_uint<2>> forwardB;     // Controle para entrada B da ULA
    
    SC_CTOR(ForwardingUnit) {
        SC_METHOD(forward_signals);
        sensitive << ex_mem_reg_write << mem_wb_reg_write 
                 << ex_mem_rd << mem_wb_rd 
                 << id_ex_rs << id_ex_rt;
    }
    
    void forward_signals() {
        // Forwarding para entrada A da ULA
        if (ex_mem_reg_write.read() && ex_mem_rd.read() != 0 && ex_mem_rd.read() == id_ex_rs.read()) {
            forwardA.write(0b10); // Forward EX/MEM
        }
        else if (mem_wb_reg_write.read() && mem_wb_rd.read() != 0 && mem_wb_rd.read() == id_ex_rs.read()) {
            forwardA.write(0b01); // Forward MEM/WB
        }
        else {
            forwardA.write(0b00);
        }

        // Forwarding para entrada B da ULA (store)
        if (ex_mem_reg_write.read() && ex_mem_rd.read() != 0 && ex_mem_rd.read() == id_ex_rt.read()) {
            forwardB.write(0b10); // Prioridade para EX/MEM
        }
        else if (mem_wb_reg_write.read() && mem_wb_rd.read() != 0 && mem_wb_rd.read() == id_ex_rt.read()) {
            forwardB.write(0b01); // Fallback para MEM/WB
        }
        else {
            forwardB.write(0b00);
        }
    }
};

#endif // FORWARDING_UNIT_H