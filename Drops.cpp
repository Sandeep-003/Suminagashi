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
    for (int i = 0; i < n; ++i)
    {
        double angle = (2.0 * PI * i) / (double)n;
        Vector2 v{(float)(center.x + cos(angle) * radius), (float)(center.y + sin(angle) * radius)};
        vertices.push_back(v);
        baseVertices.push_back(v);
    }
}

// Gradually blend current color toward target color (if set) but cap at maxBlend of target
void Drop::updateColor(float step)
{
    if (!hasTarget)
        return;
    if (step < 0)
        step = 0;
    if (step > 0.2f)
        step = 0.2f; // safety clamp
    // Exponential accumulation: effective blend = 1 - (1-step)^k
    // We maintain blendAccum separately, clamped to maxBlend.
    // Convert step to incremental increase of blendAccum toward 1.0 then clamp.
    // Instead of tracking frame count, update as: blendAccum = 1 - (1 - blendAccum)*(1 - step)
    blendAccum = 1.0f - (1.0f - blendAccum) * (1.0f - step);
    if (blendAccum > maxBlend)
        blendAccum = maxBlend;
    // Lerp between startClr and targetClr by blendAccum
    auto lerpComp = [](int a, int b, float t)
    { return (int)(a + (b - a) * t); };
    clr.r = lerpComp(startClr.r, targetClr.r, blendAccum);
    clr.g = lerpComp(startClr.g, targetClr.g, blendAccum);
    clr.b = lerpComp(startClr.b, targetClr.b, blendAccum);
    clr.a = lerpComp(startClr.a, targetClr.a, blendAccum);
    // Stop updating when fully reached maxBlend plateau
    if (blendAccum >= maxBlend - 0.0001f)
    {
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
    if (realCount >= 3)
    {
        // Ensure consistent CCW winding for current coordinate system (y-down in screen space).
        // If winding is wrong triangles will be culled (nothing visible) in WebGL.
        for (size_t i = 0; i < realCount; ++i)
        {
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
    this->center.x = c_x + ((this->center.x - c_x) * sqrt(1 + (n_r * n_r / ((this->center.x - c_x) * (this->center.x - c_x)))));
    this->center.y = c_y + ((this->center.y - c_y) * sqrt(1 + (n_r * n_r / ((this->center.y - c_y) * (this->center.y - c_y)))));
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        vertices[i].x = c_x + ((vertices[i].x - c_x) * sqrt(1 + (n_r * n_r / ((vertices[i].x - c_x) * (vertices[i].x - c_x)))));
        vertices[i].y = c_y + ((vertices[i].y - c_y) * sqrt(1 + (n_r * n_r / ((vertices[i].y - c_y) * (vertices[i].y - c_y)))));
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

void Drop::marble(const Drop &other, bool commitBaseShape)
{
    Vector2 c = other.center;
    float r = (float)other.radius;
    // Original formula mapped radial distance m to sqrt(m^2 + r^2). It was
    // implemented as m * sqrt(1 + r^2/m^2) which blows up (NaN/Inf) when m~0
    // and produced a single overly stretched / stuck vertex ("triangle" wing).
    // Re-write directly with soft clamping for very small m to keep the
    // deformation smooth and bounded.
    const float EPS = 1e-6f;      // avoid divide-by-zero
    const float MAX_SCALE = 6.0f; // tighter safety cap against extreme stretching
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        Vector2 p = vertices[i];
        float dx = p.x - c.x;
        float dy = p.y - c.y;
        float m2 = dx * dx + dy * dy;
        if (m2 < EPS)
        {
            // Vertex sits (almost) on the other drop's center; push it outward
            // along an arbitrary stable direction (keep current) by radius r.
            // (Rare unless centers coincide.)
            float invLen = 1.0f / sqrtf(EPS);
            dx *= invLen;
            dy *= invLen;
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
        if (scale > MAX_SCALE)
            scale = MAX_SCALE; // clamp extreme cases
        // Small angular jitter to keep two adjacent vertices from collapsing exactly
        float ang = atan2f(dy, dx);
        float jitter = 0.0005f * r;                                 // tiny, frame-independent
        p.x = c.x + (dx * scale) + cosf(ang + 1.5707963f) * jitter; // perpendicular nudge
        p.y = c.y + (dy * scale) + sinf(ang + 1.5707963f) * jitter;
        vertices[i] = p;
    }
    // Recompute center as polygon centroid so later radial-based effects (noise, animate)
    // remain well-behaved; a stale center caused uneven stretching and apparent wedges
    // in triangle-fan rendering.
    if (!vertices.empty())
    {
        double accx = 0.0, accy = 0.0;
        for (auto &v : vertices)
        {
            accx += v.x;
            accy += v.y;
        }
        center.x = (float)(accx / vertices.size());
        center.y = (float)(accy / vertices.size());
    }
    if (commitBaseShape)
        baseVertices = vertices; // refresh base for later noise/animation
}

// ---- Continuous 2D value noise helpers (smooth, stable over time) ----
static inline float fade(float t) { return t * t * (3.0f - 2.0f * t); } // smoothstep
static inline uint32_t hash32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}
static float lattice(uint32_t ix, uint32_t iy)
{
    uint32_t h = hash32(ix * 0x9E3779B1u ^ iy * 0x85EBCA77u);
    return (h & 0xFFFFFF) / (float)0xFFFFFF; // 0..1
}
static float valueNoise2D(float x, float y)
{
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);
    float fx = x - ix;
    float fy = y - iy;
    float v00 = lattice(ix, iy);
    float v10 = lattice(ix + 1, iy);
    float v01 = lattice(ix, iy + 1);
    float v11 = lattice(ix + 1, iy + 1);
    float sx = fade(fx);
    float sy = fade(fy);
    float ix0 = v00 + (v10 - v00) * sx;
    float ix1 = v01 + (v11 - v01) * sx;
    return ix0 + (ix1 - ix0) * sy; // 0..1
}

void Drop::applyEdgeNoise(float amplitude, float frequency, float time)
{
    // Uses 2D value noise over (angle * frequency, time * temporalScale) for smooth evolution.
    if (baseVertices.size() != vertices.size())
        baseVertices = vertices; // fallback sync
    float amp = (float)radius * amplitude;
    const float temporalScale = 0.35f; // speed factor for animation
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        if (i >= baseVertices.size())
            break;
        Vector2 base = baseVertices[i];
        float dx = base.x - center.x;
        float dy = base.y - center.y;
        float ang = atan2f(dy, dx); // -PI..PI
        // Normalize angle to 0..1 before feeding to noise (optional)
        float angNorm = (ang + PI) / (2.0f * PI); // 0..1
        float n = valueNoise2D(angNorm * frequency, time * temporalScale);
        n = n * 2.0f - 1.0f; // map to -1..1
        float r = sqrtf(dx * dx + dy * dy);
        float nr = r + n * amp;
        vertices[i].x = center.x + cosf(ang) * nr;
        vertices[i].y = center.y + sinf(ang) * nr;
    }
}

void Drop::blendColor(const color &target, float t)
{
    if (t < 0)
        t = 0;
    if (t > 1)
        t = 1;
    clr.r = (int)(clr.r + (target.r - clr.r) * t);
    clr.g = (int)(clr.g + (target.g - clr.g) * t);
    clr.b = (int)(clr.b + (target.b - clr.b) * t);
    clr.a = (int)(clr.a + (target.a - clr.a) * t);
}

void Drop::animateShape(float time, float amplitude, float speed, int harmonics)
{
    if (baseVertices.size() != vertices.size())
        baseVertices = vertices;
    float amp = (float)radius * amplitude;
    if (harmonics < 1)
        harmonics = 1;
    if (harmonics > 5)
        harmonics = 5; // limit
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        Vector2 base = baseVertices[i];
        float dx = base.x - center.x;
        float dy = base.y - center.y;
        float ang = atan2f(dy, dx);
        float r = sqrtf(dx * dx + dy * dy);
        float deform = 0.0f;
        for (int h = 1; h <= harmonics; ++h)
        {
            deform += (sinf(ang * h + time * speed * (0.4f + h * 0.2f)) / h);
        }
        deform *= (amp / harmonics);
        float nr = r + deform;
        vertices[i].x = center.x + cosf(ang) * nr;
        vertices[i].y = center.y + sinf(ang) * nr;
    }
}

void Drop::commitBase()
{
    baseVertices = vertices;
}

void Drop::resetToBase()
{
    if (!baseVertices.empty() && baseVertices.size() == vertices.size())
    {
        vertices = baseVertices;
    }
}

void Drop::applyVerticalTine(float x, float strength, float sharpness, bool commitBase)
{
    // Robust, smooth vertical tine deformation.
    // Requirements:
    //  * Maximum downward translation at the vertex whose x is closest to mouse x.
    //  * Smooth, non-linear falloff to zero displacement at an effective radius.
    //  * No effect outside that radius.
    //  * Stable (no NaNs, no harsh edges), cumulative (does not reset to base).
    // Interpretation of parameters:
    //  strength  : maximum vertical translation in pixels (downwards if positive).
    //  sharpness : controls radius & steepness. Larger -> wider, gentler falloff.
    // Mapping of sharpness -> radius: radius = clamp(sharpness, minR, maxR).
    // Falloff: quintic smoothstep (C2 continuous) w = 1 - S(u), u=d/r, S(u)=6u^5-15u^4+10u^3.
    // Additional focus: raise weight to a power to concentrate displacement near center.

    if (fabsf(strength) < 1e-5f) return;
    // Derive effective radius from sharpness. Provide sensible clamps.
    float minR = 8.0f;
    float maxR = (float)radius * 2.5f; // don't let a single tine span huge area of a large drop cluster
    float R = sharpness;
    if (R < minR) R = minR;
    if (R > maxR) R = maxR;

    // Power to control edge softness: higher -> sharper center, softer edge.
    // Map original sharpness to exponent in [1,3].
    float weightExp = 1.0f + fminf(fmaxf(sharpness, 1.0f), 512.0f) / 256.0f * 2.0f; // 1..3

    // Pre-pass: find vertex closest in x to anchor for potential center adjustment.
    int closestIdx = -1; float closestDx = 1e9f;
    for (size_t i = 0; i < vertices.size(); ++i) {
        float dx = fabsf(vertices[i].x - x);
        if (dx < closestDx) { closestDx = dx; closestIdx = (int)i; }
    }

    if (closestIdx == -1) return;

    // Capture original y for optional smoothing.
    std::vector<float> origY;
    origY.reserve(vertices.size());
    for (auto &v : vertices) origY.push_back(v.y);

    float totalDelta = 0.0f; int affected = 0;
    // Diminishing returns control: limit cumulative displacement relative to local radius
    float localMax = strength; // base cap per stroke center
    float cumulativeCap = fmaxf((float)radius * 0.65f, strength * 1.2f); // absolute max allowed downward shift from original base for any vertex
    for (size_t i = 0; i < vertices.size(); ++i) {
        Vector2 &v = vertices[i];
        float dx = fabsf(v.x - x);
        if (dx >= R) continue; // outside influence
        float u = dx / R; // 0..1
        // Quintic smoothstep S(u)
        float u2 = u * u; float u3 = u2 * u; float S = u3 * (u * (u * 6.0f - 15.0f) + 10.0f); // 6u^5-15u^4+10u^3
        float w = 1.0f - S; // 1 at center -> 0 at edge with zero slope
        // Concentrate weight near center (non-linear shaping)
        if (weightExp != 1.0f) w = powf(fmaxf(w, 0.0f), weightExp);
        if (w < 1e-4f) continue;
        float disp = strength * w; // proposed downward shift this stroke
        // Diminishing returns: reduce disp if vertex already moved a lot relative to its base (if available)
        if (baseVertices.size() == vertices.size()) {
            float already = v.y - baseVertices[i].y; // positive if moved downward
            if (already > 0) {
                // Soft approach to cap: remaining fraction = 1 - (already / cumulativeCap)
                float remaining = 1.0f - (already / cumulativeCap);
                if (remaining <= 0.0f) continue; // saturated
                // Ease remaining fraction (quintic) to avoid sudden clamp
                float rem2 = remaining * remaining; float rem3 = rem2 * remaining;
                float eased = rem3 * (remaining * (remaining * 6.0f - 15.0f) + 10.0f);
                disp *= eased;
            }
        }
        // Additionally fade very small residuals
        if (fabsf(disp) < 0.0005f) continue;
        v.y += disp;
        totalDelta += disp;
        ++affected;
    }

    // Mild localized Laplacian smoothing inside influence band to avoid a sharp "ridge" at center.
    if (affected > 3 && vertices.size() > 4) {
        const float smoothFactor = 0.25f; // 0..0.5 safe
        std::vector<float> newY = origY; // start from original for consistency
        for (size_t i = 0; i < vertices.size(); ++i) {
            float dx = fabsf(vertices[i].x - x);
            if (dx >= R) { newY[i] = vertices[i].y; continue; }
            const Vector2 &prev = vertices[(i + vertices.size() - 1) % vertices.size()];
            const Vector2 &next = vertices[(i + 1) % vertices.size()];
            float avg = (prev.y + vertices[i].y + next.y) / 3.0f;
            newY[i] = vertices[i].y * (1.0f - smoothFactor) + avg * smoothFactor;
        }
        for (size_t i = 0; i < vertices.size(); ++i) {
            float dx = fabsf(vertices[i].x - x);
            if (dx < R) vertices[i].y = newY[i];
        }
    }

    // Adjust stored center.y gently based on average displacement (keeps animations coherent).
    if (affected > 0) {
        center.y += (totalDelta / (float)affected) * 0.2f; // small bias only
    }

    if (commitBase)
        baseVertices = vertices;
}
