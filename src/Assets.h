#pragma once

#include <map>
#include <utility>

#include "Mesh.h"

// Owns all shared procedural meshes. Create after the GL context exists.
class Assets
{
public:
    void init();

    const Mesh& sphere() const { return sphere_; }      // radius 1
    const Mesh& box() const { return box_; }            // edge 1
    const Mesh& cylinder() const { return cylinder_; }  // radius 1, height 1
    const Mesh& cone() const { return cone_; }          // base radius 1, height 1

    // Capsule along Y, cached by (radius, length). Returned reference stays valid.
    const Mesh& capsule(float radius, float cylinderLength);

private:
    Mesh sphere_;
    Mesh box_;
    Mesh cylinder_;
    Mesh cone_;
    std::map<std::pair<int, int>, Mesh> capsules_;
};
