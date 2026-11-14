#include "Game.h"

#include <cstdio>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Math.h"

bool Game::init(GLFWwindow* window)
{
    window_ = window;
    input_.init(window);

    if (!renderer_.init()) return false;
    assets_.init();
    hud_.init();
    world_.init(assets_);
    horse_.init(assets_);
    goat_.init(assets_);
    particles_.init(assets_);

    spawnPoints_ = {
        glm::vec3(-18.0f, 0.0f, -8.0f),
        glm::vec3(20.0f, 0.0f, -20.0f),
        glm::vec3(-12.0f, 0.0f, 14.0f),
        glm::vec3(26.0f, 0.0f, 18.0f),
        glm::vec3(6.0f, 0.0f, -24.0f),
        glm::vec3(-28.0f, 0.0f, 28.0f),
        glm::vec3(30.0f, 0.0f, -8.0f),
    };

    resetGame();
    return true;
}

void Game::resetGame()
{
    score_ = 0;
    spawnIndex_ = 0;
    horse_.reset(glm::vec3(0.0f, 0.0f, -20.0f), 0.0f);
    goat_.respawn(spawnPoints_[0], 0.8f);
    camera_.snapTo(horse_.position(), horse_.yaw());
    particles_.clear();
}

glm::vec3 Game::nextSpawn()
{
    spawnIndex_ = (spawnIndex_ + 1) % spawnPoints_.size();
    return spawnPoints_[spawnIndex_];
}

void Game::update(float dt)
{
    input_.update();

    if (input_.pressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window_, GLFW_TRUE);
    if (input_.pressed(GLFW_KEY_R)) resetGame();

    // Cursor toggle with TAB (allows freeing mouse for window/menus)
    if (input_.pressed(GLFW_KEY_TAB)) input_.toggleCursorCaptured();
    // Clicking window recaptures cursor (without triggering in-game actions on the same click)
    bool justRecaptured = false;
    if (!input_.isCursorCaptured() && input_.mousePressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        input_.setCursorCaptured(true);
        justRecaptured = true;
    }

    // Toggle on-screen controls help with H or F1
    if (input_.pressed(GLFW_KEY_H) || input_.pressed(GLFW_KEY_F1))
    {
        hud_.toggleHelp();
    }

    world_.update(dt);
    hud_.update(dt);
    horse_.update(dt, input_, world_);

    // Emit dust puffs from galloping/trotting hooves
    std::vector<glm::vec3> hoofStrikes;
    horse_.getHoofStrikes(hoofStrikes);
    for (const auto& hpos : hoofStrikes)
    {
        particles_.emitDust(hpos, horse_.velocity(), horse_.speed01());
    }

    // ---- Goat interaction (E key or LMB = pick up / drop) ----------------
    const float dx = horse_.position().x - goat_.position().x;
    const float dz = horse_.position().z - goat_.position().z;
    const bool inRange = (dx * dx + dz * dz) <= kPickupRadius * kPickupRadius;

    const bool actionTriggered = input_.pressed(GLFW_KEY_E) ||
                                 (!justRecaptured && input_.isCursorCaptured() && input_.mousePressed(GLFW_MOUSE_BUTTON_LEFT));

    if (actionTriggered)
    {
        if (goat_.state() == Goat::State::Idle && inRange)
        {
            goat_.pickUp(horse_);
            particles_.emitPickup(goat_.position());
        }
        else if (goat_.state() == Goat::State::Carried)
        {
            // Throw with horse momentum + forward/right outward launch arc
            const glm::vec3 v = horse_.velocity() * 1.25f + horse_.forward() * 4.2f + horse_.right() * 2.2f + glm::vec3(0.0f, 6.2f, 0.0f);
            goat_.drop(v);
        }
    }

    goat_.update(dt, horse_, world_);
    particles_.update(dt);

    const bool carrying = (goat_.state() == Goat::State::Carried);
    horse_.setCarrying(carrying);
    world_.setZoneHighlight(carrying);

    // ---- Scoring: deposit goat into the Kazanlyk goal (carry in or throw in)
    const bool scored = (carrying && world_.zone().contains(horse_.position())) ||
                        (!carrying && world_.zone().contains(goat_.position()));
    if (scored)
    {
        ++score_;
        horse_.setCarrying(false);
        horse_.cheer();
        particles_.emitScore(world_.zone().center);
        hud_.triggerScoreCelebration();
        goat_.respawn(nextSpawn(), static_cast<float>(score_) * 1.7f);
    }

    camera_.update(dt, input_, horse_.position(), horse_.yaw(), horse_.speed01());
    updateWindowTitle(dt);
}

void Game::drawBlobShadow(const glm::vec3& groundPos, float radiusX, float radiusZ, float yaw, float height)
{
    const float alpha = 0.38f / (1.0f + height * 0.8f);
    renderer_.drawUnlit(assets_.cylinder(),
                        Translate(groundPos.x, 0.05f, groundPos.z) * RotY(yaw) * Scale(radiusX, 0.02f, radiusZ),
                        glm::vec3(0.08f, 0.10f, 0.04f), alpha);
}

void Game::render()
{
    int fbw = 0;
    int fbh = 0;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    if (fbw <= 0 || fbh <= 0) return;

    glViewport(0, 0, fbw, fbh);
    const glm::vec3 sky = renderer_.skyColor();
    glClearColor(sky.x, sky.y, sky.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(fbw) / static_cast<float>(fbh);
    renderer_.beginScene(camera_.view(), camera_.projection(aspect), camera_.position());

    world_.draw(renderer_);

    // Blob shadows (flat, alpha-blended, just above the ground)
    drawBlobShadow(horse_.position(), 0.82f, 1.38f, horse_.yaw(), horse_.bobHeight());
    if (goat_.state() != Goat::State::Carried)
    {
        drawBlobShadow(goat_.groundPosition(), 0.45f, 0.45f, 0.0f, goat_.heightAboveGround());
    }

    horse_.draw(renderer_);

    const float dx = horse_.position().x - goat_.position().x;
    const float dz = horse_.position().z - goat_.position().z;
    const bool hint = (dx * dx + dz * dz) <= kPickupRadius * kPickupRadius;
    goat_.draw(renderer_, hint);

    particles_.draw(renderer_);

    hud_.draw(renderer_, assets_, fbw, fbh,
              score_, goat_.state() == Goat::State::Carried,
              horse_.speed(), horse_.speed01(), horse_.gaitName(),
              horse_.position(), horse_.yaw(),
              goat_.position(), goat_.state() == Goat::State::Carried,
              world_.zone().center, world_.arenaRadius());
}

void Game::updateWindowTitle(float dt)
{
    titleTimer_ += dt;
    ++frameCounter_;
    if (titleTimer_ >= 0.5f)
    {
        fps_ = static_cast<float>(frameCounter_) / titleTimer_;
        titleTimer_ = 0.0f;
        frameCounter_ = 0;

        char buffer[220];
        // "\xC3\x96" is the UTF-8 encoding of the letter O with umlaut.
        std::snprintf(buffer, sizeof(buffer),
                      "K\xC3\x96" "KPAR: Wobbly Steppe v0.3 | Score: %d | %s | %.0f FPS | Cam: %s",
                      score_,
                      horse_.gaitName(),
                      static_cast<double>(fps_),
                      camera_.modeName());
        glfwSetWindowTitle(window_, buffer);
    }
}
