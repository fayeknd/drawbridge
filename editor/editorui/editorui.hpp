#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>
#include <imgui/imgui.h>
#include <utils/logger.hpp>

namespace Engine {

    class EditorUI : public IComponent {
    public:

        virtual ~EditorUI() = default;

        // camera used for the ui
        std::shared_ptr<Camera> m_uiCam = nullptr;
        // shortcut to the imgui IO
        ImGuiIO* m_io;

        static inline std::vector<std::string> logLines;

        void init() override;
        void lateUpdate(double delta) override;
        void drawViewport(ImGuiWindowFlags flags);
        void drawConsole(ImGuiWindowFlags flags);
        void drawUI();

        const std::filesystem::path projectPath() const { return m_projectDir; }

    private:

        static Logging::Logger log() {
            static Logging::Logger instance("EDITOR");
            return instance;
        }

        EXPORT std::filesystem::path m_projectDir;
    
    };
}