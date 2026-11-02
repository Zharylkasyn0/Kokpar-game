#include "Assets.h"

#include <cmath>

void Assets::init()
{
    sphere_ = MakeSphereMesh(18, 28);
    box_ = MakeBoxMesh();
    cylinder_ = MakeCylinderMesh(32);
    cone_ = MakeConeMesh(24);
}

const Mesh& Assets::capsule(float radius, float cylinderLength)
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
