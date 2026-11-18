#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <transform.hpp>
#include <utils/logger.hpp>
#include <layer.hpp>
#include <material.hpp>
#include <reflectable.hpp>

namespace Engine {

    // forward declaration to prevent
    // circular dependency issues.
    // in any component cpp file,
    // remember to include <entity.hpp>
    class Entity;


    // Component interface / abstract class.
    // Components are assigned to entities
    // and stored in a component map.
    // The components are kept track of via the
    // ComponentTypeID value. This value is 
    // assigned via a static template function.
    // More on that below

    class IComponent : public IEmbeddedResource {
    private:

        virtual const bool canDrawImpl() const { return false; }

        static Logging::Logger& log() {
            static Logging::Logger instance("COMPONENT");
            return instance;
        }

        // a ptr to this component's entity
        Entity* m_entity;

        // has this component's init() been run?
        bool m_hasInit;

    protected:

        // ignore this
        static inline std::string_view _reflectionName = "IComponent";
        static inline const std::vector<ReflectableField>& (*_reflectionFieldQuery)() = nullptr;


        // init() is called when the component is added to an entity, once.
        virtual void init() {};
        
        // actual physics update function
        virtual void physicsUpdate(double delta) {};

    public:

        // marks if this instance should render or not 
        // (given it can even be rendered)
        EXPORT bool m_render = true;

        // returns if the init function has been run yet.
        bool hasInitRun(); 

        virtual ~IComponent() = default; 

        // we don't want the entity to be written to
        Entity& getEntity();
        // const getter instead
        const Entity& getEntity() const;

        // we use this to make it look nicer syntactically.
        // this enables us to use entity.doSomething()
        // without entity being writable. just feels nicer to use.
        #define entity this->getEntity()

        // makes it a bit easier to run the base initialiser
        #define baseinit IComponent::init()
        
        // external init function. this is the externally visible
        // init function. used so each component does not have to call
        // the base init to have m_hasInit set to true, while calling
        // init will still set it to true. useful!
        void initImpl() {
            // set hasInit to true
            m_hasInit = true;
            // run the init function
            init();
        }


        // virtual basic behaviour functions.

        // update is obvious, called once per frame. delta time is passed through.
        // i know unity does not pass through delta time, but engines like 
        // cocos2d-x and godot do this, and it's helpful in those. 
        virtual void update(double delta) {};

        // lateUpdate is the second stage of update and all lateUpdate functions
        // are called after all update functions are finished calling.
        virtual void lateUpdate(double delta) {};

        // physics update calls an update every 50ms (same as minecraft yippee)
        void physicsUpdateImpl(double delta);

        // called right before the object gets destroyed. (or dereferenced, 
        // the object won't be destroyed if there is a shared_ptr to it.
        // this function will still run in that scenario though. don't store
        // a shared_ptr to any component.)
        virtual void onDestroy() {};

        // does this component have the ability to draw?
        // do not override canDrawImpl unless you know how
        // render commands work
        const bool canDraw() const { return canDrawImpl(); }

        // allow the Entity class full access to a component's functions.
        friend class Entity;

    };
}
/*
// use this instead of "public:" in components
#define component_public \
    public: \
        friend void Engine::initialiseReflectionData(); \
\
    protected: \
        static inline std::string_view _reflectionName = ""; \
        static inline const std::vector<Engine::ReflectableField>& (*_reflectionFieldQuery)() = nullptr; \
\
    public: \
        std::string_view getTypename() const override { \
            if (!_reflectionName.empty()) return _reflectionName; \
\
            return "UninitialisedComponent"; \
        } \
        std::vector<std::pair<ReflectableField, void*>> getFields() override { \
            using ComponentType = std::decay_t<std::remove_pointer_t<decltype(this)>>; \
            std::vector<std::pair<ReflectableField, void*>> _; \
            if (!_reflectionFieldQuery) return _; \
            auto& fields = _reflectionFieldQuery(); \
            for (Engine::ReflectableField field : fields) { \
                auto typedPtr = Reflection::getFieldPtr<ComponentType>(this, field); \
                void* genericVoidPtr = const_cast<void*>(static_cast<const void*>(typedPtr)); \
                _.push_back(std::make_pair(field, genericVoidPtr)); \
            } \
            return _; \
        } public

// if you don't like how long component_public is
#define cpublic component_public
*/