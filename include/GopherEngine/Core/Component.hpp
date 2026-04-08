#pragma once

#include <GopherEngine/Core/RenderContext.hpp>

#include <glm/glm.hpp> // general glm header for basic types like vec3 and mat4
#include <glm/gtc/quaternion.hpp> // glm quaternion functions

#include <vector>
#include <memory>

namespace GopherEngine
{
    class Node;

    class Component
    {
        public:
            Component() = default;
            virtual ~Component() = default;
            virtual void initialize(Node& node) {};
            virtual void physics_update(Node& node, float delta_time) {};
            virtual void update(Node& node, float delta_time) {};
            virtual void late_update(Node& node, float delta_time) {};
            virtual void update_render_context(const glm::mat4 &local_matrix, const glm::mat4 &world_matrix, RenderContext& context) {};
            virtual void draw(const glm::mat4 &world_matrix, const RenderContext& context) {};
    };
}
