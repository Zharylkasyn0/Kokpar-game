#pragma once

#include <glad/glad.h>

// Minimal GLSL program wrapper (vertex + fragment).
class Shader
{
public:
    Shader() = default;
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Compiles and links. Prints the info log to stderr and returns false on failure.
    bool build(const char* vertexSource, const char* fragmentSource);

    void use() const { glUseProgram(program_); }
    GLint uniform(const char* name) const { return glGetUniformLocation(program_, name); }

private:
    GLuint program_ = 0;
};
