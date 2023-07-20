#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <sstream>

const int TOTAL_MEMORY_SIZE = 20;

std::vector<int> memory(TOTAL_MEMORY_SIZE - 1);

struct Process{
    int id;
    int memorySize;
    int programCounter;
    std::string state;
    std::vector<std::string> instructions;

    Process(int _id, int _memorySize, int _programCounter, std::string _state, const std::vector<std::string>& _instructions){
        id = _id;
        memorySize = _memorySize;
        programCounter = _programCounter;
        state = _state;
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

bool receiveComand(std::string& message){
    std::ifstream pipeIn("buffer", std::ios::in);

    // Verifica se há algo no pipe antes de tentar ler a mensagem
    if (pipeIn.peek() != std::ifstream::traits_type::eof()) {
        std::getline(pipeIn, message);
        std::cout << "Mensagem recebida: " << message << std::endl;

        // Limpa o conteúdo do pipe após a leitura, abrindo-o em modo de escrita
        std::ofstream pipeOut("buffer", std::ios::out | std::ios::trunc);
        pipeOut.close();
        pipeIn.close();
        return true;
    }
    pipeIn.close();
    return false;
}

void compactBitMap(std::vector<int>& memory){

    std::vector<int> compacted(memory.size());
    for (size_t i = 0; i < memory.size(); ++i) {
        if(memory[i] > 0) compacted[i] = memory[i];
    }
    std::copy(compacted.begin(), compacted.end(), memory.begin());
}

void createProcess(std::string message, std::vector<int>& memory){

}

void killProcess(std::string message, std::vector<int>& memory){

}

//Verificar necessidade
void processCommand(std::string command){
    std::istringstream iss(command);
    std::vector<std::string> elements;
    std::string token;

    while (iss >> token) {
        elements.push_back(token);
    }

    


}

int main(){
    std::string message;
    //procesar arquivo com confugurações iniciais

    //processar comando enviado
    while(1){
        //caso não se tenha mensagens
        if(!receiveComand(message)) std::this_thread::sleep_for(std::chrono::milliseconds(500));
        else{
            if(message.find('exit') != std::string::npos) break;
            if(message.find('create') != std::string::npos){
                createProcess(message, memory);
            }
            else{
                killProcess(message, memory);
            }
            //Printar na tela
        }
        //implementar lógica de escalonamento

        //FIFO

        //Round-Robin

    }
    return 0;
}