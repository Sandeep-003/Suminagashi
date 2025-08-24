#include "drops.h"

Drop::Drop(float x, float y, color clr, double radius, int n)
{
    this->clr = clr;
    center = {(float)x, (float)y};
    this->radius = radius;
    this->n = n;
    for (int i = 0; i <= n; i++)
    {
        double angle = (2.0 * PI * i) / n;
        vertices.push_back({(float)(center.x + cos(angle) * radius), (float)(center.y + sin(angle) * radius)});
    }
}

void Drop::Draw_drops()
{
    Color raylibColor = {static_cast<unsigned char>(clr.r), static_cast<unsigned char>(clr.g), static_cast<unsigned char>(clr.b), static_cast<unsigned char>(clr.a)};
    size_t realCount = vertices.size();
    rlBegin(RL_TRIANGLES);
    rlColor4ub(raylibColor.r, raylibColor.g, raylibColor.b, raylibColor.a);

    // Triangulate the polygon (assumes convex shape for simplicity)
    for (int i = realCount-1; i >= 0; i--) {
        rlVertex2f(this->center.x, this->center.y);       // First vertex
        rlVertex2f(vertices[i].x, vertices[i].y); // Third vertex
        rlVertex2f(vertices[(i-1+realCount)%realCount].x, vertices[(i-1+realCount)%realCount].y);      // Second vertex
    }

    rlEnd();
    
    // for (size_t i = 1; i < realCount; ++i) DrawLineV(vertices[i-1], vertices[i], WHITE);

}
void Drop::update_vertices(float c_x, float c_y, double n_r)
{
    this->center.x = c_x + ((this->center.x - c_x) * sqrt(1 + (n_r * n_r / ((this->center.x - c_x) * (this->center.x - c_x))))) ;
    this->center.y = c_y + ((this->center.y - c_y) * sqrt(1 + (n_r * n_r / ((this->center.y - c_y) * (this->center.y - c_y))))) ;
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertices[i].x = c_x + ((vertices[i].x - c_x) * sqrt(1 + (n_r * n_r / ((vertices[i].x - c_x) * (vertices[i].x - c_x))))) ;
        vertices[i].y = c_y + ((vertices[i].y - c_y) * sqrt(1 + (n_r * n_r / ((vertices[i].y - c_y) * (vertices[i].y - c_y))))) ;
    }
}

void Drop::wavy_transformation()
{
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        double angle = (2.0 * PI * i) / (vertices.size() - 1);
        double wave = 10 * sin(5 * angle); // amplitude 10, frequency 5
        vertices[i].x += wave * cos(angle);
        vertices[i].y += wave * sin(angle);
    }
}
void Drop::inserve_wavy_transformation()
{
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        double angle = (2.0 * PI * i) / (vertices.size() - 1);
        double wave = 10 * sin(5 * angle); // amplitude 10, frequency 5
        vertices[i].x -= wave * cos(angle);
        vertices[i].y -= wave * sin(angle);
    }
}

void Drop::marble(const Drop& other) {
    Vector2 c = other.center;
    float r = (float)other.radius;
    for (size_t i = 0; i < vertices.size(); ++i) {
        Vector2 p = vertices[i];
        // Subtract center
        p.x -= c.x;
        p.y -= c.y;
        // Magnitude
        float m = sqrt(p.x * p.x + p.y * p.y);
        // Calculate root
        float root = sqrt(1.0f + (r * r) / (m * m));
        // Multiply by root
        p.x *= root;
        p.y *= root;
        // Add center back
        p.x += c.x;
        p.y += c.y;
        // Update vertex
        vertices[i] = p;
    }
}
