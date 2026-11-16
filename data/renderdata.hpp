#pragma once
#include <GL/gl.h>
#include <engine.h>
#include <GLFW/glfw3.h>
#include <resources.hpp>

namespace Engine {

    // a simple struct containing some object render data.
    class RenderData : public IResource {
    public:
        // which direction defines "front" faces.
        // GL_CW for clockwise face winding.
        // GL_CCW for ant-clockwise face winding.
        PUBLIC unsigned int m_frontFace = GL_CCW;

        // which face direction to cull.
        PUBLIC unsigned int m_cullFace = GL_BACK;

        PUBLIC unsigned int m_setPoly = GL_FRONT_AND_BACK;

        // what polygon mode opengl is drawing in.
        // GL_FILL for regular full tri drawing,
        // GL_LINE for wireframe rendering.
        PUBLIC unsigned int m_polyMode = GL_FILL;
        
        PUBLIC unsigned int m_blendFuncOpt = GL_SRC_ALPHA;
        PUBLIC unsigned int m_blendFuncVal = GL_ONE_MINUS_SRC_ALPHA;
        
        PUBLIC float m_lineWidth = 2.0f;

        // apply the render data
        void useRenderData();
    };
}