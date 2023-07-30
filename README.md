# Projeto_PCS3746
Projeto da Disciplina PCS3746 - Sistemas Operacionais, simulador de gerenciamento de processos e alocação de memória de um sistema operacional

## Funcionamento da main()

Basicamente define a configuração inicial, as estruturas do SO e variáveis auxiliares.

Executa infinitamente - while (1) - três "blocos":

- Leitura de comandos
- Execução e escalonamento de processos
- Impressão

## Configurações

- schedulerType -> 0-FIFO, 1-RR
- quantum -> int

## Structs

### BitMap
Responsável pelo mapa de bits e funções que o manipulam

- allocateMemory(Process process, int startPos) - aloca memória a partir de uma posição.
- deallocateMemory(int processID) - desaloca memória a  de de um processo
- compactMemory() - elimina fragmentação externa
- hasMemoryAvaliable(int memoryRequested) - verifica se memória tem espaço solicitado, retorna primeira posição.

### Process
Responsável pelos atributos de um Processo

- header é para a impressão da fila de prontos
- scope é para saber se o processo é de usuário ou SO

### Scheduler
Responsável pelos atributos e funções do scheduler

- setExecutingProcess(std::queue<Process>& readyQueue, Process& executingProcess) - coloca o próximo processo da fila em execução;

- void scheduleProcesses(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap) - faz o escalonamento de acordo com o tipo

### TCB (implementar)
Responsável pelos atributos do processo usados pelo dispatcher

## Funções

- receiveComand(std::string& message) - lê comando do arquivo
- separateString(std::string message) - cria vetor com substrings do comando lido
- getProcessInstructions() - pega instruções do processo (escolhe aleatoriamente)

- createSOProcess(std::string message) - cria um processo de sistema

- createUserProcess(Process soProcess, std::queue<Process>& readyQueue, BitMap& bitmap) - cria um processo de usuário a partir de um processo de sistema (ou aloca o processo de sistema recebido no fim da fila)

- void killProcess(std::string message, std::queue<Process>& readyQueue, BitMap& bitmap) - mata o processo e retorna fila sem ele

- runProcess(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap, int& terminated) - "Executa" um processo linha a linha

- executeOSInstruction(Process soProcess, std::queue<Process>& readyQueue, BitMap& bitmap) - Se o processo for de sistema a "linha" é executada aqui.

- terminateProcess(Process& executingProcess, std::queue<Process>& readyQueue, BitMap& bitmap, int& terminated) - Encerra processo 

- operateSystemConfig(int& schedulerKind, int& dispatcherClock) - configura o scheduler

