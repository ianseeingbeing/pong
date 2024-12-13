#include "raylib.h"
#include <math.h>
#include <sys/types.h>

#define KL_UP 'W'
#define KL_DOWN 'S'
#define KL_LEFT 'A'
#define KL_RIGHT 'D'

#define SPEED 150.0f

char title[5] = "PONG";
int fontSize = 16;

Vector2 windowSize = {600, 400};
Vector2 paddleSize = {10, 50};
Vector2 borderMargin = {15, 15};
Vector2 leftPos;
Vector2 leftMoveDir = {0.0f, 0.0f};
Vector2 ballPos;
Vector2 ballVelocity = {150.0f, 150.0f};

float deltaTime;

//#########//
// HEADERS //
//#########//
void updateLeftPaddle(Vector2 *pos, Vector2 *moveDir, Rectangle bounds);
Rectangle getLeftPaddle(Vector2 pos);
void updateBall(Vector2 *pos, Vector2 *velocity, Rectangle bounds, Rectangle leftRec);


//###########//
// FUNCTIONS //
//###########//
float CalculateMagnitude(Vector2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

void NormalizeVector(Vector2* v) {
    float magnitude = CalculateMagnitude(*v);
    if (magnitude > 0.001f) {
        v->x /= magnitude;
        v->y /= magnitude;
    }
}

int main() {
    leftPos.x = 50.0f;
    leftPos.y = windowSize.y / 2;
    ballPos.x = windowSize.x / 2;
    ballPos.y = windowSize.y / 2;


    // init
    InitWindow(windowSize.x, windowSize.y, "pong");
    SetTargetFPS(60);

    // game loop
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);

        deltaTime = GetFrameTime();

        Rectangle border = {borderMargin.x, borderMargin.y, windowSize.x - 30, windowSize.y - 30};
        DrawRectangleLinesEx(border, 2.0f, WHITE);

        updateLeftPaddle(&leftPos, &leftMoveDir, border);
        updateBall(&ballPos, &ballVelocity, border, getLeftPaddle(leftPos));

        int textX = (windowSize.x / 2) - ((float)MeasureText(title, fontSize) / 2);
        DrawText(title, textX, 0, fontSize, WHITE);

        EndDrawing();
    }
    CloseWindow();

    return 0;
}

Rectangle getLeftPaddle(Vector2 pos) {
    Rectangle rec = {pos.x, pos.y, paddleSize.x, paddleSize.y};
    return rec;
}

void updateLeftPaddle(Vector2 *pos, Vector2 *moveDir, Rectangle bounds) {
    // normalize movement
    if (IsKeyDown(KL_UP))
        moveDir->y -= 1.0f;
    else if (IsKeyUp(KL_UP))
        moveDir->y += 1.0f;
    if (IsKeyDown(KL_DOWN))
        moveDir->y += 1.0f;
    else if (IsKeyUp(KL_DOWN))
        moveDir->y -= 1.0f;

    if (IsKeyDown(KL_LEFT))
        moveDir->x -= 1.0f;
    else if (IsKeyUp(KL_LEFT))
        moveDir->x += 1.0f;
    if (IsKeyDown(KL_RIGHT))
        moveDir->x += 1.0f;
    else if (IsKeyUp(KL_RIGHT))
        moveDir->x -= 1.0f;

    NormalizeVector(moveDir);

    // movement
    if (IsKeyDown(KL_UP) || IsKeyDown(KL_DOWN))
        pos->y += SPEED * moveDir->y * deltaTime;
    if (IsKeyDown(KL_LEFT) || IsKeyDown(KL_RIGHT))
        pos->x += SPEED * moveDir->x * deltaTime;

    Vector2 top = {pos->x + (paddleSize.x / 2), pos->y};
    Vector2 bottom = {pos->x + (paddleSize.x / 2), pos->y + paddleSize.y};
    Vector2 left = {pos->x, pos->y + (paddleSize.y / 2)};
    Vector2 right = {pos->x + paddleSize.x, pos->y + (paddleSize.y / 2)};

    if (!CheckCollisionPointRec(top, bounds)) {
        pos->y = borderMargin.y;
    } else if (!CheckCollisionPointRec(bottom, bounds)) {
        pos->y = windowSize.y - borderMargin.y - paddleSize.y;
    }
    if (!CheckCollisionPointRec(left, bounds)) {
        pos->x = borderMargin.x;
    } else if (!CheckCollisionPointRec(right, bounds)) {
        pos->x = windowSize.x - borderMargin.x - paddleSize.x;
    }

    DrawRectangleRec(getLeftPaddle(*pos), WHITE);
}

void updateBall(Vector2 *pos, Vector2 *velocity, Rectangle bounds, Rectangle leftRec) {
    float radius = 5.0f;
    Color color = WHITE;

    Vector2 top = {pos->x, pos->y - radius};
    Vector2 bottom = {pos->x, pos->y + radius};
    Vector2 left = {pos->x - radius, pos->y};
    Vector2 right = {pos->x + radius, pos->y};

    // bounding box collision
    if (!(CheckCollisionPointRec(top, bounds) && CheckCollisionPointRec(bottom, bounds))) {
        velocity->y *= -1.0;
    }
    if (!(CheckCollisionPointRec(left, bounds) && CheckCollisionPointRec(right, bounds))) {
        velocity->x *= -1.0;
    }
    if (CheckCollisionPointRec(top, leftRec) || CheckCollisionPointRec(bottom, leftRec)) {
        velocity->y *= -1.0;
    }
    if (CheckCollisionPointRec(left, leftRec) || CheckCollisionPointRec(right, leftRec)) {
        velocity->x *= -1.0;
    }

    // apply velocity
    pos->x += velocity->x * deltaTime;
    pos->y += velocity->y * deltaTime;

    DrawCircle(pos->x, pos->y, radius, color);
}
