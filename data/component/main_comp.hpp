#pragma once
#include <component/component.hpp>
#include <texture.hpp>
#include <utils/debug.hpp>

namespace Engine {
    
    // The main entrypoint component. The program
    // will assign this to the root component automatically.
    class MainComponent : public IComponent {
        
    public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;

        Entity* square;
        Entity* cam;
        EXPORT float sensitivity = 0.25;
        EXPORT float speed = 3.0f;

    };
}