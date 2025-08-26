#include "drops.h"

Drop::Drop(float x, float y, color clr, double radius, int n)
{
    this->clr = clr;
    center = {(float)x, (float)y};
    this->radius = radius;
    this->n = n;
    // Generate exactly n distinct perimeter vertices (no duplicated closing vertex).
    // The old version used i <= n which duplicated the first point at the end;
    // that duplicate produced a degenerate triangle in the fan triangulation
    // after heavy marble distortions, showing up visually as a "wedge"/flat flap.
    vertices.reserve(n);
    baseVertices.reserve(n);
    for (int i = 0; i < n; ++i) {
        double angle = (2.0 * PI * i) / (double)n;
        Vector2 v{ (float)(center.x + cos(angle) * radius), (float)(center.y + sin(angle) * radius) };
        vertices.push_back(v);
        baseVertices.push_back(v);
    }
}

// Gradually blend current color toward target color (if set) but cap at maxBlend of target
void Drop::updateColor(float step) {
    if (!hasTarget) return;
    if (step < 0) step = 0; if (step > 0.2f) step = 0.2f; // safety clamp
    // Exponential accumulation: effective blend = 1 - (1-step)^k
    // We maintain blendAccum separately, clamped to maxBlend.
    // Convert step to incremental increase of blendAccum toward 1.0 then clamp.
    // Instead of tracking frame count, update as: blendAccum = 1 - (1 - blendAccum)*(1 - step)
    blendAccum = 1.0f - (1.0f - blendAccum) * (1.0f - step);
    if (blendAccum > maxBlend) blendAccum = maxBlend;
    // Lerp between startClr and targetClr by blendAccum
    auto lerpComp = [](int a, int b, float t){ return (int)(a + (b - a) * t); };
    clr.r = lerpComp(startClr.r, targetClr.r, blendAccum);
    clr.g = lerpComp(startClr.g, targetClr.g, blendAccum);
    clr.b = lerpComp(startClr.b, targetClr.b, blendAccum);
    clr.a = lerpComp(startClr.a, targetClr.a, blendAccum);
    // Stop updating when fully reached maxBlend plateau
    if (blendAccum >= maxBlend - 0.0001f) {
        hasTarget = false; // optional: disable further processing; remove if you want oscillations later
    }
}

void Drop::Draw_drops()
{
    Color raylibColor = {static_cast<unsigned char>(clr.r), static_cast<unsigned char>(clr.g), static_cast<unsigned char>(clr.b), static_cast<unsigned char>(clr.a)};
    size_t realCount = vertices.size();
    rlBegin(RL_TRIANGLES);
    rlColor4ub(raylibColor.r, raylibColor.g, raylibColor.b, raylibColor.a);
    // Triangle fan (center, v[i], v[i+1]) over n distinct vertices (convex assumption)
    if (realCount >= 3) {
        // Ensure consistent CCW winding for current coordinate system (y-down in screen space).
        // If winding is wrong triangles will be culled (nothing visible) in WebGL.
        for (size_t i = 0; i < realCount; ++i) {
            const Vector2 &a = vertices[i];
            const Vector2 &b = vertices[(i + 1) % realCount];
            // Flip a/b order from previous version to fix winding.
            rlVertex2f(center.x, center.y);
            rlVertex2f(b.x, b.y);
            rlVertex2f(a.x, a.y);
        }
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
        double wave = 4 * sin(10 * angle); // amplitude 10, frequency 5
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

void Drop::marble(const Drop& other, bool commitBaseShape) {
    Vector2 c = other.center;
    float r = (float)other.radius;
    // Original formula mapped radial distance m to sqrt(m^2 + r^2). It was
    // implemented as m * sqrt(1 + r^2/m^2) which blows up (NaN/Inf) when m~0
    // and produced a single overly stretched / stuck vertex ("triangle" wing).
    // Re-write directly with soft clamping for very small m to keep the
    // deformation smooth and bounded.
    const float EPS = 1e-6f;          // avoid divide-by-zero
    const float MAX_SCALE = 6.0f;     // tighter safety cap against extreme stretching
    for (size_t i = 0; i < vertices.size(); ++i) {
        Vector2 p = vertices[i];
        float dx = p.x - c.x;
        float dy = p.y - c.y;
        float m2 = dx * dx + dy * dy;
        if (m2 < EPS) {
            // Vertex sits (almost) on the other drop's center; push it outward
            // along an arbitrary stable direction (keep current) by radius r.
            // (Rare unless centers coincide.)
            float invLen = 1.0f / sqrtf(EPS);
            dx *= invLen; dy *= invLen;
            p.x = c.x + dx * r;
            p.y = c.y + dy * r;
            vertices[i] = p;
            continue;
        }
        float m = sqrtf(m2);
        // Smoother mapping than sqrt(m^2 + r^2): gives gentler gradient near m=0
        // newDist in [min(r, m), ...] without abrupt clustering that created a flat chord.
        float newDist = m + (r * r) / (m + r); // rational smoothing
        float scale = newDist / m;
        if (scale > MAX_SCALE) scale = MAX_SCALE; // clamp extreme cases
        // Small angular jitter to keep two adjacent vertices from collapsing exactly
        float ang = atan2f(dy, dx);
        float jitter = 0.0005f * r; // tiny, frame-independent
        p.x = c.x + (dx * scale) + cosf(ang + 1.5707963f) * jitter; // perpendicular nudge
        p.y = c.y + (dy * scale) + sinf(ang + 1.5707963f) * jitter;
        vertices[i] = p;
    }
    // Recompute center as polygon centroid so later radial-based effects (noise, animate)
    // remain well-behaved; a stale center caused uneven stretching and apparent wedges
    // in triangle-fan rendering.
    if (!vertices.empty()) {
        double accx = 0.0, accy = 0.0;
        for (auto &v : vertices) { accx += v.x; accy += v.y; }
        center.x = (float)(accx / vertices.size());
        center.y = (float)(accy / vertices.size());
    }
    if (commitBaseShape) baseVertices = vertices; // refresh base for later noise/animation
}

// ---- Continuous 2D value noise helpers (smooth, stable over time) ----
static inline float fade(float t) { return t * t * (3.0f - 2.0f * t); } // smoothstep
static inline uint32_t hash32(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16; return x;
}
static float lattice(uint32_t ix, uint32_t iy) {
    uint32_t h = hash32(ix * 0x9E3779B1u ^ iy * 0x85EBCA77u);
    return (h & 0xFFFFFF) / (float)0xFFFFFF; // 0..1
}
static float valueNoise2D(float x, float y) {
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);
    float fx = x - ix;
    float fy = y - iy;
    float v00 = lattice(ix,     iy);
    float v10 = lattice(ix + 1, iy);
    float v01 = lattice(ix,     iy + 1);
    float v11 = lattice(ix + 1, iy + 1);
    float sx = fade(fx);
    float sy = fade(fy);
    float ix0 = v00 + (v10 - v00) * sx;
    float ix1 = v01 + (v11 - v01) * sx;
    return ix0 + (ix1 - ix0) * sy; // 0..1
}

void Drop::applyEdgeNoise(float amplitude, float frequency, float time) {
    // Uses 2D value noise over (angle * frequency, time * temporalScale) for smooth evolution.
    if (baseVertices.size() != vertices.size()) baseVertices = vertices; // fallback sync
    float amp = (float)radius * amplitude;
    const float temporalScale = 0.35f; // speed factor for animation
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (i >= baseVertices.size()) break;
        Vector2 base = baseVertices[i];
        float dx = base.x - center.x;
        float dy = base.y - center.y;
        float ang = atan2f(dy, dx); // -PI..PI
        // Normalize angle to 0..1 before feeding to noise (optional)
        float angNorm = (ang + PI) / (2.0f * PI); // 0..1
        float n = valueNoise2D(angNorm * frequency, time * temporalScale);
        n = n * 2.0f - 1.0f; // map to -1..1
        float r = sqrtf(dx*dx + dy*dy);
        float nr = r + n * amp;
        vertices[i].x = center.x + cosf(ang) * nr;
        vertices[i].y = center.y + sinf(ang) * nr;
    }
}

void Drop::blendColor(const color& target, float t) {
    if (t < 0) t = 0; if (t > 1) t = 1;
    clr.r = (int)(clr.r + (target.r - clr.r) * t);
    clr.g = (int)(clr.g + (target.g - clr.g) * t);
    clr.b = (int)(clr.b + (target.b - clr.b) * t);
    clr.a = (int)(clr.a + (target.a - clr.a) * t);
}

void Drop::animateShape(float time, float amplitude, float speed, int harmonics) {
    if (baseVertices.size() != vertices.size()) baseVertices = vertices;
    float amp = (float)radius * amplitude;
    if (harmonics < 1) harmonics = 1; if (harmonics > 5) harmonics = 5; // limit
    for (size_t i = 0; i < vertices.size(); ++i) {
        Vector2 base = baseVertices[i];
        float dx = base.x - center.x;
        float dy = base.y - center.y;
        float ang = atan2f(dy, dx);
        float r = sqrtf(dx*dx + dy*dy);
        float deform = 0.0f;
        for (int h = 1; h <= harmonics; ++h) {
            deform += (sinf(ang * h + time * speed * (0.4f + h*0.2f)) / h);
        }
        deform *= (amp / harmonics);
        float nr = r + deform;
        vertices[i].x = center.x + cosf(ang) * nr;
        vertices[i].y = center.y + sinf(ang) * nr;
    }
}

void Drop::commitBase() {
    baseVertices = vertices;
}

void Drop::resetToBase() {
    if (!baseVertices.empty() && baseVertices.size() == vertices.size()) {
        vertices = baseVertices;
    }
}
