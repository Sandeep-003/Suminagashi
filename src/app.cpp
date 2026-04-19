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
constexpr float kDegToRad = 3.14159265358979323846f / 180.0f;

SuminagashiApp* gApp = nullptr;

void AddDropWithMarbling(std::vector<Drop>& drops, Drop drop)
{
    for (auto& existingDrop : drops)
    {
        existingDrop.marble(drop, true);
    }
    drops.push_back(drop);
}

color PickRandomCurrentPaletteColor(const ColorPalette& paletteSource)
{
    const auto& palette = paletteSource.getCurrentPalette();
    if (palette.empty())
    {
        return color(255, 255, 255, 255);
    }

    const int colorIndex = GetRandomValue(0, static_cast<int>(palette.size()) - 1);
    const auto& colorValue = palette[static_cast<size_t>(colorIndex)];
    return color(colorValue[0], colorValue[1], colorValue[2], colorValue[3]);
}

color PickFullyRandomColor()
{
    return color(GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255);
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
    int dropRadius = nextDropRadius;
    color dropColor = nextDropColor;

    if (interactionMode == 2)
    {
        dropRadius = GetRandomValue(kMinDropRadius, kMaxDropRadius);
        dropColor = PickRandomCurrentPaletteColor(colorGenerator);
    }
    else if (interactionMode == 3)
    {
        dropRadius = GetRandomValue(kMinDropRadius, kMaxDropRadius);
        dropColor = PickFullyRandomColor();
    }

    Drop drop(static_cast<float>(mouseX), static_cast<float>(mouseY), dropColor, dropRadius, currentN);

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
        drop.setTargetColor(PickRandomCurrentPaletteColor(colorGenerator), 0.45f);
    }
    else if (interactionMode == 3)
    {
        drop.setTargetColor(PickFullyRandomColor(), 0.45f);
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
        modeText = "Mode: Random Palette";
        modeColor = BLUE;
    }
    else if (interactionMode == 3)
    {
        modeText = "Mode: Random Full";
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
    interactionMode = std::clamp(mode, 0, 3);
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

int SuminagashiApp::GetCurrentPaletteIndex() const
{
    return static_cast<int>(colorGenerator.getCurrentPaletteIndex());
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

int SuminagashiApp::GeneratePatternBloom()
{
    const int paletteCount = GetPaletteCount();
    if (paletteCount > 0)
    {
        const int randomPalette = GetRandomValue(0, paletteCount - 1);
        SetPaletteIndex(randomPalette);
    }

    drops.clear();

    const int width = std::max(1, metrics.renderWidth > 0 ? metrics.renderWidth : GetRenderWidth());
    const int height = std::max(1, metrics.renderHeight > 0 ? metrics.renderHeight : GetRenderHeight());
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    const float cx = w * 0.5f;
    const float cy = h * 0.5f;
    const float minDim = std::min(w, h);

    auto addDrop = [&](float x, float y, float radius)
    {
        if (x < -radius || x > w + radius || y < -radius || y > h + radius)
        {
            return;
        }

        const int clampedRadius = std::clamp(static_cast<int>(std::lround(radius)), kMinDropRadius, kMaxDropRadius);
        const color baseColor = PickRandomCurrentPaletteColor(colorGenerator);
        Drop drop(x, y, baseColor, clampedRadius, currentN);
        drop.setTargetColor(PickRandomCurrentPaletteColor(colorGenerator), 0.35f + static_cast<float>(GetRandomValue(0, 20)) / 100.0f);
        AddDropWithMarbling(drops, drop);
    };

    const int patternId = patternCycleIndex % 5;
    patternCycleIndex += 1;

    switch (patternId)
    {
    case 0:
    {
        const int count = 260;
        const float scale = minDim / 28.0f;
        for (int i = 0; i < count; ++i)
        {
            const float angle = static_cast<float>(i) * 137.50776f * kDegToRad;
            const float radial = scale * std::sqrt(static_cast<float>(i));
            const float x = cx + radial * std::cos(angle);
            const float y = cy + radial * std::sin(angle);
            const float radius = minDim * (0.007f + 0.028f * (0.5f + 0.5f * std::sin(static_cast<float>(i) * 0.23f)));
            addDrop(x, y, radius);
        }
        break;
    }
    case 1:
    {
        const int count = 220;
        const float phase = static_cast<float>(GetRandomValue(0, 314)) / 100.0f;
        for (int i = 0; i < count; ++i)
        {
            const float t = (static_cast<float>(i) / static_cast<float>(count - 1)) * 6.0f * 3.14159265f;
            const float x = cx + (w * 0.42f) * std::sin(3.0f * t + phase) + static_cast<float>(GetRandomValue(-14, 14));
            const float y = cy + (h * 0.34f) * std::sin(4.0f * t) + static_cast<float>(GetRandomValue(-10, 10));
            const float radius = minDim * (0.009f + 0.03f * (0.5f + 0.5f * std::cos(5.0f * t)));
            addDrop(x, y, radius);
        }
        break;
    }
    case 2:
    {
        const int ringCount = 11;
        const float ringStep = (minDim * 0.44f) / static_cast<float>(ringCount);
        for (int ring = 1; ring <= ringCount; ++ring)
        {
            const int points = 8 + ring * 4;
            for (int p = 0; p < points; ++p)
            {
                const float theta = (static_cast<float>(p) / static_cast<float>(points)) * 2.0f * 3.14159265f;
                const float wave = minDim * 0.08f * std::sin(6.0f * theta + static_cast<float>(ring) * 0.65f);
                const float distance = static_cast<float>(ring) * ringStep + wave;
                const float x = cx + distance * std::cos(theta);
                const float y = cy + distance * std::sin(theta);
                const float radius = minDim * (0.006f + 0.024f * (1.0f - static_cast<float>(ring) / static_cast<float>(ringCount + 2)));
                addDrop(x, y, radius);
            }
        }
        break;
    }
    case 3:
    {
        const int cols = 16;
        const int rows = 11;
        const float stepX = w / static_cast<float>(cols + 1);
        const float stepY = h / static_cast<float>(rows + 1);
        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                const float fx = static_cast<float>(col + 1);
                const float fy = static_cast<float>(row + 1);
                const float waveX = std::sin(fy * 0.9f + static_cast<float>(col) * 0.21f) * stepX * 0.32f;
                const float waveY = std::cos(fx * 0.8f + static_cast<float>(row) * 0.17f) * stepY * 0.32f;
                const float x = fx * stepX + waveX;
                const float y = fy * stepY + waveY;
                const float radius = minDim * (0.008f + 0.025f * (0.5f + 0.5f * std::sin((fx + fy) * 0.7f)));
                addDrop(x, y, radius);
            }
        }
        break;
    }
    case 4:
    default:
    {
        const int arms = 5;
        const int pointsPerArm = 64;
        for (int arm = 0; arm < arms; ++arm)
        {
            const float armOffset = static_cast<float>(arm) * (2.0f * 3.14159265f / static_cast<float>(arms));
            for (int i = 0; i < pointsPerArm; ++i)
            {
                const float t = static_cast<float>(i) / static_cast<float>(pointsPerArm - 1);
                const float angle = t * 7.0f * 3.14159265f + armOffset;
                const float distance = minDim * (0.06f + 0.43f * t) + minDim * 0.07f * std::sin(8.0f * t + static_cast<float>(arm));
                const float x = cx + distance * std::cos(angle);
                const float y = cy + distance * std::sin(angle);
                const float radius = minDim * (0.009f + 0.03f * (1.0f - t));
                addDrop(x, y, radius);
            }
        }
        break;
    }
    }

    return patternId;
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