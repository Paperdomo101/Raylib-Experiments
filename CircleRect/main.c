/* 
    Implementation of OLC Circle-Rectangle collision by Basil Termini
    Original C++ implementation can be found here: 
    https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Videos/OneLoneCoder_PGE_CircleVsRect.cpp 

    License (OLC-3)
	~~~~~~~~~~~~~~~
	Copyright 2018 - 2021 OneLoneCoder.com

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistributions or derivations of source code must retain the above
	copyright notice, this list of conditions and the following disclaimer.
    
	2. Redistributions or derivative works in binary form must reproduce
	the above copyright notice. This list of conditions and the following
	disclaimer must be reproduced in the documentation and/or other
	materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its
	contributors may be used to endorse or promote products derived
	from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "screen.h"

static const int screenWidth  = 640;
static const int screenHeight = 480;

static const Vector2 screenCenter = { screenWidth * 0.5, screenHeight * 0.5 };

static int windowScale = 2;

static void InitGame(void);
static void SetupGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

Vector2 Vector2Int(Vector2 v) {
    Vector2 vi;
    vi.x = (int)(v.x);
    vi.y = (int)(v.y);
    return vi;
}

Vector2 Vector2Floor(Vector2 v) {
    Vector2 vf; 
    vf.x = floorf(v.x);
    vf.y = floorf(v.y);
    return vf;
}

Vector2 Vector2MinMax(Vector2 v, float x, bool max) {
    Vector2 vm;
    vm.x = max ? fmaxf(x, v.x) : fminf(x, v.x);
    vm.y = max ? fmaxf(x, v.y) : fminf(x, v.y);
    return vm;
}

Camera2D camera;

typedef struct WorldObject {
    Vector2 pos;
    Vector2 vel;
    float radius;
} WorldObject;

WorldObject object;

char worldMap[1026] = {
	"################################"
	"#..............................#"
	"#.......#####.#.....#####......#"
	"#.......#...#.#.....#..........#"
	"#.......#...#.#.....#..........#"
	"#.......#####.#####.#####......#"
	"#..............................#"
	"#....#####.#####.#####.#####...#"
	"#........#.#...#.....#.....#...#"
	"#....#####.#...#.#####.#####...#"
	"#....#.....#...#.#.....#.......#"
	"#....#####.#####.#####.#####...#"
	"#..............................#"
	"#..............................#"
	"#..#.#..........#....#.........#"
	"#..#.#..........#....#.........#"
	"#..#.#.......#####.#######.....#"
	"#..#.#..........#....#.........#"
	"#..#.#.............###.#.#.....#"
	"#..#.##########................#"
	"#..#..........#....#.#.#.#.....#"
	"#..#.####.###.#................#"
	"#..#.#......#.#................#"
	"#..#.#.####.#.#....###..###....#"
	"#..#.#......#.#....#......#....#"
	"#..#.########.#....#......#....#"
	"#..#..........#....#......#....#"
	"#..############....#......#....#"
	"#..................########....#"
	"#..............................#"
	"#..............................#"
	"################################"
};


Vector2 worldSize = (Vector2) {32, 32};

bool followObject = false;

Vector2 potentialPosition;

Vector2 areaTopLeft;
Vector2 areaBottomRight;

static double totalTime;
static double deltaTime;

int main() {
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
    InitWindow(screenWidth * windowScale, screenHeight * windowScale, "Circle vs Rectangle Collision");

    SetupApplicationSurface(screenWidth, screenHeight, WHITE);
    SetTextureFilter(GetFontDefault().texture, TEXTURE_FILTER_POINT);
    SetExitKey(KEY_ESCAPE);

    InitGame();

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 60, 1)
    #else
        SetTargetFPS(60);

        while (!WindowShouldClose())    // Detect window close button or ESC key
        {
            UpdateDrawFrame();
        }
    #endif
    
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context

    return 0;
}

void InitGame(void)
{
    SetupGame();
}

void SetupGame(void) 
{
    camera.target = Vector2Zero();
    camera.offset = Vector2Zero();
    camera.zoom = 20;
    camera.rotation = 0;
    
    object.pos = (Vector2) {3.0f, 3.0f};
    object.vel = (Vector2) {0.0f, 0.0f};
    object.radius = 0.5f;
    potentialPosition = object.pos;
}

void UpdateGame(void)
{
    deltaTime = GetFrameTime();
    totalTime += deltaTime;

    if (followObject) {
        camera.target.x = Lerp(camera.target.x, object.pos.x - 15.5, deltaTime * 4.0f);
        camera.target.y = Lerp(camera.target.y, object.pos.y - 12, deltaTime * 4.0f);
    } else {
        if (camera.target.x != 0 && camera.target.y != 0) {
            camera.target = Vector2Lerp(camera.target, Vector2Zero(), deltaTime * 10.0f);
            camera.offset = Vector2Zero();
        }
    }

    object.vel = (Vector2) {0.0f, 0.0f};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) object.vel.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) object.vel.y += 1.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) object.vel.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) object.vel.x += 1.0f;

    if (Vector2LengthSqr(object.vel) > 0) {
        object.vel = Vector2Scale(Vector2Normalize(object.vel), (IsKeyDown(KEY_LEFT_SHIFT) ? 5.0f : 2.0f));
    }

    if (IsKeyReleased(KEY_SPACE)) followObject = !followObject;

    // Where will object be worst case?
    potentialPosition.x = object.pos.x + object.vel.x * deltaTime;
    potentialPosition.y = object.pos.y + object.vel.y * deltaTime;

    // Extract region of world cells that could have collision this frame
    Vector2 currentCell = Vector2Int(Vector2Floor(object.pos));
    Vector2 targetCell  = Vector2Int(potentialPosition);
    areaTopLeft.x = (fminf(currentCell.x, targetCell.x) - fmaxf(1, 0));
    areaTopLeft.y = (fminf(currentCell.y, targetCell.y) - fmaxf(1, 0));
    areaBottomRight.x = (fmaxf(currentCell.x, targetCell.x) + fminf(1, worldSize.x));
    areaBottomRight.y = (fmaxf(currentCell.y, targetCell.y) + fminf(1, worldSize.y));

    Vector2 rayToNearest;

    // Iterate through each cell in test area
    Vector2 cell;
    for (cell.y = areaTopLeft.y; cell.y <= areaBottomRight.y; cell.y++)
    {
        for (cell.x = areaTopLeft.x; cell.x <= areaBottomRight.x; cell.x++)
        {
            // Check if the cell is actually solid...
            if (worldMap[(int)(cell.y * worldSize.x + cell.x)] == '#')
            {
                // ...it is! So work out nearest point to future player position, around perimeter
                // of cell rectangle. We can test the distance to this point to see if we have
                // collided. 

                Vector2 nearestPoint;
                nearestPoint.x = fmaxf((float)(cell.x), fminf(potentialPosition.x, (float)(cell.x + 1)));
                nearestPoint.y = fmaxf((float)(cell.y), fminf(potentialPosition.y, (float)(cell.y + 1)));

                rayToNearest = Vector2Subtract(nearestPoint, potentialPosition);
                float overlap = object.radius - Vector2Length(rayToNearest);
                if (isnan(overlap)) overlap = 0;

                // If overlap is positive, then a collision has occurred, so we displace backwards by the 
                // overlap amount. The potential position is then tested against other tiles in the area
                // therefore "statically" resolving the collision
                if (overlap > 0)
                {
                    // Statically resolve the collision
                    potentialPosition = Vector2Subtract(potentialPosition, Vector2Scale(Vector2Normalize(rayToNearest), overlap));
                }
            }
        }
    }

    // Set the objects new position to the allowed potential position
    object.pos = potentialPosition;

    if (IsKeyPressed(KEY_R)) SetupGame();
}

void DrawGame(void)
{
    BeginDrawing();
            
	BeginTextureMode(applicationSurface);

        BeginMode2D(camera);

        ClearBackground(DARKBLUE);

		if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) camera.offset = Vector2Add(camera.offset, GetMouseDelta());
		if (GetMouseWheelMove() > 0) camera.zoom += 1 * camera.zoom * 0.2;
		if (GetMouseWheelMove() < 0) camera.zoom -= 1 * camera.zoom * 0.2;

        camera.zoom = Clamp(camera.zoom, 1, 100);

        Vector2 topLeft = Vector2Int(Vector2Zero());
		Vector2 bottomRight = Vector2Int((Vector2){worldSize.x, worldSize.y});
		Vector2 tile;
		for (tile.y = topLeft.y; tile.y < bottomRight.y; tile.y++)
			for (tile.x = topLeft.x; tile.x < bottomRight.x; tile.x++)
			{
				if (worldMap[(int)(tile.y * worldSize.x + tile.x)] == '#')
				{
					DrawLineV(tile, Vector2Add(tile, (Vector2){1.0f, 0.0f}), WHITE);
					DrawLineV(tile, Vector2Add(tile, (Vector2){0.0f, 1.0f}), WHITE);
					DrawLineV(Vector2Add(tile, (Vector2) {0.0f, 1.0f}), Vector2Add(tile, Vector2One()), WHITE);
					DrawLineV(Vector2Add(tile, (Vector2) {1.0f, 0.0f}), Vector2Add(tile, Vector2One()), WHITE);

					DrawLineV(tile, Vector2Add(tile, Vector2One()), WHITE);
					DrawLineV(Vector2Add(tile, (Vector2) {0.0f, 1.0f}), Vector2Add(tile, (Vector2){1.0f, 0.0f}), WHITE);
				}
			}

		DrawRectangle(areaTopLeft.x, areaTopLeft.y, areaBottomRight.x - areaTopLeft.x + 1, areaBottomRight.y - areaTopLeft.y + 1, GetColor(0x00FFFF32));


        // Draw Boundary
        DrawCircleV(object.pos, object.radius, WHITE);

		// Draw Velocity
		if (Vector2LengthSqr(object.vel) > 0)
		{
			DrawLineV(object.pos, Vector2Add(object.pos, Vector2Scale(Vector2Normalize(object.vel), object.radius)), MAGENTA);
		}

       
        EndTextureMode();

        RenderApplicationSurface();

        if (followObject)
		{
			DrawText("Following Object", 10, 10, 10, WHITE);

			DrawText(TextFormat("position:%2.2f", object.pos), 10, 20, 10, BLACK);
			DrawText(TextFormat("potentialPosition:%2.2f", potentialPosition), 10, 30, 10, BLACK);
			DrawText(TextFormat("velocity:%2.2f", object.vel), 10, 40, 10, BLACK);
		} 
	
        EndMode2D();
	
    EndDrawing();
}

void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)    
    DestroyApplicationSurface();
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

