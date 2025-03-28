#include <iostream>
#include <fstream>
#include <vector>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <cstring>
#include <chrono>
using namespace std;

const int WIDTH = 10, HEIGHT = 20;
char grid[HEIGHT][WIDTH];
int score = 0, highScore = 0;

vector<vector<vector<int>>> tetrominoes = {
    {{0,0,0,0}, {1,1,1,1}, {0,0,0,0}, {0,0,0,0}}, // I
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // T
    {{0,0,0,0}, {0,1,1,0}, {0,1,1,0}, {0,0,0,0}}, // O
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // Z
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}, // S
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}, // L
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}}  // J
};

vector<vector<int>> currentTetromino;
int tetX = WIDTH/2 - 2, tetY = 0;

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

vector<vector<int>> rotateTetromino(const vector<vector<int>>& shape) {
    vector<vector<int>> rotated(4, vector<int>(4, 0));
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            rotated[j][3-i] = shape[i][j];
        }
    }
    return rotated;
}

bool isValidPosition(const vector<vector<int>>& shape, int x, int y) {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(shape[i][j]) {
                int newX = x + j;
                int newY = y + i;
                if(newX < 0 || newX >= WIDTH || newY >= HEIGHT) return false;
                if(newY >= 0 && grid[newY][newX]) return false;
            }
        }
    }
    return true;
}

void loadHighScore() {
    ifstream file("highscore.txt");
    if(file >> highScore) file.close();
}

void saveHighScore() {
    if(score > highScore) {
        ofstream file("highscore.txt");
        file << score;
        file.close();
    }
}

void displayGrid() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD cursorPos = {0, 0};
    SetConsoleCursorPosition(hConsole, cursorPos);

    cout << "Score: " << score << " | High: " << highScore << "\n";
    
    char tempGrid[HEIGHT][WIDTH];
    memcpy(tempGrid, grid, sizeof(grid));
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(currentTetromino[i][j] && tetY+i < HEIGHT && tetX+j < WIDTH && tetX+j >= 0) {
                tempGrid[tetY+i][tetX+j] = 1;
            }
        }
    }
    
    for(int i = 0; i < HEIGHT; i++) {
        cout << "|";
        for(int j = 0; j < WIDTH; j++) {
            cout << (tempGrid[i][j] ? "[]" : " .");
        }
        cout << "|\n";
    }
    cout << "Controls: WASD Move, W Rotate, R Restart, Q Quit\n";
}

void mergeTetromino() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(currentTetromino[i][j] && tetY+i >= 0) {
                grid[tetY+i][tetX+j] = 1;
            }
        }
    }
}

int clearLines() {
    int linesCleared = 0;
    for(int y = HEIGHT-1; y >= 0; y--) {
        bool complete = true;
        for(int x = 0; x < WIDTH; x++) {
            if(!grid[y][x]) {
                complete = false;
                break;
            }
        }
        
        if(complete) {
            for(int ny = y; ny > 0; ny--) {
                memcpy(grid[ny], grid[ny-1], WIDTH);
            }
            memset(grid[0], 0, WIDTH);
            linesCleared++;
            y++;
        }
    }
    return linesCleared;
}

void gameLoop() {
    system("cls"); // Clear screen before starting
    loadHighScore();
    hideCursor();
    score = 0;
    memset(grid, 0, sizeof(grid));
    srand(time(0));

    while(true) {
        currentTetromino = tetrominoes[rand() % tetrominoes.size()];
        tetX = WIDTH/2 - 2;
        tetY = 0;

        if(!isValidPosition(currentTetromino, tetX, tetY)) {
            saveHighScore();
            return;
        }

        while(true) {
            displayGrid();
            
            if(_kbhit()) {
                char key = tolower(_getch());
                if(key == 'a' && isValidPosition(currentTetromino, tetX-1, tetY)) tetX--;
                else if(key == 'd' && isValidPosition(currentTetromino, tetX+1, tetY)) tetX++;
                else if(key == 's') {
                    if(isValidPosition(currentTetromino, tetX, tetY+1)) tetY++;
                    else {
                        mergeTetromino();
                        score += clearLines() * 100;
                        saveHighScore();
                        break;
                    }
                }
                else if(key == 'w') {
                    vector<vector<int>> rotated = rotateTetromino(currentTetromino);
                    if(isValidPosition(rotated, tetX, tetY)) {
                        currentTetromino = rotated;
                    }
                }
                else if(key == ' ') {
                    while(isValidPosition(currentTetromino, tetX, tetY+1)) tetY++;
                    mergeTetromino();
                    score += clearLines() * 100;
                    saveHighScore();
                    break;
                }
                else if(key == 'r') { gameLoop(); return; }
                else if(key == 'q') { saveHighScore(); exit(0); }
            }

            // Automatic falling
            static auto lastFall = chrono::steady_clock::now();
            auto now = chrono::steady_clock::now();
            if(chrono::duration_cast<chrono::milliseconds>(now-lastFall).count() > 500) {
                if(isValidPosition(currentTetromino, tetX, tetY+1)) {
                    tetY++;
                }
                else {
                    mergeTetromino();
                    score += clearLines() * 100;
                    saveHighScore();
                    break;
                }
                lastFall = now;
            }

            Sleep(50);
        }
    }
}

int main() {
    gameLoop();
    return 0;
}