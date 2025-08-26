

#include "raylib.h"
#include <iostream>
#include "drops.h"
#include "colors.h"
#include <emscripten/emscripten.h>
using namespace std;

// Initialization
//--------------------------------------------------------------------------------------
const int margin = 40;
const int initialScreenWidth = 1200; // Default initial width
const int initialScreenHeight = 800; // Default initial height
// Adaptive vertex count for drops based on FPS
int currentN = 200; // start value
const int N_MIN = 200;
const int N_MAX = 600;
const int FPS_TARGET = 50;
const int FPS_LOW = 40;
const int FPS_HIGH = 60;
float fpsHistory[30] = {0};
int fpsIndex = 0;
float avgFPS = 60.0f;
bool f = 0;
vector<Drop> drops;
// Create a new ColorPalette instance and get a random color from current palette
ColorPalette colorGenerator;

void InitSetup();
void close();
void draw();

//
extern "C"
{
  void display(void)
  {
    std::cout << "Hello sandeep ! I'm being executed by C++";
  }
}
extern "C"
{
  void takeScreenshot()
  {
    TakeScreenshot("suminagashi_screenshot.png");
  }
}
extern "C"
{
  void clearCanvas()
  {
    // Clear your drops vector and redraw
    drops.clear();
  }
}

int main(void)
{
  // Following function causes mouse click to not work properly. I don't know why
  // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(initialScreenWidth, initialScreenHeight, "suminasashi");

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  emscripten_set_main_loop(draw, 0, 1);
  close();
}

void draw()
{

  BeginDrawing();
  ClearBackground(RAYWHITE);

  // FPS tracking
  float fps = GetFPS();
  fpsHistory[fpsIndex] = fps;
  fpsIndex = (fpsIndex + 1) % 30;
  float sum = 0.0f;
  for (int i = 0; i < 30; ++i) sum += fpsHistory[i];
  avgFPS = sum / 30.0f;

  // Adapt N for new drops based on FPS
  if (avgFPS < FPS_LOW && currentN > N_MIN) currentN -= 10;
  if (avgFPS > FPS_HIGH && currentN < N_MAX) currentN += 10;
  if (currentN < N_MIN) currentN = N_MIN;
  if (currentN > N_MAX) currentN = N_MAX;

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    int mouseX = GetMouseX();
    int mouseY = GetMouseY();
    auto colorRGBA = colorGenerator.getColor();
    color dropColor(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);
    int radius = GetRandomValue(30, 120);
    Drop d = Drop(mouseX, mouseY, dropColor, radius, currentN);
    const auto &palette = colorGenerator.getCurrentPalette();
    if (!palette.empty()) {
      auto target = palette[GetRandomValue(0, (int)palette.size() - 1)];
      d.setTargetColor(color(target[0], target[1], target[2], target[3]), 0.6f);
    }
    for (size_t i = 0; i < drops.size(); i++) {
      drops[i].marble(d, 1);
    }
    drops.push_back(d);
  }

  for (int i = 0; i < drops.size(); i++) {
    float t = GetTime();
    drops[i].applyEdgeNoise(0.16f, 6.0f, t);
    drops[i].animateShape(t, 0.12f, 2.0f, 3);
    drops[i].Draw_drops();
  }

  // Display FPS and currentN
  DrawText(TextFormat("FPS: %.1f", avgFPS), 20, 20, 24, DARKGRAY);
  // DrawText(TextFormat("Vertices per drop: %d", currentN), 20, 50, 24, DARKGRAY);

  // Optionally, draw something that adapts to screenWidth/screenHeight
  // Example: DrawRectangle(margin, margin, screenWidth - 2*margin, screenHeight - 2*margin, LIGHTGRAY);

  EndDrawing();
}

void close()
{
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
                 //--------------------------------------------------------------------------------------
}
