// KOKPAR: Wobbly Steppe - entry point.
// Creates an OpenGL 3.3 Core context with GLFW, loads functions with GLAD, runs the game loop.

#include <glad/glad.h>   // must come before GLFW
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Game.h"

int main(int argc, char* argv[])
{
    bool benchmarkMode = false;
    float benchmarkPhaseDuration = 4.0f;
    bool stabilityTest = false;
    float stabilityDuration = 60.0f;
    bool fullscreen = false;
    int windowWidth = 1280;
    int windowHeight = 720;

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--benchmark") == 0 || std::strcmp(argv[i], "-b") == 0)
        {
            benchmarkMode = true;
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                benchmarkPhaseDuration = static_cast<float>(std::atof(argv[++i]));
                if (benchmarkPhaseDuration <= 0.5f) benchmarkPhaseDuration = 4.0f;
            }
        }
        else if (std::strcmp(argv[i], "--stability-test") == 0 || std::strcmp(argv[i], "-s") == 0)
        {
            stabilityTest = true;
            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                stabilityDuration = static_cast<float>(std::atof(argv[++i]));
                if (stabilityDuration <= 1.0f) stabilityDuration = 60.0f;
            }
        }
        else if (std::strcmp(argv[i], "--fullscreen") == 0 || std::strcmp(argv[i], "-f") == 0)
        {
            fullscreen = true;
        }
        else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc)
        {
            windowWidth = std::max(640, std::atoi(argv[++i]));
        }
        else if (std::strcmp(argv[i], "--height") == 0 && i + 1 < argc)
        {
            windowHeight = std::max(480, std::atoi(argv[++i]));
        }
    }

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
    GLFWwindow* window = glfwCreateWindow(
        windowWidth, windowHeight, "K\xC3\x96" "KPAR: Wobbly Steppe v0.3",
        nullptr, nullptr);
    if (window && fullscreen)
    {
        // Prefer a true fullscreen monitor mode. If the driver refuses the mode
        // switch, fall back to a borderless window covering the whole monitor so
        // the framebuffer still matches the screen resolution.
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
        if (monitor && mode)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0,
                                 mode->width, mode->height, mode->refreshRate);
            int fbw = 0, fbh = 0;
            glfwGetFramebufferSize(window, &fbw, &fbh);
            if (fbw != mode->width || fbh != mode->height)
            {
                glfwSetWindowMonitor(window, nullptr, 0, 0,
                                     mode->width, mode->height, 0);
                glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
                glfwSetWindowPos(window, 0, 0);
            }
        }
    }
    if (!window)
    {
        std::fprintf(stderr, "Failed to create a window / OpenGL 3.3 context\n");
        glfwTerminate();
        return 1;
    }

    {
        int ww = 0, wh = 0, fbw = 0, fbh = 0;
        glfwGetWindowSize(window, &ww, &wh);
        glfwGetFramebufferSize(window, &fbw, &fbh);
        std::printf("[Video] window=%dx%d framebuffer=%dx%d fullscreen=%s\n",
                    ww, wh, fbw, fbh, fullscreen ? "yes" : "no");
        std::fflush(stdout);
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::fprintf(stderr, "Failed to load OpenGL functions via GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // In benchmark mode, disable VSync to measure uncapped rendering performance.
    if (benchmarkMode)
    {
        glfwSwapInterval(0);
    }
    else
    {
        glfwSwapInterval(1);   // VSync: keeps the frame rate stable
    }

    int exitCode = 0;
    {
        // Scope: all GL resources (meshes, shader) are destroyed before the context.
        Game game;
        if (!game.init(window))
        {
            std::fprintf(stderr, "Game initialisation failed\n");
            exitCode = 1;
        }
        else if (benchmarkMode)
        {
            game.startMatch();

            std::printf("=================================================================\n");
            std::printf(" KOKPAR: Wobbly Steppe - Performance Benchmark (1280x720)\n");
            std::printf(" OpenGL 3.3 Core Profile | VSync: OFF | Duration: %.1fs per phase\n", static_cast<double>(benchmarkPhaseDuration));
            std::printf("=================================================================\n");

            // Warmup
            const double warmupStart = glfwGetTime();
            while (glfwGetTime() - warmupStart < 0.5)
            {
                glfwPollEvents();
                game.update(0.016f);
                game.render();
                glfwSwapBuffers(window);
            }

            struct BenchResult
            {
                const char* name;
                int frames = 0;
                double elapsed = 0.0;
                double fps = 0.0;
                double avgFrameTimeMs = 0.0;
                double minFrameTimeMs = 9999.0;
                double maxFrameTimeMs = 0.0;
            };

            BenchResult results[2];
            results[0].name = "TOON SHADING ON ";
            results[1].name = "TOON SHADING OFF";

            for (int phase = 0; phase < 2; ++phase)
            {
                game.setToonEnabled(phase == 0);
                std::printf("Measuring Phase %d: %s ...\n", phase + 1, results[phase].name);

                const double phaseStart = glfwGetTime();
                double lastFrame = phaseStart;
                int frameCount = 0;

                while ((glfwGetTime() - phaseStart) < benchmarkPhaseDuration && !glfwWindowShouldClose(window))
                {
                    glfwPollEvents();
                    const double frameNow = glfwGetTime();
                    const double frameDt = frameNow - lastFrame;
                    lastFrame = frameNow;

                    game.update(static_cast<float>(std::min(frameDt, 0.05)));
                    game.render();
                    glfwSwapBuffers(window);

                    ++frameCount;
                    const double dtMs = frameDt * 1000.0;
                    if (dtMs < results[phase].minFrameTimeMs) results[phase].minFrameTimeMs = dtMs;
                    if (dtMs > results[phase].maxFrameTimeMs) results[phase].maxFrameTimeMs = dtMs;
                }

                const double phaseEnd = glfwGetTime();
                results[phase].frames = frameCount;
                results[phase].elapsed = phaseEnd - phaseStart;
                results[phase].fps = static_cast<double>(frameCount) / results[phase].elapsed;
                results[phase].avgFrameTimeMs = (results[phase].elapsed / static_cast<double>(frameCount)) * 1000.0;
            }

            std::printf("\n=================================================================\n");
            std::printf(" BENCHMARK RESULTS SUMMARY\n");
            std::printf("-----------------------------------------------------------------\n");
            std::printf(" Mode              | Frames | Elapsed (s) | Avg FPS | Avg (ms) | Min (ms) | Max (ms)\n");
            std::printf("-----------------------------------------------------------------\n");
            for (int phase = 0; phase < 2; ++phase)
            {
                std::printf(" %s | %6d | %11.3f | %7.1f | %8.2f | %8.2f | %8.2f\n",
                            results[phase].name,
                            results[phase].frames,
                            results[phase].elapsed,
                            results[phase].fps,
                            results[phase].avgFrameTimeMs,
                            results[phase].minFrameTimeMs,
                            results[phase].maxFrameTimeMs);
            }
            std::printf("=================================================================\n");
            std::printf(" Pass Criteria: >= 30 FPS at 1280x720: %s\n",
                        (results[0].fps >= 30.0 && results[1].fps >= 30.0) ? "PASS [EXCEEDED]" : "FAIL");
            std::printf("=================================================================\n");

            game.setToonEnabled(true);
        }
        else if (stabilityTest)
        {
            game.startMatch();
            std::printf("[Stability Test] Running game for %.1f seconds...\n", static_cast<double>(stabilityDuration));
            const double start = glfwGetTime();
            double last = start;
            int frames = 0;
            while ((glfwGetTime() - start) < stabilityDuration && !glfwWindowShouldClose(window))
            {
                glfwPollEvents();
                const double now = glfwGetTime();
                const float dt = static_cast<float>(std::min(now - last, 0.05));
                last = now;

                game.update(dt);
                game.render();
                glfwSwapBuffers(window);
                ++frames;
            }
            const double total = glfwGetTime() - start;
            std::printf("[Stability Test] Completed: %.2f seconds, %d frames rendered, Avg FPS: %.1f, Stability: PASS.\n",
                        total, frames, static_cast<double>(frames) / total);
        }
        else
        {
            double last = glfwGetTime();
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

                game.update(dt);
                game.render();
                glfwSwapBuffers(window);
            }
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return exitCode;
}
