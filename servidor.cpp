// Bibliotecas necessarias
#include <iostream>        // Para entrada/saida (cout, cin)
#include <winsock2.h>      // Biblioteca principal de sockets do Windows
#include <ws2tcpip.h>      // Funcoes TCP/IP adicionais
#include <vector>          // Para usar vector (lista dinamica)
#include <string>          // Para usar strings
#include <thread>          // Para criar threads (processos paralelos)

// Linka a biblioteca de sockets do Windows automaticamente
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Define a porta que o servidor vai usar
#define PORTA 8080
// Numero maximo de clientes que podem se conectar
#define MAX_CLIENTES 10

// Lista pra guardar todos os sockets dos clientes conectados
// Cada cliente tem um SOCKET que e usado pra comunicacao
vector<SOCKET> clientes;

// Funcao pra mandar mensagem pra todos os clientes (broadcast)
// Parametros:
//   mensagem = texto a ser enviado
//   remetente = socket de quem mandou (pra nao reenviar pra ele mesmo)
void broadcast(string mensagem, SOCKET remetente) {
    // Percorre todos os clientes conectados
    for (int i = 0; i < clientes.size(); i++) {
        // Nao manda pra quem enviou a mensagem
        if (clientes[i] != remetente) {
            // Envia a mensagem pro cliente
            // .c_str() converte string C++ pra formato que send() entende
            send(clientes[i], mensagem.c_str(), mensagem.length(), 0);
        }
    }
}

// Funcao que fica escutando mensagens de um cliente especifico
// Essa funcao roda em uma thread separada pra cada cliente
// Parametro: socketCliente = o socket do cliente que vai ser escutado
void escutarCliente(SOCKET socketCliente) {
    char buffer[1024];      // Buffer pra armazenar a mensagem recebida
    int bytesRecebidos;     // Quantidade de bytes que foram recebidos

    // Loop infinito - fica escutando o cliente ate ele desconectar
    while (true) {
        // Limpa o buffer antes de receber nova mensagem
        memset(buffer, 0, sizeof(buffer));
        
        // Tenta receber mensagem do cliente
        // recv() bloqueia ate receber algo ou cliente desconectar
        bytesRecebidos = recv(socketCliente, buffer, sizeof(buffer), 0);

        if (bytesRecebidos > 0) {
            // Recebeu mensagem com sucesso!
            // Mostra no console do servidor
            cout << "Mensagem recebida: " << buffer;
            
            // Envia a mensagem pra todos os outros clientes
            broadcast(string(buffer), socketCliente);
        } else {
            // bytesRecebidos <= 0 significa que o cliente desconectou
            cout << "Cliente desconectado" << endl;
            
            // Procura e remove o cliente da lista
            for (int i = 0; i < clientes.size(); i++) {
                if (clientes[i] == socketCliente) {
                    clientes.erase(clientes.begin() + i);
                    break;
                }
            }
            
            // Fecha o socket do cliente
            closesocket(socketCliente);
            break;  // Sai do loop e encerra a thread
        }
    }
}

int main() {
    // Variaveis principais do servidor
    WSADATA wsaData;                    // Estrutura com info do Winsock
    SOCKET servidor, novoCliente;       // Socket do servidor e dos novos clientes
    struct sockaddr_in enderecoServidor, enderecoCliente;  // Enderecos de rede
    int tamanhoEndereco = sizeof(enderecoCliente);
    char buffer[1024];                  // Buffer auxiliar

    cout << "=== SERVIDOR DE CHAT ===" << endl;

    // Passo 1: Inicializa a biblioteca Winsock (obrigatorio no Windows)
    // MAKEWORD(2,2) = versao 2.2 do Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Erro ao inicializar Winsock" << endl;
        return 1;
    }

    // Passo 2: Cria o socket do servidor
    // AF_INET = IPv4, SOCK_STREAM = TCP (conexao confiavel)
    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        cout << "Erro ao criar socket" << endl;
        WSACleanup();  // Limpa o Winsock antes de sair
        return 1;
    }

    // Passo 3: Configura o endereco do servidor
    enderecoServidor.sin_family = AF_INET;              // Familia IPv4
    enderecoServidor.sin_addr.s_addr = INADDR_ANY;      // Aceita conexao de qualquer IP
    enderecoServidor.sin_port = htons(PORTA);           // Define a porta (htons converte pra network byte order)

    // Passo 4: Associa o socket ao endereco e porta (bind)
    // Isso "reserva" a porta 8080 pra esse servidor
    if (bind(servidor, (struct sockaddr*)&enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR) {
        cout << "Erro no bind" << endl;
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    // Passo 5: Coloca o servidor pra escutar conexoes (listen)
    // MAX_CLIENTES = fila de espera de conexoes pendentes
    if (listen(servidor, MAX_CLIENTES) == SOCKET_ERROR) {
        cout << "Erro ao escutar" << endl;
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    cout << "Servidor rodando na porta " << PORTA << endl;
    cout << "Aguardando conexoes..." << endl;

    // Passo 6: Loop principal do servidor - aceita conexoes infinitamente
    while (true) {
        // Aceita nova conexao (accept bloqueia ate alguem conectar)
        // Quando um cliente conecta, retorna um novo socket pra comunicacao
        novoCliente = accept(servidor, (struct sockaddr*)&enderecoCliente, &tamanhoEndereco);
        if (novoCliente == INVALID_SOCKET) {
            cout << "Erro ao aceitar conexao" << endl;
            continue;  // Tenta aceitar a proxima conexao
        }

        // Adiciona o novo cliente na lista de clientes conectados
        clientes.push_back(novoCliente);
        cout << "Novo cliente conectado! Total de clientes: " << clientes.size() << endl;

        // Manda um aviso pros outros clientes que alguem entrou
        string aviso = "[SERVIDOR] Novo usuario entrou no chat!\n";
        broadcast(aviso, novoCliente);

        // Cria uma thread (processo paralelo) pra ficar escutando esse cliente
        // Assim o servidor pode aceitar outros clientes ao mesmo tempo
        thread threadCliente(escutarCliente, novoCliente);
        threadCliente.detach(); // Thread roda independente (nao bloqueia)
    }

    // Fecha tudo
    for (int i = 0; i < clientes.size(); i++) {
        closesocket(clientes[i]);
    }
    closesocket(servidor);
    WSACleanup();

    return 0;
}
