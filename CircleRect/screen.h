#ifndef SCREEN_H
#define SCREEN_H

#include <stdio.h>
#include <raylib.h>

typedef struct screenData {
    int width;
    int height;
    Color clearColor;
} screenData;


static screenData ScreenData;

RenderTexture applicationSurface;

static Vector2 leftOvers;

Vector2 GetApplicationSurfaceLeftovers() {
    return leftOvers;
}

void SetupApplicationSurface(int _width, int _height, Color _clearColor) {
    printf("Creating application surface");
    ScreenData.width = _width;
    ScreenData.height = _height;
    ScreenData.clearColor = _clearColor;

    applicationSurface = LoadRenderTexture(ScreenData.width, ScreenData.height);
}

void DestroyApplicationSurface() {
    printf("Destroying Application Surface");
    UnloadRenderTexture(applicationSurface);
}

void RenderApplicationSurface() {
    //BeginDrawing();

    ClearBackground(ScreenData.clearColor);

    float screenRatio = (float)GetScreenWidth() / (float)GetScreenHeight();
    float textureRatio = (float)ScreenData.width/(float)ScreenData.height;

    float scalar = 0.0f;
    Vector2 drawScale;

    if (screenRatio > textureRatio) {
        scalar = (float)GetScreenHeight() / (float)ScreenData.height;
    } else {
        scalar = (float)GetScreenWidth() / (float)ScreenData.width;
    }

    drawScale = (Vector2) { (float)ScreenData.width*scalar, (float)ScreenData.height*scalar };

    Vector2 leftOver = {
        GetScreenWidth() - drawScale.x,
        GetScreenHeight() - drawScale.y
    };

    leftOvers = leftOver;

    Vector2 tiling = {1.0f, -1.0f};
    Vector2 offset = {0.0f, 0.0f};
    Rectangle quad = {leftOver.x/2, leftOver.y/2, drawScale.x, drawScale.y};
    DrawTextureQuad(applicationSurface.texture, tiling, offset, quad, WHITE);

    //EndDrawing();
}

#endif
