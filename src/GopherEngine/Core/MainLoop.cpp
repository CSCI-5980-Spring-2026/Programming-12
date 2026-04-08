#include <GopherEngine/Core/MainLoop.hpp>
#include <GopherEngine/Renderer/Renderer.hpp>
#include <GopherEngine/Core/Scene.hpp>
#include <GopherEngine/Core/CameraComponent.hpp>

#include <iostream>
using namespace std;

namespace GopherEngine {
    MainLoop::MainLoop()
    {
        scene_ = make_shared<Scene>();
        Service<EventSystem>::provide(&event_system_);
        Service<ResourceManager>::provide(&resource_manager_);
    }

    MainLoop::~MainLoop()
    {
    }

    int MainLoop::run()
    {
        // Let the application configure window and renderer settings before creating the window.
        configure();

        // Create the OS window and the OpenGL context associated with it.
        window_.create_window();

        // Make the window's OpenGL context current on this thread before any GL calls.
        if(!window_.set_active(true)) {
            cerr << "Failed to set window to active" << endl;
            return EXIT_FAILURE;
        }

        // Forward window input events into the engine's event system.
        register_input_callbacks();

        // Initialize renderer-side OpenGL state and engine render services.
        renderer_.initialize();

        // Let the application build its scene content now that the window and renderer exist.
        initialize();

        // Compute the initial viewport state before entering the frame loop.
        handle_resize();
        
        // Reset the clock to start measuring time from the beginning of the main loop
        clock_.reset();

        while(window_.is_open())
        {
            // Measure the elapsed time since the previous frame.
            float delta_time = clock_.delta_time();

            // Advance any pending asynchronous resource loads on the main thread.
            resource_manager_.poll();

            // Process OS window messages and dispatch input callbacks.
            window_.handle_events();

            // Let the application author motion that should affect this frame's physics step.
            physics_update(delta_time);

            // Run the scene graph's pre-physics component phase.
            scene_->physics_update(delta_time);

            // Recompute scene transforms so physics sees current world matrices.
            scene_->sync_transforms();
            
            // Let the application react to the post-physics state of the frame.
            update(delta_time);

            // Run the scene graph's general update phase.
            scene_->update(delta_time);

            // Run the scene graph's late update phase after the main updates.
            scene_->late_update(delta_time);

            // Recompute transforms again so rendering sees the final frame state.
            scene_->sync_transforms();

            // Update the viewport if the window or active camera changed.
            handle_resize();

            // Prepare the renderer for a new frame by processing uploads and clearing buffers.
            renderer_.begin_frame();

            // Traverse the scene and submit draw calls using the final render state.
            scene_->draw();

            // Present the completed frame to the window.
            window_.display();
        }
        
        return EXIT_SUCCESS;
    }

    void MainLoop::handle_resize() {
        
        auto camera = scene_->get_main_camera();

        if(camera) 
        {
            // If either the window or the camera's projection matrix is dirty,
            // we need to resize the viewport and reset the dirty flags.
            if(window_.get_dirty() || camera->get_projection_matrix_dirty())
            {
                // This will be extended to support camera aspect ratio
                renderer_.resize_viewport(
                    window_.get_width(), 
                    window_.get_height(), 
                    camera->get_aspect_ratio(),
                    window_.get_viewport_mode()
                );

                // Reset the dirty flags after resizing the viewport
                window_.set_dirty(false);
                camera->set_projection_matrix_dirty(false);
            }

        }  
        else 
        {
            // If there is no camera in the scene, then we only need to check
            // if the window is dirty, and if so, resize the viewport.
            if(window_.get_dirty() ) 
            {
                renderer_.resize_viewport(
                    window_.get_width(), 
                    window_.get_height(), 
                    static_cast<float>(window_.get_width()) / static_cast<float>(window_.get_height()),
                    window_.get_viewport_mode()
                );

                window_.set_dirty(false);
            }
        }
    }

    void MainLoop::register_input_callbacks()
    {
        window_.set_on_mouse_move([this](const MouseMoveEvent& e) {
            event_system_.on_mouse_move(e);   
        });

        window_.set_on_mouse_down([this](const MouseButtonEvent& e) {
            event_system_.on_mouse_down(e);   
        });

        window_.set_on_mouse_up([this](const MouseButtonEvent& e) {
            event_system_.on_mouse_up(e);   
        });

        window_.set_on_mouse_scroll([this](const MouseScrollEvent& e) {
            event_system_.on_mouse_scroll(e);   
        });

        window_.set_on_key_down([this](const KeyEvent& e) {
            event_system_.on_key_down(e);   
        });

        window_.set_on_key_up([this](const KeyEvent& e) {
           event_system_.on_key_up(e);   
        });

        window_.set_on_key_press([this](const KeyEvent& e) {
            event_system_.on_key_press(e);
        });
    }
}

