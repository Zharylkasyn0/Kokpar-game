#include "Shader.h"

#include <iostream>
#include <string>

namespace
{
GLuint Compile(GLenum type, const char* source, const char* label)
{
    const GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE)
    {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetShaderInfoLog(shader, len, nullptr, &log[0]);
        std::cerr << "[Shader] " << label << " compile error:\n" << log << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
}

Shader::~Shader()
{
    if (program_ != 0) glDeleteProgram(program_);
}

bool Shader::build(const char* vertexSource, const char* fragmentSource)
{
    const GLuint vs = Compile(GL_VERTEX_SHADER, vertexSource, "vertex");
    const GLuint fs = Compile(GL_FRAGMENT_SHADER, fragmentSource, "fragment");
    if (vs == 0 || fs == 0)
    {
        if (vs != 0) glDeleteShader(vs);
        if (fs != 0) glDeleteShader(fs);
        return false;
    }

    program_ = glCreateProgram();
    glAttachShader(program_, vs);
    glAttachShader(program_, fs);
    glLinkProgram(program_);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = GL_FALSE;
    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE)
    {
        GLint len = 0;
        glGetProgramiv(program_, GL_INFO_LOG_LENGTH, &len);
        std::string log(static_cast<size_t>(len > 1 ? len : 1), '\0');
        glGetProgramInfoLog(program_, len, nullptr, &log[0]);
        std::cerr << "[Shader] link error:\n" << log << std::endl;
        glDeleteProgram(program_);
        program_ = 0;
        return false;
    }
    return true;
}
