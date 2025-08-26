#pragma once
#include "raylib.h"
#include "rlgl.h"
#include <vector>
#include <iostream>
#include <cmath>

#ifndef PI
#define PI 3.14159265358979323846
#endif

typedef struct color
{
    int r, g, b,a;
    color(float r = 255, float g = 255, float b = 255,float a = 255)
    {
        this->r = r, this->g = g, this->b = b,this->a = a;
    }
} color;

class Drop
{
private:
    Vector2 center;
    double radius;
    color clr;
    int n;
    std::vector<Vector2> vertices;
    // Store the base (original) circular vertices so we can apply time-based
    // procedural deformations each frame without accumulating floating error.
    std::vector<Vector2> baseVertices;
    // Target color to blend toward (optional)
    color targetClr;
    bool hasTarget = false;
    // Starting color when a target is assigned
    color startClr;
    // Accumulated blend factor (0..maxBlend)
    float blendAccum = 0.0f;
    // Maximum fraction toward target (e.g. 0.6 keeps 40% original)
    float maxBlend = 0.6f;
public:
    Drop(float x, float y, color clr, double radius = 100, int n = 100);
    void Draw_drops();
    void update_vertices(float c_x, float c_y, double n_r);
    void wavy_transformation();
    void inserve_wavy_transformation();
    // Apply marble distortion due to another drop. If commitBaseShape is true
    // (default) the resulting geometry becomes the new base for future
    // procedural (noise/animation) effects.
    void marble(const Drop& other, bool commitBaseShape = true);
    // Copy current vertices into baseVertices (use after permanent geometry changes)
    void commitBase();
    // Restore vertices from baseVertices (useful for experimenting with temporary deformations)
    void resetToBase();
    // Apply simple value-noise based radial perturbation to edge.
    // amplitude: fraction of radius (e.g. 0.1 = 10% variation)
    // frequency: controls number of bumps around circumference
    // time: animated seed (seconds)
    void applyEdgeNoise(float amplitude, float frequency, float time);
    // Blend current color toward target color by factor t (0..1). Also can adjust alpha.
    void blendColor(const color& target, float t);
    // Animate shape with sinusoidal waves (fluid ripple feel).
    // time: seconds, amplitude: fraction of radius, speed: phase speed, harmonics: number of sine layers.
    void animateShape(float time, float amplitude, float speed, int harmonics = 1);
    // Set transparency (0..255)
    void setAlpha(int a) { clr.a = a; }
    // Assign a target color for gradual blending
    void setTargetColor(const color& c, float maxBlendAmount = 0.6f) {
        targetClr = c; hasTarget = true; startClr = clr; blendAccum = 0.0f; maxBlend = maxBlendAmount; if (maxBlend < 0) maxBlend = 0; if (maxBlend > 1) maxBlend = 1; }
    // Per-frame update toward target color using small blend step (e.g. 0.02f)
    void updateColor(float step);
};