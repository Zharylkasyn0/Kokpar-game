#include "Renderer.h"

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
const char* kVertexShader = R"GLSL(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uViewProj;
uniform mat3 uNormalMat;

out vec3 vWorldPos;
out vec3 vNormal;

void main()
{
    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vNormal = uNormalMat * aNormal;
    gl_Position = uViewProj * world;
}
)GLSL";

const char* kFragmentShader = R"GLSL(
#version 330 core
in vec3 vWorldPos;
in vec3 vNormal;

out vec4 FragColor;

uniform vec3  uColor;
uniform float uAlpha;
uniform float uUnlit;

uniform vec3  uLightDir;       // direction TOWARD the light (normalized)
uniform vec3  uLightColor;
uniform vec3  uAmbientSky;
uniform vec3  uAmbientGround;
uniform vec3  uCamPos;
uniform vec3  uFogColor;
uniform float uFogStart;
uniform float uFogEnd;

void main()
{
    if (uUnlit > 0.5)
    {
        FragColor = vec4(uColor, uAlpha);
        return;
    }

    vec3 N = normalize(vNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = normalize(uCamPos - vWorldPos);

    float ndl = dot(N, L);
    float d = max(ndl, 0.0);
    // Soft toon: blend smooth diffuse with 4 quantised bands.
    float toon = mix(d, floor(d * 3.0 + 0.5) / 3.0, 0.45);

    vec3 ambient = mix(uAmbientGround, uAmbientSky, N.y * 0.5 + 0.5);

    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 40.0) * 0.18 * step(0.0, ndl);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 3.0) * 0.16;

    vec3 color = uColor * (ambient + uLightColor * toon) + uLightColor * spec + vec3(rim);

    float dist = length(uCamPos - vWorldPos);
    float fog = clamp((dist - uFogStart) / (uFogEnd - uFogStart), 0.0, 1.0);
    color = mix(color, uFogColor, fog);

    FragColor = vec4(color, uAlpha);
}
)GLSL";
}

bool Renderer::init()
{
    if (!shader_.build(kVertexShader, kFragmentShader)) return false;

    uViewProj_ = shader_.uniform("uViewProj");
    uModel_ = shader_.uniform("uModel");
    uNormalMat_ = shader_.uniform("uNormalMat");
    uColor_ = shader_.uniform("uColor");
    uAlpha_ = shader_.uniform("uAlpha");
    uUnlit_ = shader_.uniform("uUnlit");
    uLightDir_ = shader_.uniform("uLightDir");
    uLightColor_ = shader_.uniform("uLightColor");
    uAmbientSky_ = shader_.uniform("uAmbientSky");
    uAmbientGround_ = shader_.uniform("uAmbientGround");
    uCamPos_ = shader_.uniform("uCamPos");
    uFogColor_ = shader_.uniform("uFogColor");
    uFogStart_ = shader_.uniform("uFogStart");
    uFogEnd_ = shader_.uniform("uFogEnd");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Face culling is intentionally left disabled: robust against any winding mistakes.
    return true;
}

void Renderer::beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos)
{
    drawCalls_ = 0;
    shader_.use();

    const glm::mat4 viewProj = projection * view;
    glUniformMatrix4fv(uViewProj_, 1, GL_FALSE, glm::value_ptr(viewProj));

    const glm::vec3 toLight = glm::normalize(glm::vec3(-0.45f, 0.80f, 0.40f));
    const glm::vec3 lightColor(0.95f, 0.90f, 0.78f);
    const glm::vec3 ambientSky(0.44f, 0.50f, 0.60f);
    const glm::vec3 ambientGround(0.30f, 0.27f, 0.20f);
    const glm::vec3 fogColor = skyColor();

    glUniform3fv(uLightDir_, 1, glm::value_ptr(toLight));
    glUniform3fv(uLightColor_, 1, glm::value_ptr(lightColor));
    glUniform3fv(uAmbientSky_, 1, glm::value_ptr(ambientSky));
    glUniform3fv(uAmbientGround_, 1, glm::value_ptr(ambientGround));
    glUniform3fv(uCamPos_, 1, glm::value_ptr(cameraPos));
    glUniform3fv(uFogColor_, 1, glm::value_ptr(fogColor));
    glUniform1f(uFogStart_, 70.0f);
    glUniform1f(uFogEnd_, 280.0f);
}

void Renderer::submit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha, bool unlit)
{
    const glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(model)));
    glUniformMatrix4fv(uModel_, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix3fv(uNormalMat_, 1, GL_FALSE, glm::value_ptr(normalMat));
    glUniform3fv(uColor_, 1, glm::value_ptr(color));
    glUniform1f(uAlpha_, alpha);
    glUniform1f(uUnlit_, unlit ? 1.0f : 0.0f);
    mesh.draw();
    ++drawCalls_;
}

void Renderer::draw(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha)
{
    submit(mesh, model, color, alpha, false);
}

void Renderer::drawUnlit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha)
{
    submit(mesh, model, color, alpha, true);
}

void Renderer::beginHud(int width, int height)
{
    shader_.use();
    const glm::mat4 ortho = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
    glUniformMatrix4fv(uViewProj_, 1, GL_FALSE, glm::value_ptr(ortho));
    glDisable(GL_DEPTH_TEST);
}

void Renderer::endHud()
{
    glEnable(GL_DEPTH_TEST);
}
