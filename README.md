# Chat Cliente-Servidor com Sockets

Projeto simples de um chat usando arquitetura cliente-servidor em C++ com Winsock.

## O que é?

Um chat onde um servidor fica esperando conexões e vários clientes podem se conectar pra conversar. O servidor repassa as mensagens de um cliente pros outros.

## Arquivos

- `servidor.cpp` - Código do servidor que aceita conexões
- `cliente.cpp` - Código do cliente que conecta no servidor

## Como compilar

Você precisa de um compilador C++ no Windows (tipo MinGW ou Visual Studio).

### Compilar o servidor:
```
g++ servidor.cpp -o servidor.exe -lws2_32
```

### Compilar o cliente:
```
g++ cliente.cpp -o cliente.exe -lws2_32
```

## Como usar

### 1. Rode o servidor primeiro:
```
.\servidor.exe
```

O servidor vai ficar esperando conexões na porta 8080.

### 2. Rode os clientes (pode rodar vários):
```
.\cliente.exe
```

Quando pedir o IP, digite:
- `127.0.0.1` se for testar na mesma máquina
- O IP da máquina onde o servidor tá rodando se for em outra máquina

### 3. Converse!

Digite as mensagens e aperte Enter. Pra sair, digite `sair`.

## Observações

- Não tem criptografia nem nada muito avançado
- O servidor atual aceita conexões mas tá simplificado
- Porta usada: 8080
- Máximo de clientes: 10

## Melhorias possíveis

- Adicionar nomes de usuário
- Criar interface gráfica