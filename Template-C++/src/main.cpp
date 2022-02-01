#include "raylib.h"
#include "raymath.h"

#include <iostream>

int main(void) {
    
    InitWindow(400, 224, "Template C++");

    std::cout << "Hello, World! Proof of C++" << std::endl;

    while (!WindowShouldClose()) {

        BeginDrawing();

        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}

