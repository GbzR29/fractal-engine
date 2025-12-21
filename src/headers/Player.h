#include "Camera.h"
#include "Input.h"

class Player {
public:
    Player() = default;

    void update(float dt, Input& input);
    void applyGravity(float dt);

    void renderCamera(Shader& shader, int w, int h);

private:
    Camera camera;

    glm::vec3 position{8.0f, 8.0f, 10.0f};
    glm::vec3 velocity{0.0f};

    float speed   = 5.0f;
    float gravity = -9.8f;
};
