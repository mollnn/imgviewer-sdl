#ifndef __VEC3_HPP__
#define __VEC3_HPP__

#include <bits/stdc++.h>
using namespace std;

struct vec3
{
    double x, y, z;
    vec3 operator+(const vec3 &rhs) { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
    vec3 operator-(const vec3 &rhs) { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
    vec3 operator*(const vec3 &rhs) { return {x * rhs.x, y * rhs.y, z * rhs.z}; }
    vec3 operator*(double rhs) { return {x * rhs, y * rhs, z * rhs}; }
    vec3 operator/(double rhs) { return {x / rhs, y / rhs, z / rhs}; }
    double dot(const vec3 &rhs) { return x * rhs.x + y * rhs.y + z * rhs.z; }
    vec3 cross(const vec3 &rhs) { return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x}; }
    double norm2() { return x * x + y * y + z * z; }
    double norm() { return sqrt(norm2()); }
    vec3 unit() { return (*this) / norm(); }
};
vec3 operator*(double lhs, const vec3 &rhs) { return {lhs * rhs.x, lhs * rhs.y, lhs * rhs.z}; }

#endif