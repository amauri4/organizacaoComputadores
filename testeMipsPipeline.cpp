#include <systemc.h>
#include "organizacaoMipsPipeline.h"
#include <vector>

using namespace std;
// Função para imprimir registradores com mais detalhe
void print_registradores(MIPS_Pipelined &mips)
{
    mips.visualization_registers();
    cout << "Banco de Registradores:" << endl;
    for (int i = 0; i < 32; ++i)
    {
        cout << "$" << dec << setw(2) << i << " = 0x" << hex << setw(8) << setfill('0')
             << mips.registers[i] << " (" << dec << setfill(' ') << setw(10)
             << mips.registers[i] << ")" << endl;
    }
}

void print_memory(MIPS_Pipelined &mips, bool show_all = false)
{
    using std::cout;
    using std::dec;
    using std::endl;
    using std::hex;
    using std::setfill;
    using std::setw;

    // Verificação de segurança
    if (!mips.dmem)
    {
        cout << "Erro: Memória de dados não inicializada!" << endl;
        return;
    }

    // Tamanho seguro para a memória
    const int MEM_SIZE = 32;
    sc_int<32> memory_data[MEM_SIZE] = {0}; // Inicializa com zero

    // Copia segura (evita overflow)
    try
    {
        mips.dmem->dump_memory(memory_data);
    }
    catch (...)
    {
        cout << "Erro ao acessar memória!" << endl;
        return;
    }

    cout << "Memória de Dados (32 palavras):" << endl;
    cout << "╔════════════╦═════════════════════════╗" << endl;
    cout << "║ Endereço   ║ Dado                    ║" << endl;
    cout << "╠════════════╬═════════════════════════╣" << endl;

    int count = 0;

    for (int i = 0; i < 32; i++)
    {
        sc_int<32> word = memory_data[i];

        if (show_all || word != 0)
        {
            uint32_t unsigned_word = static_cast<uint32_t>(word);
            int addr = i * 4; // Cada palavra ocupa 4 bytes

            cout << "║ 0x" << hex << setw(8) << setfill('0') << addr << " ║ "
                 << "0x" << setw(8) << unsigned_word << " ("
                 << dec << setfill(' ') << setw(10) << word << ") ║" << endl;

            count++;
        }
    }

    cout << "╚════════════╩═════════════════════════╝" << endl;
    cout << "Palavras não-zero: " << dec << count << "/32" << endl;
}

int sc_main(int argc, char *argv[])
{
    using namespace std;
    // Criar o clock (10ns de período)
    sc_clock clk("clk", 10, SC_NS);

    // Sinais de controle
    sc_signal<bool> reset;

    // Instanciar o MIPS Pipeline
    MIPS_Pipelined mips("MIPS_Pipelined");
    mips.clk(clk);
    mips.reset(reset);

    // Criar arquivo de trace VCD
    sc_trace_file *tf = sc_create_vcd_trace_file("mips_pipeline_trace");
    sc_trace(tf, clk, "clock");
    sc_trace(tf, reset, "reset");
    sc_trace(tf, mips.pc_out, "pc");
    sc_trace(tf, mips.if_id.instruction, "if_id_instruction");
    sc_trace(tf, mips.ex_mem.alu_result, "ex_mem_alu_result");
    sc_trace(tf, mips.mem_wb.alu_result, "mem_wb_alu_result");
    sc_trace(tf, mips.regs->registers[1], "reg_1");
    sc_trace(tf, mips.regs->registers[2], "reg_2");
    sc_trace(tf, mips.regs->registers[3], "reg_3");

    // Programa de teste (sem hazards)
    vector<uint32_t> program = {
        0x20010005, // addi $3, $0, 5     (Inicializa $1 = 5)
        //0x2002000A, // addi $2, $0, 10    (Inicializa $2 = 10)
        //0x00221820, // add $3, $1, $2     ($3 = $1 + $2 = 15)
        0xAC030020, // sw $3, 0x20($0)    (Armazena 15 na memória[8])
        //0x8C040020, // lw $4, 0x20($0)    (Carrega $4 = memória[8])
       // 0x08000006  // j 0x00000018       (Pula para a última instrução)
    };

    // Carregar programa na memória
    mips.load_program(program);

    // Reset inicial (2 ciclos)
    reset.write(true);
    sc_start(20, SC_NS);
    reset.write(false);

    cout << "Iniciando execução do programa..." << endl;

    // Executar por 15 ciclos (150ns)
    for (int i = 0; i < 15; i++)
    {
        cout << "\n════════════════════════════════════════════════════\n\n";
        cout << "Ciclo " << i << " @ " << sc_time_stamp() << endl;
        cout << "PC: 0x" << hex << mips.pc_out.read() << endl;
        cout << "IF/ID: 0x" << mips.if_id.instruction.read() << endl;
        cout << "Reg $1: " << mips.regs->registers[1] << endl;
        cout << "Reg $2: " << mips.regs->registers[2] << endl;
        cout << "Reg $3: " << mips.regs->registers[3] << endl;
        cout << "Reg $4: " << mips.regs->registers[4] << endl;
        cout << "-----------------------------" << endl;
        cout << "\n════════════════════════════════════════════════════\n";

        sc_start(10, SC_NS);
    }
    print_memory(mips);
    print_registradores(mips);
    // Finalização
    sc_close_vcd_trace_file(tf);
    cout << "Simulação concluída." << endl;

    return 0;
}