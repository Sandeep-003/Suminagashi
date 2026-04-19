#include "app.h"

extern "C"
{
    void display(void)
    {
        GetApp().DrawFrame();
    }

    void takeScreenshot(void)
    {
        GetApp().RequestNativeScreenshot();
    }

    void clearCanvas(void)
    {
        GetApp().ClearCanvas();
    }

    void toggleTineMode(int on)
    {
        GetApp().ToggleTineMode(on);
    }

    void setInteractionMode(int mode)
    {
        GetApp().SetInteractionMode(mode);
    }

    void applyTineAt(float x, float strength, float sharpness)
    {
        GetApp().ApplyTineAt(x, strength, sharpness);
    }

    void setTineParams(float strength, float sharpness)
    {
        GetApp().SetTineParams(strength, sharpness);
    }

    void setNextDropRadius(int radius)
    {
        GetApp().SetNextDropRadius(radius);
    }

    void setNextDropColor(int r, int g, int b, int a)
    {
        GetApp().SetNextDropColor(r, g, b, a);
    }

    int getCurrentPaletteSize(void)
    {
        return GetApp().GetCurrentPaletteSize();
    }

    int getCurrentPaletteColor(int index)
    {
        return GetApp().GetCurrentPaletteColor(index);
    }

    int getCurrentPaletteIndex(void)
    {
        return GetApp().GetCurrentPaletteIndex();
    }

    int generatePatternBloom(void)
    {
        return GetApp().GeneratePatternBloom();
    }

    void syncCanvasViewport(int cssWidth, int cssHeight, float devicePixelRatio, float qualityScale)
    {
        GetApp().SyncCanvasViewport(cssWidth, cssHeight, devicePixelRatio, qualityScale);
    }

    void setQualityMode(int mode)
    {
        GetApp().SetQualityMode(mode);
    }

    int getQualityMode(void)
    {
        return GetApp().GetQualityMode();
    }

    int getPaletteCount(void)
    {
        return GetApp().GetPaletteCount();
    }

    void setPaletteIndex(int index)
    {
        GetApp().SetPaletteIndex(index);
    }
}