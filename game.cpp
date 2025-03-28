#include <iostream>
#include <vector>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <ctime>

using namespace std;

class Tetris {
private:
    static const int WIDTH = 10, HEIGHT = 20;
    char grid[HEIGHT][WIDTH] = {};
    vector<vector<int>> currentTetromino;
    int tetX, tetY;
    int score, highScore;
    
    vector<vector<vector<int>>> tetrominoes = {
        {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, // I
        {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // T
        {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}}, // O
        {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // Z
        {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // S
        {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // L
        {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}  // J
    };

    HANDLE hConsole;
    COORD cursorPos = {0, 0};

public:
    Tetris() {
        srand(time(0));
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        hideCursor();
        loadHighScore();
        resetGame();
    }

    void run() {
        while (true) {
            spawnTetromino();
            if (!isValidPosition(tetX, tetY)) {
                saveHighScore();
                cout << "Game Over!\n";
                return;
            }

            while (true) {
                display();
                handleInput();
                
                if (!moveDown()) break;

                Sleep(150);
            }
        }
    }

private:
    void resetGame() {
        memset(grid, 0, sizeof(grid));
        score = 0;
    }

    void hideCursor() {
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 100;
        info.bVisible = FALSE;
        SetConsoleCursorInfo(hConsole, &info);
    }

    void loadHighScore() {
        ifstream file("highscore.txt");
        if (file >> highScore) file.close();
    }

    void saveHighScore() {
        if (score > highScore) {
            ofstream file("highscore.txt");
            file << score;
            file.close();
        }
    }

    void spawnTetromino() {
        currentTetromino = tetrominoes[rand() % tetrominoes.size()];
        tetX = WIDTH / 2 - 2;
        tetY = 0;
    }

    void display() {
        SetConsoleCursorPosition(hConsole, cursorPos); // Move cursor to top-left

        cout << "Score: " << score << " | High Score: " << highScore << "\n";

        char tempGrid[HEIGHT][WIDTH];
        memcpy(tempGrid, grid, sizeof(grid));

        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (currentTetromino[i][j] && tetY + i < HEIGHT)
                    tempGrid[tetY + i][tetX + j] = 1;

        for (int i = 0; i < HEIGHT; i++) {
            cout << "|";
            for (int j = 0; j < WIDTH; j++)
                cout << (tempGrid[i][j] ? "[]" : " .");
            cout << "|\n";
        }
        cout << "Controls: A/D (Move), S (Fast Drop), W (Rotate), R (Restart), Q (Quit)\n";
    }

    bool isValidPosition(int x, int y) {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (currentTetromino[i][j]) {
                    int newX = x + j, newY = y + i;
                    if (newX < 0 || newX >= WIDTH || newY >= HEIGHT || (newY >= 0 && grid[newY][newX]))
                        return false;
                }
        return true;
    }

    void mergeTetromino() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                if (currentTetromino[i][j] && tetY + i >= 0)
                    grid[tetY + i][tetX + j] = 1;
    }

    void clearLines() {
        for (int y = HEIGHT - 1; y >= 0; y--) {
            bool full = true;
            for (int x = 0; x < WIDTH; x++)
                if (!grid[y][x]) { full = false; break; }

            if (full) {
                for (int ny = y; ny > 0; ny--)
                    memcpy(grid[ny], grid[ny - 1], WIDTH);
                memset(grid[0], 0, WIDTH);
                score += 100;
                y++;
            }
        }
    }

    vector<vector<int>> rotateTetromino() {
        vector<vector<int>> rotated(4, vector<int>(4, 0));
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                rotated[j][3 - i] = currentTetromino[i][j];
        return rotated;
    }

    void handleInput() {
        if (_kbhit()) {
            char key = tolower(_getch());

            if (key == 'a' && isValidPosition(tetX - 1, tetY)) tetX--;
            else if (key == 'd' && isValidPosition(tetX + 1, tetY)) tetX++;
            else if (key == 's') while (moveDown());
            else if (key == 'w') {
                vector<vector<int>> rotated = rotateTetromino();
                if (isValidPosition(tetX, tetY)) currentTetromino = rotated;
            }
            else if (key == 'r') { resetGame(); run(); }
            else if (key == 'q') { saveHighScore(); exit(0); }
        }
    }

    bool moveDown() {
        if (isValidPosition(tetX, tetY + 1)) {
            tetY++;
            return true;
        } else {
            mergeTetromino();
            clearLines();
            saveHighScore();
            return false;
        }
    }
};

int main() {
    Tetris game;
    game.run();
    return 0;
}
