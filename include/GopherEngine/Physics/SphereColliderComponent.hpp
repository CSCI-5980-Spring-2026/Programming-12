#pragma once

#include <GopherEngine/Physics/ColliderComponent.hpp>

namespace GopherEngine
{
    // Concrete collider component that represents a sphere in engine terms.
    class SphereColliderComponent : public ColliderComponent
    {
        public:
            // Creates a sphere collider with the provided engine-space radius.
            explicit SphereColliderComponent(float radius = 0.5f);

            // Returns the authored engine-space radius.
            float get_radius() const;

        protected:
            // Creates the corresponding Bullet sphere shape during initialization.
            ColliderId register_with_physics(PhysicsWorld& physics_world, Node& node) override;

        private:
            // The engine-authored sphere radius, stored independently from Bullet.
            float radius_{0.5f};
    };
}
