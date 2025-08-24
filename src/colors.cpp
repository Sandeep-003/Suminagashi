#include "colors.h"
#include <random>
#include <ctime>

// Static member definitions
std::vector<ColorPalette::Palette> ColorPalette::palettes;
bool ColorPalette::initialized = false;
int ColorPalette::randSeed = 0;

// Initialize predefined beautiful palettes
void ColorPalette::initializePalettes() {
    if (initialized) return;
    
    // Sunset Palette
    palettes.push_back({
        {255, 94, 77, 255},   // Coral
        {255, 154, 0, 255},   // Orange
        {255, 206, 84, 255},  // Golden
        {255, 138, 101, 255}, // Peach
        {240, 98, 146, 255}   // Pink
    });
    
    // Ocean Palette
    palettes.push_back({
        {72, 202, 228, 255},  // Sky Blue
        {0, 119, 190, 255},   // Deep Blue
        {0, 180, 216, 255},   // Cyan
        {144, 224, 239, 255}, // Light Blue
        {33, 158, 188, 255}   // Teal
    });
    
    // Forest Palette
    palettes.push_back({
        {46, 125, 50, 255},   // Forest Green
        {102, 187, 106, 255}, // Light Green
        {165, 214, 167, 255}, // Mint
        {67, 56, 202, 255},   // Purple accent
        {139, 195, 74, 255}   // Lime
    });
    
    // Autumn Palette
    palettes.push_back({
        {212, 84, 0, 255},    // Burnt Orange
        {239, 154, 154, 255}, // Light Red
        {255, 183, 77, 255},  // Amber
        {141, 110, 99, 255},  // Brown
        {205, 220, 57, 255}   // Yellow Green
    });
    
    // Pastel Palette
    palettes.push_back({
        {240, 98, 146, 255},  // Pink
        {174, 213, 129, 255}, // Light Green
        {149, 117, 205, 255}, // Lavender
        {255, 183, 77, 255},  // Peach
        {100, 181, 246, 255}  // Sky Blue
    });
    
    // Traditional Japanese Suminagashi
    palettes.push_back({
        {45, 55, 72, 255},    // Indigo
        {74, 85, 104, 255},   // Slate Blue
        {160, 174, 192, 255}, // Light Gray Blue
        {237, 242, 247, 255}, // Almost White
        {203, 213, 224, 255}  // Silver
    });
    
    // Turkish Ebru Classic
    palettes.push_back({
        {184, 59, 94, 255},   // Deep Rose
        {52, 73, 94, 255},    // Dark Blue
        {241, 196, 15, 255},  // Golden Yellow
        {231, 76, 60, 255},   // Red
        {46, 204, 113, 255}   // Emerald Green
    });
    
    // Venetian Marble
    palettes.push_back({
        {139, 69, 19, 255},   // Saddle Brown
        {205, 133, 63, 255},  // Peru
        {222, 184, 135, 255}, // Burlywood
        {245, 222, 179, 255}, // Wheat
        {255, 228, 196, 255}  // Bisque
    });
    
    // Royal Purple & Gold
    palettes.push_back({
        {75, 0, 130, 255},    // Indigo
        {138, 43, 226, 255},  // Blue Violet
        {218, 165, 32, 255},  // Goldenrod
        {255, 215, 0, 255},   // Gold
        {255, 239, 213, 255}  // Papaya Whip
    });
    
    // Midnight Blues
    palettes.push_back({
        {25, 25, 112, 255},   // Midnight Blue
        {72, 61, 139, 255},   // Dark Slate Blue
        {106, 90, 205, 255},  // Slate Blue
        {147, 112, 219, 255}, // Medium Purple
        {221, 160, 221, 255}  // Plum
    });
    
    // Earth Tones
    palettes.push_back({
        {160, 82, 45, 255},   // Saddle Brown
        {210, 180, 140, 255}, // Tan
        {238, 203, 173, 255}, // Navajo White
        {245, 245, 220, 255}, // Beige
        {112, 128, 144, 255}  // Slate Gray
    });
    
    // Cherry Blossom
    palettes.push_back({
        {255, 182, 193, 255}, // Light Pink
        {255, 105, 180, 255}, // Hot Pink
        {219, 112, 147, 255}, // Pale Violet Red
        {255, 228, 225, 255}, // Misty Rose
        {255, 240, 245, 255}  // Lavender Blush
    });
    
    // Jade Garden
    palettes.push_back({
        {0, 128, 128, 255},   // Teal
        {32, 178, 170, 255},  // Light Sea Green
        {127, 255, 212, 255}, // Aquamarine
        {175, 238, 238, 255}, // Pale Turquoise
        {240, 248, 255, 255}  // Alice Blue
    });
    
    // Copper & Verdigris
    palettes.push_back({
        {184, 115, 51, 255},  // Dark Goldenrod
        {205, 127, 50, 255},  // Chocolate
        {64, 224, 208, 255},  // Turquoise
        {72, 209, 204, 255},  // Medium Turquoise
        {175, 238, 238, 255}  // Pale Turquoise
    });
    
    // Wine & Cream
    palettes.push_back({
        {128, 0, 32, 255},    // Dark Red
        {165, 42, 42, 255},   // Brown
        {205, 92, 92, 255},   // Indian Red
        {250, 235, 215, 255}, // Antique White
        {255, 248, 220, 255}  // Cornsilk
    });
    
    initialized = true;
}

// Add a new palette
void ColorPalette::addPalette(const Palette& palette) {
    initializePalettes();
    palettes.push_back(palette);
}

// Constructor: randomly select a palette
ColorPalette::ColorPalette() {
    initializePalettes();
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)) + randSeed++);
    std::uniform_int_distribution<size_t> paletteDist(0, palettes.size() - 1);
    currentPaletteIndex = paletteDist(rng);
}

// Get a random color from the current palette
ColorPalette::ColorValue ColorPalette::getColor() {
    std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)) + randSeed++);
    std::uniform_int_distribution<size_t> colorDist(0, palettes[currentPaletteIndex].size() - 1);
    size_t colorIndex = colorDist(rng);
    return palettes[currentPaletteIndex][colorIndex];
}

// Get the current palette index
size_t ColorPalette::getCurrentPaletteIndex() const { 
    return currentPaletteIndex; 
}

// Get the current palette
const ColorPalette::Palette& ColorPalette::getCurrentPalette() const { 
    return palettes[currentPaletteIndex]; 
}

// Get all palettes
const std::vector<ColorPalette::Palette>& ColorPalette::getAllPalettes() { 
    initializePalettes();
    return palettes; 
}