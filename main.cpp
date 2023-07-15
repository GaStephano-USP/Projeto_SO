#include <vector>
#include <string>

const int TOTAL_MEMORY_SIZE = 20;

std::vector<int> memory(TOTAL_MEMORY_SIZE - 1);


struct Process{
    int id;
    int memorySize;
    int programCounter;
    std::vector<std::string> instructions;

    Process(int _id, int _memorySize, int _programCounter, const std::vector<std::string>& _instructions){
        id = _id;
        memorySize = _memorySize;
        programCounter = _programCounter;
        instructions = _instructions;
    };
};

bool allocateMemory(std::vector<int>& memory, Process& process) {
    int memorySize = process.memorySize;
    int start = -1;
    int count = 0;
    // Percorre a memória em busca de posições livres
    for (int i = 0; i < memory.size(); i++) {
        if (memory[i] == 0) {
            if (count == 0) {
                start = i;
            }
            count++;
        } else {
            count = 0;
            start = -1;
        }

        if (count == memorySize) {
            for (int j = start; j < start + memorySize; j++) {
                memory[j] = process.id; // Marcando a memória como alocada com o ID do processo
            }
            return true;
        }
    }

    return false; // Não há memória suficiente disponível
}

bool deallocateMemory(std::vector<int>& memory, Process& process) {
    int processID = process.id;
    bool status = false;

    for (int i = 0; i < memory.size(); i++) {
        if (memory[i] == processID) {
            memory[i] = 0;
            status = true;
        }
    }
    return status;
}

