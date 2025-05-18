#ifndef DECODIFICADOR_H
#define DECODIFICADOR_H

#include <systemc.h>

SC_MODULE(Decodificador)
{
    sc_in<sc_uint<32>> instruction;
    sc_out<sc_uint<6>> opcode;
    sc_out<sc_uint<5>> rs, rt, rd;
    sc_out<sc_uint<16>> imm;
    sc_out<sc_uint<6>> funct;
    sc_out<sc_uint<26>> jump_address;

    SC_CTOR(Decodificador)
    {
        SC_METHOD(decode);
        sensitive << instruction;
    }

    void decode()
    {
        sc_uint<32> inst = instruction.read();
        opcode.write(inst.range(31, 26));
        rs.write(inst.range(25, 21));
        rt.write(inst.range(20, 16));
        rd.write(inst.range(15, 11));
        imm.write(inst.range(15, 0));
        funct.write(inst.range(5, 0));
        // jump_address.write(inst.range(25, 0));
        jump_address.write((inst >> 0) & 0x03FFFFFF);
        cout << "DECODER DEBUG: "
             << "Instr=0x" << hex << instruction.read()
             << " | rt field=0x" << instruction.read().range(20, 16)
             << " | rt output=" << rt.read() << endl;
    }
};

#endif // DECODIFICADOR_H