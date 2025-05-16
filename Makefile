# Configurações do compilador
CXX := g++
CXXFLAGS := -std=c++17 -stdlib=libc++ -Wall -Wextra
SYSTEMC_HOME := /Users/amauri/Documents/organizacaoProjeto1/systemc-2.3.4/build-arm64/install

# Flags de inclusão e bibliotecas
INCLUDES := -I$(SYSTEMC_HOME)/include
LDFLAGS := -L$(SYSTEMC_HOME)/lib -Wl,-rpath,$(SYSTEMC_HOME)/lib
LIBS := -lsystemc

# Nomes dos arquivos
SRC := testeMips2.cpp
TARGET := test

# Regra principal
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) $(LDFLAGS) $(LIBS) -o $(TARGET)

# Limpeza
clean:
	rm -f $(TARGET) *.o

.PHONY: all clean