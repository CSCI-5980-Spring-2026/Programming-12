#pragma once

#include <GopherEngine/Physics/ColliderComponent.hpp>

#include <glm/vec3.hpp>

namespace GopherEngine
{
    // Concrete collider component that represents an oriented box in engine terms.
    class BoxColliderComponent : public ColliderComponent
    {
        public:
            // Creates a box collider using full engine-space dimensions.
            explicit BoxColliderComponent(const glm::vec3& size = glm::vec3(1.f));

            // Returns the authored full box dimensions.
            glm::vec3 get_size() const;

        protected:
            // Creates the corresponding Bullet box shape during initialization.
            ColliderId register_with_physics(PhysicsWorld& physics_world, Node& node) override;

        private:
            // The engine-authored full box size, not Bullet's half-extents.
            glm::vec3 size_{1.f, 1.f, 1.f};
    };
}
