//
// Math hacks by cat02e@fsu.edu
//
export module Calc;

import <algorithm>;
import <iterator>;
import <numbers>;
import <vector>;


//////// Integer square root ////////
// Jim Ulery's revision of Mark Borgerding's square root algorithm

export uint32_t SQRT(uint32_t val)
{
    uint32_t temp;
    uint32_t g = 0;
    uint32_t b = 0x8000;
    uint32_t bshft = 15;

    do {
        if (val >= (temp = (((g << 1) + b) << bshft--))) {
            g += b;
            val -= temp;
        }
    } while (b >>= 1);

    return g;
}


export int32_t neg_quadratic(int32_t a, int32_t b, int32_t c)
{
    return (-b - SQRT(b * b - a * c * 4)) / (2 * a);
}


export int32_t pos_quadratic(int32_t a, int32_t b, int32_t c)
{
    return (-b + SQRT(b * b - a * c * 4)) / (2 * a);
}


//////// 1D math ////////

export uint32_t distance(uint32_t a, uint32_t b)
{
    if (b < a)
        return a - b;
    else
        return b - a;
}


export int32_t distance(int32_t a, int32_t b)
{
    if (b < a)
        return a - b;
    else
        return b - a;
}


export int32_t sgn(int32_t a)
{
    if (a > 0)
        return 1;
    else if (a < 0)
        return -1;
    else
        return 0;
}


export uint32_t trim(uint32_t large, uint32_t little)
{
    if (large <= little)
        return 0;        // If the subtraction would produce a negative number (or 0 for speed), return 0.
    return (large - little);    // Else, do the subtraction
}


export uint32_t limit(uint32_t a, uint32_t max)
{
    if (a <= max)
        return a;
    return max;
}


//////// 2D math ////////

export uint32_t distancesqr(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    uint32_t dx = distance(x1, x2);
    uint32_t dy = distance(y1, y2);

    return dx * dx + dy * dy;
}


export uint32_t distance(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    return SQRT(distancesqr(x1, y1, x2, y2));
}


//////// Vectors ////////

export std::vector<int32_t> scalarMult(std::vector<int32_t> v, int32_t scalar)
{
    std::transform(v.begin(), v.end(), v.begin(), [&scalar](auto& c) { return c * scalar; });
    return v;
}


export std::vector<int32_t> scalarDiv(std::vector<int32_t> v, int32_t scalar)
{
    std::transform(v.begin(), v.end(), v.begin(), [&scalar](auto& c) { return c / scalar; });
    return v;
}


export std::vector<int32_t> vectorAdd(std::vector<int32_t> v1, std::vector<int32_t> v2)
{
    std::vector<int32_t> res;

    std::transform(v1.begin(), v1.end(), v2.begin(), back_inserter(res), std::plus<int32_t>());
    return res;
}


export std::vector<int32_t> vectorSub(std::vector<int32_t> v1, std::vector<int32_t> v2)
{
    std::vector<int32_t> res;

    std::transform(v1.begin(), v1.end(), v2.begin(), back_inserter(res), std::minus<int32_t>());
    return res;
}


export uint32_t distance(std::vector<int32_t> v1, std::vector<int32_t> v2)
{
    return distance(v1[0], v2[0], v1[1], v2[1]);
}


// no prediction
export uint8_t triangulateFireAngle(const std::vector<int32_t>& rel)
{
    double dx = -rel[0];
    double dy = rel[1];

    if (dy == 0) {
        if (dx > 0)
            return 10;
        else
            return 30;
    }

    double angle = atan(dx / dy) + std::numbers::pi;

    if (dy >= 0) {
        if (dx >= 0)
            angle -= std::numbers::pi;
        else
            angle += std::numbers::pi;
    }
    return uint8_t(angle * 40.0 / (2.0 * std::numbers::pi));
}


// use prediction
export uint8_t triangulateFireAngle(const std::vector<int32_t>& pos, const std::vector<int32_t>& vel, int32_t scalar)
{
    // pos = relative positions
    // vel = relative velocities
    // scalar = velocity of bullets

    double a = vel[0] * vel[0] + vel[1] * vel[1] - scalar;
    double b = 2 * (pos[0] * vel[0] + pos[1] * vel[1]);
    double c = pos[0] * pos[0] + pos[1] * pos[1];
    double time = (-b - sqrt((b * b) - (a * c * 2))) / (2.0 * a);

    if (time <= 0.0)
        time = (-b + sqrt((b * b) - (a * c * 2))) / (2.0 * a);

    return triangulateFireAngle(std::vector(pos[0] + int32_t(double(vel[1]) * time), 
        pos[1] + int32_t(double(vel[1]) * time)));
}


// opposite direction
export uint8_t oppositeDirection(uint8_t d)
{
    return (d + 20) % 40;
}
