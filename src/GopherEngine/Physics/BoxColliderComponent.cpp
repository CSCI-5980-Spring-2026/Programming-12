#include <GopherEngine/Physics/BoxColliderComponent.hpp>

namespace GopherEngine
{
    BoxColliderComponent::BoxColliderComponent(const glm::vec3& size)
        : size_(size)
    {
        // The authored full box size is stored locally until initialization registers
        // the matching Bullet shape.
    }

    glm::vec3 BoxColliderComponent::get_size() const
    {
        return size_;
    }

    ColliderId BoxColliderComponent::register_with_physics(PhysicsWorld& physics_world, Node& node)
    {
        // Delegate Bullet-specific shape creation to PhysicsWorld.
        return physics_world.register_box_collider(this, node);
    }
}
