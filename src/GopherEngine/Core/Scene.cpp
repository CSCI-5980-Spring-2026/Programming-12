#include <GopherEngine/Core/Scene.hpp>
#include <GopherEngine/Core/RenderContext.hpp>

using namespace std;

namespace GopherEngine {

    Scene::Scene() {

        // Create the root node of the scene graph. This node will 
        // serve as the parent for all other nodes in the scene.
        root_ = make_shared<Node>();

    }

    Scene::~Scene() {

    }

    shared_ptr<Node> Scene::create_default_camera() {

        auto camera_component = make_shared<CameraComponent>();
        auto camera_node = create_node();
        camera_node->add_component(camera_component);

        main_camera_ = camera_component->get_camera();
        main_camera_->set_perspective(60.f, 4.f/3.f, 0.1f, 1000.f);    

        return camera_node;
    }

    shared_ptr<Camera> Scene::get_main_camera() const {
        return main_camera_;
    }

    void Scene::set_main_camera(shared_ptr<Camera> camera) {
        main_camera_ = camera;
        main_camera_->set_projection_matrix_dirty(true);
    }

    shared_ptr<Node> Scene::get_root() {
        return root_;
    }

    shared_ptr<Node> Scene::create_node() {

        shared_ptr<Node> node = make_shared<Node>();
        root_->add_child(node);
        return node;

    }

    shared_ptr<Node> Scene::add_node(shared_ptr<Node> node) {

        root_->add_child(node);
        return node;

    }

    void Scene::physics_update(float delta_time) {
        root_->physics_update(delta_time);
    }

    void Scene::update(float delta_time) {

        root_->update(delta_time);
    }

    void Scene::late_update(float delta_time) {
        root_->late_update(delta_time);
    }

    void Scene::sync_transforms() {
        root_->sync_transforms(glm::mat4(1.f), false);
    }

    void Scene::draw() {

        RenderContext context;
        root_->update_render_context(context);

        if(main_camera_) {
            context.view_matrix_ = main_camera_->get_view_matrix();
            context.projection_matrix_ = main_camera_->get_projection_matrix();
            context.eye_position_ = main_camera_->get_world_position();
            
            root_->draw(context);
        }

    }
}
