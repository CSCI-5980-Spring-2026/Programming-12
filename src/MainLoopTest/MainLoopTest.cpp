/* Programming 12: Collision System Implementation
 * CSCI 5980, Spring 2026, University of Minnesota
 * Instructor: Evan Suma Rosenberg <suma@umn.edu>
 */ 


#include <GopherEngine/Core/MainLoop.hpp>
#include <GopherEngine/Core/EventHandler.hpp>
#include <GopherEngine/Core/LightComponent.hpp>
#include <GopherEngine/Core/MeshComponent.hpp>
#include <GopherEngine/Core/OrbitControls.hpp>
#include <GopherEngine/Core/Utils.hpp>
#include <GopherEngine/Renderer/BlinnPhongMaterial.hpp>
#include <GopherEngine/Resource/MeshFactory.hpp>
#include <GopherEngine/Core/FileLoader.hpp>
#include <GopherEngine/Physics/BoxColliderComponent.hpp>
#include <GopherEngine/Physics/SphereColliderComponent.hpp>

#include <glm/gtc/quaternion.hpp>
using namespace GopherEngine;

#include <memory>
#include <iostream>
#include <cmath>
#include <numbers>
#include <vector>
using namespace std;

// A simple subclass of MainLoop to test that the main loop is working
// and the window, scene, and node classes are functioning correctly
class MainLoopTest: public MainLoop, public EventHandler
{
	public:
		// Constructor and destructor
		MainLoopTest();
		~MainLoopTest();

	private:

		// Override the pure virtual functions from MainLoop
		void configure() override;
		void initialize() override;
		void physics_update(float delta_time) override;
		void update(float delta_time) override;
		void on_key_down(const KeyEvent& event) override;
		void on_key_up(const KeyEvent& event) override;
		void on_key_press(const KeyEvent& event) override;

		std::shared_ptr<Node> create_sphere_node(float radius);
		std::shared_ptr<Node> create_cube_node(glm::vec3 dimensions);

		std::shared_ptr<Node> sun_node_;
		std::vector<std::shared_ptr<Node>> planet_nodes_;
		bool rotation_paused_{false};

		std::shared_ptr<Node> movable_node_;
		glm::vec3 move_direction_{0.f};
		
};

MainLoopTest::MainLoopTest() {

}

MainLoopTest::~MainLoopTest() {

}

// This function is called once at the beginning of the main loop, before the window is created.
// This means the OpenGL context is not yet available. It should be used for initial configuration.
void MainLoopTest::configure() {

	window_.set_title("CSCI 5980 Programming 12");
	window_.set_vertical_sync(true);
	window_.set_framerate_limit(60);
	renderer_.set_background_color(glm::vec4(0.5f, 0.75f, .9f, 1.f));
}

// Helper function to create a sphere node with a random color material
std::shared_ptr<Node> MainLoopTest::create_sphere_node(float radius) {

	auto node = make_shared<Node>();

	auto mesh_component = make_shared<MeshComponent>();
	mesh_component->set_mesh(MeshFactory::create_sphere(radius));
	node->add_component(mesh_component);

	auto collider = make_shared<SphereColliderComponent>(radius);
	node->add_component(collider);

	const glm::vec3 diffuse_color(Random::value(), Random::value(), Random::value());
	auto material = make_shared<BlinnPhongMaterial>();
	material->set_ambient_color(diffuse_color * 0.3f);
	material->set_diffuse_color(diffuse_color);
	material->set_specular_color(glm::vec3(1.5f));
	mesh_component->set_material(material);

	return node;
}


// Helper function to create a cube node with a random color material
std::shared_ptr<Node> MainLoopTest::create_cube_node(glm::vec3 dimensions) {

	auto node = make_shared<Node>();

	auto mesh_component = make_shared<MeshComponent>();
	mesh_component->set_mesh(MeshFactory::create_cube(dimensions.x, dimensions.y, dimensions.z));
	node->add_component(mesh_component);

	auto collider = make_shared<BoxColliderComponent>(dimensions);
	node->add_component(collider);
	
	const glm::vec3 diffuse_color(Random::value(), Random::value(), Random::value());
	auto material = make_shared<BlinnPhongMaterial>();
	material->set_ambient_color(diffuse_color * 0.3f);
	material->set_diffuse_color(diffuse_color);
	material->set_specular_color(glm::vec3(1.5f));
	mesh_component->set_material(material);

	return node;
}


// This function is called once at the beginning of the main loop, after the window is created
// and the OpenGL context is available. It should be used for initializing the scene.
void MainLoopTest::initialize() {

	// Enable verbose logging in the resource manager so we can see when resources are loaded
	ResourceManager::get().set_verbose(true);
	
	// Create default camera and set its initial position
	auto camera_node = scene_->create_default_camera();

	// Add orbit controls to the camera so we can move it around with the mouse
	auto camera_controls = make_shared<OrbitControls>();
	camera_controls->set_distance(50.0f);
	camera_controls->set_zoom_speed(50.f);
	camera_controls->set_orbit_x(glm::angleAxis(glm::radians(-30.f), glm::vec3(1.f, 0.f, 0.f)));
	camera_node->add_component(camera_controls);

	// Create a point light
	auto light_component = make_shared<LightComponent>(LightType::Point);
	light_component->get_light()->ambient_intensity_ = glm::vec3(0.2f, 0.2f, 0.2f);
	light_component->get_light()->diffuse_intensity_ = glm::vec3(1.f, 1.f, 1.f);
	light_component->get_light()->specular_intensity_ = glm::vec3(1.f, 1.f, 1.f);

	// Add the point light to the scene
	auto light_node = scene_->create_node();
	light_node->add_component(light_component);
	light_node->transform().position_ = glm::vec3(-10.f, 10.f, 10.f);

	sun_node_ = create_sphere_node(3);
	scene_->add_node(sun_node_);
	sun_node_->transform().position_ = glm::vec3(0.f, 0.f, 0.f);

	const float planet_radii[] = {1.2f, 1.f, 1.5f, 1.f, 1.25f, 0.75f, 1.5f, 2.f};
	const float planet_distances[] = {10.f, 14.f, 18.f, 22.f, 26.f, 30.f, 34.f, 38.f};

	for (size_t i = 0; i < 8; ++i) {

		auto planet_node = create_sphere_node(planet_radii[i]);
		sun_node_->add_child(planet_node);
		planet_nodes_.push_back(planet_node);

		const float angle = Random::value() * 2.f * std::numbers::pi_v<float>;
		const float distance = planet_distances[i];
		planet_node->transform().position_ = glm::vec3(
			cos(angle) * distance,
			0.f,
			sin(angle) * distance
		);

		auto moon_node = create_cube_node(glm::vec3(1.f));
		moon_node->transform().position_ = glm::vec3(planet_radii[i] + 2.f, 0.f, 0.f);
		planet_node->add_child(moon_node);
	}

	movable_node_ = create_cube_node(glm::vec3(2.f, 2.f, 4.f));
	scene_->add_node(movable_node_);
	movable_node_->transform().position_ = glm::vec3(0.f, 0.f, -45.f);

}

// This function is called once per frame, before the physics system is updated.
// It should be used for updating the movement of objects that need to be processed
// for collision detection in the physics update, such as moving platforms or player characters.
void MainLoopTest::physics_update(float delta_time) {

	// Move the movable node based on the current move direction and delta time
	if (glm::length(move_direction_) > 0.f) {
		const glm::vec3 movement = glm::normalize(move_direction_);
		movable_node_->transform().position_ += movement * 15.f * delta_time;
	}

	// If the rotation is paused, skip the rest of the update function to prevent the sun and planets from rotating
	if(rotation_paused_) return;

	// Rotate the sun around the Y axis.
	// Re-normalize after repeated quaternion multiplies to prevent floating-point drift over time.
	const glm::quat delta_rotation = glm::angleAxis(delta_time * glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));
	sun_node_->transform().rotation_ = glm::normalize(delta_rotation * sun_node_->transform().rotation_);

	// Rotate each planet in alternating directions.
	for (size_t i = 0; i < planet_nodes_.size(); ++i) {
		const float direction = (i % 2 == 0) ? 1.f : -1.f;
		const glm::quat delta_rotation = glm::angleAxis(delta_time * glm::radians(25.f) * direction, glm::vec3(0.f, 1.f, 0.f));
		planet_nodes_[i]->transform().rotation_ = glm::normalize(delta_rotation * planet_nodes_[i]->transform().rotation_);
	}
}

// This function is called once per frame, after the physics update and before rendering.
// Here, we can read the collision state that has been updated based on the movement in physics_update.
void MainLoopTest::update(float delta_time) {

	if (!movable_node_) return;

	// Check if the movable node is colliding with anything and print the IDs of any colliders it is overlapping with
	const auto colliders = movable_node_->get_components<ColliderComponent>();
	if (!colliders.empty() && colliders.front()->is_colliding()) {

		cout << clock_.get_elapsed_time() <<
			"sec: collider " <<
			colliders.front()->get_collider_id() << 
			" collision detected with ";

		for (const auto collider_id : colliders.front()->get_overlapping_colliders()) {
			cout << collider_id << " ";
		}

		cout << endl;
	}

}

void MainLoopTest::on_key_down(const KeyEvent& event) {
	if (event.key == Key::W) move_direction_.z = -1.f;
	if (event.key == Key::S) move_direction_.z = 1.f;
	if (event.key == Key::A) move_direction_.x = -1.f;
	if (event.key == Key::D) move_direction_.x = 1.f;
}

void MainLoopTest::on_key_up(const KeyEvent& event) {
	if (event.key == Key::W || event.key == Key::S) move_direction_.z = 0.f;
	if (event.key == Key::A || event.key == Key::D) move_direction_.x = 0.f;
}

void MainLoopTest::on_key_press(const KeyEvent& event) {
	if (event.key == Key::Space) rotation_paused_ = !rotation_paused_;
}

int main()
{
	// Create an instance of the MainLoop subclass and start the main game loop
	MainLoopTest app;
	return app.run();
}
