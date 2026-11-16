#include <component/component.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <renderer.hpp>

using namespace Engine;

bool IComponent::hasInitRun() {
    return m_hasInit;
}

// get the next available ID for a component type.
ComponentTypeID IComponent::_nextID() {
    // create static variable (once)
    static ComponentTypeID ctr = 0;
    // increment
    return ctr++;
}

// get the entity this component is assigned to
Entity& IComponent::getEntity() {
    return *m_entity;
}

// get the entity this component is assigned to
const Entity& IComponent::getEntity() const {
    return *m_entity;
}

// get the type id
const ComponentTypeID IComponent::getTypeID() const {
    return m_typeID;
}

void IComponent::physicsUpdateImpl(double delta) {
    // do our physics update
    physicsUpdate(delta);
    if (entity.transform.isDirty()) {
        // we did do physics this frame.
        entity.m_didPhysicsThisTick = true;
    }
}