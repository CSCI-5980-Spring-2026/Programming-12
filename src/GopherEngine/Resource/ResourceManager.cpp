#include <GopherEngine/Resource/ResourceManager.hpp>
#include <GopherEngine/Resource/MeshFactory.hpp>
#include <GopherEngine/Renderer/BlinnPhongMaterial.hpp>

#include <algorithm>
#include <iostream>
#include <sstream>
using namespace std;

namespace GopherEngine
{
    void ResourceManager::on_load_complete(std::function<void()> callback)
    {
        if (!callback)
            return;

        on_load_complete_callbacks_.push_back(std::move(callback));
    }

    void ResourceManager::resolve_material_texture_references()
    {
        for (const auto& [material_guid, texture_guid] : material_texture_references_)
        {
            auto material = get_material(material_guid);
            if (!material)
                continue;

            auto blinn_phong_material = std::dynamic_pointer_cast<BlinnPhongMaterial>(material);
            if (!blinn_phong_material || blinn_phong_material->get_texture())
                continue;

            auto texture = get_texture(texture_guid);
            if (!texture)
                continue;

            blinn_phong_material->set_texture(texture);
        }
    }

    bool ResourceManager::poll()
    {
        if(!load_handles_.empty())
        {
            // Partition load handles into "not ready" and "ready to fire"
            auto it = std::partition(
                load_handles_.begin(), load_handles_.end(),
                [](const LoadHandle& h) { return !h.is_ready(); }
            );

            // Record the ready range as indices before firing any callbacks.
            // A callback may push new handles onto load_handles_ (e.g. load_mesh
            // triggering load_material), and those new handles must survive into
            // the next poll() cycle — they must not be included in the erase below.
            size_t ready_start = std::distance(load_handles_.begin(), it);
            size_t ready_count = load_handles_.size() - ready_start;

            for (size_t i = 0; i < ready_count; ++i)
                load_handles_[ready_start + i].fire_callback();

            // Erase only the handles that were ready at the start of this frame
            load_handles_.erase(
                load_handles_.begin() + ready_start,
                load_handles_.begin() + ready_start + ready_count
            );
        }

        // If no loads are pending, run any queued post-load callbacks on the main thread.
        // Callbacks registered by these callbacks belong to the next idle poll.
        if (load_handles_.empty() && !on_load_complete_callbacks_.empty())
        {
            resolve_material_texture_references();

            auto callbacks = std::move(on_load_complete_callbacks_);
            on_load_complete_callbacks_.clear();

            for (auto& callback : callbacks)
            {
                if (callback)
                    callback();
            }
        }

        // Returns true if the pending load queue is now empty
        return load_handles_.empty();
    }

    
    std::shared_ptr<Mesh> ResourceManager::get_mesh(const Guid& guid) const
    {
        auto it = mesh_registry_.find(guid);
        if (it != mesh_registry_.end())
            return it->second;
        return nullptr;
    }

    std::shared_ptr<Texture> ResourceManager::get_texture(const Guid& guid) const
    {
        auto it = texture_registry_.find(guid);
        if (it != texture_registry_.end())
            return it->second;
        return nullptr;
    }

    bool ResourceManager::has_mesh(const Guid& guid) const
    {
        return mesh_registry_.find(guid) != mesh_registry_.end();
    }

    bool ResourceManager::has_texture(const Guid& guid) const
    {
        return texture_registry_.find(guid) != texture_registry_.end();
    }

    std::shared_ptr<Material> ResourceManager::get_material(const Guid& guid) const
    {
        auto it = material_registry_.find(guid);
        if (it != material_registry_.end())
            return it->second;
        return nullptr;
    }

    std::optional<Guid> ResourceManager::get_referenced_material_guid(const Guid& mesh_guid) const
    {
        auto it = mesh_material_references_.find(mesh_guid);
        if (it != mesh_material_references_.end())
            return it->second;
        return std::nullopt;
    }

    std::optional<Guid> ResourceManager::get_referenced_texture_guid(const Guid& material_guid) const
    {
        auto it = material_texture_references_.find(material_guid);
        if (it != material_texture_references_.end())
            return it->second;
        return std::nullopt;
    }

    bool ResourceManager::has_material(const Guid& guid) const
    {
        return material_registry_.find(guid) != material_registry_.end();
    }

    std::shared_ptr<Mesh> ResourceManager::register_mesh(const std::shared_ptr<Mesh>& mesh)
    {
        if (!mesh)
            return nullptr;

        auto it = mesh_registry_.find(mesh->guid_);
        if (it != mesh_registry_.end())
            return it->second;

        mesh_registry_[mesh->guid_] = mesh;
        return mesh;
    }

    std::shared_ptr<Texture> ResourceManager::register_texture(const std::shared_ptr<Texture>& texture)
    {
        if (!texture)
            return nullptr;

        auto it = texture_registry_.find(texture->guid_);
        if (it != texture_registry_.end())
            return it->second;

        texture_registry_[texture->guid_] = texture;
        return texture;
    }

    std::shared_ptr<Material> ResourceManager::register_material(const std::shared_ptr<Material>& material)
    {
        if (!material)
            return nullptr;

        if(!material->guid_)
        {
            if(material->name_)
            {
                // If the material has a name but no GUID, we can still register it by deriving a GUID from the name. 
                material->guid_ = Guid::from_name(material->name_.value());
            }
            else
            {
                cerr << "Warning: Attempted to register a material without a name or GUID." << endl; 
                return nullptr;
            } 
        }

        auto it = material_registry_.find(material->guid_.value());
        if (it != material_registry_.end())
            return it->second;

        material_registry_[material->guid_.value()] = material;
        return material;
    }

    bool ResourceManager::remove_mesh(const Guid& guid)
    {
        mesh_material_references_.erase(guid);
        return mesh_registry_.erase(guid) > 0;
    }

    bool ResourceManager::remove_texture(const Guid& guid)
    {
        return texture_registry_.erase(guid) > 0;
    }

    bool ResourceManager::remove_material(const Guid& guid)
    {
        material_texture_references_.erase(guid);
        return material_registry_.erase(guid) > 0;
    }

    void ResourceManager::remove_all_meshes()
    {
        mesh_registry_.clear();
        mesh_material_references_.clear();
    }

    void ResourceManager::remove_all_textures()
    {
        texture_registry_.clear();
    }

    void ResourceManager::remove_all_materials()
    {
        material_registry_.clear();
        material_texture_references_.clear();
    }

    void ResourceManager::remove_all()
    {
        mesh_registry_.clear();
        texture_registry_.clear();
        material_registry_.clear();
        mesh_material_references_.clear();
        material_texture_references_.clear();
    }

    void ResourceManager::set_verbose(bool v)
    {
        verbose = v;
    }

    bool ResourceManager::get_verbose() const
    {
        return verbose;
    }
}
