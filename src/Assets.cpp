#include "Assets.h"

#include <cmath>

void Assets::init()
{
    sphere_ = MakeSphereMesh(18, 28);
    box_ = MakeBoxMesh();
    cylinder_ = MakeCylinderMesh(32);
    cone_ = MakeConeMesh(24);
    horseBody_ = MakeHorseBodyMesh();
    horseNeck_ = MakeHorseNeckMesh();
    horseHead_ = MakeHorseHeadMesh();
}

const Mesh& Assets::capsule(float radius, float cylinderLength) // radius is the radius of the hemispheres, cylinderLength is the length of the cylinder
{
    const std::pair<int, int> key(static_cast<int>(std::lround(radius * 1000.0f)),
                                  static_cast<int>(std::lround(cylinderLength * 1000.0f)));
    auto it = capsules_.find(key);
    if (it == capsules_.end())
    {
        it = capsules_.emplace(key, MakeCapsuleMesh(radius, cylinderLength, 20, 8)).first;
    }
    return it->second;
}

const Mesh& Assets::taperedCapsule(float rBottom, float rTop, float height) // rBottom is the radius of the bottom hemisphere, rTop is the radius of the top hemisphere
{
    const std::tuple<int, int, int> key(
        static_cast<int>(std::lround(rBottom * 1000.0f)),
        static_cast<int>(std::lround(rTop * 1000.0f)),
        static_cast<int>(std::lround(height * 1000.0f)));
    auto it = taperedCapsules_.find(key);
    if (it == taperedCapsules_.end())
    {
        it = taperedCapsules_.emplace(key, MakeTaperedCapsuleMesh(rBottom, rTop, height, 20, 8)).first;
    }
    return it->second;
}

const Mesh& Assets::customLeg(float rTop, float rUpperMid, float rCenter, float rLowerMid, float rBottom, float length)
{
    const std::tuple<int, int, int, int, int, int> key(
        static_cast<int>(std::lround(rTop * 10000.0f)),
        static_cast<int>(std::lround(rUpperMid * 10000.0f)),
        static_cast<int>(std::lround(rCenter * 10000.0f)),
        static_cast<int>(std::lround(rLowerMid * 10000.0f)),
        static_cast<int>(std::lround(rBottom * 10000.0f)),
        static_cast<int>(std::lround(length * 10000.0f)));
    auto it = customLegs_.find(key);
    if (it == customLegs_.end())
    {
        it = customLegs_.emplace(key, MakeCustomLegMesh(rTop, rUpperMid, rCenter, rLowerMid, rBottom, length, 20, 8)).first;
    }
    return it->second;
}

const Mesh& Assets::torus(float majorR, float minorR)
{
    const std::pair<int, int> key(static_cast<int>(std::lround(majorR * 1000.0f)),
                                  static_cast<int>(std::lround(minorR * 1000.0f)));
    auto it = tori_.find(key);
    if (it == tori_.end())
    {
        it = tori_.emplace(key, MakeTorusMesh(majorR, minorR, 24, 12)).first;
    }
    return it->second;
}
