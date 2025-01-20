#include "Saper.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

Saper::Saper(int height, int width, int bombs) : height(height), width(width), bombs(bombs) {
    createBombsMap();
    createRealMap();
    createVisibleMap();
    tabForUser.resize(height, std::vector<char>(width, '*'));
}

Saper::Saper(const std::string& mapString) {
    parseRealMapFromString(mapString);
    createVisibleMap();
    initializeTabForUser();
    updateBombsMapBasedOnRealMap();
}

int Saper::getPos_x(){
    return this->pos_x;
}

int Saper::getPos_y(){
    return this->pos_y;
}

void Saper::initializeTabForUser() {
    tabForUser = std::vector<std::vector<char>>(height, std::vector<char>(width, '*'));
}

void Saper::parseRealMapFromString(const std::string& mapString) {
    std::istringstream stream(mapString);
    std::string line;
    height = 0;
    width = -1;
    while (std::getline(stream, line)) {
        if (width == -1) {
            width = line.size();
        } else if (line.size() != static_cast<size_t>(width)) {
            throw std::invalid_argument("Invalid mapString: inconsistent row widths");
        }
        realMap.push_back(std::vector<char>(line.begin(), line.end()));
        ++height;
    }
    if (width == -1 || height == 0) {
        throw std::invalid_argument("Invalid mapString: empty map");
    }
}

void Saper::createBombsMap() {
    bombsMap.resize(height, std::vector<bool>(width, false));
    int bombsPlaced = 0;
    while (bombsPlaced < bombs) {
        int x = rand() % height;
        int y = rand() % width;
        if (!bombsMap[x][y]) {
            bombsMap[x][y] = true;
            bombsPlaced++;
        }
    }
}

void Saper::createRealMap() {
    realMap.resize(height, std::vector<char>(width, ' '));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (bombsMap[i][j]) {
                realMap[i][j] = 'X';
            } else {
                int count = bombsAround(i, j);
                realMap[i][j] = count > 0 ? '0' + count : ' ';
            }
        }
    }
}

void Saper::createVisibleMap() {
    visibleMap.resize(height, std::vector<bool>(width, false));
}

int Saper::bombsAround(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && nx < height && ny >= 0 && ny < width && bombsMap[nx][ny]) {
                count++;
            }
        }
    }
    return count;
}

void Saper::reveal(int x, int y) {
    if (x < 0 || x >= height || y < 0 || y >= width || visibleMap[x][y]) return;
    visibleMap[x][y] = true;
    if (realMap[x][y] == ' ') {
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                reveal(x + dx, y + dy);
            }
        }
    }
}

void Saper::updateTabForUser() {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (visibleMap[i][j]) {
                tabForUser[i][j] = realMap[i][j];
            } else if (tabForUser[i][j] != 'F' && tabForUser[i][j] != 'X') {
                tabForUser[i][j] = '*';
            }
        }
    }
}

void Saper::updateTabForUserAfterLosing(){
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (visibleMap[i][j] || bombsMap[i][j]) {
                tabForUser[i][j] = realMap[i][j];
            } else if (tabForUser[i][j] != 'F' && tabForUser[i][j] != 'X') {
                tabForUser[i][j] = '*';
            }
        }
    }
}

void Saper::updateBombsMapBasedOnRealMap(){
    this->bombsMap = std::vector<std::vector<bool>>(height, std::vector<bool>(width, false));
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (realMap[i][j] == 'X') {
                bombsMap[i][j] = true;
                bombs++;
            }
        }
    }
}

bool Saper::winCondition() {
    int uncovered = 0;
    int totalCells = height * width;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (tabForUser[i][j] != '*' && tabForUser[i][j] != 'F' && tabForUser[i][j] != 'X') {
                uncovered++;
            }
        }
    }
    return uncovered == totalCells - bombs;
}

bool Saper::isWin() {
    return winCondition();
}

std::string Saper::realMapToStrBuffer() {
    std::string buffer;
    for (const auto& row : realMap) {
        for (const auto& cell : row) {
            buffer += cell;
        }
        buffer += '\n'; // Dodajemy znak nowej linii po każdym wierszu
    }
    
    if (!buffer.empty() && buffer.back() == '\n') { //usun ostatni /n po calej wiadomosci
        buffer.pop_back();
    }
    return buffer;
}

std::string Saper::userMapToStrBuffer() {
    std::ostringstream buffer;
    buffer << "╔";
    for (int i = 0; i < width * 3 + 2; i++) buffer << "═";
    buffer << "╗\n";

    for (int i = 0; i < height; ++i) {
        buffer << "║  ";
        for (int j = 0; j < width; j++) {
            if (this->pos_x == i && this->pos_y == j) {
                buffer << "\x1B[41m" << tabForUser[i][j] << "\033[0m  "; // Podświetlenie pozycji kursora
                continue;
            }
            switch (tabForUser[i][j]) {
                case '*': buffer << tabForUser[i][j] << "  "; break;
                case ' ': buffer << tabForUser[i][j] << "  "; break;
                case '1': buffer << "\x1B[31m" << tabForUser[i][j] << "\033[0m  "; break;
                case '2': buffer << "\x1B[32m" << tabForUser[i][j] << "\033[0m  "; break;
                case '3': buffer << "\x1B[33m" << tabForUser[i][j] << "\033[0m  "; break;
                case '4': buffer << "\x1B[34m" << tabForUser[i][j] << "\033[0m  "; break;
                case '5': buffer << "\x1B[36m" << tabForUser[i][j] << "\033[0m  "; break;
                case '6': buffer << "\x1B[35m" << tabForUser[i][j] << "\033[0m  "; break;
                case '7': buffer << "\x1B[35m" << tabForUser[i][j] << "\033[0m  "; break;
                case '8': buffer << "\x1B[35m" << tabForUser[i][j] << "\033[0m  "; break;
                default: buffer << tabForUser[i][j] << "  "; break;
            }
        }
        buffer << "║\n";
    }

    buffer << "╚";
    for (int i = 0; i < width * 3 + 2; i++) buffer << "═";
    buffer << "╝\n";

    return buffer.str();
}

int Saper::getPlayer(){
    return this->player;
}

void Saper::changePlayer(){
    this->player = 1-this->player;
}


void Saper::displayUserTable() const {
    std::wcout << L"╔";
    for (int i = 0; i < width * 3 + 2; i++) std::wcout << L"═";
    std::wcout << L"╗\n";

    for (int i = 0; i < height; ++i) {
        std::wcout << L"║  ";
        for (int j = 0; j < width; j++) {
            if(this->pos_x == i && this->pos_y == j)
            {
                std::wcout << "\x1B[41m" << tabForUser[i][j] << "\033[0m  "; // Podświetlenie pozycji kursora
                continue;
            }
            switch (tabForUser[i][j]) {
            case '*': {
                std::wcout << tabForUser[i][j] << "  ";
                break;
            }
            case ' ': {
                std::wcout << tabForUser[i][j] << "  ";
                break;
            }
            case '1': {
                std::wcout << L"\x1B[31m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '2': {
                std::wcout << L"\x1B[32m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '3': {
                std::wcout << L"\x1B[33m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '4': {
                std::wcout << L"\x1B[34m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '5': {
                std::wcout << L"\x1B[36m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '6': {
                std::wcout << L"\x1B[35m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '7': {
                std::wcout << L"\x1B[35m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            case '8': {
                std::wcout << L"\x1B[35m" << tabForUser[i][j] << "\033[0m  ";
                break;
            }
            default: {
                std::wcout << tabForUser[i][j] << "  ";
                break;
            }
            }
        }
        std::wcout << L"║\n";
    }

    std::wcout << L"╚";
    for (int i = 0; i < width * 3 + 2; i++) std::wcout << L"═";
    std::wcout << L"╝\n";
}

void  Saper::displayRealMap() const{
    std::wcout << L"╔";
    for (int i = 0; i < width * 3 + 2; i++) std::wcout << L"═";
    std::wcout << L"╗\n";

    for (int i = 0; i < height; ++i) {
        std::wcout << L"║  ";
        for (int j = 0; j < width; j++) {
            switch (realMap[i][j]) {
            case '*': {
                std::wcout << realMap[i][j] << "  ";
                break;
            }
            case ' ': {
                std::wcout << realMap[i][j] << "  ";
                break;
            }
            case '1': {
                std::wcout << L"\x1B[31m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '2': {
                std::wcout << L"\x1B[32m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '3': {
                std::wcout << L"\x1B[33m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '4': {
                std::wcout << L"\x1B[34m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '5': {
                std::wcout << L"\x1B[36m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '6': {
                std::wcout << L"\x1B[35m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '7': {
                std::wcout << L"\x1B[35m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            case '8': {
                std::wcout << L"\x1B[35m" << realMap[i][j] << "\033[0m  ";
                break;
            }
            default: {
                std::wcout << realMap[i][j] << "  ";
                break;
            }
            }
        }
        std::wcout << L"║\n";
    }

    std::wcout << L"╚";
    for (int i = 0; i < width * 3 + 2; i++) std::wcout << L"═";
    std::wcout << L"╝\n";
}


void Saper::handleMove(char move) {
    switch (move) {
    case 'w': 
    {
        if (pos_x > 0) 
        {
            this->pos_x--;
        }
        break;
    } 
    case 's': 
    {
        if (pos_x < height - 1)
        {
            this->pos_x++; 
        }
        break;
    }
    case 'a':
    {
        if (pos_y > 0)
        {
            this->pos_y--; 
        }
        break;
    }
    case 'd':
    {
        if (pos_y < width - 1) 
        {
            this->pos_y++; 
        }
        break;
    }
    case '\n': 
    {
        if(visibleMap[pos_x][pos_y]) break;
        reveal(pos_x, pos_y); 
        changePlayer();
        break;
    }
    case ' ': tabForUser[pos_x][pos_y] = tabForUser[pos_x][pos_y] == 'F' ? '*' : 'F'; break;
    }
    updateTabForUser();
}

bool Saper::isGameOver() {
    if(bombsMap[pos_x][pos_y] && visibleMap[pos_x][pos_y])
    {
        updateTabForUserAfterLosing();
    }
    return bombsMap[pos_x][pos_y] && visibleMap[pos_x][pos_y];
}

void Saper::enableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void Saper::disableRawMode() {
    struct termios term;
    tcgetattr(STDIN_FILENO, &term);
    term.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
