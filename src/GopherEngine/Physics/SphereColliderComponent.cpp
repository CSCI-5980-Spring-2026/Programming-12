#include <GopherEngine/Physics/SphereColliderComponent.hpp>

namespace GopherEngine
{
    SphereColliderComponent::SphereColliderComponent(float radius)
        : radius_(radius)
    {
        // The authored radius is stored locally until initialization registers
        // the matching Bullet shape.
    }

    float SphereColliderComponent::get_radius() const
    {
        return radius_;
    }

    ColliderId SphereColliderComponent::register_with_physics(PhysicsWorld& physics_world, Node& node)
    {
        // Delegate Bullet-specific shape creation to PhysicsWorld.
        return physics_world.register_sphere_collider(this, node);
    }
}
