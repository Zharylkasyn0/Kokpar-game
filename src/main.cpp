// KOKPAR: Wobbly Steppe - entry point.
// Creates an OpenGL 3.3 Core context with GLFW, loads functions with GLAD, runs the game loop.

#include <glad/glad.h>   // must come before GLFW
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>

#include "Game.h"

int main()
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "Failed to initialise GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    // "\xC3\x96" = UTF-8 for the letter O with umlaut.
    GLFWwindow* window = glfwCreateWindow(1280, 720, "K\xC3\x96" "KPAR: Wobbly Steppe", nullptr, nullptr);
    if (!window)
    {
        std::fprintf(stderr, "Failed to create a window / OpenGL 3.3 context\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::fprintf(stderr, "Failed to load OpenGL functions via GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    glfwSwapInterval(1);   // VSync: keeps the frame rate stable

    int exitCode = 0;
    {
        std::fprintf(stderr, "[DEBUG] Creating Game object\n");
        // Scope: all GL resources (meshes, shader) are destroyed before the context.
        Game game;
        std::fprintf(stderr, "[DEBUG] Game object created, calling game.init\n");
        if (!game.init(window))
        {
            std::fprintf(stderr, "Game initialisation failed\n");
            exitCode = 1;
        }
        else
        {
            std::fprintf(stderr, "[DEBUG] Game initialised, entering loop\n");
            double last = glfwGetTime();
            int frame = 0;
            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();

                if (glfwGetWindowAttrib(window, GLFW_ICONIFIED))
                {
                    glfwWaitEventsTimeout(0.1);
                    last = glfwGetTime();
                    continue;
                }

                const double now = glfwGetTime();
                // Clamp dt so a hitch (window drag, breakpoint) cannot tunnel through obstacles.
                const float dt = static_cast<float>(std::min(now - last, 0.05));
                last = now;

                if (frame < 5) std::fprintf(stderr, "[DEBUG] frame %d update start\n", frame);
                game.update(dt);
                if (frame < 5) std::fprintf(stderr, "[DEBUG] frame %d render start\n", frame);
                game.render();
                if (frame < 5) std::fprintf(stderr, "[DEBUG] frame %d swapBuffers start\n", frame);
                glfwSwapBuffers(window);
                if (frame < 5) std::fprintf(stderr, "[DEBUG] frame %d complete\n", frame);
                frame++;
            }
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return exitCode;
}
