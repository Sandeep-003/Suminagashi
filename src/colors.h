#ifndef COLOR_H
#define COLOR_H

#include <vector>
#include <array>

class ColorPalette {
public:
    using ColorValue = std::array<int, 4>; // RGBA
    using Palette = std::vector<ColorValue>;

    // Static palettes shared by all instances
    static std::vector<Palette> palettes;
    static bool initialized;

    // Initialize predefined beautiful palettes
    static void initializePalettes();

    // Add a new palette
    static void addPalette(const Palette& palette);

    // Constructor: randomly select a palette
    ColorPalette();

    // Get a random color from the current palette
    ColorValue getColor();

    // Get the current palette index
    size_t getCurrentPaletteIndex() const;

    // Get the current palette
    const Palette& getCurrentPalette() const;

    // Get all palettes
    static const std::vector<Palette>& getAllPalettes();

private:
    size_t currentPaletteIndex;
    static int randSeed;
};

#endif // COLOR_H