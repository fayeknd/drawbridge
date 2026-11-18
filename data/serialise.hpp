#pragma once
#include <reflectable.hpp>
#include <fstream>

namespace Engine {
    class IResource;
}

namespace Engine::Serialise {

    // utility to get an asset path from a shared_ptr that may potentially be a resource.
    // see a registerResourcePtrType to see how this works
    using SharedPtrExtractor = std::function<IResource*(const void* sharedPtrAddr)>;
    std::unordered_map<std::string, SharedPtrExtractor>& sharedPtrPathExtMap();

    // register a resource type (usage: Engine::Serialise::registerResourcePtrType<MyResource>("MyResource"))
    template <typename T>
    void registerSharedPtrType(std::string typeName) {
        // emplace any existing resource path.
        // ensure safety against re-registering
        // a resource.
        // sharedPathExtMap holds a registered lambda.
        // this is where the lambda is registered.
        sharedPtrPathExtMap()[typeName] = [](const void* addr) -> IResource* {

            // attempt to convert a string ptr (such as "shared_ptr<MyResource>" into an actual shared_ptr<MyResource>*)
            const auto& ptr = *static_cast<const std::shared_ptr<T>*>(addr);
            return ptr.get();
        };
    }

    // same for weak ptrs
    template <typename T>
    void registerWeakPtrType(std::string typeName) {
        sharedPtrPathExtMap()[typeName] = [](const void* addr) -> IResource* {
            const auto& ptr = *static_cast<const std::weak_ptr<T>*>(addr);
            if (!ptr.expired()) return ptr.lock().get();
            return nullptr;
        };
    }

    // and unique ptrs 
    template <typename T>
    void registerUniquePtrType(std::string typeName) {
        sharedPtrPathExtMap()[typeName] = [](const void* addr) -> IResource* {
            const auto& ptr = *static_cast<const std::unique_ptr<T>*>(addr);
            return ptr.get();
        };
    }

    // for containers like vector and unordered_map
    using ContainerSerialiser = std::function<void(const void* containerPtr, std::ostream& out, int indentLevel)>;
    std::unordered_map<std::string, ContainerSerialiser>& containerSerialiserMap();

    // serialise fields to an out stream
    void serialiseFieldsToStream(const std::vector<std::pair<ReflectableField, void*>>& fields, std::ostream& out, int indentLv = 0);
    void serialiseField(std::string_view fieldName, const void* fieldPtr, std::ostream& out, int indentLv = 0);

    // register a vector type
    template <typename T>
    void registerVectorType(const std::string& fullTypeName, std::string_view elementTypeName) {

        // createa new lambda function for evaluating this container.
        containerSerialiserMap()[fullTypeName] = [elementTypeName](const void* ptr, std::ostream& out, int indentLevel) {
            const auto& vec = *static_cast<const std::vector<T>*>(ptr);
            // indentation is for serialisation
            std::string innerIndent((indentLevel + 1) * 4, ' ');
            std::string outerIndent(indentLevel * 4, ' ');

            // print this as an array type.
            // example:
            // myVector = [
            //     value1,
            //     value2
            // ]
            out << "[\n";
            for (size_t i = 0; i < vec.size(); ++i) {
                out << innerIndent;
                serialiseField(elementTypeName, &vec[i], out, indentLevel + 1);
                out << "\n";
            }
            out << outerIndent << "]";
        };
    }

    // register a map type 
    template <typename T>
    void registerMapType(const std::string& fullTypeName, std::string_view keyTypeName, std::string_view valTypeName) {
        
        // this one is different because maps have a key and a value, 
        // but other than that, it is exactly the same as registerVectorType.
        containerSerialiserMap()[fullTypeName] = [keyTypeName, valTypeName](const void* ptr, std::ostream& out, int indentLevel) {
            const auto& map = *static_cast<const T*>(ptr);
            std::string innerIndent((indentLevel + 1) * 4, ' ');
            std::string outerIndent(indentLevel * 4, ' ');

            out << "{\n";
            for (const auto& [k, v] : map) {
                out << innerIndent;
                serialiseField(keyTypeName, &k, out, indentLevel + 1);
                out << " : ";
                serialiseField(valTypeName, &v, out, indentLevel + 1);
                out << "\n";
            }
            out << outerIndent << "}";
        };
    }
}