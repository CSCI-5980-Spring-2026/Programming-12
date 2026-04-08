#pragma once

#include <GopherEngine/Core/Component.hpp>
#include <GopherEngine/Physics/PhysicsWorld.hpp>

#include <vector>

namespace GopherEngine
{
    // Abstract base class shared by all collider components.
    // It owns the registration lifecycle with PhysicsWorld and exposes
    // simple collision state queries to the rest of the engine.
    class ColliderComponent : public Component
    {
        public:
            // Unregisters the collider from PhysicsWorld if it was registered.
            ~ColliderComponent() override;

            // Stores the owning node and registers the concrete collider shape with PhysicsWorld.
            void initialize(Node& node) override;

            // Returns whether this collider overlapped anything during the last physics update.
            bool is_colliding() const;

            // Returns the engine-side id assigned by PhysicsWorld.
            ColliderId get_collider_id() const;

            // Returns the cached list of overlapping collider ids from the last physics update.
            const std::vector<ColliderId>& get_overlapping_colliders() const;

        protected:
            ColliderComponent() = default;

            // Implemented by each concrete collider type to create the proper
            // Bullet shape during initialization.
            virtual ColliderId register_with_physics(PhysicsWorld& physics_world, Node& node) = 0;

        private:
            // PhysicsWorld updates the collision flag directly after each detection pass.
            friend class PhysicsWorld;

             // Engine-facing identifier assigned when the collider registers.
            ColliderId collider_id_{InvalidColliderId};

            // Cached list of overlapping collider ids from the last physics update.
            std::vector<ColliderId> overlapping_colliders_;
    };
}