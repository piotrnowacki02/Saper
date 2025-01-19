#include "Saper.hpp"
#include <locale>
#include <iostream>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

#define PORT 8080
#define MAX_CLIENTS 2

std::mutex queueMutex;
std::condition_variable queueCV;
std::queue<int> waitingPlayers;

void sendString(int socket, const std::string& message) {
    int sizeOfMsg = message.size();
    send(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    send(socket, message.c_str(), sizeOfMsg, 0);
}

std::string recvString(int socket) {
    int sizeOfMsg;
    recv(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    char buffer[sizeOfMsg + 1];
    recv(socket, buffer, sizeOfMsg, 0);
    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

void handleGame(int client1, int client2) {
    setlocale(LC_ALL, "");
    srand(time(NULL));

    Saper game(10, 10, 15);

    // Wysłanie początkowego stanu gry do obu graczy
    std::string mapForUser = game.userMapToStrBuffer();
    sendString(client1, mapForUser);
    sendString(client2, mapForUser);

    bool gameOver = false;
    while (!game.isWin() && !gameOver) {

        // Powiadomienie aktywnego gracza, że jest jego tura
        int currentPlayerSocket = (game.getPlayer() == 0) ? client1 : client2;
        int opponentSocket = (game.getPlayer() == 0) ? client2 : client1;

        sendString(currentPlayerSocket, "Your turn");

        // Oczekiwanie na ruch aktywnego gracza
        std::string moveStr = recvString(currentPlayerSocket);
        if (moveStr.empty()) {
            std::cerr << "Player " << game.getPlayer() + 1 << " disconnected." << std::endl;
            gameOver = true;
            break;
        }

        // Obsługa ruchu
        game.handleMove(moveStr[0]);

        // Sprawdzenie czy po odkryciu tego pola gra się zakończy (WIN/LOSE)
        gameOver = game.isGameOver();

        // Aktualizacja planszy dla obu graczy (odesłanie im planszy)
        mapForUser = game.userMapToStrBuffer();
        sendString(client1, mapForUser);
        sendString(client2, mapForUser);

        // Obsługa zakończenia gry
        if (gameOver) {
            sendString(currentPlayerSocket, "LOSE");
            sendString(opponentSocket, "WIN");
            break;
        }
    }

    // Obsługa remisu (wysłanie do obu graczy komunikatu)
    if (game.isWin() && !gameOver) {
        sendString(client1, "DRAW");
        sendString(client2, "DRAW");
    }

    // Zamknięcie połączeń
    close(client1);
    close(client2);
}

void acceptClients(int server_fd) {
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    while (true) {
        int client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        std::cout << "Player connected." << std::endl;

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            waitingPlayers.push(client_fd);
        }
        queueCV.notify_one();
    }
}

void matchPlayers() {
    while (true) {
        int player1, player2;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [] { return waitingPlayers.size() >= 2; });

            player1 = waitingPlayers.front();
            waitingPlayers.pop();
            player2 = waitingPlayers.front();
            waitingPlayers.pop();
        }

        std::cout << "Starting game between two players." << std::endl;

        std::thread gameThread(handleGame, player1, player2);
        gameThread.detach();
    }
}

int main() {
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) { // Zwiększ limit oczekujących połączeń
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Waiting for connections on port " << PORT << "..." << std::endl;

    std::thread acceptThread(acceptClients, server_fd);
    std::thread matchThread(matchPlayers);

    acceptThread.join();
    matchThread.join();

    close(server_fd);
    return 0;
}
