

#include "raylib.h"
#include <iostream>
#include "drops.h"
#include "colors.h"
#include <emscripten/emscripten.h>
using namespace std;

// Initialization
//--------------------------------------------------------------------------------------
const int screenWidth = 1200;
const int screenHeight = 850;
const int N = 100;
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
  extern "C" {
    void clearCanvas() {
        // Clear your drops vector and redraw
        drops.clear();
    }
}
}

int main(void)
{

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "suminasashi");
  
  // draw();
  
  SetTargetFPS(60); // Set our game to run at 60 frames-per-second
  emscripten_set_main_loop(draw, 0, 1);
  close();
}



void draw()
{
  BeginDrawing();
  ClearBackground(RAYWHITE); 

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
  {
    int mouseX = GetMouseX(); // Get mouse position X
    int mouseY = GetMouseY(); // Get mouse position Y

    auto colorRGBA = colorGenerator.getColor();

    // Create custom color struct (from drops.h) using our palette color
    color dropColor(colorRGBA[0], colorRGBA[1], colorRGBA[2], colorRGBA[3]);

    Drop d = Drop(mouseX, mouseY, dropColor, 100, 100);
    for (size_t i = 0; i < drops.size(); i++)
    {
      drops[i].marble(d);
    }

    drops.push_back(d);
  }

  for (int i = 0; i < drops.size(); i++)
  {
    drops[i].Draw_drops();
  }

  EndDrawing();
}

void close()
{
  //--------------------------------------------------------------------------------------
  CloseWindow(); // Close window and OpenGL context
                 //--------------------------------------------------------------------------------------
}
