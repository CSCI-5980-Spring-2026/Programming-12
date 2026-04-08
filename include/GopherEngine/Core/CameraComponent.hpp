#pragma once

#include <GopherEngine/Core/Component.hpp>
#include <GopherEngine/Core/Camera.hpp>

#include <memory>

namespace GopherEngine {

    class CameraComponent : public Component
    {
        public:
            void update_render_context(const glm::mat4 &local_matrix, const glm::mat4 &world_matrix, RenderContext& context) override;

            std::shared_ptr<Camera> get_camera() const;
            void set_camera(std::shared_ptr<Camera> camera);

        private:
            std::shared_ptr<Camera> camera_{std::make_shared<Camera>()};
    };
} 
