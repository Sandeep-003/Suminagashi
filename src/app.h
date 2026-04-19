#pragma once

#include "colors.h"
#include "drops.h"
#include "raylib.h"

#include <array>
#include <string>
#include <vector>

enum class QualityMode
{
    Performance = 0,
    Balanced = 1,
    High = 2
};

struct CanvasMetrics
{
    int cssWidth = 1200;
    int cssHeight = 800;
    int renderWidth = 1200;
    int renderHeight = 800;
    float devicePixelRatio = 1.0f;
    float qualityScale = 1.0f;
};

class SuminagashiApp
{
public:
    SuminagashiApp();

    void Initialize();
    void Shutdown();
    void DrawFrame();

    void ClearCanvas();
    void ToggleTineMode(int on);
    void SetInteractionMode(int mode);
    void ApplyTineAt(float x, float strength, float sharpness);
    void SetTineParams(float strength, float sharpness);
    void SetNextDropRadius(int radius);
    void SetNextDropColor(int r, int g, int b, int a);

    int GetCurrentPaletteSize() const;
    int GetCurrentPaletteColor(int index) const;
    int GetCurrentPaletteIndex() const;
    int GetPaletteCount() const;
    void SetPaletteIndex(int index);
    int GeneratePatternBloom();

    void SetQualityMode(int mode);
    int GetQualityMode() const;
    float GetQualityScale() const;

    void SyncCanvasViewport(int cssWidth, int cssHeight, float devicePixelRatio, float qualityScale);
    void RequestNativeScreenshot();

private:
    void HandleInput();
    void UpdateDrops();
    void UpdateAdaptiveVertexCount(float fps);
    void EnsureDefaultNextDropColor();
    void LogCanvasMetrics(const char* reason) const;
    static QualityMode ClampQualityMode(int mode);
    static float QualityScaleForMode(QualityMode mode);

    ColorPalette colorGenerator;
    std::vector<Drop> drops;
    CanvasMetrics metrics;
    std::array<float, 30> fpsHistory{};
    int fpsIndex = 0;
    float averageFps = 60.0f;
    int currentN = 200;
    int interactionMode = 0;
    int nextDropRadius = 80;
    int patternCycleIndex = 0;
    color nextDropColor;
    QualityMode qualityMode = QualityMode::Balanced;
};

SuminagashiApp& GetApp();