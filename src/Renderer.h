#pragma once

#include <glm/glm.hpp>

#include "Mesh.h"
#include "Shader.h"

// Owns the main shader and issues draw calls. One directional light, hemispheric
// ambient, soft toon banding and distance fog (all in GLSL, see Renderer.cpp).
class Renderer
{
public:
    bool init();

    // Sets camera + lighting uniforms. Call once per frame before drawing the scene.
    void beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);

    // Lit draw (directional light + fog).
    void draw(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha = 1.0f);
    // Flat-colour draw (no lighting, no fog). Used for blob shadows and HUD.
    void drawUnlit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha = 1.0f);

    // Pixel-space orthographic mode for the HUD (origin = bottom-left corner).
    void beginHud(int width, int height);
    void endHud();

    glm::vec3 skyColor() const { return glm::vec3(0.62f, 0.80f, 0.95f); }
    int drawCalls() const { return drawCalls_; }

private:
    void submit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha, bool unlit);

    Shader shader_;
    int drawCalls_ = 0;

    GLint uViewProj_ = -1;
    GLint uModel_ = -1;
    GLint uNormalMat_ = -1;
    GLint uColor_ = -1;
    GLint uAlpha_ = -1;
    GLint uUnlit_ = -1;
    GLint uLightDir_ = -1;
    GLint uLightColor_ = -1;
    GLint uAmbientSky_ = -1;
    GLint uAmbientGround_ = -1;
    GLint uCamPos_ = -1;
    GLint uFogColor_ = -1;
    GLint uFogStart_ = -1;
    GLint uFogEnd_ = -1;
};
