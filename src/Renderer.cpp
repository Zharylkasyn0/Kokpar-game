#include "Renderer.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Assets.h"
#include "Math.h"

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
    // Refined soft toon shading: 4 distinct stepped cel bands
    float toon = mix(d, floor(d * 4.0 + 0.5) / 4.0, 0.42);

    // Hemispheric ambient lighting with ground proximity ambient occlusion
    float ao = clamp(vWorldPos.y * 1.2 + 0.25, 0.72, 1.0);
    vec3 ambient = mix(uAmbientGround, uAmbientSky, N.y * 0.5 + 0.5) * ao;

    // Specular and rim highlights
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0) * 0.22 * step(0.05, ndl);
    float rim = pow(1.0 - max(dot(N, V), 0.0), 3.0) * 0.18;

    vec3 color = uColor * (ambient + uLightColor * toon) + uLightColor * spec + vec3(rim);

    // Atmospheric distance fog smoothly fading into the horizon sky
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

    // Generate deterministic celestial star dome
    Rng rng(421337u);
    stars_.clear();
    stars_.reserve(140);
    for (int i = 0; i < 140; ++i)
    {
        const float theta = rng.range(0.0f, kTwoPi);
        const float phi = rng.range(0.12f, kHalfPi * 0.95f); // Upper hemisphere
        const float x = std::cos(phi) * std::cos(theta);
        const float y = std::sin(phi);
        const float z = std::cos(phi) * std::sin(theta);

        Star s;
        s.dir = glm::normalize(glm::vec3(x, y, z));
        s.size = rng.range(0.9f, 2.2f);
        s.twinklePhase = rng.range(0.0f, kTwoPi);
        stars_.push_back(s);
    }

    return true;
}

void Renderer::beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos)
{
    LightingState def;
    def.toLight = glm::normalize(glm::vec3(-0.45f, 0.85f, 0.40f));
    def.lightColor = glm::vec3(1.00f, 0.94f, 0.80f);
    def.ambientSky = glm::vec3(0.48f, 0.56f, 0.68f);
    def.ambientGround = glm::vec3(0.34f, 0.30f, 0.22f);
    def.skyColor = glm::vec3(0.62f, 0.80f, 0.95f);
    def.fogColor = def.skyColor;
    def.fogStart = 80.0f;
    def.fogEnd = 310.0f;

    beginScene(view, projection, cameraPos, def);
}

void Renderer::beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, const LightingState& lighting)
{
    drawCalls_ = 0;
    shader_.use();

    const glm::mat4 viewProj = projection * view;
    glUniformMatrix4fv(uViewProj_, 1, GL_FALSE, glm::value_ptr(viewProj));

    skyColor_ = lighting.skyColor;

    glUniform3fv(uLightDir_, 1, glm::value_ptr(lighting.toLight));
    glUniform3fv(uLightColor_, 1, glm::value_ptr(lighting.lightColor));
    glUniform3fv(uAmbientSky_, 1, glm::value_ptr(lighting.ambientSky));
    glUniform3fv(uAmbientGround_, 1, glm::value_ptr(lighting.ambientGround));
    glUniform3fv(uCamPos_, 1, glm::value_ptr(cameraPos));
    glUniform3fv(uFogColor_, 1, glm::value_ptr(lighting.fogColor));
    glUniform1f(uFogStart_, lighting.fogStart);
    glUniform1f(uFogEnd_, lighting.fogEnd);
}

void Renderer::drawCelestials(const Assets& assets, const glm::vec3& cameraPos, const LightingState& lighting, float time)
{
    // 1. Sun Disc
    if (lighting.sunWorldDir.y > -0.15f)
    {
        const glm::vec3 sunPos = cameraPos + lighting.sunWorldDir * 420.0f;
        const float sunSize = 24.0f;
        const glm::vec3 sunCol = glm::mix(glm::vec3(1.0f, 0.65f, 0.25f), glm::vec3(1.0f, 0.98f, 0.85f),
                                          std::clamp(lighting.sunWorldDir.y * 3.0f, 0.0f, 1.0f));

        // Corona glow
        drawUnlit(assets.sphere(), Translate(sunPos) * Scale(sunSize * 1.8f), sunCol * 0.7f, 0.35f);
        // Sun core disc
        drawUnlit(assets.sphere(), Translate(sunPos) * Scale(sunSize), glm::vec3(1.0f, 1.0f, 0.95f), 1.0f);
    }

    // 2. Moon Disc
    if (lighting.moonWorldDir.y > -0.15f)
    {
        const glm::vec3 moonPos = cameraPos + lighting.moonWorldDir * 420.0f;
        const float moonSize = 16.0f;

        // Moon halo
        drawUnlit(assets.sphere(), Translate(moonPos) * Scale(moonSize * 1.6f), glm::vec3(0.50f, 0.65f, 0.95f), 0.25f);
        // Moon luminous silver core
        drawUnlit(assets.sphere(), Translate(moonPos) * Scale(moonSize), glm::vec3(0.92f, 0.95f, 1.00f), 0.95f);
    }

    // 3. Stars Dome at Night
    if (lighting.starVisibility > 0.02f)
    {
        const float alpha = lighting.starVisibility;
        for (const Star& s : stars_)
        {
            const float twinkle = 0.75f + 0.25f * std::sin(time * 3.5f + s.twinklePhase);
            const glm::vec3 starPos = cameraPos + s.dir * 430.0f;
            drawUnlit(assets.sphere(), Translate(starPos) * Scale(s.size * twinkle), glm::vec3(0.95f, 0.96f, 1.0f), alpha);
        }
    }
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
