#pragma once

#include <GopherEngine/Core/Service.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

// Forward declarations for Bullet types so the header does not need to include Bullet headers.
class btBroadphaseInterface;
class btCollisionConfiguration;
class btCollisionDispatcher;
class btCollisionObject;
class btCollisionShape;
class btCollisionWorld;

namespace GopherEngine
{
    class ColliderComponent;
    class BoxColliderComponent;
    class SphereColliderComponent;
    class Node;

    // A simple engine-facing identifier so game code does not need to know
    // anything about Bullet object pointers or internal storage.
    using ColliderId = std::uint32_t;
    constexpr ColliderId InvalidColliderId = 0;

    // Represents one overlap detected during the most recent physics update.
    struct CollisionPair
    {
        ColliderId a_;
        ColliderId b_;
    };

    // PhysicsWorld is the engine-facing wrapper around Bullet's collision-only API.
    // For this first pass it answers only one question: which colliders overlap?
    class PhysicsWorld : public Service<PhysicsWorld>
    {
        public:
            // Builds the Bullet collision world and the supporting objects it needs.
            PhysicsWorld();

            // Declared out-of-line so the header can forward-declare Bullet types.
            ~PhysicsWorld();

            // Synchronizes engine transforms into Bullet, runs collision detection,
            // and caches the overlap results for later queries this frame.
            void update();

            // Returns the owning node for a collider id, or nullptr if the id is unknown.
            Node* get_node_for_collider(ColliderId collider_id) const;

            // Returns the collider component for a collider id, or nullptr if the id is unknown.
            ColliderComponent* get_component_for_collider(ColliderId collider_id) const;

            // Creates and registers a Bullet sphere shape for a collider component.
            ColliderId register_sphere_collider(SphereColliderComponent* component, Node& node);

            // Creates and registers a Bullet box shape for a collider component.
            ColliderId register_box_collider(BoxColliderComponent* component, Node& node);

            // Removes a collider from the Bullet world and the engine registry.
            void unregister_collider(ColliderId collider_id);

        private:

            // Shared helper used by the public collider-registration methods.
            ColliderId register_collider(
                ColliderComponent* component,
                Node& node,
                std::unique_ptr<btCollisionShape> shape);

            // Stores the Bullet objects and engine back-pointers associated with one engine collider.
            struct ColliderEntry
            {
                // Owns the Bullet collision shape used by this collider.
                std::unique_ptr<btCollisionShape> shape_;

                // Owns the Bullet collision object inserted into the world.
                std::unique_ptr<btCollisionObject> object_;

                // Points at the engine node whose world matrix drives this collider.
                Node* owner_node_{nullptr};

                // Points back at the engine component so collision flags can be updated.
                ColliderComponent* component_{nullptr};
            };

            // Bullet object that stores low-level collision settings and factories.
            std::unique_ptr<btCollisionConfiguration> collision_configuration_;

            // Bullet object that creates and owns the narrow-phase contact manifolds.
            std::unique_ptr<btCollisionDispatcher> dispatcher_;

            // Bullet broadphase acceleration structure used to prune likely pairs.
            std::unique_ptr<btBroadphaseInterface> broadphase_;

            // The collision-only Bullet world used in this assignment.
            std::unique_ptr<btCollisionWorld> collision_world_;

            // Maps engine collider ids to the Bullet objects and engine pointers they own.
            std::unordered_map<ColliderId, ColliderEntry> colliders_;

            // Cached overlap results from the most recent update.
            std::vector<CollisionPair> collision_pairs_;

            // Monotonic id generator for newly registered colliders.
            ColliderId next_collider_id_{1};
    };
}