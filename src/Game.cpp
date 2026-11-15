#include "Game.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

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
    menuSystem_.init();
    settingsManager_.init();
    locationManager_.init();
    lightingManager_.init();

    // Apply saved settings
    applyLoadedSettings();

    world_.init(assets_);
    world_.setLocation(locationManager_.selectedLocation());
    goat_.init(assets_);
    particles_.init(assets_);

    // Initialize all 4 horses (0 = Player, 1..3 = Bots)
    for (size_t i = 0; i < horses_.size(); ++i)
    {
        horses_[i].init(assets_);
    }

    // Traditional Kipchak / Steppe participant tunic colors
    horses_[0].setVestColor(glm::vec3(0.80f, 0.14f, 0.14f)); // Player: Kipchak Crimson
    horses_[1].setVestColor(glm::vec3(0.18f, 0.45f, 0.90f)); // Bot 1: Steppe Azure Blue
    horses_[2].setVestColor(glm::vec3(0.9f, 0.9f, 0.18f)); // Bot 2: Nomad Vermillion
    horses_[3].setVestColor(glm::vec3(0.20f, 0.68f, 0.32f)); // Bot 3: Grassland Emerald Green

    // Personality offsets for bot flanking
    botAIs_[0].init(1, -0.6f);
    botAIs_[1].init(2,  0.0f);
    botAIs_[2].init(3,  0.6f);

    spawnPoints_ = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(-18.0f, 0.0f, -8.0f),
        glm::vec3(20.0f, 0.0f, -20.0f),
        glm::vec3(-12.0f, 0.0f, 14.0f),
        glm::vec3(26.0f, 0.0f, 18.0f),
        glm::vec3(6.0f, 0.0f, -24.0f),
        glm::vec3(-28.0f, 0.0f, 28.0f),
        glm::vec3(30.0f, 0.0f, -8.0f),
    };

    resetGame();

    // Start in Main Menu automatically on launch
    state_ = GameState::MainMenu;
    input_.setCursorCaptured(false);

    return true;
}

void Game::applyLoadedSettings()
{
    const auto& s = settingsManager_.settings();
    locationManager_.selectLocation(static_cast<LocationId>(s.selectedLocation));

    const LightingPresetId pId = static_cast<LightingPresetId>(std::clamp(s.defaultTimePreset, 0, 6));
    lightingManager_.applyPreset(pId);

    const LightingManager::CycleDuration cDur = static_cast<LightingManager::CycleDuration>(std::clamp(s.autoCycleIndex, 0, 4));
    lightingManager_.setCycleDuration(cDur);
}

void Game::setState(GameState s)
{
    state_ = s;
    if (s == GameState::Playing)
    {
        input_.setCursorCaptured(true);
    }
    else
    {
        input_.setCursorCaptured(false);
    }
}

void Game::startMatch()
{
    resetGame();
    setState(GameState::Playing);
}

void Game::resetGame()
{
    scores_.fill(0);
    carrierIndex_ = -1;
    lastCarrierIndex_ = -1;
    playerStealCooldown_ = 0.0f;
    victimImmunity_.fill(0.0f);
    matchTimer_ = kMatchDuration;
    matchOver_ = false;
    winnerIndex_ = -1;
    bannerMessage_.clear();
    bannerTimer_ = 0.0f;

    spawnIndex_ = 0;

    // Four-corner cardinal arena spawn positions
    horses_[0].reset(glm::vec3(0.0f, 0.0f, -22.0f), 0.0f);       // South facing North
    horses_[1].reset(glm::vec3(22.0f, 0.0f, 0.0f), -kHalfPi);    // East facing West
    horses_[2].reset(glm::vec3(0.0f, 0.0f, 22.0f), kPi);         // North facing South
    horses_[3].reset(glm::vec3(-22.0f, 0.0f, 0.0f), kHalfPi);     // West facing East

    for (size_t i = 0; i < botAIs_.size(); ++i)
    {
        botAIs_[i].reset();
    }

    goat_.respawn(spawnPoints_[0], 0.8f);
    camera_.snapTo(horses_[0].position(), horses_[0].yaw());
    particles_.clear();

    setBannerMessage("MATCH START! 1 PLAYER + 3 BOTS FFA!", 3.0f);
}

glm::vec3 Game::nextSpawn()
{
    spawnIndex_ = (spawnIndex_ + 1) % spawnPoints_.size();
    return spawnPoints_[spawnIndex_];
}

void Game::setCarrier(int index)
{
    carrierIndex_ = index;
    for (size_t i = 0; i < horses_.size(); ++i)
    {
        horses_[i].setCarrying(static_cast<int>(i) == index);
    }
    if (index >= 0 && index < static_cast<int>(horses_.size()))
    {
        goat_.pickUp(horses_[index]);
    }
}

void Game::setBannerMessage(const std::string& msg, float duration)
{
    bannerMessage_ = msg;
    bannerTimer_ = duration;
}

void Game::resolveHorseCollisions()
{
    const float minDist = 2.0f * Horse::probeRadius(); // 1.10m
    const float minDistSq = minDist * minDist;

    for (int iter = 0; iter < 2; ++iter)
    {
        for (size_t i = 0; i < horses_.size(); ++i)
        {
            glm::vec3 chestA, rumpA;
            horses_[i].getCollisionProbes(chestA, rumpA);
            const glm::vec3 probesA[2] = { chestA, rumpA };

            for (size_t j = i + 1; j < horses_.size(); ++j)
            {
                glm::vec3 chestB, rumpB;
                horses_[j].getCollisionProbes(chestB, rumpB);
                const glm::vec3 probesB[2] = { chestB, rumpB };

                for (int pa = 0; pa < 2; ++pa)
                {
                    for (int pb = 0; pb < 2; ++pb)
                    {
                        const float dx = probesA[pa].x - probesB[pb].x;
                        const float dz = probesA[pa].z - probesB[pb].z;
                        const float dSq = dx * dx + dz * dz;

                        if (dSq < minDistSq)
                        {
                            const float d = std::sqrt(std::max(dSq, 1e-6f));
                            const float overlap = (minDist - d) * 0.5f;
                            glm::vec2 push;
                            if (d > 1e-3f)
                            {
                                push = glm::vec2(dx / d, dz / d) * overlap;
                            }
                            else
                            {
                                push = glm::vec2(1.0f, 0.0f) * overlap;
                            }
                            horses_[i].displace(push);
                            horses_[j].displace(-push);
                        }
                    }
                }
            }
        }
    }
}

void Game::updateMenuNavigation()
{
    int fbw = 0, fbh = 0;
    glfwGetFramebufferSize(window_, &fbw, &fbh);
    const float s = std::max(1.0f, static_cast<float>(fbh) / 720.0f);
    const float sw = static_cast<float>(fbw);
    const float sh = static_cast<float>(fbh);
    const float mx = static_cast<float>(input_.mouseX());
    const float my = sh - static_cast<float>(input_.mouseY());

    const bool lmb = input_.mousePressed(GLFW_MOUSE_BUTTON_LEFT);
    const bool enter = input_.pressed(GLFW_KEY_ENTER) || input_.pressed(GLFW_KEY_KP_ENTER);
    const bool esc = input_.pressed(GLFW_KEY_ESCAPE);
    const bool up = input_.pressed(GLFW_KEY_UP) || input_.pressed(GLFW_KEY_W);
    const bool down = input_.pressed(GLFW_KEY_DOWN) || input_.pressed(GLFW_KEY_S);
    const bool left = input_.pressed(GLFW_KEY_LEFT) || input_.pressed(GLFW_KEY_A);
    const bool right = input_.pressed(GLFW_KEY_RIGHT) || input_.pressed(GLFW_KEY_D);

    // =======================================================================
    // 1. MAIN MENU
    // =======================================================================
    if (state_ == GameState::MainMenu)
    {
        if (down) menuSelectionIndex_ = (menuSelectionIndex_ + 1) % 5;
        if (up)   menuSelectionIndex_ = (menuSelectionIndex_ + 4) % 5;

        const float cardW = 390.0f * s;
        const float cardH = 460.0f * s;
        const float cardX = (sw - cardW) * 0.5f;
        const float cardY = (sh - cardH) * 0.5f - 15.0f * s;
        const float btnW = cardW - 60.0f * s;
        const float btnH = 42.0f * s;
        const float btnX = cardX + 30.0f * s;
        const float startY = cardY + cardH - 140.0f * s;
        const float spacingY = 52.0f * s;

        int clickedIndex = -1;
        for (int i = 0; i < 5; ++i)
        {
            const float by = startY - static_cast<float>(i) * spacingY;
            if (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH)
            {
                menuSelectionIndex_ = i;
                if (lmb) clickedIndex = i;
            }
        }

        if (enter) clickedIndex = menuSelectionIndex_;

        if (clickedIndex == 0)
        {
            startMatch();
        }
        else if (clickedIndex == 1)
        {
            previousMenuState_ = GameState::MainMenu;
            state_ = GameState::LocationSelect;
            menuSelectionIndex_ = 2; // Default focus to Select
        }
        else if (clickedIndex == 2)
        {
            previousMenuState_ = GameState::MainMenu;
            state_ = GameState::TimeLightingMenu;
            menuSelectionIndex_ = 2;
        }
        else if (clickedIndex == 3)
        {
            previousMenuState_ = GameState::MainMenu;
            state_ = GameState::SettingsMenu;
            menuSelectionIndex_ = 0;
        }
        else if (clickedIndex == 4)
        {
            glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
    }
    // =======================================================================
    // 2. LOCATION SELECTION
    // =======================================================================
    else if (state_ == GameState::LocationSelect)
    {
        if (left)
        {
            locationManager_.previous();
            world_.setLocation(locationManager_.selectedLocation());
        }
        if (right)
        {
            locationManager_.next();
            world_.setLocation(locationManager_.selectedLocation());
        }

        const float cardW = 580.0f * s;
        const float cardH = 480.0f * s;
        const float cardX = (sw - cardW) * 0.5f;
        const float cardY = (sh - cardH) * 0.5f;

        const float prevX = cardX + 30.0f * s;
        const float prevY = cardY + 95.0f * s;
        const float navW = 140.0f * s;
        const float navH = 38.0f * s;

        const float nextX = cardX + cardW - 30.0f * s - navW;
        const float nextY = prevY;

        const float selW = 190.0f * s;
        const float selH = 42.0f * s;
        const float selX = cardX + (cardW - selW) * 0.5f;
        const float selY = cardY + 93.0f * s;

        const float backW = 160.0f * s;
        const float backH = 34.0f * s;
        const float backX = cardX + (cardW - backW) * 0.5f;
        const float backY = cardY + 30.0f * s;

        if (lmb)
        {
            if (mx >= prevX && mx <= prevX + navW && my >= prevY && my <= prevY + navH)
            {
                locationManager_.previous();
                world_.setLocation(locationManager_.selectedLocation());
            }
            else if (mx >= nextX && mx <= nextX + navW && my >= nextY && my <= nextY + navH)
            {
                locationManager_.next();
                world_.setLocation(locationManager_.selectedLocation());
            }
            else if (mx >= selX && mx <= selX + selW && my >= selY && my <= selY + selH)
            {
                settingsManager_.settings().selectedLocation = static_cast<int>(locationManager_.selectedLocation());
                settingsManager_.save();
                setState(previousMenuState_);
            }
            else if (mx >= backX && mx <= backX + backW && my >= backY && my <= backY + backH)
            {
                setState(previousMenuState_);
            }
        }

        if (enter)
        {
            settingsManager_.settings().selectedLocation = static_cast<int>(locationManager_.selectedLocation());
            settingsManager_.save();
            setState(previousMenuState_);
        }
        if (esc)
        {
            setState(previousMenuState_);
        }
    }
    // =======================================================================
    // 3. TIME & LIGHTING MENU
    // =======================================================================
    else if (state_ == GameState::TimeLightingMenu)
    {
        if (left)  lightingManager_.setTimeOfDay(lightingManager_.timeOfDay() - 0.5f);
        if (right) lightingManager_.setTimeOfDay(lightingManager_.timeOfDay() + 0.5f);
        if (up)    lightingManager_.setManualAngles(lightingManager_.sunElevation() + 4.0f, lightingManager_.sunAzimuth());
        if (down)  lightingManager_.setManualAngles(lightingManager_.sunElevation() - 4.0f, lightingManager_.sunAzimuth());

        // Quick numbers 1..7 for presets
        for (int k = GLFW_KEY_1; k <= GLFW_KEY_7; ++k)
        {
            if (input_.pressed(k))
            {
                lightingManager_.applyPreset(static_cast<LightingPresetId>(k - GLFW_KEY_1));
            }
        }

        const float cardW = 540.0f * s;
        const float cardH = 490.0f * s;
        const float cardX = (sw - cardW) * 0.5f;
        const float cardY = (sh - cardH) * 0.5f;

        const float pBtnW = 105.0f * s;
        const float pBtnH = 34.0f * s;
        const float pStartX = cardX + 32.0f * s;
        const float pStartY = cardY + cardH - 180.0f * s;

        const LightingPresetId presets[7] = {
            LightingPresetId::Dawn, LightingPresetId::Morning, LightingPresetId::Noon,
            LightingPresetId::Afternoon, LightingPresetId::Sunset, LightingPresetId::Night, LightingPresetId::Midnight
        };

        if (lmb)
        {
            for (int i = 0; i < 7; ++i)
            {
                const int col = i % 4;
                const int row = i / 4;
                const float bx = pStartX + static_cast<float>(col) * (pBtnW + 16.0f * s);
                const float by = pStartY - static_cast<float>(row) * (pBtnH + 12.0f * s);
                if (mx >= bx && mx <= bx + pBtnW && my >= by && my <= by + pBtnH)
                {
                    lightingManager_.applyPreset(presets[i]);
                }
            }

            // Bottom Buttons
            const float btnW = 120.0f * s;
            const float btnH = 36.0f * s;
            const float btnY = cardY + 40.0f * s;
            const float applyX = cardX + 50.0f * s;
            const float resetX = cardX + (cardW - btnW) * 0.5f;
            const float backX = cardX + cardW - 50.0f * s - btnW;

            if (mx >= applyX && mx <= applyX + btnW && my >= btnY && my <= btnY + btnH)
            {
                setState(previousMenuState_);
            }
            else if (mx >= resetX && mx <= resetX + btnW && my >= btnY && my <= btnY + btnH)
            {
                lightingManager_.applyPreset(LightingPresetId::Noon);
            }
            else if (mx >= backX && mx <= backX + btnW && my >= btnY && my <= btnY + btnH)
            {
                setState(previousMenuState_);
            }
        }

        if (enter || esc)
        {
            setState(previousMenuState_);
        }
    }
    // =======================================================================
    // 4. SETTINGS MENU
    // =======================================================================
    else if (state_ == GameState::SettingsMenu)
    {
        const float cardW = 520.0f * s;
        const float cardH = 460.0f * s;
        const float cardX = (sw - cardW) * 0.5f;
        const float cardY = (sh - cardH) * 0.5f;

        const float backW = 150.0f * s;
        const float backH = 36.0f * s;
        const float backX = cardX + (cardW - backW) * 0.5f;
        const float backY = cardY + 25.0f * s;

        if (lmb)
        {
            if (mx >= backX && mx <= backX + backW && my >= backY && my <= backY + backH)
            {
                settingsManager_.save();
                setState(previousMenuState_);
            }
        }

        if (enter || esc)
        {
            settingsManager_.save();
            setState(previousMenuState_);
        }
    }
    // =======================================================================
    // 5. PAUSE MENU
    // =======================================================================
    else if (state_ == GameState::Paused)
    {
        if (down) menuSelectionIndex_ = (menuSelectionIndex_ + 1) % 6;
        if (up)   menuSelectionIndex_ = (menuSelectionIndex_ + 5) % 6;

        const float cardW = 380.0f * s;
        const float cardH = 430.0f * s;
        const float cardX = (sw - cardW) * 0.5f;
        const float cardY = (sh - cardH) * 0.5f;
        const float btnW = cardW - 60.0f * s;
        const float btnH = 38.0f * s;
        const float btnX = cardX + 30.0f * s;
        const float startY = cardY + cardH - 95.0f * s;
        const float spacingY = 48.0f * s;

        int clickedIndex = -1;
        for (int i = 0; i < 6; ++i)
        {
            const float by = startY - static_cast<float>(i) * spacingY;
            if (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH)
            {
                menuSelectionIndex_ = i;
                if (lmb) clickedIndex = i;
            }
        }

        if (enter) clickedIndex = menuSelectionIndex_;
        if (esc)   clickedIndex = 0; // ESC resumes

        if (clickedIndex == 0)
        {
            setState(GameState::Playing);
        }
        else if (clickedIndex == 1)
        {
            resetGame();
            setState(GameState::Playing);
        }
        else if (clickedIndex == 2)
        {
            previousMenuState_ = GameState::Paused;
            state_ = GameState::LocationSelect;
        }
        else if (clickedIndex == 3)
        {
            previousMenuState_ = GameState::Paused;
            state_ = GameState::TimeLightingMenu;
        }
        else if (clickedIndex == 4)
        {
            previousMenuState_ = GameState::Paused;
            state_ = GameState::SettingsMenu;
        }
        else if (clickedIndex == 5)
        {
            state_ = GameState::MainMenu;
            menuSelectionIndex_ = 0;
            input_.setCursorCaptured(false);
        }
    }
}

void Game::update(float dt)
{
    input_.update();

    if (input_.pressed(GLFW_KEY_ESCAPE))
    {
        if (state_ == GameState::Playing)
        {
            if (showInGameLighting_)
            {
                showInGameLighting_ = false;
            }
            else
            {
                setState(GameState::Paused);
                menuSelectionIndex_ = 0;
                return;
            }
        }
    }

    // Toggle In-Game Quick Lighting Controls with F6
    if (state_ == GameState::Playing && input_.pressed(GLFW_KEY_F6))
    {
        showInGameLighting_ = !showInGameLighting_;
    }

    // Quick lighting panel input handling
    if (state_ == GameState::Playing && showInGameLighting_)
    {
        if (input_.pressed(GLFW_KEY_LEFT))  lightingManager_.setTimeOfDay(lightingManager_.timeOfDay() - 0.5f);
        if (input_.pressed(GLFW_KEY_RIGHT)) lightingManager_.setTimeOfDay(lightingManager_.timeOfDay() + 0.5f);
        if (input_.pressed(GLFW_KEY_UP))    lightingManager_.setManualAngles(lightingManager_.sunElevation() + 4.0f, lightingManager_.sunAzimuth());
        if (input_.pressed(GLFW_KEY_DOWN))  lightingManager_.setManualAngles(lightingManager_.sunElevation() - 4.0f, lightingManager_.sunAzimuth());

        for (int k = GLFW_KEY_1; k <= GLFW_KEY_7; ++k)
        {
            if (input_.pressed(k))
            {
                lightingManager_.applyPreset(static_cast<LightingPresetId>(k - GLFW_KEY_1));
            }
        }
        if (input_.pressed(GLFW_KEY_ENTER) || input_.pressed(GLFW_KEY_KP_ENTER))
        {
            showInGameLighting_ = false;
        }
    }

    // Menu States: Update menu navigation and orbit camera
    if (state_ != GameState::Playing)
    {
        updateMenuNavigation();
        menuOrbitYaw_ += dt * 0.12f;
        world_.update(dt);
        menuSystem_.update(dt);
        lightingManager_.update(dt, true);
        return;
    }

    // In-game reset with R
    if (input_.pressed(GLFW_KEY_R)) resetGame();

    // Cursor toggle with TAB
    if (input_.pressed(GLFW_KEY_TAB)) input_.toggleCursorCaptured();
    bool justRecaptured = false;
    if (!input_.isCursorCaptured() && input_.mousePressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        input_.setCursorCaptured(true);
        justRecaptured = true;
    }

    // Toggle controls overlay with H or F1
    if (input_.pressed(GLFW_KEY_H) || input_.pressed(GLFW_KEY_F1))
    {
        hud_.toggleHelp();
    }

    world_.update(dt);
    hud_.update(dt);
    particles_.update(dt);
    lightingManager_.update(dt, false);

    // Timers & cooldowns
    playerStealCooldown_ = std::max(0.0f, playerStealCooldown_ - dt);
    for (size_t i = 0; i < victimImmunity_.size(); ++i)
    {
        victimImmunity_[i] = std::max(0.0f, victimImmunity_[i] - dt);
    }
    if (bannerTimer_ > 0.0f)
    {
        bannerTimer_ = std::max(0.0f, bannerTimer_ - dt);
    }

    // Match Timer Countdown
    if (!matchOver_)
    {
        matchTimer_ = std::max(0.0f, matchTimer_ - dt);
        if (matchTimer_ <= 0.0f)
        {
            matchOver_ = true;
            // Determine winner
            int bestScore = -1;
            int bestIdx = -1;
            bool tie = false;
            for (int i = 0; i < 4; ++i)
            {
                if (scores_[i] > bestScore)
                {
                    bestScore = scores_[i];
                    bestIdx = i;
                    tie = false;
                }
                else if (scores_[i] == bestScore && bestScore > 0)
                {
                    tie = true;
                }
            }
            winnerIndex_ = (tie || bestScore <= 0) ? -1 : bestIdx;
        }
    }

    // ---- 1. Locomotion & AI Updates --------------------------------------
    if (!matchOver_ && !showInGameLighting_)
    {
        // Player horse locomotion
        horses_[0].update(dt, input_, world_);
    }
    else
    {
        HorseControlInput idle;
        horses_[0].update(dt, idle, world_);
    }

    struct PickupCandidate
    {
        int index;
        float distSq;
    };
    std::vector<PickupCandidate> pickupCandidates;

    struct StealCandidate
    {
        int stealer;
        float dist;
    };
    std::vector<StealCandidate> stealCandidates;

    // AI Bots 1..3 update
    for (int i = 1; i <= 3; ++i)
    {
        if (!matchOver_)
        {
            BotAI& ai = botAIs_[i - 1];
            BotAI::Output botOut = ai.update(dt, horses_[i], carrierIndex_, goat_, horses_, world_);
            horses_[i].update(dt, botOut.ctrl, world_);

            if (botOut.wantPickup && carrierIndex_ == -1 && goat_.state() == Goat::State::Idle)
            {
                const float dx = horses_[i].position().x - goat_.position().x;
                const float dz = horses_[i].position().z - goat_.position().z;
                const float dSq = dx * dx + dz * dz;
                if (dSq <= kPickupRadius * kPickupRadius)
                {
                    pickupCandidates.push_back({i, dSq});
                }
            }

            if (botOut.wantSteal && carrierIndex_ >= 0 && carrierIndex_ != i)
            {
                horses_[i].triggerStealAnim();
                const glm::vec3 botPos = horses_[i].position();
                const glm::vec3 carrierPos = horses_[carrierIndex_].position();
                const glm::vec3 toCarrier = carrierPos - botPos;
                const float dist = glm::length(glm::vec2(toCarrier.x, toCarrier.z));
                const float cosHalfAngle = std::cos(kStealAngleMax * 0.5f);

                float dotFwd = 0.0f;
                if (dist > 1e-4f)
                {
                    dotFwd = glm::dot(horses_[i].forward(), toCarrier / dist);
                }

                if (dist <= kStealRange && dotFwd >= cosHalfAngle && victimImmunity_[carrierIndex_] <= 0.0f)
                {
                    stealCandidates.push_back({i, dist});
                }
            }
        }
        else
        {
            HorseControlInput idle;
            horses_[i].update(dt, idle, world_);
        }
    }

    // ---- 2. Rider-Rider Collision Separation ------------------------------
    resolveHorseCollisions();

    // ---- 3. Dust Puffs from All 4 Horses ----------------------------------
    for (size_t i = 0; i < horses_.size(); ++i)
    {
        std::vector<glm::vec3> hoofStrikes;
        horses_[i].getHoofStrikes(hoofStrikes);
        for (const auto& hpos : hoofStrikes)
        {
            particles_.emitDust(hpos, horses_[i].velocity(), horses_[i].speed01());
        }
    }

    // ---- 4. Player Interaction (E key or LMB = Steal / Pickup / Throw) ----
    const bool actionTriggered = !matchOver_ && !showInGameLighting_ &&
                                 (input_.pressed(GLFW_KEY_E) ||
                                 (!justRecaptured && input_.isCursorCaptured() && input_.mousePressed(GLFW_MOUSE_BUTTON_LEFT)));

    if (actionTriggered)
    {
        if (carrierIndex_ == 0)
        {
            // Throw goat with horse momentum + forward/right outward launch arc
            const glm::vec3 v = horses_[0].velocity() * 1.25f + horses_[0].forward() * 4.2f + horses_[0].right() * 2.2f + glm::vec3(0.0f, 6.2f, 0.0f);
            goat_.drop(v);
            horses_[0].setCarrying(false);
            lastCarrierIndex_ = 0;
            carrierIndex_ = -1;
            setBannerMessage("PLAYER THREW THE GOAT!", 2.0f);
        }
        else if (carrierIndex_ > 0)
        {
            // Opponent carrying goat: Player attempts FIFA-style steal
            horses_[0].triggerStealAnim();
            const glm::vec3 playerPos = horses_[0].position();
            const glm::vec3 carrierPos = horses_[carrierIndex_].position();
            const glm::vec3 toCarrier = carrierPos - playerPos;
            const float dist = glm::length(glm::vec2(toCarrier.x, toCarrier.z));
            const float cosHalfAngle = std::cos(kStealAngleMax * 0.5f);

            float dotFwd = 0.0f;
            if (dist > 1e-4f)
            {
                dotFwd = glm::dot(horses_[0].forward(), toCarrier / dist);
            }

            const bool inStealRange = (dist <= kStealRange);
            const bool inStealCone = (dotFwd >= cosHalfAngle);

            if (inStealRange && inStealCone && playerStealCooldown_ <= 0.0f && victimImmunity_[carrierIndex_] <= 0.0f)
            {
                // Steal succeeded!
                const int oldCarrier = carrierIndex_;
                horses_[oldCarrier].setCarrying(false);
                carrierIndex_ = 0;
                horses_[0].setCarrying(true);
                goat_.pickUp(horses_[0]);
                particles_.emitPickup(goat_.position());

                playerStealCooldown_ = kStealCooldown;
                victimImmunity_[0] = 1.0f;
                botAIs_[oldCarrier - 1].triggerStealCooldown(1.5f);

                std::string msg = "PLAYER STOLE THE GOAT FROM BOT " + std::to_string(oldCarrier) + "!";
                setBannerMessage(msg, 2.5f);
            }
            else
            {
                playerStealCooldown_ = 0.4f;
            }
        }
        else if (carrierIndex_ == -1 && goat_.state() == Goat::State::Idle)
        {
            const float dx = horses_[0].position().x - goat_.position().x;
            const float dz = horses_[0].position().z - goat_.position().z;
            const float dSq = dx * dx + dz * dz;
            if (dSq <= kPickupRadius * kPickupRadius)
            {
                pickupCandidates.push_back({0, dSq});
            }
        }
    }

    // ---- 5. Authoritative Resolution of Simultaneous Pickups & Steals -----
    if (carrierIndex_ == -1 && !pickupCandidates.empty() && goat_.state() == Goat::State::Idle)
    {
        std::sort(pickupCandidates.begin(), pickupCandidates.end(),
                  [](const PickupCandidate& a, const PickupCandidate& b) {
                      return a.distSq < b.distSq;
                  });

        const int winner = pickupCandidates[0].index;
        carrierIndex_ = winner;
        horses_[winner].setCarrying(true);
        goat_.pickUp(horses_[winner]);
        particles_.emitPickup(goat_.position());

        if (winner == 0)
        {
            setBannerMessage("PLAYER PICKED UP THE GOAT!", 2.0f);
        }
        else
        {
            setBannerMessage("BOT " + std::to_string(winner) + " PICKED UP THE GOAT!", 2.0f);
        }
    }

    if (carrierIndex_ >= 0 && !stealCandidates.empty())
    {
        std::sort(stealCandidates.begin(), stealCandidates.end(),
                  [](const StealCandidate& a, const StealCandidate& b) {
                      return a.dist < b.dist;
                  });

        const int stealer = stealCandidates[0].stealer;
        const int oldCarrier = carrierIndex_;

        if (stealer != oldCarrier && victimImmunity_[oldCarrier] <= 0.0f)
        {
            horses_[oldCarrier].setCarrying(false);
            carrierIndex_ = stealer;
            horses_[stealer].setCarrying(true);
            goat_.pickUp(horses_[stealer]);
            particles_.emitPickup(goat_.position());
            victimImmunity_[stealer] = 1.0f;

            if (oldCarrier == 0)
            {
                playerStealCooldown_ = 1.5f;
                botAIs_[stealer - 1].triggerStealCooldown(1.5f);
                setBannerMessage("BOT " + std::to_string(stealer) + " STOLE THE GOAT FROM PLAYER!", 2.5f);
            }
            else
            {
                botAIs_[oldCarrier - 1].triggerStealCooldown(1.5f);
                botAIs_[stealer - 1].triggerStealCooldown(1.5f);
                setBannerMessage("BOT " + std::to_string(stealer) + " STOLE FROM BOT " + std::to_string(oldCarrier) + "!", 2.5f);
            }
        }
    }

    // ---- 6. Goat Physics & Carrying Attachment ---------------------------
    if (carrierIndex_ >= 0)
    {
        goat_.update(dt, horses_[carrierIndex_], world_);
    }
    else
    {
        goat_.update(dt, horses_[0], world_);
    }

    for (size_t i = 0; i < horses_.size(); ++i)
    {
        horses_[i].setCarrying(carrierIndex_ == static_cast<int>(i));
    }
    world_.setZoneHighlight(carrierIndex_ >= 0);

    // ---- 7. Goal Scoring Check (Carried into or Thrown into Kazanlyk) -----
    bool scored = false;
    int scoringParticipant = -1;

    if (carrierIndex_ >= 0)
    {
        if (world_.zone().contains(horses_[carrierIndex_].position()))
        {
            scored = true;
            scoringParticipant = carrierIndex_;
        }
    }
    else if (goat_.state() == Goat::State::Thrown || goat_.state() == Goat::State::Idle)
    {
        if (world_.zone().contains(goat_.position()))
        {
            scored = true;
            scoringParticipant = (lastCarrierIndex_ >= 0 && lastCarrierIndex_ < 4) ? lastCarrierIndex_ : 0;
        }
    }

    if (scored && scoringParticipant >= 0 && !matchOver_)
    {
        ++scores_[scoringParticipant];
        if (carrierIndex_ >= 0)
        {
            horses_[carrierIndex_].setCarrying(false);
            horses_[carrierIndex_].cheer();
        }
        carrierIndex_ = -1;
        lastCarrierIndex_ = -1;
        particles_.emitScore(world_.zone().center);
        hud_.triggerScoreCelebration();

        std::string msg;
        if (scoringParticipant == 0)
        {
            msg = "PLAYER SCORED A GOAL!";
        }
        else
        {
            msg = "BOT " + std::to_string(scoringParticipant) + " SCORED A GOAL!";
        }
        setBannerMessage(msg, 3.0f);

        goat_.respawn(nextSpawn(), static_cast<float>(scores_[scoringParticipant]) * 1.7f);
    }

    // Camera tracks human player
    camera_.update(dt, input_, horses_[0].position(), horses_[0].yaw(), horses_[0].speed01());
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

    const LightingState& lighting = lightingManager_.state();
    const glm::vec3 sky = lighting.skyColor;
    glClearColor(sky.x, sky.y, sky.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(fbw) / static_cast<float>(fbh);

    if (state_ == GameState::Playing || state_ == GameState::Paused)
    {
        // Active 3D gameplay scene
        renderer_.beginScene(camera_.view(), camera_.projection(aspect), camera_.position(), lighting);

        // Sky Celestials (Sun disc, Moon disc, Night stars)
        renderer_.drawCelestials(assets_, camera_.position(), lighting, static_cast<float>(glfwGetTime()));

        world_.draw(renderer_);

        // Blob shadows
        for (size_t i = 0; i < horses_.size(); ++i)
        {
            drawBlobShadow(horses_[i].position(), 0.82f, 1.38f, horses_[i].yaw(), horses_[i].bobHeight());
        }

        if (carrierIndex_ < 0 && goat_.state() != Goat::State::Carried)
        {
            drawBlobShadow(goat_.groundPosition(), 0.45f, 0.45f, 0.0f, goat_.heightAboveGround());
        }

        // Draw horses
        for (size_t i = 0; i < horses_.size(); ++i)
        {
            horses_[i].draw(renderer_);
        }

        // Pickup hint circle
        const float dx = horses_[0].position().x - goat_.position().x;
        const float dz = horses_[0].position().z - goat_.position().z;
        const bool hint = (carrierIndex_ == -1 && (dx * dx + dz * dz) <= kPickupRadius * kPickupRadius);
        goat_.draw(renderer_, hint);

        particles_.draw(renderer_);

        // HUD
        hud_.draw(renderer_, assets_, fbw, fbh,
                  scores_, carrierIndex_,
                  matchTimer_, matchOver_, winnerIndex_,
                  bannerMessage_, bannerTimer_,
                  horses_[0].speed(), horses_[0].speed01(), horses_[0].gaitName(),
                  horses_,
                  goat_.position(), carrierIndex_ >= 0,
                  world_.zone().center, world_.arenaRadius());

        // Quick lighting overlay HUD (F6)
        if (showInGameLighting_)
        {
            menuSystem_.drawInGameLightingPanel(renderer_, assets_, fbw, fbh, lightingManager_);
        }

        // Pause Menu modal overlay
        if (state_ == GameState::Paused)
        {
            menuSystem_.drawPauseMenu(renderer_, assets_, fbw, fbh, menuSelectionIndex_, input_.mouseX(), input_.mouseY());
        }
    }
    else
    {
        // Cinematic orbiting camera for Menus
        const glm::vec3 menuCamPos(std::sin(menuOrbitYaw_) * 48.0f, 16.0f, std::cos(menuOrbitYaw_) * 48.0f);
        const glm::mat4 menuView = glm::lookAt(menuCamPos, glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 menuProj = glm::perspective(glm::radians(55.0f), aspect, 0.1f, 600.0f);

        renderer_.beginScene(menuView, menuProj, menuCamPos, lighting);

        // Sky Celestials
        renderer_.drawCelestials(assets_, menuCamPos, lighting, static_cast<float>(glfwGetTime()));

        // Active Kazakhstan location world
        world_.draw(renderer_);

        // Render current Menu Screen
        const LocationInfo& locInfo = locationManager_.currentInfo();

        if (state_ == GameState::MainMenu)
        {
            menuSystem_.drawMainMenu(renderer_, assets_, fbw, fbh, menuSelectionIndex_, input_.mouseX(), input_.mouseY(), locInfo, lightingManager_);
        }
        else if (state_ == GameState::LocationSelect)
        {
            menuSystem_.drawLocationSelect(renderer_, assets_, fbw, fbh, locInfo, static_cast<int>(locInfo.id), locationManager_.count(), menuSelectionIndex_, input_.mouseX(), input_.mouseY());
        }
        else if (state_ == GameState::TimeLightingMenu)
        {
            menuSystem_.drawTimeLightingMenu(renderer_, assets_, fbw, fbh, lightingManager_, locInfo, menuSelectionIndex_, input_.mouseX(), input_.mouseY());
        }
        else if (state_ == GameState::SettingsMenu)
        {
            menuSystem_.drawSettingsMenu(renderer_, assets_, fbw, fbh, settingsManager_, menuSelectionIndex_, input_.mouseX(), input_.mouseY());
        }
    }
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

        char buffer[256];
        const int mins = std::max(0, static_cast<int>(matchTimer_)) / 60;
        const int secs = std::max(0, static_cast<int>(matchTimer_)) % 60;

        // "\xC3\x96" is UTF-8 encoding of O with umlaut
        std::snprintf(buffer, sizeof(buffer),
                      "K\xC3\x96" "KPAR: Wobbly Steppe | %s | %s | P:%d B1:%d B2:%d B3:%d | Time: %02d:%02d | %.0f FPS",
                      locationManager_.currentInfo().nameEn,
                      lightingManager_.formattedTime().c_str(),
                      scores_[0], scores_[1], scores_[2], scores_[3],
                      mins, secs,
                      static_cast<double>(fps_));
        glfwSetWindowTitle(window_, buffer);
    }
}
