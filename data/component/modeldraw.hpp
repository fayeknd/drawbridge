#pragma once
#include <component/component.hpp>
#include <model.hpp>
#include <shader.hpp>
#include <gl_context.hpp>
#include <material.hpp>

namespace Engine {
    
    // a basic renderable interface.
    // will take a mesh and shader (to be material w/ shader)
    // and draws them.
    class ModelDraw : public IComponent {
    private:
    
        const bool canDrawImpl() const override { return true; }

        // tell the instance to run draw(). 
        static inline void _distributeDraw(ModelDraw* instance) { instance->draw(); }
        
    protected:

        // final stage in the game loop. used for anything to do with drawing.
        virtual void draw();

    public:


        // material(s) to use
        // this was moved here so we don't have to dyanmic
        // cast to an ModelDraw when sending render data.
        // if the material is null, no render data.
        // else, we can determine if the material is transparent,
        // and use the appropriate depth function.
        PUBLIC std::vector<std::shared_ptr<Material>> m_materials;
        #define m_material m_materials[0]


        // instance's render data
        PUBLIC RenderData m_renderData = RenderData();

        // model to draw
        PUBLIC std::shared_ptr<Model> m_model = nullptr;
        
        // material to use
        //std::shared_ptr<Material> m_material = nullptr; moved to component.hpp (explanation there)

        // general setup
        void init() override;

        // adds draw() to the render bucket callback in renderer.hpp
        void drawImpl(const Layer* layer);
        
    };
}