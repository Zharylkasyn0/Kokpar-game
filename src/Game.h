#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Assets.h"
#include "BotAI.h"
#include "Camera.h"
#include "Goat.h"
#include "Horse.h"
#include "Hud.h"
#include "Input.h"
#include "Particles.h"
#include "Renderer.h"
#include "World.h"

struct GLFWwindow;

// Top-level game: owns all subsystems and runs update/render each frame.
// Free-for-All Mode: 1 human player + 3 autonomous AI bots.
class Game
{
public:
    bool init(GLFWwindow* window);
    void resetGame();
    void update(float dt);
    void render();

    // Inspection accessors for gameplay verification & tests
    int carrierIndex() const { return carrierIndex_; }
    int score(int index) const { return (index >= 0 && index < 4) ? scores_[index] : 0; }
    const std::array<int, 4>& scores() const { return scores_; }
    float matchTimer() const { return matchTimer_; }
    bool isMatchOver() const { return matchOver_; }
    int winnerIndex() const { return winnerIndex_; }
    const Horse& horse(size_t index) const { return horses_[index]; }
    Horse& horse(size_t index) { return horses_[index]; }
    const Goat& goat() const { return goat_; }
    Goat& goat() { return goat_; }
    const BotAI& botAI(size_t index) const { return botAIs_[index]; }
    BotAI& botAI(size_t index) { return botAIs_[index]; }

    // Simulation helpers for deterministic scenario verification
    void setCarrier(int index);
    void setScores(const std::array<int, 4>& s) { scores_ = s; }
    void setMatchTimer(float t) { matchTimer_ = t; }
    void setMatchOver(bool over) { matchOver_ = over; }
    void setWinnerIndex(int idx) { winnerIndex_ = idx; }
    void setLastCarrier(int idx) { lastCarrierIndex_ = idx; }

private:
    glm::vec3 nextSpawn();
    void drawBlobShadow(const glm::vec3& groundPos, float radiusX, float radiusZ, float yaw, float height);
    void updateWindowTitle(float dt);
    void setBannerMessage(const std::string& msg, float duration);
    void resolveHorseCollisions();

    static constexpr float kPickupRadius = 2.8f;
    static constexpr float kStealRange = 3.2f;
    static constexpr float kStealAngleMax = 1.92f; // ~110 degrees total frontal cone
    static constexpr float kStealCooldown = 1.5f;
    static constexpr float kMatchDuration = 180.0f; // 3:00 minutes match

    GLFWwindow* window_ = nullptr;
    Input input_;
    Renderer renderer_;
    Assets assets_;
    World world_;

    // Four FFA participants: index 0 = Player, 1 = Bot 1, 2 = Bot 2, 3 = Bot 3
    std::array<Horse, 4> horses_;
    std::array<BotAI, 3> botAIs_;
    std::array<int, 4> scores_ = {0, 0, 0, 0};

    // Shared possession state
    int carrierIndex_ = -1;       // -1 = free / thrown, 0 = player, 1..3 = bots
    int lastCarrierIndex_ = -1;   // Participant who last carried/threw goat (for projectile scoring)
    float playerStealCooldown_ = 0.0f;
    std::array<float, 4> victimImmunity_ = {0.0f, 0.0f, 0.0f, 0.0f};

    // Match state & timer
    float matchTimer_ = kMatchDuration;
    bool matchOver_ = false;
    int winnerIndex_ = -1;        // -1 = tie, 0..3 = participant

    // Event notification banner
    std::string bannerMessage_;
    float bannerTimer_ = 0.0f;

    Goat goat_;
    FollowCamera camera_;
    Hud hud_;
    Particles particles_;

    std::vector<glm::vec3> spawnPoints_;
    size_t spawnIndex_ = 0;

    // FPS / title bookkeeping
    float titleTimer_ = 0.0f;
    int frameCounter_ = 0;
    float fps_ = 0.0f;
};
