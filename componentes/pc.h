#ifndef PC_H
#define PC_H

#include <systemc.h>

SC_MODULE(PC) {
    sc_in<bool> reset;
    sc_in<bool> enable;
    sc_in<sc_uint<32>> next_addr;
    sc_in_clk clk; 
    sc_out<sc_uint<32>> current_addr;
    
    sc_uint<32> pc_reg;

    void update() {
        if (reset.read()) {
            pc_reg = 0;
            current_addr.write(0);
            cout << "PC RESET @ " << sc_time_stamp() << endl;
        }
        else if (enable && clk.posedge()) {  
            pc_reg = next_addr.read();
            current_addr.write(pc_reg);
            cout << "PC UPDATE @ " << sc_time_stamp() 
                 << " | New PC: 0x" << hex << pc_reg << endl;
        }
    }

    SC_CTOR(PC) {
        SC_METHOD(update);
        sensitive << clk.pos(); //<< reset << enable;
        dont_initialize();
    }
};

#endif // PC_H