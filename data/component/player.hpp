#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>
#include <physics/rigidbody.hpp>
#include <component/collider.hpp>
#include <texture.hpp>
#include <utils/debug.hpp>

namespace Engine {
    
    // The main entrypoint component. The program
    // will assign this to the root component automatically.
    class Player : public IComponent {
        
    public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;
        void movementInput(double delta);
        void mouseInput(double delta);

        Rigidbody* rb;
        Collider* collider;
        std::shared_ptr<Entity> camObject;
        Camera* cam;
        EXPORT float sensitivity = 0.25;
        EXPORT float speed = 3.0f;
        EXPORT bool toggleMovement;
        EXPORT bool toggleMouseMovement;
    };
}