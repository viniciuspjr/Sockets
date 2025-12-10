#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORTA 8080

SOCKET socketCliente;
bool conectado = true;

// Funcao que fica recebendo mensagens do servidor
void receberMensagens() {
    char buffer[1024];
    int bytesRecebidos;

    while (conectado) {
        // Limpa o buffer
        memset(buffer, 0, sizeof(buffer));

        // Recebe mensagem do servidor
        bytesRecebidos = recv(socketCliente, buffer, sizeof(buffer), 0);

        if (bytesRecebidos > 0) {
            cout << buffer;
        } else if (bytesRecebidos == 0) {
            cout << "\n[SISTEMA] Desconectado do servidor" << endl;
            conectado = false;
            break;
        } else {
            cout << "\n[ERRO] Erro ao receber mensagem" << endl;
            conectado = false;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    struct sockaddr_in enderecoServidor;
    string ip, mensagem;

    cout << "=== CLIENTE DE CHAT ===" << endl;

    // Inicializa o Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Erro ao inicializar Winsock" << endl;
        return 1;
    }

    // Cria o socket
    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == INVALID_SOCKET) {
        cout << "Erro ao criar socket" << endl;
        WSACleanup();
        return 1;
    }

    // Pergunta o IP do servidor
    cout << "Digite o IP do servidor (ou 127.0.0.1 para local): ";
    getline(cin, ip);

    // Configura o endereco do servidor
    enderecoServidor.sin_family = AF_INET;
    enderecoServidor.sin_port = htons(PORTA);
    inet_pton(AF_INET, ip.c_str(), &enderecoServidor.sin_addr);

    // Tenta conectar no servidor
    cout << "Conectando ao servidor..." << endl;
    if (connect(socketCliente, (struct sockaddr*)&enderecoServidor, sizeof(enderecoServidor)) == SOCKET_ERROR) {
        cout << "Erro ao conectar no servidor" << endl;
        closesocket(socketCliente);
        WSACleanup();
        return 1;
    }

    cout << "Conectado com sucesso!" << endl;
    cout << "Digite suas mensagens (ou 'sair' para desconectar):" << endl;
    cout << "-----------------------------------" << endl;

    // Inicia thread pra receber mensagens
    thread threadRecepcao(receberMensagens);
    threadRecepcao.detach();

    // Loop principal - envia mensagens
    while (conectado) {
        getline(cin, mensagem);

        // Verifica se quer sair
        if (mensagem == "sair") {
            conectado = false;
            break;
        }

        // Envia a mensagem pro servidor
        if (!mensagem.empty()) {
            mensagem += "\n";
            if (send(socketCliente, mensagem.c_str(), mensagem.length(), 0) == SOCKET_ERROR) {
                cout << "[ERRO] Falha ao enviar mensagem" << endl;
                conectado = false;
                break;
            }
        }
    }

    // Encerra a conexao
    cout << "Encerrando..." << endl;
    closesocket(socketCliente);
    WSACleanup();

    return 0;
}
