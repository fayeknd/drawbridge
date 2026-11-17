#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>

namespace Engine::Debug {
    
    class DebugCam : public IComponent {

    public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;
        void movementInput(double delta);
        void mouseInput(double delta);

        EXPORT float cameraSpeed = 3.f;
        EXPORT float sensitivity = 0.25;
        Camera* cam;
    };
}