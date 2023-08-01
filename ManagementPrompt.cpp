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
    std::string path = "./Process/" + filename;
    std::cout << path << "\n";
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

struct Process {
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

struct TCB {
    int process_id;
    int programCounter;
    std::vector<std::string> registers;
};

struct BitMap {
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
        int mapRows = memory.size() / 4;
        std::cout << "+" << std::string(23, '-') << "+\n";
        std::cout << "|" << std::string(6, ' ') << "Mapa de bits" << std::string(5, ' ') << "|\n";
        std::cout << "+" << std::string(23, '-') << "+\n";
        for(int i = 0; i < mapRows; i++) {
            std::cout << "|  " << memory[0 + 4*i];
            std::cout << "  |  " << memory[1 + 4*i];
            std::cout << "  |  " << memory[2 + 4*i];
            std::cout << "  |  " << memory[3 + 4*i] << "  |\n";
            std::cout << "+" << std::string(23, '-') << "+\n";
        }
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

struct Scheduler {
    int schedulerType;
    int quantum;

    Scheduler(int _schedulerType, int _quantum) {
        schedulerType = _schedulerType;
        quantum = _quantum;
    };

    void setExecutingProcess(std::queue<Process>& readyQueue, Process& executingProcess) {
        executingProcess = readyQueue.front();
        executingProcess.state = "EXECUTING";
        readyQueue.pop();
    };

    void scheduleProcesses(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap) {
        if (readyQueue.size() != 0) {
            if (schedulerType == 1) {
                scheduleRoundRobin(executingProcess, readyQueue, bitmap);
            } else {
                scheduleFifo(executingProcess, readyQueue, bitmap);
            }
        }
    }

    void scheduleRoundRobin(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap) {
            Process nextInQueue = readyQueue.front();       
            if (executingProcess.state == "EMPTY") {
                setExecutingProcess(readyQueue, executingProcess);
            } else {
                executingProcess.state = "READY";
                readyQueue.push(executingProcess);
                setExecutingProcess(readyQueue, executingProcess);       
            }
    }

    void scheduleFifo(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap) {
            Process nextInQueue = readyQueue.front();       
            if (executingProcess.state == "EMPTY") {
                setExecutingProcess(readyQueue, executingProcess);
            }
    }
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

void operateSystemConfig(int& schedulerType, int& quantum) {
    std::ifstream inConfig("config.txt", std::ios::in);
    std::string line_config;
    std::vector<std::string> config_parameter;
    while (inConfig.peek() != std::ifstream::traits_type::eof()) {
        std::getline(inConfig, line_config);
        config_parameter = separateString(line_config);
        if (config_parameter[0] == "-schedulerType") {
            schedulerType = stoi(config_parameter[1]);
        } else {
            quantum = stoi(config_parameter[1]);
        }
    }
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
}


std::string getProcessInstructions(){
    std::random_device rd;
    std::mt19937 gerador(rd());
    std::uniform_int_distribution<int> distribuicao(0, AvailableProcess.size() - 1);
    std::string randomProcess = AvailableProcess[distribuicao(gerador)];
    
    return randomProcess;
}

void createUserProcess(Process soProcess, std::queue<Process>& readyQueue, BitMap& bitmap){
    std::string soCommand =  soProcess.instructions[0];
    // instrução no formato create -m 4,
    int memorySize = std::stoi(separateString(soCommand)[2]); 
    int freeSegment = bitmap.hasMemoryAvaliable(memorySize);

    // Cria processo se há memória suficiente
    if(freeSegment != -1) {
        int processID = PROCESS_ID++;
        std::string processHeader = "PID " + std::to_string(processID);
        int initPC = 0;
        std::string instructions = getProcessInstructions();
        
        Process userProcess(processID, processHeader, memorySize, initPC, true, "READY", "USER", readFile(instructions));
        bitmap.allocateMemory(userProcess, freeSegment);
        readyQueue.push(userProcess);
    } else {
        soProcess.state = "READY";
        readyQueue.push(soProcess);
    }

}

Process createSOProcess(std::string message) {
    int id = PROCESS_ID++;
    std::string header = separateString(message)[0];
    return Process(id, header, 0, 0, false, "READY", "SO", {message});
}


void killProcess(std::string message, std::queue<Process>& readyQueue, BitMap& bitmap) {
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
}

void executeOSInstruction(Process soProcess, std::queue<Process>& readyQueue, BitMap& bitmap) {
    std::string soCommand = soProcess.instructions[0];
    if (soProcess.header == "create"){
        createUserProcess(soProcess, readyQueue, bitmap);
    } else {
        killProcess(soCommand, readyQueue, bitmap);
    }
}

// Função responsável por executar um processo
void runProcess(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap, int& terminated) {
    if (executingProcess.scope == "USER") {
        int programCounter = executingProcess.programCounter;
        std::string instruction = executingProcess.instructions[programCounter];
        if (instruction.find("HLT") != std::string::npos) {
            terminated = 1;
        } else {
            executingProcess.programCounter++;
        }
    } else if (executingProcess.scope == "SO") {
        executeOSInstruction(executingProcess, readyQueue, bitmap);
        terminated = 1;
    } 
}

void terminateProcess(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap, int& terminated) {
    terminated = 0;
    bitmap.deallocateMemory(executingProcess.id);
    bitmap.compacted = 0;
    executingProcess = Process(0, "", 0, 0, false, "EMPTY", "UNDEF", {});
}


void printReadyQueue(std::queue<Process> readyQueue) {
 
    std::string limitLine = "+"+std::string(17,'-')+"+";
    std::string headerList = "| Fila de Prontos |";
    
    while(!readyQueue.empty()) {
        std::string header = readyQueue.front().header;
        limitLine += std::string(header.size()+2, '-');
        limitLine += '+';
        headerList += " ";
        headerList.append(header);
        headerList += " |";
        readyQueue.pop();
    }
    std::cout << limitLine << "\n";
    std::cout << headerList << "\n";
    std::cout << limitLine << "\n\n";

};

std::vector<std::string> getRegistersToSave(std::vector<std::string> instructions) {
    std::vector<std::string> registers;
    return registers;
}


void printExecutingProcess(Process executingProcess) {
    std::cout << "Executando: " << executingProcess.id << "  " << executingProcess.state;
    std::cout << " PC: " << executingProcess.programCounter;
    std::cout << std::endl;
    for(int i = 0; i < executingProcess.instructions.size(); i++) {
        std::string instruction = executingProcess.instructions[i];
        std::cout << instruction;
        if (i == executingProcess.programCounter - 1) {
            std::cout << " <--";
        }
        std::cout << "\n";
    }
    std::cout << executingProcess.instructions.size() << "\n";
};

int main(){
    // Configuração inicial
    int schedulerType;
    int quantum;
    operateSystemConfig(schedulerType, quantum);

    // "Estruturas" vistas pelo SO 
    std::queue<Process> readyQueue;
    BitMap bitmap(true, memory);
    Scheduler scheduler(schedulerType, quantum);
    Process executingProcess(0, "", 0, 0, false, "EMPTY", "UNDEF", {});

    // Variáveis de apoio
    std::string message;
    int execCounter = 0;
    int processEnd = 0;
    
    // EXECUTA SO
    while(1) {

        // BLOCO DE COMANDOS RECEBIDOS
        if(!receiveComand(message)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        } else {
            if(message.find("exit") != std::string::npos) break;
            
            Process newProcess = createSOProcess(message); // create or kill
            if (executingProcess.state == "EMPTY") {
                executeOSInstruction(newProcess, readyQueue, bitmap);
                scheduler.scheduleProcesses(executingProcess, readyQueue, bitmap);
            } else {
                readyQueue.push(newProcess);
            }
        }

        // BLOCO DE EXECUÇÃO
        if (executingProcess.state != "EMPTY") {
            if (execCounter != scheduler.quantum) {
                runProcess(executingProcess, readyQueue, bitmap, processEnd);
                execCounter++;
            } else {
                scheduler.scheduleProcesses(executingProcess, readyQueue, bitmap);
                execCounter = 0;
            }
            if (processEnd == 1) {
                terminateProcess(executingProcess, readyQueue, bitmap, processEnd);
                scheduler.scheduleProcesses(executingProcess, readyQueue, bitmap);
                execCounter = 0;
            }
        }

        // Imprime detalhes

        printReadyQueue(readyQueue);
        printExecutingProcess(executingProcess);
        bitmap.printMemoryMap();
        std::cout << "\n\n";

    }
    return 0;
}