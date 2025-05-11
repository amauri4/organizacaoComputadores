#include "../componentes/ula.h"
#include "../componentes/controleUla.h"
#include <systemc.h>
#include <string>

using namespace std;

SC_MODULE(ALUTestbench) {
    sc_clock clk;
    sc_signal<sc_uint<32>> a, b;
    sc_signal<sc_uint<4>> alu_ctrl_direct;
    sc_signal<sc_uint<4>> alu_ctrl_auto;
    sc_signal<sc_uint<4>> alu_ctrl_muxed;
    sc_signal<sc_uint<32>> result;
    sc_signal<bool> zero;
    
    sc_signal<sc_uint<2>> alu_op;
    sc_signal<sc_uint<6>> funct;
    sc_signal<bool> use_auto_control;

    ALU alu;
    ALUControl alu_control;

    SC_CTOR(ALUTestbench) :
        clk("clk", 10, SC_NS),
        alu("alu"),
        alu_control("alu_control")
    {
        // Conectar entradas da ULA
        alu.a(a);
        alu.b(b);
        alu.alu_control(alu_ctrl_muxed);
        alu.result(result);
        alu.zero(zero);

        // Conectar ALUControl
        alu_control.alu_op(alu_op);
        alu_control.funct(funct);
        alu_control.alu_control(alu_ctrl_auto);

        // Mux de controle da ALU
        SC_METHOD(select_control);
        sensitive << use_auto_control << alu_ctrl_auto << alu_ctrl_direct;

        SC_THREAD(run_tests);
    }

    void select_control() {
        alu_ctrl_muxed.write(use_auto_control.read() ? alu_ctrl_auto.read() : alu_ctrl_direct.read());
    }

    void run_tests() {
        cout << "========== TESTE DA ULA ==========" << endl;

        // Teste com controle direto
        use_auto_control.write(false);
        cout << "\n[TESTE 1] Teste direto da ULA (controle manual):" << endl;
        test_ula_direct();

        // Teste com controle automático via ALUControl
        use_auto_control.write(true);
        cout << "\n[TESTE 2] Teste do ALUControl + ULA:" << endl;
        test_alu_control();

        cout << "\n========== FIM DOS TESTES ==========" << endl;
        sc_stop();
    }

    void test_ula_direct() {
        test_operation(10, 20, 0b0010, 30, "ADD");
        test_operation(30, 10, 0b0110, 20, "SUB");
        test_operation(0xF0F0F0F0, 0xFFFF0000, 0b0000, 0xF0F00000, "AND");
    }

    void test_alu_control() {
        alu_op.write(0b10); // Tipo R

        funct.write(0x20); wait(10, SC_NS);
        verify_control(0b0010, "ADD (funct 0x20)");
        test_operation(10, 20, alu_ctrl_auto.read(), 30, "ADD (auto)");

        funct.write(0x22); wait(10, SC_NS);
        verify_control(0b0110, "SUB (funct 0x22)");
        test_operation(30, 10, alu_ctrl_auto.read(), 20, "SUB (auto)");

        funct.write(0x24); wait(10, SC_NS);
        verify_control(0b0000, "AND (funct 0x24)");
        test_operation(0xF0F0F0F0, 0xFFFF0000, alu_ctrl_auto.read(), 0xF0F00000, "AND (auto)");
    }

    void test_operation(sc_uint<32> op1, sc_uint<32> op2, sc_uint<4> ctrl, 
                        sc_uint<32> expected, const string& name) {
        a.write(op1);
        b.write(op2);
        alu_ctrl_direct.write(ctrl);
        wait(10, SC_NS);

        sc_uint<32> res = result.read();
        if (res != expected) {
            cerr << "  [ERRO] " << name << ": esperado 0x" << hex << expected 
                 << ", obtido 0x" << res << dec << endl;
        } else {
            cout << "  [OK] " << name << ": 0x" << hex << res << dec << endl;
        }
    }

    void verify_control(sc_uint<4> expected, const string& name) {
        sc_uint<4> actual = alu_ctrl_auto.read();
        if (actual != expected) {
            cerr << "  [ERRO] " << name << ": esperado " << hex << (int)expected 
                 << ", obtido " << (int)actual << dec << endl;
        } else {
            cout << "  [OK] " << name << ": " << hex << (int)actual << dec << endl;
        }
    }
};

int sc_main(int argc, char* argv[]) {
    ALUTestbench tb("tb");
    sc_trace_file* tf = sc_create_vcd_trace_file("alu_trace");

    sc_trace(tf, tb.clk, "clk");
    sc_trace(tf, tb.a, "a");
    sc_trace(tf, tb.b, "b");
    sc_trace(tf, tb.alu_ctrl_direct, "alu_ctrl_direct");
    sc_trace(tf, tb.alu_ctrl_auto, "alu_ctrl_auto");
    sc_trace(tf, tb.alu_ctrl_muxed, "alu_ctrl_muxed");
    sc_trace(tf, tb.result, "result");
    sc_trace(tf, tb.zero, "zero");
    sc_trace(tf, tb.alu_op, "alu_op");
    sc_trace(tf, tb.funct, "funct");
    sc_trace(tf, tb.use_auto_control, "use_auto_control");

    sc_start();
    sc_close_vcd_trace_file(tf);
    return 0;
}
