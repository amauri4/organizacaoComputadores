#ifndef HAZARD_UNIT_H
#define HAZARD_UNIT_H

#include <systemc.h>

SC_MODULE(HazardUnit) {
    // Entradas (conforme seus sinais existentes)
    sc_in<bool> id_ex_mem_read;     // Indica operação de load no EX
    sc_in<bool> id_ex_mem_write;    // Indica operação de store no EX
    sc_in<sc_uint<5>> id_ex_rt;     // Registrador destino no EX (para loads/stores)
    sc_in<sc_uint<5>> if_id_rs;     // Registrador fonte rs no ID
    sc_in<sc_uint<5>> if_id_rt;     // Registrador fonte rt no ID
    
    // Saídas (conforme seus sinais existentes)
    sc_out<bool> control_mux;       // 1 = hazard (insere NOP), 0 = normal
    sc_out<bool> pc_write;          // 1 = PC atualiza, 0 = congela PC
    sc_out<bool> if_id_write;       // 1 = IF/ID atualiza, 0 = congela IF/ID

    SC_CTOR(HazardUnit) {
        SC_METHOD(detect_hazard);
        sensitive << id_ex_mem_read << id_ex_mem_write 
                 << id_ex_rt << if_id_rs << if_id_rt;
    }
    
    void detect_hazard() {
        // 1. Detecta hazard load-use (dependência de dados)
        bool load_use_hazard = id_ex_mem_read.read() && 
                             (id_ex_rt.read() == if_id_rs.read() || 
                              id_ex_rt.read() == if_id_rt.read());
        
        // 2. Detecta hazard para store (dados não prontos)
        // Modificação: Verifica apenas dependência com a instrução anterior (load)
        bool store_hazard = id_ex_mem_write.read() && 
                            ((id_ex_rt.read() == if_id_rs.read()) ||  // Verifica rs
                            (id_ex_rt.read() == if_id_rt.read()));   // Verifica rt
        
        // Sinalização de controle
        bool hazard = load_use_hazard || store_hazard;
        
        // Controle do pipeline
        pc_write.write(!hazard);       // 0 = congela PC
        if_id_write.write(!hazard);    // 0 = congela IF/ID
        control_mux.write(hazard);     // 1 = insere NOP

        // Debug (opcional)
        if (hazard) {
            cout << "HAZARD DETECTED @ " << sc_time_stamp() << " | ";
            if (load_use_hazard) cout << "Load-Use";
            if (store_hazard) cout << "Store";
            cout << " | Freezing pipeline" << endl;
            cout << "  Affected registers: $" 
                 << (load_use_hazard ? id_ex_rt.read() : if_id_rt.read()) << endl;
        }
    }
};

#endif // HAZARD_UNIT_H