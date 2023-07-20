#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>
#include <random>
#include <queue>

const std::vector<std::string> AvailableProcess{"Process_A.txt", "Process_B.txt", "Process_C.txt"};

const int TOTAL_MEMORY_SIZE = 20;

int PROCESS_ID = 1;

std::vector<int> memory(TOTAL_MEMORY_SIZE - 1);

std::vector<std::string> readFile(std::string filename){
    std::string path = "Process\\"+ filename;

    std::vector<std::string> lines;

    std::ifstream file(path);

    if (file.is_open()) {
        std::string line;

        while (std::getline(file, line)) {
            lines.push_back(line);
        };

        file.close();
        return lines;

    }
};

struct Process{
    int id;
    int memorySize;
    int programCounter;
    bool isAlocated;
    std::string state;
    std::vector<std::string> instructions;

    Process(int _id, int _memorySize, int _programCounter, bool _isAlocated, std::string _state, const std::vector<std::string>& _instructions){
        id = _id;
        memorySize = _memorySize;
        programCounter = _programCounter;
        isAlocated = _isAlocated;
        state = _state;
        instructions = _instructions;
    };
};

struct TCB{
    int process_id;
    int programCounter;
    std::vector<std::string> registers;
};

Process chooseProcess(int memorySize){
    std::random_device rd;
    std::mt19937 gerador(rd());
    std::uniform_int_distribution<int> distribuicao(0, AvailableProcess.size() - 1);
    std::string randomProcess = AvailableProcess[distribuicao(gerador)];

    Process process(PROCESS_ID++, memorySize, 0, false, "READY", readFile(randomProcess));
    return process;
};

bool allocateMemory(std::vector<int>& memory, Process process) {
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
};

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
};

bool receiveComand(std::string& message){
    std::ifstream pipeIn("buffer", std::ios::in);

    // Verifica se há algo no pipe antes de tentar ler a mensagem
    if (pipeIn.peek() != std::ifstream::traits_type::eof()) {
        std::getline(pipeIn, message);
 
        // Limpa o conteúdo do pipe após a leitura, abrindo-o em modo de escrita
        std::ofstream pipeOut("buffer", std::ios::out | std::ios::trunc);
        pipeOut.close();
        pipeIn.close();
        return true;
    }
    pipeIn.close();
    return false;
};

void compactBitMap(std::vector<int>& memory){

    std::vector<int> compacted(memory.size());
    for (size_t i = 0; i < memory.size(); ++i) {
        if(memory[i] > 0) compacted[i] = memory[i];
    }
    std::copy(compacted.begin(), compacted.end(), memory.begin());
};

void createProcess(std::string message, std::vector<int>& memory, std::queue<std::string>& readyQueue){
    //seleciona um processo aleatório da pasta de processos
    std::istringstream iss(message);
    std::vector<std::string> elements;
    std::string token;

    while (iss >> token) {
        elements.push_back(token);
    }
    // instrução no formato create -m 4, element[2] = 4
    int memorySize = std::stoi(elements[2]);
    Process process = chooseProcess(memorySize);
    
    //alocar memória
    if(allocateMemory(memory, process)) process.isAlocated = true;

    //Colocar processo na fila de prontos
    readyQueue.push("PID " + std::to_string(process.id));

};

void killProcess(std::string message, std::vector<int>& memory){

};

int main(){
    std::string message;
    std::queue<std::string> readyQueue;
    //procesar arquivo com confugurações iniciais

    //processar comando enviado
    while(1){
        //caso não se tenha mensagens
        if(!receiveComand(message)) std::this_thread::sleep_for(std::chrono::milliseconds(500));
        else{
            if(message.find("exit") != std::string::npos) break;
            if(message.find("create") != std::string::npos){
                createProcess(message, memory, readyQueue);
            }
            else{
                killProcess(message, memory);
            }
            //Printar na tela
        };
        //implementar lógica de escalonamento de processos

        //FIFO

        //Round-Robin

    }
    return 0;
}