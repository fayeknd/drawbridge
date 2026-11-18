
#pragma once
#include <string_view>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <utils/logger.hpp>

namespace Engine {
    // no runtime overhead with marcos!
    #define EXPORT \
        friend void Engine::Reflection::initialiseReflectionData(); \
        friend class ResourceManager; \
        template <typename T> friend struct Engine::Reflection::TypeInfo; \
        [[maybe_unused]]

    // the reflectable field. stores information about a reflected field. 
    struct ReflectableField {
        std::string_view m_fieldName;
        size_t m_fieldOffset;
        std::string_view m_fieldType;
    };

    // this did not have to be a namespace but it grew large quickly; 
    // i dont want to fix it. it functions the same
    namespace Reflection {
      
        constexpr const char* reflection_uninitialised = "UninitialisedReflectable";


        template <typename T, typename ComponentType>
        static const T* getFieldPtr(const ComponentType& component, const ReflectableField& field) {
            // get byte address of component
            auto* byteAddress = reinterpret_cast<uint8_t*>(component);
            
            // add the field offset to the byte address to resolve the field address
            auto* fieldAddress = byteAddress + field.m_fieldOffset;

            // cast to type T and return
            return reinterpret_cast<T*>(fieldAddress);

        }
        static const void* getRawFieldPtr(const void* component, const ReflectableField& field) {
            return static_cast<const char*>(component) + field.m_fieldOffset;
        }

        // get the base class offset for adjusting parent reflectable fields
        template <typename Derived, typename Base>
        static inline size_t getBaseOffset() {
            if constexpr (std::is_base_of<Base, Derived>()) {

                alignas(Derived) char _[sizeof(Derived)];
                auto* _derived = reinterpret_cast<const Derived*>(_);
                auto* _base    = static_cast<const Base*>(_derived);
                // return the offset
                return reinterpret_cast<const char*>(_base) - reinterpret_cast<const char*>(_derived);
            }
            return 0;
        }

        // descriptor for a reflected class or type 
        struct TypeDescriptor {
            
            std::string_view name     = reflection_uninitialised;
            
            std::string_view fullName = reflection_uninitialised;
            
            const std::vector<ReflectableField>& (*getFields)() = nullptr;
            
            std::vector<std::pair<ReflectableField, void*>> (*getFieldsForInstance)(void* basePtr) = nullptr;
            
            std::shared_ptr<void>(*createInstance)() = nullptr;

            void (*serialise)(const void* instancePtr, std::ostream& out, int indentLevel) = nullptr;
        };

        TypeDescriptor& getOrCreateTypeDescriptor(std::string_view key);

        // use this to get the type information on type T
        template <typename T>
        struct TypeInfo {
            static constexpr std::string_view getTypeKey() {
                return __PRETTY_FUNCTION__;
            }
            static inline TypeDescriptor& descriptor = getOrCreateTypeDescriptor(getTypeKey());
            static void initTypeInfo() {}
        };

        // get the type registry
        std::unordered_map<std::string_view, const TypeDescriptor*>& getTypeRegistry();

        // get object fields from types
        template <typename T>
        std::vector<std::pair<ReflectableField, void*>> getObjectFields(T* instance) {
            std::vector<std::pair<ReflectableField, void*>> result;
            const auto& desc = TypeInfo<std::decay_t<T>>::descriptor;
            if (!desc.getFields) return result;

            for (const auto& field : desc.getFields()) {
                auto typedPtr = getFieldPtr<std::decay_t<T>>(instance, field);
                result.emplace_back(field, const_cast<void*>(static_cast<const void*>(typedPtr)));
            }
            return result;
        }

        // this is public but please only use this for anything to do with reflection
        static Logging::Logger reflectLogger() {
            static Logging::Logger instance("REFLECTION");
            return instance;
        }

        // get the type name of T
        template <typename T>
        static inline std::string_view getTypenameOf() {
            return TypeInfo<T>::descriptor.fullName;
        }

        template <typename T>
        static inline std::string_view getFullTypenameOf() {
            return TypeInfo<T>::descriptor.fullName;
        }

        // codegen.py generates this function
        void initialiseReflectionData();

        // reflectable interface. anything that derives from this is reflectable.
        class IReflectable {
        protected:

            const Reflection::TypeDescriptor* _typeDesc = nullptr;

        public:

            friend void ::Engine::Reflection::initialiseReflectionData();

            virtual ~IReflectable() = default;

            // allows factories or Entity::addComponent<T>() to add the derived descriptor
            void setTypeDescriptor(const Reflection::TypeDescriptor* desc) {
                _typeDesc = desc;
            }

            std::string_view getTypename() const {
                return _typeDesc ? _typeDesc->name : reflection_uninitialised;
            }
            std::string_view getFullTypename() const {
                return _typeDesc ? _typeDesc->fullName : reflection_uninitialised;
            }

        
            std::vector<std::pair<ReflectableField, void*>> getFields() {
                if (!_typeDesc || !_typeDesc->getFieldsForInstance) return {};
                return _typeDesc->getFieldsForInstance(this);
            }
        };
    }
}


// cast a reflectable type without crashing on fail,
// taking advantage of reflection.
// this is MUCH quicker than dynamic_cast<T>.
// dynamic_refl_cast does not support down/upcasting.
// it will only succeed if T is the exact type of ptr.
template <typename T>
T dynamic_refl_cast(Engine::Reflection::IReflectable* ptr) {
    if (!ptr) return nullptr;

    // getFullTypenameOf<MyType*> would fail,
    // whereas getFullTypenameOf<MyType> wouldn't.
    auto typeName = Engine::Reflection::getFullTypenameOf<std::remove_pointer_t<T>>();

    if (ptr->getFullTypename() == typeName) {
        return static_cast<T>(ptr);
    }
    return nullptr;
}

/// cast a reflectable type without crashing on fail,
// taking advantage of reflection.
// this is MUCH quicker than dynamic_cast<T>.
template <typename T>
std::shared_ptr<T> dynamic_refl_pointer_cast(std::shared_ptr<Engine::Reflection::IReflectable> ptr) {
    if (!ptr) return nullptr;

    auto typeName = Engine::Reflection::getFullTypenameOf<std::remove_pointer_t<T>>();

    if (ptr->getFullTypename() == typeName) {
        return static_pointer_cast<T>(ptr);
    }
    return nullptr;
}
