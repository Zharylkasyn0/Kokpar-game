#pragma once

#include <array>

struct GLFWwindow;

// Polls keyboard and mouse input once per frame and exposes level/edge queries.
class Input
{
public:
    void init(GLFWwindow* window);
    void update();                      // Call once per frame, after glfwPollEvents()

    // Keyboard queries
    bool down(int key) const;           // Currently held
    bool pressed(int key) const;        // Went down this frame

    // Mouse button queries
    bool mouseDown(int button) const;   // Currently held (e.g. GLFW_MOUSE_BUTTON_RIGHT)
    bool mousePressed(int button) const;// Went down this frame

    // Mouse movement & scroll
    double mouseX() const { return cursorX_; }
    double mouseY() const { return cursorY_; }
    double mouseDeltaX() const { return deltaX_; }
    double mouseDeltaY() const { return deltaY_; }
    float scrollDeltaY() const { return scrollY_; }

    // Cursor capture mode (GLFW_CURSOR_DISABLED for modern TPS camera)
    void setCursorCaptured(bool captured);
    bool isCursorCaptured() const { return cursorCaptured_; }
    void toggleCursorCaptured() { setCursorCaptured(!cursorCaptured_); }
    void resetMouseDelta() { deltaX_ = 0.0; deltaY_ = 0.0; firstCursor_ = true; }

    // Internal callback handler for GLFW scroll events
    void onScroll(double yoffset) { pendingScroll_ += static_cast<float>(yoffset); }

private:
    static constexpr int kMaxKeys = 512;
    static constexpr int kMaxMouseButtons = 8;

    GLFWwindow* window_ = nullptr;
    std::array<bool, kMaxKeys> currentKeys_{};
    std::array<bool, kMaxKeys> previousKeys_{};

    std::array<bool, kMaxMouseButtons> currentMouse_{};
    std::array<bool, kMaxMouseButtons> previousMouse_{};

    double cursorX_ = 0.0;
    double cursorY_ = 0.0;
    double deltaX_ = 0.0;
    double deltaY_ = 0.0;
    bool firstCursor_ = true;
    bool cursorCaptured_ = true;

    float pendingScroll_ = 0.0f;
    float scrollY_ = 0.0f;
};
