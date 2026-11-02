#include "Input.h"

#include <GLFW/glfw3.h>

#include <cstddef>

namespace
{
const int kTrackedKeys[] = {
    GLFW_KEY_W, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D,
    GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_LEFT, GLFW_KEY_RIGHT,
    GLFW_KEY_LEFT_SHIFT, GLFW_KEY_RIGHT_SHIFT,
    GLFW_KEY_E, GLFW_KEY_R, GLFW_KEY_ESCAPE,
};
}

void Input::init(GLFWwindow* window)
{
    window_ = window;
    current_.fill(false);
    previous_.fill(false);
}

void Input::update()
{
    previous_ = current_;
    for (int key : kTrackedKeys)
    {
        current_[static_cast<size_t>(key)] = (glfwGetKey(window_, key) == GLFW_PRESS);
    }
}

bool Input::down(int key) const
{
    return key >= 0 && key < kMaxKeys && current_[static_cast<size_t>(key)];
}

bool Input::pressed(int key) const
{
    return key >= 0 && key < kMaxKeys
        && current_[static_cast<size_t>(key)]
        && !previous_[static_cast<size_t>(key)];
}
