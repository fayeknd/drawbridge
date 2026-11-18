#include <reflectable.hpp>


namespace Engine::Reflection{
    // get the type registry
    std::unordered_map<std::string_view, const TypeDescriptor*>& getTypeRegistry() {
        static std::unordered_map<std::string_view, const TypeDescriptor*> registry;
        return registry;
    }

    
    TypeDescriptor& getOrCreateTypeDescriptor(std::string_view typeKey) {
        static std::unordered_map<std::string_view, TypeDescriptor> storage;
        return storage[typeKey];
    }

}
