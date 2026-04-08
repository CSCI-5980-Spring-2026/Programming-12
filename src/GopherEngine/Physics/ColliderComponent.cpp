#include <GopherEngine/Physics/ColliderComponent.hpp>

#include <GopherEngine/Core/Node.hpp>

namespace GopherEngine
{
    ColliderComponent::~ColliderComponent()
    {
        // The component owns its registration lifetime, so destroying the
        // component must also remove its Bullet representation.
        if (collider_id_ != InvalidColliderId)
            PhysicsWorld::get().unregister_collider(collider_id_);
    }

    void ColliderComponent::initialize(Node& node)
    {
        // Ask the derived class to register its concrete shape with PhysicsWorld.
        collider_id_ = register_with_physics(PhysicsWorld::get(), node);
    }

    bool ColliderComponent::is_colliding() const
    {
        // Return whether this collider overlapped anything during the last physics update.
        return !overlapping_colliders_.empty();
    }

    const std::vector<ColliderId>& ColliderComponent::get_overlapping_colliders() const
    {
        // Return the cached list of overlapping collider ids from the last physics update.
        return overlapping_colliders_;
    }
}