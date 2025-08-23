#pragma once
#include <glm/glm.hpp>
#include <algorithm>
#include "light/light.h"   // assumes LightBase has: position, color, intensity (I_base), etc.
#include "shader.h"
#include "config.h"

class SpotLight : public LightBase {
public:
    // cosIn/cosOut are cosines of HALF angles (not radians). direction will be normalized.
    SpotLight(glm::vec3 lightPos,
              glm::vec3 lightColorLinear,
              glm::vec3 lightDirection,
              float intensityIBase,   // I_base (cd). If you prefer lumens, call SetLumens() after setting angles.
              float cosOuterHalf,     // cos(outerHalfAngle)
              float cosInnerHalf,     // cos(innerHalfAngle)  -- must be >= cosOuterHalf
              float attenuationRadius /*R*/);

    // Frostbite flux model: Φ = π I  ->  I = Φ / π (independent of cone angle)
    void SetLumens(float lumens);                 

    // --- Degree-based angle setters (more author-friendly)
    void SetAnglesDegrees(float innerDeg, float outerDeg); // order-insensitive; will enforce inner>=outer
    void SetInnerDegrees(float innerDeg);
    void SetOuterDegrees(float outerDeg);

    // --- Cosine-based setters (if you already have cosines)
    void SetCosIn(float v);
    void SetCosOut(float v);

    // --- Other setters
    void SetDirection(const glm::vec3& d);       // will normalize
    void SetAttenuationRadius(float R);          // sets R and precomputes 1/R^2; clamps to a small positive value

    // GPU upload (uniforms or UBO fields)
    void UploadToShader(const std::shared_ptr<Shader>& shader) override;

    // Accessors
    float GetCosIn()  const { return cosIn;  }
    float GetCosOut() const { return cosOut; }
    float GetAngleScale()  const { return angleScale; }
    float GetAngleOffset() const { return angleOffset; }
    float GetInvSqrAttRadius() const { return invSqrAttRadius; }

private:
    // Recompute scale/offset used by the angle attenuation: t = saturate(cd * scale + offset)
    void RecalcAngleParams();

    // Helpers
    static inline float SafeInv(float x, float eps = 1e-6f) { return 1.0f / std::max(x, eps); }

private:
    glm::vec3 lightDirection;  // normalized
    float cosOut;              // cos(outerHalfAngle)
    float cosIn;               // cos(innerHalfAngle)  (must satisfy cosIn >= cosOut)
    float angleScale = 1.0f;   // 1 / max(ε, cosIn - cosOut)
    float angleOffset = 0.0f;  // -cosOut * angleScale
    float attRadius;           // distance window radius R
    float invSqrAttRadius = 0.0f; // 1 / (R*R)
};
