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

std::mutex queueMutex;
std::condition_variable queueCV;
std::queue<int> waitingPlayers;

int sendString(int socket, const std::string& message) {
    int sizeOfMsg = message.size();
    if (send(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0) <= 0) {
        return -1; // Błąd przy wysyłaniu rozmiaru wiadomości
    }
    if (send(socket, message.c_str(), sizeOfMsg, 0) <= 0) {
        return -1; // Błąd przy wysyłaniu właściwej wiadomości
    }
    return 0;
}


std::string recvString(int socket) {
    int sizeOfMsg;
    if (recv(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0) <= 0) {
        return "";
    }
    char buffer[sizeOfMsg + 1];
    if (recv(socket, buffer, sizeOfMsg, 0) <= 0) {
        return "";
    }
    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

void handleGame(int client1, int client2) {
    setlocale(LC_ALL, "");
    srand(time(NULL));

    Saper game(10, 10, 15);

    std::string mapForUser = game.userMapToStrBuffer();

    // Wyślij początkowy stan gry do obu graczy
    if (sendString(client1, mapForUser) < 0 || sendString(client2, mapForUser) < 0) {
        std::cerr << "Failed to send initial map. A player may have disconnected." << std::endl;
        close(client1);
        close(client2);
        return;
    }

    bool gameOver = false;

    while (!gameOver && !game.isWin()) {
        int currentPlayerSocket = (game.getPlayer() == 0) ? client1 : client2;
        int opponentSocket = (game.getPlayer() == 0) ? client2 : client1;

        // Sprawdź, czy przeciwnik jest połączony
        if (sendString(opponentSocket, "Check") < 0) {
            std::cerr << "Opponent disconnected before their turn." << std::endl;
            sendString(currentPlayerSocket, "WIN");
            gameOver = true;
            break;
        }

        // Powiadomienie aktywnego gracza o jego turze
        if (sendString(currentPlayerSocket, "Your turn") < 0) {
            std::cerr << "Active player disconnected during their turn." << std::endl;
            sendString(opponentSocket, "WIN");
            gameOver = true;
            break;
        }

        // Oczekiwanie na ruch gracza
        std::string moveStr = recvString(currentPlayerSocket);
        if (moveStr.empty()) {  // Gracz rozłączył się
            std::cerr << "Player " << game.getPlayer() + 1 << " disconnected." << std::endl;
            sendString(opponentSocket, "WIN");
            gameOver = true;
            break;
        }

        // Obsługa ruchu gracza
        game.handleMove(moveStr[0]);

        // Sprawdzenie stanu gry
        gameOver = game.isGameOver();

        // Aktualizacja planszy dla obu graczy
        mapForUser = game.userMapToStrBuffer();
        if (sendString(client1, mapForUser) < 0 || sendString(client2, mapForUser) < 0) {
            std::cerr << "Failed to send updated map. A player may have disconnected." << std::endl;
            gameOver = true;
            break;
        }

        // Jeśli gra się zakończyła, wyślij odpowiedni komunikat
        if (gameOver) {
            sendString(currentPlayerSocket, "LOSE");
            sendString(opponentSocket, "WIN");
        }
    }

    // Obsługa remisu
    if (game.isWin() && !gameOver) {
        sendString(client1, "DRAW");
        sendString(client2, "DRAW");
    }

    // Zamknięcie gniazd
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

    if (listen(server_fd, 10) < 0) {
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
