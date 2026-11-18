#pragma once

#include <zip.h>
#include <filesystem>
#include <unordered_map>
#include <memory.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <utils/logger.hpp>
#include <time.hpp>
#include <reflectable.hpp>
#include <serialise.hpp>

namespace Engine {

    constexpr const char* resource_fileIdentifier    = "DRAWBRIDGE_RES";
    constexpr const char* resource_defaultProjectDir = "project/";
    constexpr const char* resource_defaultFileExt    = ".meta";

    #define proj_ std::string(resource_defaultProjectDir + std::string("resources/")) + 

    // the resource interface. 
    class IResource : public Reflection::IReflectable {
    protected:

        EXPORT std::string m_resourcePath;
        std::string m_metaResourcePath; 

    public:

        const std::string getResourcePath() const { return m_resourcePath; }
        const std::string getMetaResourcePath() const { return m_metaResourcePath; }

    };

    class IEmbeddedResource : public IResource {
    public:
        virtual ~IEmbeddedResource() = default;
    };

    // this doesn't do anything except prevent having to type
    // std::vector<unsigned char> often.
    // it's just an alias.
    using bytes = std::vector<unsigned char>;

    // the Archive is basically our version of Unity's AssetBundle.
    // but because of how resources get loaded, you won't need to
    // interact with archives at all.

    class Archive {
    protected:

        // zip from libzip, the Archive can be treated as a 
        // "wrapper" for these zips. 
        zip* m_zip;

        // archive file path on the disk, for caching.
        std::string m_arPath;

    public:

        // returns the archive path.
        const std::string& getPath();

        // returns an archive from a provided archive path.
        // if the archive is not cached, load an archive and cache it.
        static std::shared_ptr<Archive> getArchive(const std::string& path); 
        
        // basic deconstructor
        ~Archive();

        // basically give the ResourceManager trusted access to this class'
        // protected variables, but no other class.
        friend class ResourceManager;
    };

    // just a constexpr for the separator to look for in path strings.
    // In this case, given an archive is named simply "ar" in our files,
    // the path to an image titled "image.png" under "images" in that archive 
    // would look like this:
    // "ar=>images/image.png" 
    constexpr const char* resource_archiveSeparator = "=>";

    // the length of the above separator. can be computed at runtime
    // but  there's no point and it takes up like half a bit of memory so
    constexpr unsigned int resource_archiveSeparatorLen = 2;


    // the principle i have gone with for this is to keep every archive
    // loaded via the ResourceManager in memory until calling freeArchives().

    // The reason it's this way is because constantly loading an archive into 
    // memory just to get one object from it and then unload that archive, 
    // just to potentially require more objects from that archive is expensive.

    // so archives remain loaded, so access to their objects is quick,
    // and then when we are sure we don't need to be loading anything else
    // at least for a while, we can call freeArchives() to remove them
    // from memory.

    // This is not a flawless design, and there might be issues if we are loading
    // gigabytes of items, but until then we should be okay.


    // the ResourceManager is a static class responsible for the loading
    // and unloading of any sort of data stored in an archive, 
    // and general management for archives.

    // it is not a singleton with a get() function, simply because
    // it feels wrong to type ResourceManager::get().loadRaw();

    class ResourceManager {
    protected:

        // stores the path to the program executable.
        // computed once and then stored permanently in this.
        // this would be a const, but because it has to be
        // computed at runtime it can't be.

        static inline std::filesystem::path s_binaryDir = "none";

        static Logging::Logger& log();

        friend class Archive;

    public:

        // stores all the loaded archives. Uses their file path as the key.
        static std::unordered_map<std::string, std::shared_ptr<Archive>> s_archiveCache;

        // Returns the path to the program executable.
        // if s_binaryDir is "none", it will compute the 
        // path. otherwise, just return s_binaryDir.
        static const std::filesystem::path& getBinaryDir();

        // free all the archives from memory.
        static void freeArchives();

        // attempts to return / load a new (if none exists in memory)
        // archive that is present in memory.
        // for instance, "archive=>some/asset/here"
        // will attempt to load an archive titled "archive".
        // returns nullptr if failed.

        // path is the path provided, outPath is the 
        // new path with the "archive=>" part removed.
        // Usage looks like:
        // std::string newPath;
        // auto ar = ResourceManager::getArchiveInPath(fullPath, &newPath);
        // if (!ar) { error }
        // else ( dostuff )
        static std::shared_ptr<Archive> getArchiveInPath(const std::string& path, std::string* outPath);
        
        // attempt to load a file from a provided archive. returns an unsigned char vector.
        // usually you should not have to use this.
        static bytes getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path);

        // most abstracted method. provide a string such as "ar=>file.txt"
        // and this will return file.txt as a vector of uchars.
        // if you're only going to be loading a single file from
        // the archive, you can set keepArchive to false. this will
        // unload the archive immediately. 
        static bytes loadRaw(const std::string& path, bool keepArchive = false);

        template <typename T>
        static inline bool saveResource(std::shared_ptr<T> resource, const std::string& path = "") {
            return saveResource(resource.get(), path);
        }

        template <typename T>
        static inline std::shared_ptr<T> newResource(T* res) {
            auto _ = std::shared_ptr<T>(res);
            res->setTypeDescriptor(&Engine::Reflection::TypeInfo<T>::descriptor);
            return _;
        }

        // save/serialise a resource to disk given a filepath.
        // this uses the existing reflection 
        template <typename T>
        static inline bool saveResource(T* resource, const std::string& path = "") {
            // is this resource valid?
            if (!resource) { 
                // invalid. how did this happen?
                log().logWarning("Tried to serialise resource at '{}', but the resource instance was null.", path);
                return false;
            }
            // if this resource is not reflectable, then there won't be
            // anything to serialise.
            if constexpr (!std::is_base_of_v<IResource, T> && !std::is_same_v<IResource, T>) {
                // no, return early and let the user know they suck
                log().logWarning("Attempted to serialise a non-resource object! Nothing to serialise; nothing written.");
                return false;
            }
            const auto& registry = Reflection::getTypeRegistry();
            std::string_view key = resource->getFullTypename();
            auto it = registry.find(key);
            if (it == registry.end()) {
                log().logError("Reflection miss for '{}' (registry size: {})", key, registry.size());
                return false;
            }
            const auto& desc = it->second;
            // ok, we can actually serialise something now

            std::string fullPath;
            // is path empty?
            if (path == "") {
                std::string filename;
                // yes, generate one from the resource path if getResourcePath is not empty
                if (resource->getResourcePath() == "") {
                    // path is empty. try meta path?
                    filename = resource->getMetaResourcePath();
                    if (filename == "") {
                        // meta path is also empty. no name to go off of.
                        log().logWarning("Tried to serialise resource of type '{}', but there was no path given, and no name could be evaluated.", desc->fullName);
                        return false;
                    }
                    // yes, meta path already exists. that should have the full path to use already.
                }
                else {
                    // get the resource path
                    std::string path = resource->getResourcePath();
                    
                    // try remove any archive prefix
                    if (auto it = path.find(resource_archiveSeparator); it != std::string::npos) {
                        path = path.substr(it + resource_archiveSeparatorLen);
                    }
                    filename = proj_(path) + resource_defaultFileExt;
                }
                fullPath = filename;
            }
            else {
                fullPath = path;
            }

            // save the resource path
            resource->m_metaResourcePath = fullPath;
            fullPath = getBinaryDir().generic_string() + '/' + fullPath;

            // create all the potential parent directories this file might need
            std::filesystem::path parentDir = std::filesystem::path(fullPath).parent_path();
            // does this directory exist?
            if (!parentDir.empty()) {
                // no, create it
                std::error_code ec;
                std::filesystem::create_directories(parentDir, ec);
                // is there an error?
                if (ec) {
                    // yes
                    log().logError("Could not create directory for serialised resource of type '{}' at '{}'. Error: {}", desc->fullName, fullPath, ec.message());
                    return false;
                }
                // no!!
            }
            // create a new file
            std::ofstream file(fullPath);
            // did the file open successfully?
            if (!file.is_open()) {
                // no
                log().logWarning("Tried to serialise resource of type '{}' at '{}', but the file failed to open!", desc->fullName, fullPath);
                return false;
            }
            // ok now we can serialise something for real this time

            // write out the resource identifier. 
            // the engine needs this identifier to confirm the file
            // is indeed a valid resource file. 
            file << resource_fileIdentifier << "\n";
            // write out the resource type
            file << "@type " << desc->fullName << " {\n"; 
            // write out all the resource's serialisable fields to the file
            Serialise::serialiseFieldsToStream(resource->getFields(), file);
            // close off the file
            file << "}\n";

            file.close();

            log().logInfo("Serialised resource of type '{}' at '{}'.", desc->fullName, fullPath);

            return true;

        }

    };
}