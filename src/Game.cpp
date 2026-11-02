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
    world_.init(assets_);
    horse_.init(assets_);
    goat_.init(assets_);

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

    world_.update(dt);
    horse_.update(dt, input_, world_);

    // ---- Goat interaction (E = pick up / drop) ---------------------------
    const float dx = horse_.position().x - goat_.position().x;
    const float dz = horse_.position().z - goat_.position().z;
    const bool inRange = (dx * dx + dz * dz) <= kPickupRadius * kPickupRadius;

    if (input_.pressed(GLFW_KEY_E))
    {
        if (goat_.state() == Goat::State::Idle && inRange)
        {
            goat_.pickUp(horse_);
        }
        else if (goat_.state() == Goat::State::Carried)
        {
            const glm::vec3 v = horse_.velocity() + horse_.right() * 2.5f + glm::vec3(0.0f, 5.0f, 0.0f);
            goat_.drop(v);
        }
    }

    goat_.update(dt, horse_, world_);

    const bool carrying = (goat_.state() == Goat::State::Carried);
    horse_.setCarrying(carrying);
    world_.setZoneHighlight(carrying);

    // ---- Scoring: ride into the zone while carrying the goat -------------
    if (carrying && world_.zone().contains(horse_.position()))
    {
        ++score_;
        horse_.setCarrying(false);
        horse_.cheer();
        goat_.respawn(nextSpawn(), static_cast<float>(score_) * 1.7f);
    }

    camera_.update(dt, horse_.position(), horse_.yaw(), horse_.speed01());
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
    drawBlobShadow(horse_.position(), 0.80f, 1.35f, horse_.yaw(), horse_.bobHeight());
    if (goat_.state() != Goat::State::Carried)
    {
        drawBlobShadow(goat_.groundPosition(), 0.45f, 0.45f, 0.0f, goat_.heightAboveGround());
    }

    horse_.draw(renderer_);

    const float dx = horse_.position().x - goat_.position().x;
    const float dz = horse_.position().z - goat_.position().z;
    const bool hint = (dx * dx + dz * dz) <= kPickupRadius * kPickupRadius;
    goat_.draw(renderer_, hint);

    hud_.draw(renderer_, assets_, fbw, fbh, score_, goat_.state() == Goat::State::Carried);
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

        char buffer[200];
        // "\xC3\x96" is the UTF-8 encoding of the letter O with umlaut.
        std::snprintf(buffer, sizeof(buffer),
                      "K\xC3\x96" "KPAR: Wobbly Steppe  |  Score: %d  |  %s  |  %.0f FPS  |  calls: %d",
                      score_,
                      goat_.state() == Goat::State::Carried ? "carrying goat" : "find the goat (E)",
                      static_cast<double>(fps_),
                      renderer_.drawCalls());
        glfwSetWindowTitle(window_, buffer);
    }
}
