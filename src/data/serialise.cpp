#include <serialise.hpp>
#include <fm/fmath.hpp>
#include <resources.hpp>
#include <gl_context.hpp>
#include <entity.hpp>

std::unordered_map<std::string, Engine::Serialise::SharedPtrExtractor>& Engine::Serialise::sharedPtrPathExtMap() {
    static std::unordered_map<std::string, SharedPtrExtractor> map;
    return map;
}

std::unordered_map<std::string, Engine::Serialise::ContainerSerialiser>& Engine::Serialise::containerSerialiserMap() {
    static std::unordered_map<std::string, ContainerSerialiser> map;
    return map;
}

// serialise reflected instance fields into an out stream
void Engine::Serialise::serialiseFieldsToStream(const std::vector<std::pair<ReflectableField, void*>>& fields, std::ostream& out, int indentLevel) {
    // loop through all the fields
    std::string indent(indentLevel * 4, ' ');

    for (const auto& [field, fieldPtr] : fields) {
        if (!fieldPtr) continue;

        out << indent << field.m_fieldName << " = ";
        serialiseField(field.m_fieldType, fieldPtr, out, indentLevel);
        out << "\n";
    }
}

void Engine::Serialise::serialiseField(std::string_view fieldType, const void* fieldPtr, std::ostream& out, int indentLevel) {

    #define SER(T, EXTRA) out << EXTRA << *static_cast<const T*>(fieldPtr) << EXTRA
    #define TRY_SERIALISE_STR(T) if (fieldType == #T) { SER(T, "\""); return; } 
    #define TRY_SERIALISE(T) if (fieldType == #T) { SER(T, ""); return; } 
    // primitive fields
    TRY_SERIALISE(float)
    TRY_SERIALISE(int)
    TRY_SERIALISE(unsigned int)
    TRY_SERIALISE(bool)
    TRY_SERIALISE(uint8_t)
    TRY_SERIALISE(uint16_t)
    TRY_SERIALISE(uint32_t)
    TRY_SERIALISE(uint64_t)
    TRY_SERIALISE(short)
    TRY_SERIALISE(double)
    TRY_SERIALISE(unsigned int)
    TRY_SERIALISE(unsigned short)
    TRY_SERIALISE(unsigned long)
    TRY_SERIALISE(unsigned long long)
    TRY_SERIALISE(long long)
    TRY_SERIALISE(long)
    TRY_SERIALISE(GLuint)
    TRY_SERIALISE(GLushort)
    TRY_SERIALISE(GLint)
    TRY_SERIALISE(GLshort)
    TRY_SERIALISE(GLbyte)
    TRY_SERIALISE(GLdouble)
    TRY_SERIALISE(GLbitfield)
    TRY_SERIALISE(bitfield)
    TRY_SERIALISE(EntityID)
    TRY_SERIALISE(LayerID)
    TRY_SERIALISE(fm::vec2)
    TRY_SERIALISE(fm::vec3)
    TRY_SERIALISE(fm::vec4)
    TRY_SERIALISE(fm::col)
    TRY_SERIALISE(fm::uvec2)
    TRY_SERIALISE(fm::uvec3)
    TRY_SERIALISE(fm::uvec4)
    TRY_SERIALISE(fm::ivec2)
    TRY_SERIALISE(fm::ivec3)
    TRY_SERIALISE(fm::ivec4)
    TRY_SERIALISE(fm::dvec2)
    TRY_SERIALISE(fm::dvec3)
    TRY_SERIALISE(fm::dvec4)
    TRY_SERIALISE_STR(std::string)
    TRY_SERIALISE_STR(std::string_view)
    TRY_SERIALISE_STR(char)
    TRY_SERIALISE_STR(unsigned char)
    TRY_SERIALISE_STR(std::filesystem::path);

    if (auto contIt = containerSerialiserMap().find(std::string(fieldType)); contIt != containerSerialiserMap().end()) {
        contIt->second(fieldPtr, out, indentLevel);
        return;
    }

    // resource pointers (std::shared_ptr<Texture2D>, for example)
    if (auto ptrIt = sharedPtrPathExtMap().find(std::string(fieldType)); ptrIt != sharedPtrPathExtMap().end() ) {
        IResource* res = ptrIt->second(fieldPtr);

        // is this an embedded resource?
        // (dynamic_cast should be fine here,
        // serialisation doesn't run very often)
        if (auto obj = dynamic_cast<IEmbeddedResource*>(res); !obj) {
            if (res) {
                std::string path = "";
                // does this path have a resource it can choose a name from?
                if (res->getResourcePath() == "") {
                    // nope, make up a path
                    path = proj_"autogen/_auto-";
                    path += std::string(res->getTypename()) + "-" + Time::timeStampForFile() + resource_defaultFileExt;
                }
                ResourceManager::saveResource(res, path);
                out << "\"" << res->getMetaResourcePath() << "\"";
            } else {
                out << "\"\"";
            }
        }
        else {
            // this IS an embedded resource, treat it as such
            out << "@type " << obj->getFullTypename() << " {\n";
            serialiseFieldsToStream(obj->getFields(), out, indentLevel);
            out << std::string(indentLevel * 4, ' ') << "}";
        }
        return;
    }
    // nested reflected structs/classes
    if (auto typeIt = Reflection::getTypeRegistry().find(fieldType); typeIt != Reflection::getTypeRegistry().end()) {
        out << "{\n";
        std::vector<std::pair<ReflectableField, void*>> nestedFields;
        for (const auto& sub : typeIt->second->getFields()) {
            nestedFields.emplace_back(
                sub,
                const_cast<char*>(static_cast<const char*>(fieldPtr)) + sub.m_fieldOffset
            );
        }
        serialiseFieldsToStream(nestedFields, out, indentLevel + 1);
        out << std::string(indentLevel * 4, ' ') << "}";
        return;
    }
}