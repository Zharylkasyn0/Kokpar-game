#pragma once

#include <cstddef>
#include <vector>

#include <glm/glm.hpp>

#include "Assets.h"
#include "Camera.h"
#include "Goat.h"
#include "Horse.h"
#include "Hud.h"
#include "Input.h"
#include "Renderer.h"
#include "World.h"

struct GLFWwindow;

// Top-level game: owns all subsystems and runs update/render each frame.
class Game
{
public:
    bool init(GLFWwindow* window);
    void update(float dt);
    void render();

private:
    void resetGame();
    glm::vec3 nextSpawn();
    void drawBlobShadow(const glm::vec3& groundPos, float radiusX, float radiusZ, float yaw, float height);
    void updateWindowTitle(float dt);

    static constexpr float kPickupRadius = 2.8f;

    GLFWwindow* window_ = nullptr;
    Input input_;
    Renderer renderer_;
    Assets assets_;
    World world_;
    Horse horse_;
    Goat goat_;
    FollowCamera camera_;
    Hud hud_;

    std::vector<glm::vec3> spawnPoints_;
    size_t spawnIndex_ = 0;
    int score_ = 0;

    // FPS / title bookkeeping
    float titleTimer_ = 0.0f;
    int frameCounter_ = 0;
    float fps_ = 0.0f;
};
