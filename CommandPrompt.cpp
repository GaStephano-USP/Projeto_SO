#include <iostream>
#include <string>
#include <fstream>
#include <cctype>

int main() {
    std::string message;

    while (true) {
        std::cout << "Insira o comando desejado ('exit' para encerrar): ";
        std::getline(std::cin, message);

        //Converter comando para minusculo
        for (size_t i = 0; i < message.length(); ++i) {
            message[i] = std::tolower(message[i]);
        }

        std::ofstream pipeOut("buffer", std::ios::out);
        pipeOut << message << std::endl;
        pipeOut.close();
        if (message == "exit") {
            break;
        }
    }

    return 0;
}
