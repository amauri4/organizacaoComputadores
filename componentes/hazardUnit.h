#ifndef HAZARD_UNIT_H
#define HAZARD_UNIT_H

#include <systemc.h>

SC_MODULE(HazardUnit) {
    // Entradas
    sc_in<bool> id_ex_mem_read;  // Se está fazendo load no estágio EX
    sc_in<sc_uint<5>> id_ex_rt;  // Registrador destino do estágio EX
    sc_in<sc_uint<5>> if_id_rs;  // Registrador fonte rs do estágio ID
    sc_in<sc_uint<5>> if_id_rt;  // Registrador fonte rt do estágio ID
    
    // Saídas
    sc_out<bool> pc_write;       // Permite escrita no PC
    sc_out<bool> if_id_write;    // Permite escrita no registrador IF/ID
    sc_out<bool> control_mux;    // Seleciona se zera os sinais de controle
    
    SC_CTOR(HazardUnit) {
        SC_METHOD(detect_hazard);
        sensitive << id_ex_mem_read << id_ex_rt << if_id_rs << if_id_rt;
    }
    
    void detect_hazard() {
        bool hazard = false;
        
        // Detecta hazard de dados quando:
        // 1. A instrução no EX é um load (mem_read=1)
        // 2. O registrador de destino no EX é igual a rs ou rt no ID
        if (id_ex_mem_read.read() && 
           (id_ex_rt.read() == if_id_rs.read() || id_ex_rt.read() == if_id_rt.read())) {
            hazard = true;
        }
        
        // Quando há hazard:
        // - Congela PC e IF/ID
        // - Zera os sinais de controle no ID/EX (insere NOP)
        pc_write.write(!hazard);
        if_id_write.write(!hazard);
        control_mux.write(hazard);
    }
};

#endif // HAZARD_UNIT_H