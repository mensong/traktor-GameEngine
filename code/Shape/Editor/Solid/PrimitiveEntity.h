#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding3.h"
#include "World/Entity.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
    namespace shape
    {

class T_DLLCLASS PrimitiveEntity : public world::Entity
{
    T_RTTI_CLASS;

public:
    PrimitiveEntity(const Transform& transform = Transform::identity());

    virtual void setTransform(const Transform& transform) override final;

    virtual bool getTransform(Transform& outTransform) const override final;

	virtual Aabb3 getBoundingBox() const override final;

    virtual void update(const world::UpdateParams& update) override final;

    const Transform& getTransform() const { return m_transform; }

    const AlignedVector< Winding3 >& getWindings() const { return m_windings; }

    bool isDirty() const { return m_dirty; }

    void resetDirty() { m_dirty = false; }

protected:
    friend class PrimitiveEntityData;

    Transform m_transform;
    AlignedVector< Winding3 > m_windings;
    bool m_dirty;
};

    }
}