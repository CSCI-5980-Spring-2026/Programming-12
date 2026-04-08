#include <GopherEngine/Physics/PhysicsWorld.hpp>
#include <GopherEngine/Physics/ColliderComponent.hpp>
#include <GopherEngine/Physics/BoxColliderComponent.hpp>
#include <GopherEngine/Physics/SphereColliderComponent.hpp>

#include <GopherEngine/Core/Node.hpp>

#include <glm/gtc/quaternion.hpp>

#include <btBulletCollisionCommon.h>

#include <algorithm>
#include <memory>
#include <stdexcept>

namespace
{
    // Converts an engine world matrix into the Bullet transform format used by
    // btCollisionObject. This keeps the GLM-to-Bullet translation in one place.
    btTransform to_bullet_transform(const glm::mat4& world_matrix)
    {
        btTransform transform;
        transform.setIdentity();

        // The translation lives in the fourth column of the engine world matrix.
        const glm::vec3 translation(world_matrix[3]);

        // The first three columns contain the transformed basis vectors.
        glm::vec3 x_axis = glm::vec3(world_matrix[0]);
        glm::vec3 y_axis = glm::vec3(world_matrix[1]);
        glm::vec3 z_axis = glm::vec3(world_matrix[2]);

        // If any basis vector is degenerate, fall back to the corresponding
        // world axis so quaternion conversion remains well-defined.
        if (glm::length(x_axis) == 0.f)
            x_axis = glm::vec3(1.f, 0.f, 0.f);
        if (glm::length(y_axis) == 0.f)
            y_axis = glm::vec3(0.f, 1.f, 0.f);
        if (glm::length(z_axis) == 0.f)
            z_axis = glm::vec3(0.f, 0.f, 1.f);

        // Normalize the basis vectors before converting them into a quaternion.
        // This removes any scale encoded in the world matrix so Bullet receives
        // only translation and rotation in its btTransform.
        const glm::mat3 rotation_matrix(
            glm::normalize(x_axis),
            glm::normalize(y_axis),
            glm::normalize(z_axis));

        const glm::quat rotation = glm::normalize(glm::quat_cast(rotation_matrix));

        // Bullet stores translation and rotation separately inside btTransform.
        transform.setOrigin(btVector3(translation.x, translation.y, translation.z));
        transform.setRotation(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w));
        return transform;
    }

}

namespace GopherEngine
{
    PhysicsWorld::PhysicsWorld()
    {
        // Bullet's collision world is assembled from several collaborating pieces:
        // configuration, dispatcher, broadphase, and finally the world itself.
        collision_configuration_ = std::make_unique<btDefaultCollisionConfiguration>();
        dispatcher_ = std::make_unique<btCollisionDispatcher>(collision_configuration_.get());
        broadphase_ = std::make_unique<btDbvtBroadphase>();
        collision_world_ = std::make_unique<btCollisionWorld>(
            dispatcher_.get(),
            broadphase_.get(),
            collision_configuration_.get());
    }

    PhysicsWorld::~PhysicsWorld() = default;

    ColliderId PhysicsWorld::register_sphere_collider(SphereColliderComponent* component, Node& node)
    {
        const float clamped_radius = std::max(component->get_radius(), 0.001f);
        return register_collider(component, node, std::make_unique<btSphereShape>(clamped_radius));
    }

    ColliderId PhysicsWorld::register_box_collider(BoxColliderComponent* component, Node& node)
    {
        // BoxColliderComponent exposes full size, but Bullet boxes are defined
        // by half-extents, so convert here inside the physics wrapper during registration.
        const glm::vec3 clamped_size = glm::max(component->get_size(), glm::vec3(0.001f));
        return register_collider(
            component, 
            node, 
            std::make_unique<btBoxShape>(btVector3(clamped_size.x * .5f, clamped_size.y * .5f, clamped_size.z * .5f)));
    }

    ColliderId PhysicsWorld::register_collider(
        ColliderComponent* component,
        Node& node,
        std::unique_ptr<btCollisionShape> shape)
    {
        if (component == nullptr)
            throw std::invalid_argument("Collider registration requires a valid component");

        // Allocate a new engine-facing id for this collider.
        const ColliderId collider_id = next_collider_id_++;

        // Build the Bullet collision object and attach the shape created by the
        // public registration helper.
        auto object = std::make_unique<btCollisionObject>();
        object->setCollisionShape(shape.get());
        object->setWorldTransform(to_bullet_transform(node.world_matrix()));
        object->setUserIndex(static_cast<int>(collider_id));

        // Insert the object into Bullet so it participates in overlap testing.
        collision_world_->addCollisionObject(object.get());

        // Store the shape, object, and engine back-pointers together in one registry entry.
        colliders_[collider_id] = ColliderEntry{
            std::move(shape),
            std::move(object),
            &node,
            component
        };

        return collider_id;
    }

    void PhysicsWorld::unregister_collider(ColliderId collider_id)
    {
        // Ignore unknown ids so repeated teardown stays harmless.
        const auto collider_it = colliders_.find(collider_id);
        if (collider_it == colliders_.end())
            return;

        // Clear the overlap list so stale state is not left behind on the component.
        if (collider_it->second.component_ != nullptr)
            collider_it->second.component_->overlapping_colliders_.clear();

        if (collider_it->second.object_ != nullptr)
        {
            // Remove the object from Bullet before destroying our local ownership.
            collision_world_->removeCollisionObject(collider_it->second.object_.get());
        }

        // Erasing the entry releases both the shape and the collision object.
        colliders_.erase(collider_it);
    }

    void PhysicsWorld::update()
    {
        // Start each frame with a fresh list of overlap pairs.
        collision_pairs_.clear();

        for (auto& [collider_id, entry] : colliders_)
        {
            // Clear each collider's cached overlap list before detecting the
            // current frame's overlaps.
            if (entry.component_ != nullptr)
                entry.component_->overlapping_colliders_.clear();

            // Skip incomplete entries rather than crashing; this keeps the
            // wrapper tolerant of registration teardown edge cases.
            if (entry.owner_node_ == nullptr || entry.object_ == nullptr)
                continue;

            // Copy the latest engine world transform into Bullet before running
            // collision detection.
            entry.object_->setWorldTransform(to_bullet_transform(entry.owner_node_->world_matrix()));
        }

        // Ask Bullet to detect all overlaps for the current object transforms.
        collision_world_->performDiscreteCollisionDetection();

        const int manifold_count = dispatcher_->getNumManifolds();

        for (int manifold_index = 0; manifold_index < manifold_count; ++manifold_index)
        {
            // Each manifold represents the contact data Bullet found for one pair.
            auto* manifold = dispatcher_->getManifoldByIndexInternal(manifold_index);
            bool overlapping = false;

            for (int contact_index = 0; contact_index < manifold->getNumContacts(); ++contact_index)
            {
                // For this teaching pass, a contact with non-positive distance
                // counts as an overlap.
                if (manifold->getContactPoint(contact_index).getDistance() <= 0.f)
                {
                    overlapping = true;
                    break;
                }
            }

            // Ignore manifolds that do not currently represent an overlap.
            if (!overlapping)
                continue;

            // Recover the Bullet objects that participated in this overlap.
            const auto* object_a = static_cast<const btCollisionObject*>(manifold->getBody0());
            const auto* object_b = static_cast<const btCollisionObject*>(manifold->getBody1());

            // Each Bullet collision object stores the engine collider id in its
            // user index, so no separate reverse-lookup table is needed.
            const int object_a_id = object_a->getUserIndex();
            const int object_b_id = object_b->getUserIndex();

            if (object_a_id <= 0 || object_b_id <= 0)
                continue;

            const ColliderId collider_a = static_cast<ColliderId>(object_a_id);
            const ColliderId collider_b = static_cast<ColliderId>(object_b_id);

            // Store the pair in a consistent sorted order to simplify later queries.
            collision_pairs_.push_back({
                std::min(collider_a, collider_b),
                std::max(collider_a, collider_b)
            });

            // Mark both colliders as colliding so component-side queries stay simple.
            if (auto collider_it = colliders_.find(collider_a); collider_it != colliders_.end() &&
                collider_it->second.component_ != nullptr)
                collider_it->second.component_->overlapping_colliders_.push_back(collider_b);

            if (auto collider_it = colliders_.find(collider_b); collider_it != colliders_.end() &&
                collider_it->second.component_ != nullptr)
                collider_it->second.component_->overlapping_colliders_.push_back(collider_a);
        }
    }
}