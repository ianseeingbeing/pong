#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#define LP_UP 'W'
#define LP_DOWN 'S'
#define LP_LEFT 'A'
#define LP_RIGHT 'D'

#define SPEED 105.0f
#define WINNING_SCORE 7

//###########//
// Variables //
//###########//
char scoreStr[31] = "";
char winnerText[31] = "";
const char continueText[31] = "press ENTER to start";
const char title[5] = "PONG";
const int fontSize = 16;

const Vector2 windowSize = {600, 400};
const Vector2 borderMargin = {17, 17};

float deltaTime;

int leftScore = 0;
int rightScore = 0;

const Vector2 paddleSize = {10, 50};
Vector2 leftPos;
Vector2 leftMoveDir = {0.0f, 0.0f};
Vector2 rightPos;
Vector2 rightMoveDir = {0.0f, 0.0f};

float ballRadius = 5.0f;
Vector2 ballVelocity = {115.0f, 115.0f};
Vector2 ballPos;
bool isBallCollidingLeft = false;
bool isBallCollidingRight = false;

bool isGameOver = true;
bool playedEndSound = false;
int gamesPlayed = 0;
double timeOfReset;

Sound hitSound;
Sound scoreSound;
Sound gameEndSound;

//#########//
// HEADERS //
//#########//
Rectangle getPaddle(Vector2 pos);
void updateLeftPaddle(Rectangle bounds);
void updateRightPaddle(Rectangle bounds);
void updateBall(Rectangle bounds, Rectangle leftRec, Rectangle rightRec);
bool updateScore(Rectangle bounds);
void drawScore();
void drawTitle();
void resetBall();
void resetPaddles();
void resetScore();
void resetGame();
void drawWinnerText(char* winner);
void drawContinueText();

void NormalizeVector(Vector2* v);

//###########//
// FUNCTIONS //
//###########//
int main() {
    // init
    InitWindow(windowSize.x, windowSize.y, "pong");
    InitAudioDevice();
    SetMasterVolume(35.0f);
    SetTargetFPS(60);
    resetGame();

    // variables
    Rectangle border = {borderMargin.x, borderMargin.y, windowSize.x - (borderMargin.x*2), windowSize.y - (borderMargin.y*2)};
    Rectangle leftBorder = {borderMargin.x, borderMargin.y, (windowSize.x / 2) - borderMargin.x, windowSize.y - (borderMargin.y*2)};
    Rectangle rightBorder = {windowSize.x / 2, borderMargin.y, (windowSize.x / 2) - borderMargin.x, windowSize.y - (borderMargin.y*2)};

    Vector2 borderTop[] = {{borderMargin.x, borderMargin.y}, {windowSize.x - borderMargin.x, borderMargin.y}};
    Vector2 borderBottom[] = {{borderMargin.x, windowSize.y - borderMargin.y}, {windowSize.x - borderMargin.x, windowSize.y - borderMargin.y}};
    Vector2 centerBarrier[] = {{windowSize.x / 2, borderMargin.y}, {windowSize.x / 2, (windowSize.y) - borderMargin.y}};

    timeOfReset = GetTime();

    hitSound = LoadSound("../audio/hit.wav");
    scoreSound = LoadSound("../audio/score.wav");
    gameEndSound = LoadSound("../audio/game_end.wav");

    // game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        deltaTime = GetFrameTime();

        // starts game
        if (IsKeyPressed(KEY_ENTER)) {
            resetGame();
            isGameOver = false;
            gamesPlayed++;
        }

        // draws playing area
        DrawLineEx(borderTop[0], borderTop[1], 1.0f, WHITE);
        DrawLineEx(borderBottom[0], borderBottom[1], 1.0f, WHITE);
        DrawLineEx(centerBarrier[0], centerBarrier[1], 1.0f, WHITE);

        // updates
        if (!isGameOver) {
            updateLeftPaddle(leftBorder);
            updateRightPaddle(rightBorder);
            if (GetTime() - timeOfReset > 1.0) {
                updateBall(border, getPaddle(leftPos), getPaddle(rightPos));
            }
        }

        DrawRectangleRec(getPaddle(leftPos), WHITE);
        DrawRectangleRec(getPaddle(rightPos), WHITE);
        DrawCircle(ballPos.x, ballPos.y, ballRadius, WHITE);

        if (updateScore(border)) {
            resetBall();
            PlaySound(scoreSound);
        }

        if (leftScore >= WINNING_SCORE) {
            isGameOver = true;
            drawWinnerText("LEFT");
        } else if (rightScore >= WINNING_SCORE) {
            isGameOver = true;
            drawWinnerText("RIGHT");
        }

        if (isGameOver && gamesPlayed == 0) {
            drawContinueText();
        } else if (isGameOver && gamesPlayed > 0) {
            drawContinueText();
            if (!playedEndSound) {
                PlaySound(gameEndSound);
                playedEndSound = true;
            }
        }

        drawTitle();
        drawScore();

        EndDrawing();
    }
    UnloadSound(hitSound);
    UnloadSound(scoreSound);
    UnloadSound(gameEndSound);

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

Rectangle getPaddle(Vector2 pos) {
    Rectangle rec = {pos.x, pos.y, paddleSize.x, paddleSize.y};
    return rec;
}

void updateLeftPaddle(Rectangle bounds) {
    // normalize movement
    if (IsKeyDown(LP_UP))
        leftMoveDir.y -= 1.0f;
    else if (IsKeyUp(LP_UP))
        leftMoveDir.y += 1.0f;
    if (IsKeyDown(LP_DOWN))
        leftMoveDir.y += 1.0f;
    else if (IsKeyUp(LP_DOWN))
        leftMoveDir.y -= 1.0f;

    if (IsKeyDown(LP_LEFT))
        leftMoveDir.x -= 1.0f;
    else if (IsKeyUp(LP_LEFT))
        leftMoveDir.x += 1.0f;
    if (IsKeyDown(LP_RIGHT))
        leftMoveDir.x += 1.0f;
    else if (IsKeyUp(LP_RIGHT))
        leftMoveDir.x -= 1.0f;

    NormalizeVector(&leftMoveDir);

    // movement
    if (IsKeyDown(LP_UP) || IsKeyDown(LP_DOWN))
        leftPos.y += SPEED * leftMoveDir.y * deltaTime;
    if (IsKeyDown(LP_LEFT) || IsKeyDown(LP_RIGHT))
        leftPos.x += SPEED * leftMoveDir.x * deltaTime;

    Vector2 top = {leftPos.x + (paddleSize.x / 2), leftPos.y};
    Vector2 bottom = {leftPos.x + (paddleSize.x / 2), leftPos.y + paddleSize.y};
    Vector2 left = {leftPos.x, leftPos.y + (paddleSize.y / 2)};
    Vector2 right = {leftPos.x + paddleSize.x, leftPos.y + (paddleSize.y / 2)};

    if (!CheckCollisionPointRec(top, bounds)) {
        leftPos.y = borderMargin.y;
    } else if (!CheckCollisionPointRec(bottom, bounds)) {
        leftPos.y = windowSize.y - borderMargin.y - paddleSize.y;
    }
    if (!CheckCollisionPointRec(left, bounds)) {
        leftPos.x = borderMargin.x;
    } else if (!CheckCollisionPointRec(right, bounds)) {
        leftPos.x = bounds.width + borderMargin.x - paddleSize.x;
    }
}

void updateRightPaddle(Rectangle bounds) {
    // normalize movement
    if (IsKeyDown(KEY_UP))
        rightMoveDir.y -= 1.0f;
    else if (IsKeyUp(KEY_UP))
        rightMoveDir.y += 1.0f;
    if (IsKeyDown(KEY_DOWN))
        rightMoveDir.y += 1.0f;
    else if (IsKeyUp(KEY_DOWN))
        rightMoveDir.y -= 1.0f;

    if (IsKeyDown(KEY_LEFT))
        rightMoveDir.x -= 1.0f;
    else if (IsKeyUp(KEY_LEFT))
        rightMoveDir.x += 1.0f;
    if (IsKeyDown(KEY_RIGHT))
        rightMoveDir.x += 1.0f;
    else if (IsKeyUp(KEY_RIGHT))
        rightMoveDir.x -= 1.0f;

    NormalizeVector(&rightMoveDir);

    // movement
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN))
        rightPos.y += SPEED * rightMoveDir.y * deltaTime;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))
        rightPos.x += SPEED * rightMoveDir.x * deltaTime;
    Vector2 top = {rightPos.x + (paddleSize.x / 2), rightPos.y};
    Vector2 bottom = {rightPos.x + (paddleSize.x / 2), rightPos.y + paddleSize.y};
    Vector2 left = {rightPos.x, rightPos.y + (paddleSize.y / 2)};
    Vector2 right = {rightPos.x + paddleSize.x, rightPos.y + (paddleSize.y / 2)};

    if (!CheckCollisionPointRec(top, bounds)) {
        rightPos.y = borderMargin.y;
    } else if (!CheckCollisionPointRec(bottom, bounds)) {
        rightPos.y = windowSize.y - borderMargin.y - paddleSize.y;
    }
    if (!CheckCollisionPointRec(left, bounds)) {
        rightPos.x = bounds.width + borderMargin.x;
    } else if (!CheckCollisionPointRec(right, bounds)) {
        rightPos.x = windowSize.x - borderMargin.x - paddleSize.x;
    }
}

void updateBall(Rectangle bounds, Rectangle leftRec, Rectangle rightRec) {
    Vector2 topRight = {(ballRadius * cosf(45)) + ballPos.x, (ballRadius * sinf(45)) + ballPos.y};
    Vector2 topLeft = {(ballRadius * -cosf(45)) + ballPos.x, (ballRadius * sinf(45)) + ballPos.y};
    Vector2 bottomLeft = {(ballRadius * -cosf(45)) + ballPos.x, (ballRadius * -sinf(45)) + ballPos.y};
    Vector2 bottomRight = {(ballRadius * cosf(45)) + ballPos.x, (ballRadius * -sinf(45)) + ballPos.y};
    Vector2 top = {ballPos.x, ballPos.y - ballRadius};
    Vector2 bottom = {ballPos.x, ballPos.y + ballRadius};
    Vector2 left = {ballPos.x - ballRadius, ballPos.y};
    Vector2 right = {ballPos.x + ballRadius, ballPos.y};

    // BORDER COLLISION
    if (!CheckCollisionPointRec(top, bounds) || !CheckCollisionPointRec(bottom, bounds))
        ballVelocity.y *= -1.0;

    // LEFT PADDLE COLLISION
    if (CheckCollisionCircleRec(ballPos, ballRadius, leftRec) && !isBallCollidingLeft) {
        isBallCollidingLeft = true;
        if (CheckCollisionPointRec(top, leftRec) || CheckCollisionPointRec(bottom, leftRec))
            ballVelocity.y *= -1.0;
        else if (CheckCollisionPointRec(left, leftRec) || CheckCollisionPointRec(right, leftRec))
            ballVelocity.x *= -1.0;
        else if (CheckCollisionPointRec(topRight, leftRec) || CheckCollisionPointRec(topLeft, leftRec) || CheckCollisionPointRec(bottomLeft, leftRec) || CheckCollisionPointRec(bottomRight, leftRec)) {
            ballVelocity.x *= -1.0;
            ballVelocity.y *= -1.0;
        } else {
            ballVelocity.x *= -1.0;
            ballVelocity.y *= -1.0;
        }
        PlaySound(hitSound);
    }
    else if (!CheckCollisionCircleRec(ballPos, ballRadius, leftRec) && isBallCollidingLeft) {
        isBallCollidingLeft = false;
    }
    // RIGHT PADDLE COLLISION
    if (CheckCollisionCircleRec(ballPos, ballRadius, rightRec) && !isBallCollidingRight) {
        isBallCollidingRight = true;
        if (CheckCollisionPointRec(top, rightRec) || CheckCollisionPointRec(bottom, rightRec))
            ballVelocity.y *= -1.0;
        else if (CheckCollisionPointRec(left, rightRec) || CheckCollisionPointRec(right, rightRec))
            ballVelocity.x *= -1.0;
        else if (CheckCollisionPointRec(topRight, rightRec) || CheckCollisionPointRec(topLeft, rightRec) || CheckCollisionPointRec(bottomLeft, rightRec) || CheckCollisionPointRec(bottomRight, rightRec)) {
            ballVelocity.x *= -1.0;
            ballVelocity.y *= -1.0;
        } else {
            ballVelocity.x *= -1.0;
            ballVelocity.y *= -1.0;
        }
        PlaySound(hitSound);
    }
    else if (!CheckCollisionCircleRec(ballPos, ballRadius, rightRec) && isBallCollidingRight) {
        isBallCollidingRight = false;
    }

    // apply velocity
    ballPos.x += ballVelocity.x * deltaTime;
    ballPos.y += ballVelocity.y * deltaTime;

}

void NormalizeVector(Vector2* v) {
    float magnitude = sqrtf(v->x * v->x + v->y * v->y);
    if (magnitude > 0.001f) {
        v->x /= magnitude;
        v->y /= magnitude;
    }
}

bool updateScore(Rectangle bounds) {
    Vector2 ballLeft = {ballPos.x - ballRadius, ballPos.y};
    Vector2 ballRight = {ballPos.x + ballRadius, ballPos.y};

    if (!CheckCollisionPointRec(ballLeft, bounds)) {
        leftScore += 1;
        return true;
    }
    if (!CheckCollisionPointRec(ballRight, bounds)) {
        rightScore += 1;
        return true;
    }
    return false;
}

void resetBall() {
    srand(time(NULL));
    int xVal = rand() % 2 == 0 ? 1 : -1;
    srand(time(NULL)+1);
    int yVal = rand() % 2 == 0 ? 1 : -1;
    ballPos.x = windowSize.x / 2;
    ballPos.y = windowSize.y / 2;
    ballVelocity.x *= xVal;
    ballVelocity.y *= yVal;
    timeOfReset = GetTime();
}

void resetPaddles() {
    leftPos.x = 50.0f;
    leftPos.y = (windowSize.y / 2) - (paddleSize.y / 2);
    rightPos.x = windowSize.x - 50.0f - paddleSize.x;
    rightPos.y = (windowSize.y / 2) - (paddleSize.y / 2);
}

void resetScore() {
    leftScore = 0;
    rightScore = 0;
}

void resetGame() {
    resetBall();
    resetPaddles();
    resetScore();
    playedEndSound = false;
}

void drawScore() {
    sprintf(scoreStr, "%02d - %02d", leftScore, rightScore);
    int x = (windowSize.x / 2) - (((float)MeasureText(scoreStr, fontSize)) / 2);
    int y = 0;
    DrawText(scoreStr, x, y, fontSize, WHITE);
}

void drawTitle() {
    int x = (windowSize.x / 2) - ((float)MeasureText(title, fontSize) / 2);
    int y = windowSize.y - borderMargin.y;
    DrawText(title, x, y, fontSize, WHITE);
}

void drawWinnerText(char* winner) {
    sprintf(winnerText, "%s WINS", winner);
    int xPos = (windowSize.x / 2) - (((float)MeasureText(winnerText, 32)) / 2);
    int yPos = (windowSize.y / 4) - 16;
    DrawText(winnerText, xPos, yPos, 32, YELLOW);
}

void drawContinueText() {
    int xPos = (windowSize.x / 2) - (((float)MeasureText(continueText, fontSize)) / 2);
    int yPos = ((windowSize.y / 4) * 3) - ((float)fontSize / 2);
    DrawText(continueText, xPos, yPos, fontSize, YELLOW);
}
