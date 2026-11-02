#pragma once

#include <array>

struct GLFWwindow;

// Polls a fixed set of keys once per frame and exposes level/edge queries.
class Input
{
public:
    void init(GLFWwindow* window);
    void update();                      // call once per frame, after glfwPollEvents()

    bool down(int key) const;           // currently held
    bool pressed(int key) const;        // went down this frame

private:
    static constexpr int kMaxKeys = 512;

    GLFWwindow* window_ = nullptr;
    std::array<bool, kMaxKeys> current_{};
    std::array<bool, kMaxKeys> previous_{};
};
