#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORTA 8080
#define MAX_CLIENTES 10

// Lista pra guardar os clientes conectados
vector<SOCKET> clientes;

// Funcao pra mandar mensagem pra todos os clientes
void broadcast(string mensagem, SOCKET remetente) {
    for (int i = 0; i < clientes.size(); i++) {
        // Nao manda pra quem enviou
        if (clientes[i] != remetente) {
            send(clientes[i], mensagem.c_str(), mensagem.length(), 0);
        }
    }
}

// Funcao que fica escutando mensagens de um cliente
void escutarCliente(SOCKET socketCliente) {
    char buffer[1024];
    int bytesRecebidos;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytesRecebidos = recv(socketCliente, buffer, sizeof(buffer), 0);

        if (bytesRecebidos > 0) {
            // Recebeu mensagem, mostra no servidor e envia pros outros
            cout << "Mensagem recebida: " << buffer;
            broadcast(string(buffer), socketCliente);
        } else {
            // Cliente desconectou
            cout << "Cliente desconectado" << endl;
            
            // Remove da lista
            for (int i = 0; i < clientes.size(); i++) {
                if (clientes[i] == socketCliente) {
                    clientes.erase(clientes.begin() + i);
                    break;
                }
            }
            
            closesocket(socketCliente);
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET servidor, novoCliente;
    struct sockaddr_in enderecoServidor, enderecoCliente;
    int tamanhoEndereco = sizeof(enderecoCliente);
    char buffer[1024];

    cout << "=== SERVIDOR DE CHAT ===" << endl;

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Erro ao inicializar Winsock" << endl;
        return 1;
    }

    // Cria o socket do servidor
    servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (servidor == INVALID_SOCKET) {
        cout << "Erro ao criar socket" << endl;
        WSACleanup();
        return 1;
    }

    // Configura o endereco do servidor
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_addr.s_addr = INADDR_ANY; // Aceita conexao de qualquer IP
    enderecoServidor.sin_port = htons(PORTA);

    // Associa o socket ao endereco e porta
    if (bind(servidor, (struct sockaddr*)&enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR) {
        cout << "Erro no bind" << endl;
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    // Coloca o servidor pra escutar conexoes
    if (listen(servidor, MAX_CLIENTES) == SOCKET_ERROR) {
        cout << "Erro ao escutar" << endl;
        closesocket(servidor);
        WSACleanup();
        return 1;
    }

    cout << "Servidor rodando na porta " << PORTA << endl;
    cout << "Aguardando conexoes..." << endl;

    // Loop principal do servidor
    while (true) {
        // Aceita nova conexao
        novoCliente = accept(servidor, (struct sockaddr*)&enderecoCliente, &tamanhoEndereco);
        if (novoCliente == INVALID_SOCKET) {
            cout << "Erro ao aceitar conexao" << endl;
            continue;
        }

        // Adiciona o cliente na lista
        clientes.push_back(novoCliente);
        cout << "Novo cliente conectado! Total de clientes: " << clientes.size() << endl;

        // Avisa os outros que alguem entrou
        string aviso = "[SERVIDOR] Novo usuario entrou no chat!\n";
        broadcast(aviso, novoCliente);

        // Cria uma thread pra escutar esse cliente
        thread threadCliente(escutarCliente, novoCliente);
        threadCliente.detach(); // Thread roda independente
    }

    // Fecha tudo
    for (int i = 0; i < clientes.size(); i++) {
        closesocket(clientes[i]);
    }
    closesocket(servidor);
    WSACleanup();

    return 0;
}
