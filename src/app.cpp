#include "app.h"

#include "screenshot.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
constexpr const char* kWindowTitle = "suminasashi";
constexpr int kDefaultWidth = 1200;
constexpr int kDefaultHeight = 800;
constexpr int kMinDropRadius = 5;
constexpr int kMaxDropRadius = 400;
constexpr int kFpsLow = 40;
constexpr int kFpsHigh = 60;
constexpr int kVertexMin = 200;
constexpr int kVertexMax = 600;
constexpr int kVertexStep = 10;

SuminagashiApp* gApp = nullptr;

color PickRandomPaletteColor()
{
    const auto& palettes = ColorPalette::getAllPalettes();
    if (palettes.empty())
    {
        return color(255, 255, 255, 255);
    }

    const int paletteIndex = GetRandomValue(0, static_cast<int>(palettes.size()) - 1);
    const auto& palette = palettes[static_cast<size_t>(paletteIndex)];
    if (palette.empty())
    {
        return color(255, 255, 255, 255);
    }

    const int colorIndex = GetRandomValue(0, static_cast<int>(palette.size()) - 1);
    const auto& colorValue = palette[static_cast<size_t>(colorIndex)];
    return color(colorValue[0], colorValue[1], colorValue[2], colorValue[3]);
}
} // namespace

SuminagashiApp& GetApp()
{
    if (!gApp)
    {
        gApp = new SuminagashiApp();
    }
    return *gApp;
}

SuminagashiApp::SuminagashiApp()
    : nextDropColor(255, 255, 255, 255)
{
    EnsureDefaultNextDropColor();
}

QualityMode SuminagashiApp::ClampQualityMode(int mode)
{
    if (mode <= 0)
    {
        return QualityMode::Performance;
    }
    if (mode >= 2)
    {
        return QualityMode::High;
    }
    return QualityMode::Balanced;
}

float SuminagashiApp::QualityScaleForMode(QualityMode mode)
{
    switch (mode)
    {
    case QualityMode::Performance:
        return 0.75f;
    case QualityMode::High:
        return 1.0f;
    case QualityMode::Balanced:
    default:
        return 0.9f;
    }
}

void SuminagashiApp::Initialize()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(kDefaultWidth, kDefaultHeight, kWindowTitle);
    SetTargetFPS(60);
    metrics.renderWidth = GetRenderWidth();
    metrics.renderHeight = GetRenderHeight();
    metrics.cssWidth = kDefaultWidth;
    metrics.cssHeight = kDefaultHeight;
    metrics.devicePixelRatio = 1.0f;
    metrics.qualityScale = QualityScaleForMode(qualityMode);
    EnsureDefaultNextDropColor();
    LogCanvasMetrics("init");
}

void SuminagashiApp::Shutdown()
{
    if (IsWindowReady())
    {
        CloseWindow();
    }
}

void SuminagashiApp::EnsureDefaultNextDropColor()
{
    const auto& palette = colorGenerator.getCurrentPalette();
    if (!palette.empty())
    {
        const auto colorValue = palette.front();
        nextDropColor = color(colorValue[0], colorValue[1], colorValue[2], colorValue[3]);
    }
}

void SuminagashiApp::HandleInput()
{
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || interactionMode == 1)
    {
        return;
    }

    const int mouseX = GetMouseX();
    const int mouseY = GetMouseY();
    color dropColor = nextDropColor;
    if (interactionMode == 2)
    {
        dropColor = PickRandomPaletteColor();
    }

    Drop drop(static_cast<float>(mouseX), static_cast<float>(mouseY), dropColor, nextDropRadius, currentN);

    if (interactionMode == 0)
    {
        const auto& palette = colorGenerator.getCurrentPalette();
        if (palette.size() > 1)
        {
            const int pick = GetRandomValue(0, static_cast<int>(palette.size()) - 1);
            const auto& selected = palette[static_cast<size_t>(pick)];
            drop.setTargetColor(color(selected[0], selected[1], selected[2], selected[3]), 0.4f);
        }
    }
    else if (interactionMode == 2)
    {
        drop.setTargetColor(PickRandomPaletteColor(), 0.45f);
    }

    for (auto& existingDrop : drops)
    {
        existingDrop.marble(drop, true);
    }

    drops.push_back(drop);
    (void)mouseY;
}

void SuminagashiApp::UpdateAdaptiveVertexCount(float fps)
{
    fpsHistory[static_cast<size_t>(fpsIndex)] = fps;
    fpsIndex = (fpsIndex + 1) % static_cast<int>(fpsHistory.size());

    float total = 0.0f;
    for (const float sample : fpsHistory)
    {
        total += sample;
    }
    averageFps = total / static_cast<float>(fpsHistory.size());

    if (averageFps < kFpsLow && currentN > kVertexMin)
    {
        currentN = std::max(kVertexMin, currentN - kVertexStep);
    }
    if (averageFps > kFpsHigh && currentN < kVertexMax)
    {
        currentN = std::min(kVertexMax, currentN + kVertexStep);
    }
}

void SuminagashiApp::UpdateDrops()
{
    const float time = static_cast<float>(GetTime());
    for (auto& drop : drops)
    {
        drop.applyEdgeNoise(0.16f, 6.0f, time);
        drop.animateShape(time, 0.12f, 2.0f, 3);
        drop.Draw_drops();
    }
}

void SuminagashiApp::DrawFrame()
{
    if (!IsWindowReady())
    {
        return;
    }

    if (IsWindowResized())
    {
        metrics.renderWidth = GetRenderWidth();
        metrics.renderHeight = GetRenderHeight();
        LogCanvasMetrics("resize");
    }

    HandleInput();
    UpdateAdaptiveVertexCount(GetFPS());

    BeginDrawing();
    ClearBackground(RAYWHITE);
    UpdateDrops();
    DrawText(TextFormat("FPS: %.1f", averageFps), 20, 20, 24, DARKGRAY);
    const char* modeText = "Mode: Drops";
    Color modeColor = DARKGREEN;
    if (interactionMode == 1)
    {
        modeText = "Mode: Tine";
        modeColor = MAROON;
    }
    else if (interactionMode == 2)
    {
        modeText = "Mode: Random";
        modeColor = BLUE;
    }
    DrawText(modeText, 20, 50, 20, modeColor);
    EndDrawing();
}

void SuminagashiApp::ClearCanvas()
{
    drops.clear();
}

void SuminagashiApp::ToggleTineMode(int on)
{
    interactionMode = on ? 1 : 0;
}

void SuminagashiApp::SetInteractionMode(int mode)
{
    interactionMode = std::clamp(mode, 0, 2);
}

void SuminagashiApp::ApplyTineAt(float x, float strength, float sharpness)
{
    for (auto& drop : drops)
    {
        drop.applyVerticalTine(x, strength, sharpness, true);
    }
}

void SuminagashiApp::SetTineParams(float, float)
{
}

void SuminagashiApp::SetNextDropRadius(int radius)
{
    nextDropRadius = std::clamp(radius, kMinDropRadius, kMaxDropRadius);
}

void SuminagashiApp::SetNextDropColor(int r, int g, int b, int a)
{
    nextDropColor = color(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), std::clamp(a, 0, 255));
}

int SuminagashiApp::GetCurrentPaletteSize() const
{
    return static_cast<int>(colorGenerator.getCurrentPalette().size());
}

int SuminagashiApp::GetCurrentPaletteColor(int index) const
{
    const auto& palette = colorGenerator.getCurrentPalette();
    if (index < 0 || index >= static_cast<int>(palette.size()))
    {
        return 0xFFFFFFFF;
    }

    const auto colorValue = palette[static_cast<size_t>(index)];
    return ((colorValue[0] & 0xFF) << 24) | ((colorValue[1] & 0xFF) << 16) | ((colorValue[2] & 0xFF) << 8) | (colorValue[3] & 0xFF);
}

int SuminagashiApp::GetPaletteCount() const
{
    return static_cast<int>(colorGenerator.getPaletteCount());
}

void SuminagashiApp::SetPaletteIndex(int index)
{
    colorGenerator.setCurrentPaletteIndex(static_cast<size_t>(std::max(index, 0)));
    EnsureDefaultNextDropColor();
}

void SuminagashiApp::SetQualityMode(int mode)
{
    qualityMode = ClampQualityMode(mode);
}

int SuminagashiApp::GetQualityMode() const
{
    return static_cast<int>(qualityMode);
}

float SuminagashiApp::GetQualityScale() const
{
    return QualityScaleForMode(qualityMode);
}

void SuminagashiApp::SyncCanvasViewport(int cssWidth, int cssHeight, float devicePixelRatio, float qualityScale)
{
    metrics.cssWidth = std::max(1, cssWidth);
    metrics.cssHeight = std::max(1, cssHeight);
    metrics.devicePixelRatio = std::max(1.0f, devicePixelRatio);
    metrics.qualityScale = std::max(0.25f, qualityScale);

    const int nextRenderWidth = std::max(1, static_cast<int>(std::lround(metrics.cssWidth * metrics.devicePixelRatio * metrics.qualityScale)));
    const int nextRenderHeight = std::max(1, static_cast<int>(std::lround(metrics.cssHeight * metrics.devicePixelRatio * metrics.qualityScale)));

    if (nextRenderWidth == metrics.renderWidth && nextRenderHeight == metrics.renderHeight)
    {
        return;
    }

    metrics.renderWidth = nextRenderWidth;
    metrics.renderHeight = nextRenderHeight;

    if (IsWindowReady())
    {
        SetWindowSize(metrics.renderWidth, metrics.renderHeight);
    }

    LogCanvasMetrics("sync");
}

void SuminagashiApp::RequestNativeScreenshot()
{
    const std::string filename = MakeTimestampedScreenshotName("suminagashi");
    if (!SaveNativeScreenshot(filename))
    {
        std::cout << "[screenshot] failed to save " << filename << std::endl;
        return;
    }

    std::cout << "[screenshot] saved " << filename << std::endl;
}

void SuminagashiApp::LogCanvasMetrics(const char* reason) const
{
    std::cout << "[layout] " << reason
              << " css=" << metrics.cssWidth << "x" << metrics.cssHeight
              << " render=" << metrics.renderWidth << "x" << metrics.renderHeight
              << " dpr=" << metrics.devicePixelRatio
              << " quality=" << metrics.qualityScale
              << " mode=" << static_cast<int>(qualityMode)
              << std::endl;
}