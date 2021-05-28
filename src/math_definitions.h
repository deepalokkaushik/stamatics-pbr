#pragma once

#include <cmath>
#include <cstdint>
#include <cassert>
#include <vector>
#include <random>

///////////////////////////////////////////////////////////////////////////////
// Constants and conversions
///////////////////////////////////////////////////////////////////////////////

#define PBR_PI 3.1415926535897932384626433832795
#define PBR_INF 1e20
#define PBR_EPSILON 1e-4

#define PBR_DEG_TO_RAD(DEG) (DEG * PBR_PI / 180)

///////////////////////////////////////////////////////////////////////////////
// Math types
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// vvv CAN IGNORE

struct Point2D
{
    double x, y;
    Point2D(double x_ = 0, double y_ = 0) : x(x_), y(y_) {}
};

struct Vec
{
    double x, y, z;


    Vec(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
    Vec(double scalar = 0.0) : Vec(scalar, scalar, scalar) {}

    inline double sqlen() const
    {
        return x * x + y * y + z * z;
    }

    inline double len() const
    {
        return std::sqrt(sqlen());
    }

    inline Vec operator*(double s) const
    {
        return { x * s, y * s, z * s };
    }

    inline Vec operator*(const Vec& v) const
    {
        return { x * v.x, y * v.y, z * v.z };
    }

    inline Vec operator/(double s) const
    {
        return *this * (1. / s);
    }

    inline Vec operator+(const Vec& b) const
    {
        return {x + b.x, y + b.y, z + b.z};
    }

    inline Vec operator-(const Vec& b) const
    {
        return *this + b * (-1.);
    }

};

/** Normalize the vector, return a unit vector in the same direction as the parameter. */
inline Vec normalize(const Vec& v)
{
    return v / v.len();
}

/** Calculate the dot product between vectors a and b. */
inline double dot(const Vec& a, const Vec& b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

/** Calculate the cos of the angle between vectors a and b. */
inline double cosv(const Vec& a, const Vec& b)
{
    return dot(normalize(a), normalize(b));
}

/** Calculate the cross product between a and b. */
inline Vec cross(const Vec& a, const Vec& b)
{
    return { 
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x)
    };
}

// ^^^
///////////////////////////////////////////////////////////////////////////////

/** Reflect the incident vector v about a normal n. */
inline Vec reflect(const Vec& v, const Vec& n)
{
    return v - n * 2 * cosv(v, n) * v.len();
}

/** Structure that represents a ray in 3D. */
struct Ray
{
    Vec origin;
    Vec direction;
};

///////////////////////////////////////////////////////////////////////////////
// Colors, materials and objects placed in the scene
///////////////////////////////////////////////////////////////////////////////

/** Color as a (r, g, b) vector, where each component is between 0 and 1. Alpha is assumed to be 1. */
using Colorf = Vec;

/** A 4-byte representation of color. R, G, B and A each are 1-byte integers (between 0 and 255). */
using Colori = uint32_t;

/** Clamp a value x between min and max. */
inline double clamp(double x, double min = 0, double max = 1)
{
    return (x < min) ? min : (x > max) ? max : x;
}

/** Define the kind of reflection that the surface has. */
enum class EMaterialType
{
    DIFFUSE,
    SPECULAR
};

/** Structure that represents the surface material. */
struct Material
{
    /** Color of the surface */
    Colorf color;

    /** Color of the light that this surface emits */
    Colorf emission;

    /** Behaviour of the surface */
    EMaterialType type;
};

/** Structure that represents a spherical object. */
struct SphereGeometry
{
    Vec center;
    float radius;

    bool intersect(const Ray& ray, Vec& point) const
    {
        // For intersection, solve
        // |(o + t*dir) - position| = radius
        // (op + t * dir).(op + t * dir) = radius^2
        // (dir.dir)t^2 + 2(op.dir)t + op.op - radius^2 = 0
        // i.e solve At^2 + Bt + C = 0

        Vec op = ray.origin - center;
        double A = ray.direction.sqlen();
        double B = 2 * dot(op, ray.direction);
        double C = op.sqlen() - radius * radius;

        double D = B * B - 4 * A * C;
        if (D < 0) return false; // no solution
        else D = std::sqrt(D);

        double t1 = (-1 * B + D) / (2 * A);
        double t2 = (-1 * B - D) / (2 * A);

        // returns distance, 0 if nohit
        if (t1 > PBR_EPSILON && t1 < t2)
        {
            point = ray.origin + ray.direction * t1;
            return true;
        }
        else if (t2 > PBR_EPSILON)
        {
            point = ray.origin + ray.direction * t2;
            return true;
        }
        else
        {
            return false;
        }
    }
};

/** Information required from each intersection. */
struct HitResult
{
    double param;
    Vec point;
    Vec normal;
    Material material;
};

/** An object that can be placed in the scene. Contains material and geometry for the object. */
struct Actor
{
    Material material;
    SphereGeometry geometry;

    /*!
     * @brief Calculate ray intersection with the geometry for this actor
     * 
     * @param ray Ray that will intersect this object
     * @param hit Output hit data
     * @return bool Indicates if the ray intersects with this actor
     */
    bool intersect(const Ray& ray, HitResult& hit) const
    {
        Vec point;
        if (geometry.intersect(ray, point))
        {
            hit.param = (point - ray.origin).len() / ray.direction.len();
            hit.point = point;
            hit.material = material;
            hit.normal = normalize(hit.point - geometry.center);
            return true;
        }
        else
        {
            return false;
        }
    }
};

/** Scene alias for convenience. */
using Scene = std::vector<Actor>;

//// These are externs and defined in scene.cpp because we're going to pass pointers and such
//// So I don't want this to be defined in each translation unit separately.

extern Scene PBR_SCENE_RTWEEKEND;
// extern Scene PBR_SCENE_CORNELL;
