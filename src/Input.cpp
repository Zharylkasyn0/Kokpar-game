#include "Input.h"

#include <GLFW/glfw3.h>

#include <cmath>
#include <cstddef>

namespace
{
const int kTrackedKeys[] = {
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_C, GLFW_KEY_Z, GLFW_KEY_V,
    GLFW_KEY_F1, GLFW_KEY_H, GLFW_KEY_TAB, GLFW_KEY_SPACE,
    GLFW_KEY_PAGE_UP, GLFW_KEY_PAGE_DOWN, GLFW_KEY_ESCAPE,
    GLFW_KEY_G, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5,
};

const int kTrackedMouseButtons[] = {
    GLFW_MOUSE_BUTTON_LEFT,
    GLFW_MOUSE_BUTTON_RIGHT,
    GLFW_MOUSE_BUTTON_MIDDLE,
};

void ScrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset)
{
    auto* input = static_cast<Input*>(glfwGetWindowUserPointer(window));
    if (input)
    {
        input->onScroll(yoffset);
    }
}
}

void Input::init(GLFWwindow* window)
{
    window_ = window;
    currentKeys_.fill(false);
    previousKeys_.fill(false);
    currentMouse_.fill(false);
    previousMouse_.fill(false);
    firstCursor_ = true;
    cursorCaptured_ = true;
    deltaX_ = 0.0;
    deltaY_ = 0.0;
    pendingScroll_ = 0.0f;
    scrollY_ = 0.0f;

    glfwSetWindowUserPointer(window, this);
    glfwSetScrollCallback(window, ScrollCallback);

    if (window_)
    {
        glfwGetCursorPos(window_, &cursorX_, &cursorY_);
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }
}

void Input::setCursorCaptured(bool captured)
{
    if (!window_) return;
    cursorCaptured_ = captured;
    if (cursorCaptured_)
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    firstCursor_ = true;
    deltaX_ = 0.0;
    deltaY_ = 0.0;
}

void Input::update()
{
    // Keyboard key polling
    previousKeys_ = currentKeys_;
    for (int key : kTrackedKeys)
    {
        if (key >= 0 && key < kMaxKeys)
        {
            currentKeys_[static_cast<size_t>(key)] = (glfwGetKey(window_, key) == GLFW_PRESS);
        }
    }

    // Mouse button polling
    previousMouse_ = currentMouse_;
    for (int btn : kTrackedMouseButtons)
    {
        if (btn >= 0 && btn < kMaxMouseButtons)
        {
            currentMouse_[static_cast<size_t>(btn)] = (glfwGetMouseButton(window_, btn) == GLFW_PRESS);
        }
    }

    // Cursor position & relative delta
    double currentX = 0.0;
    double currentY = 0.0;
    glfwGetCursorPos(window_, &currentX, &currentY);

    if (firstCursor_)
    {
        cursorX_ = currentX;
        cursorY_ = currentY;
        deltaX_ = 0.0;
        deltaY_ = 0.0;
        firstCursor_ = false;
    }
    else if (cursorCaptured_)
    {
        deltaX_ = currentX - cursorX_;
        deltaY_ = currentY - cursorY_;
        cursorX_ = currentX;
        cursorY_ = currentY;
    }
    else
    {
        // Cursor is free for menus/window - no camera delta
        deltaX_ = 0.0;
        deltaY_ = 0.0;
        cursorX_ = currentX;
        cursorY_ = currentY;
    }

    // Scroll wheel delta
    scrollY_ = pendingScroll_;
    pendingScroll_ = 0.0f;
}

bool Input::down(int key) const
{
    if (key < 0 || key >= kMaxKeys) return false;
    return currentKeys_[static_cast<size_t>(key)];
}

bool Input::pressed(int key) const
{
    if (key < 0 || key >= kMaxKeys) return false;
    return currentKeys_[static_cast<size_t>(key)] && !previousKeys_[static_cast<size_t>(key)];
}

bool Input::mouseDown(int button) const
{
    if (button < 0 || button >= kMaxMouseButtons) return false;
    return currentMouse_[static_cast<size_t>(button)];
}

bool Input::mousePressed(int button) const
{
    if (button < 0 || button >= kMaxMouseButtons) return false;
    return currentMouse_[static_cast<size_t>(button)] && !previousMouse_[static_cast<size_t>(button)];
}
