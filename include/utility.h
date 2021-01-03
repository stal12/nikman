#if !defined NIKMAN_UTILITY_H
#define NIKMAN_UTILITY_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static constexpr char* const kShaderRoot = "../shaders";
static constexpr char* const kTextureRoot = "../resources";

static constexpr float kRatio = 16.f / 9.f;
static constexpr float kWorldHeight = 6.f;
static constexpr float kWorldWidth = kWorldHeight * kRatio;

static const glm::mat4 kProjection = glm::ortho(-kWorldWidth / 2, kWorldWidth / 2, -kWorldHeight / 2, kWorldHeight / 2, 0.1f, 10.f);



#endif // NIKMAN_UTILITY_H