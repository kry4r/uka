#include "application.hpp"

namespace uka
{
    Uka_application::Uka_application()
    {
        camera.type = Camera::camera_type::look_at;
        camera.movement_speed = 1.0f;
        camera.rotation_speed = 1.0f;
        camera.position = glm::vec3(0.0f, 0.0f, 1.0f);
        camera.set_rotation(glm::vec3(0.0f, 0.0f, 0.0f));
        camera.set_perspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
    }
} // namespace uka

