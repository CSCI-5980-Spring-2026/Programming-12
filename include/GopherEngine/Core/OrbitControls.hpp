#pragma once

#include <GopherEngine/Core/Component.hpp>
#include <GopherEngine/Core/EventHandler.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace GopherEngine
{

    class OrbitControls: public Component, EventHandler
    {
        public:
            OrbitControls(
                const glm::vec3& target = {0.f, 0.f, 0.f},
                float distance = 1.f
            );

            void late_update(Node& node, float delta_time) override;

            void on_mouse_move(const MouseMoveEvent& event) override;
            void on_mouse_down(const MouseButtonEvent& event) override;
            void on_mouse_up(const MouseButtonEvent& event) override;
            void on_mouse_scroll(const MouseScrollEvent& event) override;

            glm::vec3 get_target_point() const;
            void set_target_point(const glm::vec3& target);

            float get_distance() const;
            void set_distance(float distance);

            float get_rotation_speed() const;
            void set_rotation_speed(float rotation_speed);

            float get_zoom_speed() const;
            void set_zoom_speed(float zoom_speed);

            glm::quat get_orbit_x() const;
            void set_orbit_x(const glm::quat& orbit_x);

            glm::quat get_orbit_y() const;
            void set_orbit_y(const glm::quat& orbit_y);

        private:
            float rotation_speed_{glm::pi<float>() / 4.f}; 
            float zoom_speed_{10.f};
            glm::vec3 target_point_{0.f, 0.f, 0.f};
            float distance_{1.f};
            glm::quat orbit_x_{1.f, 0.f, 0.f, 0.f};
            glm::quat orbit_y_{1.f, 0.f, 0.f, 0.f};
            glm::vec2 mouse_movement_{0.f, 0.f};
            float zoom_{0.f};
            bool mouse_drag_{false};
    };

}
