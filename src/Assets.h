#pragma once

#include <map>
#include <tuple>
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

    // Cohesive low-poly horse body, neck & head meshes
    const Mesh& horseBody() const { return horseBody_; }
    const Mesh& horseNeck() const { return horseNeck_; }
    const Mesh& horseHead() const { return horseHead_; }

    // Capsule along Y, cached by (radius, length). Returned reference stays valid.
    const Mesh& capsule(float radius, float cylinderLength);

    // Tapered capsule along Y, cached by (rBottom, rTop, height).
    const Mesh& taperedCapsule(float rBottom, float rTop, float height);

    // 5-point customizable leg segment along Y, cached by (rTop, rUpperMid, rCenter, rLowerMid, rBottom, length).
    const Mesh& customLeg(float rTop, float rUpperMid, float rCenter, float rLowerMid, float rBottom, float length);

    // Torus in XZ plane, cached by (majorR, minorR).
    const Mesh& torus(float majorR, float minorR);

private:
    Mesh sphere_;
    Mesh box_;
    Mesh cylinder_;
    Mesh cone_;
    Mesh horseBody_;
    Mesh horseNeck_;
    Mesh horseHead_;
    std::map<std::pair<int, int>, Mesh> capsules_;
    std::map<std::tuple<int, int, int>, Mesh> taperedCapsules_;
    std::map<std::tuple<int, int, int, int, int, int>, Mesh> customLegs_;
    std::map<std::pair<int, int>, Mesh> tori_;
};
