#ifndef PC_H
#define PC_H

#include <systemc.h>

SC_MODULE(PC) {
    sc_in_clk clk;
    sc_in<bool> reset;
    sc_in<sc_uint<32>> next_addr;
    sc_out<sc_uint<32>> current_addr;

    SC_HAS_PROCESS(PC);

    PC(sc_module_name name) : sc_module(name) {
        SC_METHOD(update);
        sensitive << clk.pos();
        async_reset_signal_is(reset, true);
    }

    void update() {
        if (reset.read()) {
            current_addr.write(0);
        } else {
            current_addr.write(next_addr.read());
        }
    }
};

#endif // PC_H