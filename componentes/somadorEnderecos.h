#ifndef SOMADORENDERECOS_H
#define SOMADORENDERECOS_H

#include <systemc.h>

SC_MODULE(AdderAddress) {
    // Portas modificadas para aceitar sc_uint na entrada A
    sc_in<sc_uint<32>> a;     // PC+4 (agora sc_uint)
    sc_in<sc_int<32>> b;      // Offset estendido (mantém sc_int)
    sc_out<sc_int<32>> sum;   // Saída (mantém sc_int)

    void add() {
        // Verificação de overflow
        sc_uint<32> a_val = a.read();
        sc_int<32> b_val = b.read();
        sc_int<32> result = static_cast<sc_int<32>>(a_val) + b_val;
        
        // Detecta overflow
        if ((b_val > 0 && result < static_cast<sc_int<32>>(a_val)) ||
            (b_val < 0 && result > static_cast<sc_int<32>>(a_val))) {
            cout << "Aviso: Overflow no somador de endereços @ " << sc_time_stamp() << endl;
        }
        
        sum.write(result);
    }

    SC_CTOR(AdderAddress) : a("a"), b("b"), sum("sum") {
        SC_METHOD(add);
        sensitive << a << b;
        dont_initialize();
    }
};

#endif