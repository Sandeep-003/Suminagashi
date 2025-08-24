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
public:
    Drop(float x, float y, color clr, double radius = 100, int n = 100);
    void Draw_drops();
    void update_vertices(float c_x, float c_y, double n_r);
    void wavy_transformation();
    void inserve_wavy_transformation();
    void marble(const Drop&);
};