#include <utils/logger.hpp>
#include <shader.hpp>
#include <component/modeldraw.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <renderer.hpp>
#include <layer.hpp>

using namespace Engine;

void ModelDraw::init() {

    // initialise the material so we don't have to do this manually.
    // if we replace this, then the old reference will automatically
    // get discarded.
    m_materials.push_back(Material::create());
    entity.m_isRenderable = true;
}


// draw
void ModelDraw::draw() {
    // if the model or material or shader is null, or if the renderer is disabled, return early
    if (!m_model) return;
    m_renderData.useRenderData(); // use our render data
    
    // draw the model
    m_model->draw(m_materials, 
        entity.getRealMatrix(),
        (Camera::activeCamera()) ? Camera::activeCamera()->getVPMatrix() : fm::mat4::identity()
    );

}

void ModelDraw::drawImpl(const Layer* layer) {
    if (!m_render || !Camera::activeCamera() || !entity.isRenderable()) return;

    bool hasTransparency = false; // true if any material has transparency
    DepthType dt;
    if (m_materials.size() <= 0) dt = DepthType::Planar;
    else {
        
        // check if any of the materials have transparency.
        // this will not be any more expensive with one material.
    

        // loop through the materials
        for (const auto& mat : m_materials) {
        
            // does this material have transparency?
            if (mat->m_data.m_hasTransparency) {
                hasTransparency = true;
                break;
            }
        }
        dt = (hasTransparency) ? DepthType::SquaredRadial : DepthType::Planar;
    }
    // get the depth from the camera
    float depth = entity.transform.getDepth(
        Camera::activeCamera()->getEntity().transform.getGlobalPosition(),
        Camera::activeCamera()->getEntity().transform.getGlobalFront(),
        dt
    );

    Renderer& renderer = Renderer::get();

    // create the sort key
    uint64_t sortKey;
    if (m_materials.size() <= 0) sortKey = 1000; // render it above everything else
    else                         sortKey = renderer.genSortKey(hasTransparency, m_materials[0].get(), depth); // pass the first material, objects are most likely to mainly use the first material
    renderer.queueRenderCommand(layer->m_layerID, sortKey, this, _distributeDraw);
}