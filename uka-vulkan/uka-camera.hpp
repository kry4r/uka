#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace uka{

    struct Uka_Camera
    {
        private:
        float zn,zf,fov;

        auto update_view_martix()-> void
        {
            glm::mat4 currentMatrix = matrices.view;

		    glm::mat4 rotM = glm::mat4(1.0f);
		    glm::mat4 transM;

		    rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		    rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		    rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		    glm::vec3 translation = position;
		    if (flipY) {
		    	translation.y *= -1.0f;
		    }
		    transM = glm::translate(glm::mat4(1.0f), translation);

		    if (type == camera_type::first_person)
		    {
		    	matrices.view = rotM * transM;
		    }
		    else
		    {
		    	matrices.view = transM * rotM;
		    }

		    view_pos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

		    if (matrices.view != currentMatrix) {
		    	updated = true;
		    }
        }
        public:
        enum struct camera_type
        {
            look_at,
            first_person,
        };
        camera_type type = camera_type::look_at;
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 view_pos;

        float rotation_speed = 1.0f;
        float movement_speed = 1.0f;

        bool updated = true;
        bool flipY = false;

        struct
        {
            glm::mat4 perspective;
            glm::mat4 view;
        } matrices;

        struct
        {
            bool left = false;
            bool right = false;
            bool up = false;
            bool down = false;
        } keys;

        auto moving()->bool
        {
            return keys.left || keys.right || keys.up || keys.down;
        }

        auto get_near_clip()->float
        {
            return zn;
        }

        auto get_far_clip()->float
        {
            return zf;
        }

        auto set_perspective(float fov, float aspect, float zn, float zf)->void
        {
            auto current = matrices.perspective;
            this->zn = zn;
            this->zf = zf;
            this->fov = fov;
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, zn, zf);
            if (flipY)
            {
                matrices.perspective[1][1] *= -1.0f;
            }
            if (current != matrices.perspective)
            {
                updated = true;
            }
        }

        auto update_aspect_ratio(float aspect)->void
        {
            matrices.perspective = glm::perspective(glm::radians(fov), aspect, zn, zf);
            if (flipY)
            {
                matrices.perspective[1][1] *= -1.0f;
            }
            updated = true;
        }

        auto set_position(glm::vec3 position)->void
        {
            if (this->position != position)
            {
                this->position = position;
                update_view_martix();
            }
        }

        auto set_rotation(glm::vec3 rotation)->void
        {
            if (this->rotation != rotation)
            {
                this->rotation = rotation;
                update_view_martix();
            }
        }

        auto rotate(glm::vec3 delta)->void
        {
            rotation += delta;
            update_view_martix();
        }

        auto set_translation(glm::vec3 translation)->void
        {
            position = translation;
            update_view_martix();
        }

        auto set_rotation_speed(float rotation_speed)->void
        {
            this->rotation_speed = rotation_speed;
        }

        auto set_movement_speed(float movement_speed)->void
        {
            this->movement_speed = movement_speed;
        }

        auto update(float delta_time)->void
        {
            updated = false;
            if (type == camera_type::first_person)
            {
                if (moving())
                {
                    glm::vec3 camFront;
                    camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                    camFront.y = sin(glm::radians(rotation.x));
                    camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                    camFront = glm::normalize(camFront);

                    float move_speed = delta_time * movement_speed;

                    if (keys.up)
                    {
                        position += camFront * move_speed;
                    }
                    if (keys.down)
                    {
                        position -= camFront * move_speed;
                    }
                    if (keys.left)
                    {
                        position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
                    }
                    if (keys.right)
                    {
                        position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * move_speed;
                    }

                    update_view_martix();
                }
            }

        }

        auto update_pad(glm::vec2 axis_left,glm::vec2 axis_right,float delta_time)
        {
            auto ret_val =false;

            if (type == camera_type::first_person)
            {
                const float dead_zone = 0.0015f;
                const float range = 1.0f - dead_zone;

                glm::vec3 camFront;
                camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
                camFront.y = sin(glm::radians(rotation.x));
                camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
                camFront = glm::normalize(camFront);

                float move_speed = delta_time * movement_speed * 2.0f;
                float rot_speed = delta_time * rotation_speed * 50.0f;

                if (fabs(axis_left.y) > dead_zone)
                {
                    float pos = (fabs(axis_left.y) - dead_zone) / range;
                    position += camFront * pos * ((axis_left.y < 0.0f) ? -1.0f : 1.0f) * move_speed;
                    ret_val = true;
                }
                if (fabs(axis_left.x) > dead_zone)
                {
                    float pos = (fabs(axis_left.x) - dead_zone) / range;
                    position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axis_left.x < 0.0f) ? -1.0f : 1.0f) * move_speed;
                    ret_val = true;
                }

                if (fabs(axis_right.x) > dead_zone)
                {
                    float pos = (fabs(axis_right.x) - dead_zone) / range;
                    rotation.y += pos * ((axis_right.x < 0.0f) ? -1.0f : 1.0f) * rot_speed;
                    ret_val = true;
                }
                if (fabs(axis_right.y) > dead_zone)
                {
                    float pos = (fabs(axis_right.y) - dead_zone) / range;
                    rotation.x -= pos * ((axis_right.y < 0.0f) ? -1.0f : 1.0f) * rot_speed;
                    ret_val = true;
                }

                if (ret_val)
                {
                    update_view_martix();
                }
                return ret_val;

            }

        }
    };
}
