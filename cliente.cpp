// Bibliotecas necessarias
#include <iostream>        // Para entrada/saida (cout, cin)
#include <winsock2.h>      // Biblioteca principal de sockets do Windows
#include <ws2tcpip.h>      // Funcoes TCP/IP adicionais
#include <string>          // Para usar strings
#include <thread>          // Para criar threads (processos paralelos)

// Linka a biblioteca de sockets do Windows automaticamente
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Porta do servidor (tem que ser a mesma que o servidor usa)
#define PORTA 8080

// Variaveis globais (acessadas pela thread e main)
SOCKET socketCliente;      // Socket da conexao com o servidor
bool conectado = true;     // Flag pra controlar se ainda ta conectado

// Funcao que fica recebendo mensagens do servidor
// Essa funcao roda em uma thread separada, em paralelo com a main
// Assim o cliente consegue receber e enviar mensagens ao mesmo tempo
void receberMensagens() {
    char buffer[1024];      // Buffer pra armazenar mensagens recebidas
    int bytesRecebidos;     // Quantidade de bytes recebidos

    // Loop enquanto estiver conectado
    while (conectado) {
        // Limpa o buffer antes de receber nova mensagem
        memset(buffer, 0, sizeof(buffer));

        // Tenta receber mensagem do servidor
        // recv() bloqueia ate receber algo ou conexao cair
        bytesRecebidos = recv(socketCliente, buffer, sizeof(buffer), 0);

        if (bytesRecebidos > 0) {
            // Recebeu mensagem com sucesso - mostra na tela
            cout << buffer;
        } else if (bytesRecebidos == 0) {
            // bytesRecebidos == 0 significa que o servidor fechou a conexao
            cout << "\n[SISTEMA] Desconectado do servidor" << endl;
            conectado = false;
            break;
        } else {
            // bytesRecebidos < 0 significa erro na recepcao
            cout << "\n[ERRO] Erro ao receber mensagem" << endl;
            conectado = false;
            break;
        }
    }
}

int main() {
    // Variaveis principais
    WSADATA wsaData;                    // Estrutura com info do Winsock
    struct sockaddr_in enderecoServidor; // Endereco do servidor
    string ip, mensagem;                 // IP digitado e mensagens

    cout << "=== CLIENTE DE CHAT ===" << endl;

    // Passo 1: Inicializa a biblioteca Winsock (obrigatorio no Windows)
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Erro ao inicializar Winsock" << endl;
        return 1;
    }

    // Passo 2: Cria o socket do cliente
    // AF_INET = IPv4, SOCK_STREAM = TCP
    socketCliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socketCliente == INVALID_SOCKET) {
        cout << "Erro ao criar socket" << endl;
        WSACleanup();
        return 1;
    }

    // Passo 3: Pergunta o IP do servidor pro usuario
    cout << "Digite o IP do servidor (ou 127.0.0.1 para local): ";
    getline(cin, ip);

    // Passo 4: Configura o endereco do servidor que vai conectar
    enderecoServidor.sin_family = AF_INET;              // Familia IPv4
    enderecoServidor.sin_port = htons(PORTA);           // Porta do servidor
    inet_pton(AF_INET, ip.c_str(), &enderecoServidor.sin_addr);  // Converte IP de texto pra formato binario

    // Passo 5: Tenta conectar no servidor
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

    // Passo 6: Inicia thread pra receber mensagens do servidor
    // Assim conseguimos receber e enviar ao mesmo tempo
    thread threadRecepcao(receberMensagens);
    threadRecepcao.detach();  // Thread roda independente

    // Passo 7: Loop principal - fica enviando mensagens digitadas pelo usuario
    while (conectado) {
        // Le uma linha de texto do usuario
        getline(cin, mensagem);

        // Verifica se o usuario quer sair
        if (mensagem == "sair") {
            conectado = false;
            break;
        }

        // Envia a mensagem pro servidor (se nao estiver vazia)
        if (!mensagem.empty()) {
            mensagem += "\n";  // Adiciona quebra de linha
            
            // send() envia os dados pro servidor
            if (send(socketCliente, mensagem.c_str(), mensagem.length(), 0) == SOCKET_ERROR) {
                cout << "[ERRO] Falha ao enviar mensagem" << endl;
                conectado = false;
                break;
            }
        }
    }

    // Passo 8: Encerra a conexao e limpa recursos
    cout << "Encerrando..." << endl;
    closesocket(socketCliente);  // Fecha o socket
    WSACleanup();                // Limpa o Winsock

    return 0;
}
