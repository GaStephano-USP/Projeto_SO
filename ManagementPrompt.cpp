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

struct BitMap{
    bool compacted;
    std::vector<int> memory;

    BitMap(bool _compacted, std::vector<int> _memory){
        compacted = _compacted;
        memory = _memory;
    }
};

Process chooseProcess(int memorySize){
    std::random_device rd;
    std::mt19937 gerador(rd());
    std::uniform_int_distribution<int> distribuicao(0, AvailableProcess.size() - 1);
    std::string randomProcess = AvailableProcess[distribuicao(gerador)];

    Process process(PROCESS_ID++, memorySize, 0, false, "READY", readFile(randomProcess));
    return process;
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

void compactMemory(BitMap& bitmap){

    int index = 0;

    for (int i = 0; i < bitmap.memory.size(); i++) {
        if (bitmap.memory[i] > 0) {
            bitmap.memory[index] = bitmap.memory[i];
            index++;
        }
    }

    // Preenche os espaços restantes com um valor especial (nesse exemplo, -1)
    while (index < bitmap.memory.size()) {
        bitmap.memory[index] = 0;
        index++;
    }
    bitmap.compacted = true;
};

bool allocateMemory(BitMap& bitmap, Process process) {
    int memorySize = process.memorySize;
    int start = -1;
    int count = 0;
    // Percorre a memória em busca de posições livres
    for (int i = 0; i < bitmap.memory.size(); i++) {
        if (bitmap.memory[i] == 0) {
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
                bitmap.memory[j] = process.id; // Marcando a memória como alocada com o ID do processo
            }
            return true;
        }
    }
    if(!bitmap.compacted) {
        compactMemory(bitmap);
        if(allocateMemory(bitmap, process)) return true;
    };

    return false; // Não há memória suficiente disponível
};

bool deallocateMemory(BitMap& bitmap, int processID) {
    bool status = false;

    for (int i = 0; i < bitmap.memory.size(); i++) {
        if (bitmap.memory[i] == processID) {
            bitmap.memory[i] = 0;
            status = true;
        }
    }
    return status;
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

void createProcess(std::string message, std::vector<int>& memory, std::queue<std::string>& readyQueue, std::vector<Process>& createdProcess, BitMap& bitmap){
    std::vector<std::string> elements = separateString(message);
    // instrução no formato create -m 4, element[2] = 4
    int memorySize = std::stoi(elements[2]);
    Process process = chooseProcess(memorySize);
    
    //alocar memória
    if(allocateMemory(bitmap, process)) process.isAlocated = true;

    createdProcess.push_back(process);

    //Colocar processo na fila de prontos
    readyQueue.push("PID " + std::to_string(process.id));

};

void killProcess(std::string message, BitMap& bitmap, std::queue<std::string>& readyQueue, std::vector<Process>& createdProcess){
    std::queue<std::string> copy = readyQueue;

    std::string PID = separateString(message)[1];
    
    //Verifica process na fila de prontos
    while (!copy.empty()) {
        std::string element = copy.front(); 
        //buscando mensagem no formato PID 9
         if(element.find("PID") != std::string::npos){      
            if(PID == (separateString(element)[1])){
                 for (int i = 0; i < createdProcess.size(); i++){
                    if(createdProcess[i].isAlocated){
                        if(!deallocateMemory(bitmap, std::stoi(PID))) std::cout << "ERRO AO LIBERAR MEMÓRIA";
                    }
                    else createdProcess.erase(createdProcess.begin() + i);
                 }
                break;
            }
         }
        copy.pop();
        //Falta implementar como apagar da fila de prontos
    }

};

void executeProcess(std::string PID, Process& process, std::vector<Process> createdProcess){
    std::vector<std::string> elements = separateString(PID);
    int id = std::stoi(elements[1]);
    for(int i = 0; i < createdProcess.size(); i++){
        if(createdProcess[i].id == std::stoi(elements[1])){
            createdProcess[i].state = "EXECUTING";
            process = createdProcess[i];
            break;
        }
    }

};

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
}

int main(){
    std::string message;
    std::queue<std::string> readyQueue;
    std::vector<Process> createdProcess;
    int schedulerKind;
    int dispatcherClock;
    int disptacherCounter;
    int printed = 0;
    Process execProcess(0, 0, 0, false, "EMPTY",{});
    BitMap bitmap(true, memory);
    //procesar arquivo com confugurações iniciais - IMPLEMENTAR
    operateSystemConfig(schedulerKind, dispatcherClock);
    //processar comando enviado
    while(1){
        //caso não se tenha mensagens
        if(!receiveComand(message)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2500));
            std::cout << "Esperando por instrução \n";
        }
        else{
            if(message.find("exit") != std::string::npos) break;
            if(message.find("create") != std::string::npos){
                std::cout << "Instrucao recebida\n";
                if(execProcess.state=="EMPTY"){ 
                    createProcess(message, memory, readyQueue, createdProcess, bitmap);
                    executeProcess(readyQueue.front(), execProcess, createdProcess);
                    std::cout << message;
                    //readyQueue.pop();
                }
                else readyQueue.push(message);
            }
            else{
                if(execProcess.state=="EMPTY") killProcess(message, bitmap, readyQueue, createdProcess);
                else readyQueue.push(message);
                std::cout << "mata esse cara";
            }
            //Printar na tela - IMPLEMENTAR (verificar se aqui é o melhor lugar para isso)
        }
        if (schedulerKind == 0) {
            std::cout << "FIFO Algoritmo \n";
        } else {
            std::cout << "Round Robin Algortimo \n";
        }
            std::cout << dispatcherClock << "\n";
        if (printed == 0) {
            std::cout << "PID" << execProgicess.id << "Memoria" << execProcess.memorySize << "\n";
            printed = 1;
        }

        //implementar lógica de escalonamento de processos

        //FIFO - IMPLEMENTAR

        //Round-Robin - IMPLEMENTAR 

    }
    return 0;
}