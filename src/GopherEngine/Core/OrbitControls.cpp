#include <GopherEngine/Core/OrbitControls.hpp>
#include <GopherEngine/Core/Node.hpp>

using namespace glm;

#include <iostream>
using namespace std;

namespace GopherEngine {

    OrbitControls::OrbitControls(const glm::vec3& target, float distance) 
    {
        target_point_ = target;
        distance_ = distance;
    }

    void OrbitControls::late_update(Node& node, float delta_time)
    {
        auto& transform = node.transform();
        orbit_x_ = orbit_x_ * angleAxis(mouse_movement_.x * rotation_speed_ * delta_time, vec3(0.f, 1.f, 0.f));
        orbit_y_ = orbit_y_ * angleAxis(mouse_movement_.y * rotation_speed_ * delta_time, vec3(1.f, 0.f, 0.f));
        distance_ += zoom_ * zoom_speed_ * delta_time;

        // Reset the rotation direction and zoom after applying them
        mouse_movement_ = vec2(0.f, 0.f);
        zoom_ = 0.f;

        transform.rotation_ = orbit_x_ * orbit_y_;
        transform.position_ = target_point_ + transform.rotation_ * vec3(0.f, 0.f, distance_);
    }

    void OrbitControls::on_mouse_move(const MouseMoveEvent& event) {
        if(mouse_drag_) {
            mouse_movement_ += vec2(-event.delta_x, -event.delta_y);
        }
    }
            
    void OrbitControls::on_mouse_down(const MouseButtonEvent& event) {
        if(event.button == MouseButton::Left) {
            mouse_drag_ = true;
        }
    }

    void OrbitControls::on_mouse_up(const MouseButtonEvent& event) {
        if(event.button == MouseButton::Left) {
            mouse_drag_ = false;
            mouse_movement_ = vec2(0.f, 0.f);
        }
    }

    void OrbitControls::on_mouse_scroll(const MouseScrollEvent& event) {
        zoom_ -= event.delta;
    }

    glm::vec3 OrbitControls::get_target_point() const
    {
        return target_point_;
    }

    void OrbitControls::set_target_point(const glm::vec3& target)
    {
        target_point_ = target;
    }

    float OrbitControls::get_distance() const
    {
        return distance_;
    }

    void OrbitControls::set_distance(float distance)
    {
        distance_ = distance;
    }

    float OrbitControls::get_rotation_speed() const
    {
        return rotation_speed_;
    }

    void OrbitControls::set_rotation_speed(float rotation_speed)
    {
        rotation_speed_ = rotation_speed;
    }

    float OrbitControls::get_zoom_speed() const
    {
        return zoom_speed_;
    }

    void OrbitControls::set_zoom_speed(float zoom_speed)
    {
        zoom_speed_ = zoom_speed;
    }

    glm::quat OrbitControls::get_orbit_x() const
    {
        return orbit_x_;
    }

    void OrbitControls::set_orbit_x(const glm::quat& orbit_x)
    {
        orbit_x_ = orbit_x;
    }

    glm::quat OrbitControls::get_orbit_y() const
    {
        return orbit_y_;
    }

    void OrbitControls::set_orbit_y(const glm::quat& orbit_y)
    {
        orbit_y_ = orbit_y;
    }

}
