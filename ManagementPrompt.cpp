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

std::vector<int> memory(TOTAL_MEMORY_SIZE);

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
    return {};
};

struct Process{
    int id;
    std::string header;
    int memorySize;
    int programCounter;
    bool isAlocated;
    std::string state;
    std::string scope;
    std::vector<std::string> instructions;

    Process(
        int _id, 
        std::string _header,
        int _memorySize,
        int _programCounter,
        bool _isAlocated,
        std::string _state,
        std::string _scope,
        const std::vector<std::string>& _instructions
        ) {
        id = _id;
        header = _header;
        memorySize = _memorySize;
        programCounter = _programCounter;
        isAlocated = _isAlocated;
        state = _state;
        scope = _scope;
        instructions = _instructions;
    };
};



struct TCB{
    int process_id;
    int programCounter;
    std::vector<std::string> registers;
};

struct BitMap{
    bool compacted;
    std::vector<int> memory;

    BitMap(bool _compacted, std::vector<int> _memory){
        compacted = _compacted;
        memory = _memory;
    }

    void allocateMemory(Process process, int startPos) {
        int memorySize = process.memorySize;
        std::cout << "Alocando " << memorySize << " a partir de" << startPos << "Para ID: "<< process.id <<"\n";
        for (int j = startPos; j < startPos + memorySize; j++) {
            memory[j] = process.id; // Marcando a memória como alocada com o ID do processo
        }

    };

    bool deallocateMemory(int processID) {
        bool status = false;

        for (int i = 0; i < memory.size(); i++) {
            if (memory[i] == processID) {
                memory[i] = 0;
                status = true;
            }
        }
        return status;
    };

    void printMemoryMap() {
        for(int m: memory) {
            std::cout << " | "<< m;
        }
        std::cout << " | " << std::endl;
    };

    void compactMemory(){
        int index = 0;

        for (int i = 0; i < memory.size(); i++) {
            if (memory[i] > 0) {
                memory[index] = memory[i];
                index++;
            }
        }
        // Preenche os espaços restantes com um valor especial (nesse exemplo, -1)
        while (index < memory.size()) {
            memory[index] = 0;
            index++;
        }
        compacted = true;
    };

    // retorna -1 se não tem memória, ou a primeira posicao do segmento livre
    int hasMemoryAvaliable(int memoryRequested){
        int start = 0;
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
            if (count >= memoryRequested) {
                return start;
            }
        }
        
        // Remove fragmentação externa e tenta novamente
        if(!compacted) {
            compactMemory();
            return hasMemoryAvaliable(memoryRequested);
        };

        return -1;
    };
};


bool receiveComand(std::string& message){
    std::ifstream pipeIn("buffer.txt", std::ios::in);

    // Verifica se há algo no pipe antes de tentar ler a mensagem
    if (pipeIn.peek() != std::ifstream::traits_type::eof()) {
        std::getline(pipeIn, message);
 
        // Limpa o conteúdo do pipe após a leitura, abrindo-o em modo de escrita
        std::ofstream pipeOut("buffer.txt", std::ios::out | std::ios::trunc);
        pipeOut.close();
        pipeIn.close();
        return true;
    }
    pipeIn.close();
    return false;
};

std::vector<std::string> separateString(std::string message){
    std::istringstream iss(message);
    std::vector<std::string> elements;
    std::string token;
    while (iss >> token) {
        elements.push_back(token);
    }
    return elements;
}

std::string getProcessInstructions(){
    std::random_device rd;
    std::mt19937 gerador(rd());
    std::uniform_int_distribution<int> distribuicao(0, AvailableProcess.size() - 1);
    std::string randomProcess = AvailableProcess[distribuicao(gerador)];
    
    return randomProcess;
};

void createProcess(std::string message, std::queue<Process>& readyQueue, BitMap& bitmap){
    std::vector<std::string> elements = separateString(message);
    int processID = PROCESS_ID++;
    // instrução no formato create -m 4, element[2] = 4
    int memorySize = std::stoi(elements[2]);
    std::string processHeader;
    int initPC = 0;
    std::string instructions = getProcessInstructions();
    int freeSegment = bitmap.hasMemoryAvaliable(memorySize);

    // Cria processo se há memória suficiente
    if(freeSegment != -1) {
        processHeader = "PID " + std::to_string(processID);
        Process process(processID, processHeader, memorySize, initPC, true, "READY", "USER", readFile(instructions));
        bitmap.allocateMemory(process, freeSegment);
        readyQueue.push(process);

    } else {
        processHeader = "create";
        Process process(processID, processHeader, memorySize, initPC, false, "READY", "SO", {message});
        //Colocar processo na fila de prontos
        readyQueue.push(process);
    }


};

void killProcess(std::string message, std::queue<Process>& readyQueue, BitMap& bitmap){
    std::queue<Process> copyQueue = readyQueue;
    std::queue<Process> newQueue;

    int targetPID = std::stoi((separateString(message)[1]));
    
    // Verifica process a ser eliminado na fila de prontos
    while (!copyQueue.empty()) {
        Process process = copyQueue.front(); 
        if(targetPID == process.id) {
            if(process.isAlocated) {
                if(!bitmap.deallocateMemory(targetPID)) std::cout << "ERRO AO LIBERAR MEMÓRIA \n";
            }
        } else {
            newQueue.push(process);
        }
        copyQueue.pop();
    }
    readyQueue = newQueue;

};

void setExecutingProcess(std::queue<Process>& readyQueue, Process& executingProcess) {
    executingProcess = readyQueue.front();
    executingProcess.state = "EXECUTING";
    readyQueue.pop();
};

void scheduleProcesses(Process& executingProcess, std::queue<Process>& readyQueue, int schedulerType, BitMap& bitmap) {
    if(schedulerType == 1 && readyQueue.size() != 0) {
        Process nextInQueue = readyQueue.front();
        executingProcess.state = "READY";
        readyQueue.push(executingProcess);
        executingProcess = nextInQueue;
        executingProcess.state = "EXECUTING";
        readyQueue.pop();
    } 
}

// Função responsável por executar um processo
void runProcess(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap) {
    if (executingProcess.scope == "USER") {
        executingProcess.programCounter++;
    } else if (executingProcess.scope == "SO") {
        std::string message = executingProcess.instructions[0];
        if (message.find("create") != std::string::npos){
            createProcess(message, readyQueue, bitmap);
        } else {
            killProcess(message, readyQueue, bitmap);
        }
        executingProcess = readyQueue.front();
        readyQueue.pop();
    } 
    else return;
}

void operateSystemConfig(int& schedulerKind, int& dispatcherClock) {
    std::ifstream inConfig("config.txt", std::ios::in);
    std::string line_config;
    std::vector<std::string> config_parameter;
    while (inConfig.peek() != std::ifstream::traits_type::eof()) {
        std::getline(inConfig, line_config);
        config_parameter = separateString(line_config);
        if (config_parameter[0] == "-schedulerKind") {
            schedulerKind = stoi(config_parameter[1]);
        } else {
            dispatcherClock = stoi(config_parameter[1]);
        }
    }
};

void printReadyQueue(std::queue<Process> readyQueue) {
    std::cout << "Fila de Prontos - ";
    while(!readyQueue.empty()) {
        std::cout << readyQueue.front().header << "(" << readyQueue.front().state << ")" << " ";
        readyQueue.pop();
    }
    std::cout << std::endl;

};

void printExecutingProcess(Process executingProcess) {
    std::cout << "Executando: " << executingProcess.id << "  " << executingProcess.state;
    std::cout << " PC: " << executingProcess.programCounter;
    std::cout << std::endl;
};

int main(){
    int schedulerKind;
    int dispatcherClock;
    int execCounter = 0;
    std::string message;
    std::queue<Process> readyQueue;
    Process executingProcess(0, "", 0, 0, false, "EMPTY", "UNDEF",{});
    BitMap bitmap(true, memory);

    //procesa arquivo com confugurações iniciais
    operateSystemConfig(schedulerKind, dispatcherClock);
    
    // EXECUTA SO
    while(1) {

        // BLOCO DE COMANDOS RECEBIDOS
        if(!receiveComand(message)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        } else {
            if(message.find("exit") != std::string::npos) break;
            if(message.find("create") != std::string::npos) {
                if(executingProcess.state == "EMPTY") { 
                    createProcess(message, readyQueue, bitmap);
                    setExecutingProcess(readyQueue, executingProcess);
                    //rdQueChange.pop(); // provavelmente não será mais usado
                } else {
                    int processID = PROCESS_ID++;
                    std::string processHeader = "create";
                    Process process(processID, processHeader, 0, 0, false, "READY", "SO", {message});
                    readyQueue.push(process); 
                }
            }
            if (message.find("kill") != std::string::npos) {
                if(executingProcess.state=="EMPTY") {
                    std::cout << "Não há processos em execução";
                } else {
                    int processID = PROCESS_ID++;
                    std::string processHeader = "kill";
                    Process process(processID, processHeader, 0, 0, false, "READY", "SO", {message});
                    readyQueue.push(process);
                    std::cout << "Mata esse cara \n";
                }
            }
            //Printar na tela - IMPLEMENTAR (verificar se aqui é o melhor lugar para isso)
        }

        // BLOCO DE EXECUÇÃO
        if (executingProcess.state != "EMPTY") {
            if (execCounter != dispatcherClock) {
                runProcess(executingProcess, readyQueue, bitmap);
                execCounter++;
            } else {
                std::cout << "Scheduler Called \n";
                scheduleProcesses(executingProcess, readyQueue, schedulerKind, bitmap);
                execCounter = 0;
            }
        }
        std::cout << std::string(25, '-') << "\n";

        printReadyQueue(readyQueue);
        printExecutingProcess(executingProcess);
        bitmap.printMemoryMap();
        std::cout << std::string(25, '-') << "\n";
        std::cout << "\n\n";
        //implementar lógica de escalonamento de processos

        //FIFO - IMPLEMENTAR

        //Round-Robin - IMPLEMENTAR 

    }
    return 0;
}