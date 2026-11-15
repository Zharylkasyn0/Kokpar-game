#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "Shader.h"
#include "Lighting.h"

class Assets;

// Owns the main shader and issues draw calls. One directional light, hemispheric
// ambient, soft toon banding, celestial discs, stars, and dynamic distance fog.
class Renderer
{
public:
    bool init();

    // Sets camera + lighting uniforms. Call once per frame before drawing the scene.
    void beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);
    void beginScene(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, const LightingState& lighting);

    // Celestial objects (sun, moon, stars)
    void drawCelestials(const Assets& assets, const glm::vec3& cameraPos, const LightingState& lighting, float time);

    // Lit draw (directional light + fog).
    void draw(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha = 1.0f);
    // Flat-colour draw (no lighting, no fog). Used for blob shadows and HUD.
    void drawUnlit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha = 1.0f);

    // Pixel-space orthographic mode for the HUD (origin = bottom-left corner).
    void beginHud(int width, int height);
    void endHud();

    // Main demonstrated graphics technique: Toon/Cel Shading toggle
    bool isToonEnabled() const { return toonEnabled_; }
    void setToonEnabled(bool on) { toonEnabled_ = on; }
    void toggleToon() { toonEnabled_ = !toonEnabled_; }

    void setSkyColor(const glm::vec3& col) { skyColor_ = col; }
    glm::vec3 skyColor() const { return skyColor_; }
    int drawCalls() const { return drawCalls_; }

private:
    void submit(const Mesh& mesh, const glm::mat4& model, const glm::vec3& color, float alpha, bool unlit);

    Shader shader_;
    int drawCalls_ = 0;
    bool toonEnabled_ = true;
    glm::vec3 skyColor_ = glm::vec3(0.62f, 0.80f, 0.95f);

    // Starfield points
    struct Star {
        glm::vec3 dir;
        float size;
        float twinklePhase;
    };
    std::vector<Star> stars_;

    GLint uViewProj_ = -1;
    GLint uModel_ = -1;
    GLint uNormalMat_ = -1;
    GLint uColor_ = -1;
    GLint uAlpha_ = -1;
    GLint uUnlit_ = -1;
    GLint uToon_ = -1;
    GLint uLightDir_ = -1;
    GLint uLightColor_ = -1;
    GLint uAmbientSky_ = -1;
    GLint uAmbientGround_ = -1;
    GLint uCamPos_ = -1;
    GLint uFogColor_ = -1;
    GLint uFogStart_ = -1;
    GLint uFogEnd_ = -1;
};
