#include "raylib.h"
#include "time.h"
#include "stdlib.h"

//BOOLEAN
#define false 0
#define true 1
#define bool int

#define ROWS 16
#define COLS 16

const int screenWidth = 600;
const int screenHeight = 800;

const int cellWidth = screenWidth / COLS;
const int cellHeight = screenHeight / ROWS;

enum GameState {PLAYING, WIN, LOSE};
enum GameState state = PLAYING;
bool gameOver = false;
int curMines;

Texture2D flag, mine;

bool isValidIndex(int x, int y){
    if(x < 0 || x >= ROWS || y < 0 || y >= COLS) return false;
    return true;
}

typedef struct Cell
{
    int i, j;
    bool containMines;
    bool isRevealed;
    int nearbyMines;
    bool flagged;
} Cell;

void CellReveal(Cell cell){
    if(cell.containMines) {
        Rectangle source = {0, 0, mine.width, mine.height};
        Rectangle dest = {cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight};
        Vector2 origin = {0, 0};

        DrawTexturePro(mine, source, dest, origin, 0.0f, RED);
    }
    else{        
        DrawRectangle(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, LIGHTGRAY);
        if(cell.nearbyMines != 0) DrawText(TextFormat("%d", cell.nearbyMines), cell.j * cellWidth + 6, cell.i * cellHeight + 8, cellHeight - 8, BLACK);
    }
}

void CellDraw(Cell cell){
    if(cell.flagged) {
        Rectangle source = {0, 0, flag.width, flag.height};
        Rectangle dest = {cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight};
        Vector2 origin = {0, 0};

        DrawTexturePro(flag, source, dest, origin, 0.0f, WHITE);

        DrawRectangleLines(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, BLACK);
    }
    else {
        if(!cell.isRevealed) DrawRectangleLines(cell.j * cellWidth, cell.i * cellHeight, cellWidth, cellHeight, BLACK);
        else CellReveal(cell);
    }
}

Cell grid[100][100];

int CellCountMines(int x, int y){
    int res = 0;
    
    for(int i1 = -1; i1 <= 1; ++i1){
        for(int j1 = -1; j1 <= 1; ++j1){
            if(i1 == 0 && j1 == 0) continue;

            if(isValidIndex(x + i1, y + j1)){
                if(grid[x + i1][y + j1].containMines) ++res;
            }
        }
    }

    return res;
}

void RevealCell(int x, int y){
    if(grid[x][y].nearbyMines == 0 && !grid[x][y].containMines){
        for(int i = -1; i <= 1; ++i){
            for(int j = -1; j <= 1; ++j){
                if(isValidIndex(x+i, y+j) && !grid[x+i][y+j].isRevealed && !grid[x+i][y+j].flagged && i != j){
                    grid[x+i][y+j].isRevealed = true;
                    if(grid[x+i][y+j].nearbyMines == 0) RevealCell(x+i, y+j);
                }
            }
        }
    }
}

void GameInit(){
    state = PLAYING;

    //Init grid 
    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLS; ++j){
            grid[i][j] = (Cell){
                .i = i,
                .j = j,
                .containMines = false,
                .isRevealed = false,
                .flagged = false
            };
        }
    }

    int mineToPlace = (int)(ROWS * COLS * 0.1f);
    curMines = mineToPlace;
    while(mineToPlace > 0){
        int i = rand() % COLS;
        int j = rand() % ROWS;

        if(grid[i][j].containMines == false){
            --mineToPlace;
            grid[i][j].containMines = true;
        }
    }


    for(int i = 0; i < ROWS; ++i){
        for(int j = 0; j < COLS; ++j){
            if(!grid[i][j].containMines) grid[i][j].nearbyMines = CellCountMines(i, j);
        }
    }
}

void run(){
    srand(time(0));

    InitWindow(screenWidth, screenHeight, "Minesweeper");    

    flag = LoadTexture("flag.png");
    mine = LoadTexture("mines.png");

    GameInit();

    while (!WindowShouldClose())
    {
        //Mouse Input
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && state == PLAYING){
            Vector2 mousePos = GetMousePosition();
            int y = mousePos.x / cellWidth;
            int x = mousePos.y / cellHeight;

            if(!grid[x][y].isRevealed && !grid[x][y].flagged){
                grid[x][y].isRevealed = true;
                RevealCell(x, y);
            }

            if(grid[x][y].containMines){
                grid[x][y].isRevealed = true;
                gameOver = true;
                state = LOSE;
            }
        }

        //Place Flag
        if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && state == PLAYING){
            Vector2 mousePos = GetMousePosition();
            int y = mousePos.x / cellWidth;
            int x = mousePos.y / cellHeight;

            if(!grid[x][y].isRevealed){
                if(grid[x][y].flagged) {
                    grid[x][y].flagged = false;
                    if(grid[x][y].containMines) curMines += 1;
                }
                else {
                    grid[x][y].flagged = true;
                    if(grid[x][y].containMines) curMines -= 1;
                }
            }
        }

        if(IsKeyPressed(KEY_R)){
            GameInit();
        }

        if(curMines == 0) state = WIN;

        BeginDrawing();
            ClearBackground(RAYWHITE);

            for(int i = 0; i < ROWS; ++i){
                for(int j = 0; j < COLS; ++j){
                    CellDraw(grid[i][j]);
                }
            }

            if(state == LOSE){
                DrawRectangle(screenWidth / 2 - MeasureText("GAME OVER!", 64) / 2 - 10, screenHeight / 2 - 20, 420, 200, WHITE);
                DrawText("GAME OVER!", screenWidth / 2 - MeasureText("GAME OVER!", 64) / 2, screenHeight / 2, 64, RED);
                DrawText("Press 'r' to restart!", screenWidth / 2 - MeasureText("Press 'r' to restart!", 20) / 2, screenHeight / 2 + 150, 20, RED);                
            }
            if(state == WIN){
                DrawRectangle(screenWidth / 2 - MeasureText("YOU WIN!", 64) / 2 - 10, screenHeight / 2 - 20, 420, 200, WHITE);
                DrawText("YOU WIN!", screenWidth / 2 - MeasureText("YOU WIN!", 64) / 2, screenHeight / 2, 64, RED);
                DrawText("Press 'r' to restart!", screenWidth / 2 - MeasureText("Press 'r' to restart!", 20) / 2, screenHeight / 2 + 150, 20, RED);                
            }
        EndDrawing();   
    }
}
