#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>
#include <imgui/imgui.h>
#include <logger.hpp>

namespace Engine {

    class EditorUI : public IComponent {
    public:

        // camera used for the ui
        std::shared_ptr<Camera> m_uiCam = nullptr;
        // shortcut to the imgui IO
        ImGuiIO* m_io;

        void init() override;
        void lateUpdate(double delta) override;
        void drawViewport(ImGuiWindowFlags flags);
        void drawUI();

        const std::filesystem::path projectPath() const { return m_projectDir; }

    private:

        static Logging::Logger log() {
            static Logging::Logger instance("EDITOR");
            return instance;
        }

        std::filesystem::path m_projectDir;
    
    };
}