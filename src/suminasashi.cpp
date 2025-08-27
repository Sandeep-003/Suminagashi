

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
// Interaction modes (numeric for future extensibility)
// 0 = drops, 1 = tine
static int interactionMode = 0;
// Create a new ColorPalette instance and get a random color from current palette
ColorPalette colorGenerator;
// User-selected parameters for next drop (instead of random)
static int nextDropRadius = 80; // default radius
static color nextDropColor(255,255,255,255); // will be initialized from palette on runtime init

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
extern "C" {
  void toggleTineMode(int on) { interactionMode = (on?1:0); }
  // Set interaction mode explicitly (0 drops, 1 tine, others reserved)
  void setInteractionMode(int mode) { if (mode < 0) mode = 0; if (mode > 10) mode = 10; interactionMode = mode; }
  // Directly apply a tine at x (in screen coords) with optional strength & sharpness (radius param reused as sharpness)
  void applyTineAt(float x, float strength, float sharpness) {
    for (auto &d : drops) d.applyVerticalTine(x, strength, sharpness, true);
  }
  void setTineParams(float strength, float sharpness) { /* kept for backward compatibility but unused now */ }
  // Setters for next drop parameters
  void setNextDropRadius(int r) { if (r < 5) r = 5; if (r > 400) r = 400; nextDropRadius = r; }
  void setNextDropColor(int r, int g, int b, int a) {
    if (r<0) r=0; if (r>255) r=255; if (g<0) g=0; if (g>255) g=255; if (b<0) b=0; if (b>255) b=255; if (a<0) a=0; if (a>255) a=255;
    nextDropColor = color(r,g,b,a);
  }
  // Palette query helpers (pack RGBA into 32-bit int: 0xRRGGBBAA)
  int getCurrentPaletteSize() {
    const auto &p = colorGenerator.getCurrentPalette();
    return (int)p.size();
  }
  int getCurrentPaletteColor(int idx) {
    const auto &p = colorGenerator.getCurrentPalette();
    if (idx < 0 || idx >= (int)p.size()) return 0xFFFFFFFF; // default white
    auto c = p[idx];
    return ((c[0] & 0xFF) << 24) | ((c[1] & 0xFF) << 16) | ((c[2] & 0xFF) << 8) | (c[3] & 0xFF);
  }
}

int main(void)
{
  // Following function causes mouse click to not work properly. I don't know why
  // SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
  InitWindow(initialScreenWidth, initialScreenHeight, "suminasashi");

  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  // Initialize default nextDropColor from current palette first color if available
  const auto &cp = colorGenerator.getCurrentPalette();
  if (!cp.empty()) {
    auto c = cp[0];
    nextDropColor = color(c[0], c[1], c[2], c[3]);
  }
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

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    int mouseX = GetMouseX();
    int mouseY = GetMouseY(); // reserved for future stylus angle features
    if (interactionMode == 0) {
      // Construct drop using user-selected radius/color
      Drop d = Drop(mouseX, mouseY, nextDropColor, nextDropRadius, currentN);
      // Optional: still assign a gentle target blend to another palette color for subtle evolution
      const auto &palette = colorGenerator.getCurrentPalette();
      if (!palette.empty()) {
        // Choose a different color than selected for slow blend (optional)
        if (palette.size() > 1) {
          int pick = GetRandomValue(0, (int)palette.size() - 1);
          // Avoid identical color unless only one color
          auto csel = palette[pick];
          d.setTargetColor(color(csel[0], csel[1], csel[2], csel[3]), 0.4f);
        }
      }
      for (size_t i = 0; i < drops.size(); i++) {
        drops[i].marble(d, 1);
      }
      drops.push_back(d);
    } // mode 1 (tine) handled from JS by calling applyTineAt directly
  }

  for (int i = 0; i < drops.size(); i++) {
    float t = GetTime();
    drops[i].applyEdgeNoise(0.16f, 6.0f, t);
    drops[i].animateShape(t, 0.12f, 2.0f, 3);
    drops[i].Draw_drops();
  }

  // Display FPS and currentN
  DrawText(TextFormat("FPS: %.1f", avgFPS), 20, 20, 24, DARKGRAY);
  DrawText(interactionMode == 1 ? "Mode: Tine" : "Mode: Drops", 20, 50, 20, interactionMode == 1 ? MAROON : DARKGREEN);
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
