#include <iostream>
#include <renderer.hpp>
#include <entity.hpp>
#include <editorui/editorui.hpp>

using namespace Engine;

// don't add stuff here, add stuff in component/main_comp.cpp in the init() function.
int main(int, char** args){
    Renderer::get().create({300,300}, args[1] ? args[1] : "-1");

    // add the main entrypoint component to the root entity.
    //Entity::s_root->addComponent<MainComponent>();
    Entity::s_root->addComponent<EditorUI>();

    Renderer::get().beginGameLoop();
}
