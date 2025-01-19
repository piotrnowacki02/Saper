#ifndef SAPER_HPP
#define SAPER_HPP

#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <termios.h>
#include <unistd.h>

class Saper {
private:
    int height, width, bombs = 0;
    int pos_x = 0, pos_y = 0;
    int player = 0;
    std::vector<std::vector<bool>> bombsMap;
    std::vector<std::vector<char>> realMap;
    std::vector<std::vector<bool>> visibleMap;
    std::vector<std::vector<char>> tabForUser;

    void initializeTabForUser();
    void parseRealMapFromString(const std::string& mapString);

    void createBombsMap();
    void createRealMap();
    void createVisibleMap();

    void updateTabForUser();
    void updateTabForUserAfterLosing();
    void updateBombsMapBasedOnRealMap();

    int bombsAround(int x, int y);
    void reveal(int x, int y);
    bool winCondition();

public:
    Saper(int height, int width, int bombs);
    Saper(const std::string& mapString);

    std::string realMapToStrBuffer();
    std::string userMapToStrBuffer();

    int getPos_x();
    int getPos_y();
    int getPlayer();
    void changePlayer();

    void displayUserTable() const;
    void displayRealMap() const;
    void handleMove(char move);
    bool isGameOver();
    bool isWin();
    static void enableRawMode();
    static void disableRawMode();
};

#endif // SAPER_HPP
