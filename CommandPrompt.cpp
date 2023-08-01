#include <iostream>
#include <string>
#include <fstream>
#include <cctype>
#include <sstream>
#include <vector>

bool verifyCommand(std::string message){
    std::istringstream iss(message); 
    std::vector<std::string> tokens; 
    std::string word;

    while (iss >> word) { 
        tokens.push_back(word); 
    }

    if(tokens[0] == "create"){
        if(tokens.size() != 3){
            std::cout << "Faltam argumentos, tente create -m 4.\n" << std::endl;
            return false;
        }
        if(tokens[1] != "-m"){
            std::cout << "Argumento não reconhecido, válido somente -m.\n" << std::endl;
            return false;
        }
        try {
            int num = std::stoi(tokens[2]);
            if(num > 20){
                std::cout << "Alocação máxima de 20 unidades de memória.\n" << std::endl;
                return false;
            }
        } 
        catch (const std::invalid_argument& ex) {
            std::cout << "Erro: A unidade de memória não representa um número válido.\n" << std::endl;
            return false;
        }
        return true;
    }
    if(tokens[0] == "kill"){
        if(tokens.size() != 2){
            std::cout << "Faltam argumentos, tente kill 2.\n" << std::endl;
            return false;
        }
        try {
            int num = std::stoi(tokens[1]);
        } 
        catch (const std::invalid_argument& ex) {
            std::cout << "Erro: O ID não representa um número válido.\n" << std::endl;
            return false;
        }
        return true;
    }

    if(tokens[0] == "exit") return true;

    std::cout << "Comando inválido.\n" << std::endl;
    return false;
}

int main() {
    std::string message;

    while (true) {
        std::cout << "Insira o comando desejado ('exit' para encerrar): ";
        std::getline(std::cin, message);

        //Converter comando para minusculo
        for (size_t i = 0; i < message.length(); ++i) {
            message[i] = std::tolower(message[i]);
        }

        std::ofstream pipeOut("buffer.txt", std::ios::out);

        if(verifyCommand(message)){
            pipeOut << message << std::endl;
            pipeOut.close();
            if (message == "exit") {
                break;
            }
        }
        pipeOut.close();
    }
    return 0;
}
