#include <locale>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <termios.h>

#define PORT 8080

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
        return ""; // Rozłączenie lub błąd
    }
    char buffer[sizeOfMsg + 1];
    if (recv(socket, buffer, sizeOfMsg, 0) <= 0) {
        return ""; // Rozłączenie lub błąd
    }
    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

int main() 
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    std::cout << "Waiting for the second player..." << std::endl;

    std::string gameState = recvString(sock);
    system("clear");
    std::cout << gameState << std::endl;

    char move;
    bool gameOver = false;

    while (!gameOver) {
        std::string serverMessage = recvString(sock);

        if (serverMessage.empty()) {
            std::cout << "Server disconnected unexpectedly. Exiting game." << std::endl;
            break;
        }

        if (serverMessage == "Your turn") {
            std::cout << "Your turn! Enter move (w/a/s/d/space/enter): " << std::endl;
            read(STDIN_FILENO, &move, 1);
            std::cout << std::endl;
            if (sendString(sock, std::string(1, move)) < 0) {
                std::cout << "Failed to send move. Server might be down." << std::endl;
                break;
            }

        } else if (serverMessage == "WIN" || serverMessage == "LOSE" || serverMessage == "DRAW") {
            std::cout << serverMessage << std::endl;
            gameOver = true;
        } else if (serverMessage == "Check") {
            continue;
        } else {
            system("clear");
            std::cout << serverMessage << std::endl;
        }
    }


    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);

    close(sock);
    return 0;
}
