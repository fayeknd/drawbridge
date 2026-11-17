#pragma once 
#include <texture.hpp>
#include <shader.hpp>
#include <resources.hpp>

namespace Engine {
    
    constexpr const char* materialUniform_albedoFac     = ".albedo_fac";
    constexpr const char* materialUniform_emissionFac   = ".emission_fac";
    constexpr const char* materialUniform_roughnessFac  = ".roughness_fac";
    constexpr const char* materialUniform_metallicFac   = ".metallic_fac";
    constexpr const char* materialUniform_ambientFac    = ".metallic_fac";

    constexpr const char* materialUniform_hasAlbedo     = ".has_albedo_tx";
    constexpr const char* materialUniform_hasNormal     = ".has_normal_tx";
    constexpr const char* materialUniform_hasMetallic   = ".has_metallic_tx";
    constexpr const char* materialUniform_hasEmission   = ".has_emission_tx";
    constexpr const char* materialUniform_hasTransp     = ".has_transparency";

    constexpr const char* materialUniform_albedoTx      = "u_albedo_tx";
    constexpr const char* materialUniform_normalTx      = "u_normal_tx";
    constexpr const char* materialUniform_metallicTx    = "u_metallic_tx";
    constexpr const char* materialUniform_emissionTx    = "u_emission_tx";

    // the physically-based rendering data.
    class PBRData : public IEmbeddedResource {
    public:
        EXPORT fm::vec4 m_albedoFac                            = {1.0f, 1.0f, 1.0f, 1.0f};
        EXPORT fm::vec4 m_emissionFac                          = {0.0f, 0.0f, 0.0f, 0.0f};
        EXPORT float m_roughnessFac                            = 0.5f;
        EXPORT float m_metallicFac                             = 0.0f;
        EXPORT float m_ambientFac                              = 1.0f;
        
        EXPORT bool m_hasAlbedoTx                              = false;
        EXPORT bool m_hasNormalTx                              = false;
        EXPORT bool m_hasMetallicTx                            = false;
        EXPORT bool m_hasEmissionTx                            = false;

        EXPORT bool m_hasTransparency                          = false;
    };

    class Material : public ICachable<Material>, public IResource {
    protected:

        // texture pointers
        EXPORT std::shared_ptr<Texture2D> m_albedoTx = nullptr;
        EXPORT std::shared_ptr<Texture2D> m_normalTx = nullptr;
        EXPORT std::shared_ptr<Texture2D> m_metallicTx = nullptr;
        EXPORT std::shared_ptr<Texture2D> m_emissionTx = nullptr;


    public:

        // this will run the _genMaterial() function.
        // also return a pointer to this instance.
        static std::shared_ptr<Material> create();

        // texture setters/getters
        const std::shared_ptr<Texture2D> getAlbedoTx() const { return m_albedoTx; };
        const std::shared_ptr<Texture2D> getNormalTx() const { return m_normalTx; };
        const std::shared_ptr<Texture2D> getMetallicTx() const { return m_metallicTx; };
        const std::shared_ptr<Texture2D> getEmissionTx() const { return m_emissionTx; };
  
        void setAlbedoTx(std::shared_ptr<Texture2D> tx);
        void setNormalTx(std::shared_ptr<Texture2D> tx);
        void setMetallicTx(std::shared_ptr<Texture2D> tx);
        void setEmissionTx(std::shared_ptr<Texture2D> tx);

        // PBR data that stores PBR values.
        EXPORT PBRData m_data;
        
        // the shader program
        EXPORT std::shared_ptr<ShaderProgram> m_program;

        // use this material for drawing.
        // (bind the textures.)
        void useMaterial();

    };
}