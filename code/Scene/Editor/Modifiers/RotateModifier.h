/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_scene_RotateModifier_H
#define traktor_scene_RotateModifier_H

#include "Core/RefArray.h"
#include "Core/Containers/AlignedVector.h"
#include "Scene/Editor/IModifier.h"

namespace traktor
{
	namespace scene
	{

class EntityAdapter;
class SceneEditorContext;

/*! \brief Rotation modifier. */
class RotateModifier : public IModifier
{
	T_RTTI_CLASS;

public:
	RotateModifier(SceneEditorContext* context);

	/*! \name Notifications */
	//\{

	virtual void selectionChanged() T_OVERRIDE T_FINAL;

	virtual bool cursorMoved(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection
	) T_OVERRIDE T_FINAL;

	virtual bool handleCommand(const ui::Command& command) T_OVERRIDE T_FINAL;

	//\}

	/*! \name Modifications */
	//\{

	virtual bool begin(
		const TransformChain& transformChain,
		int32_t mouseButton
	) T_OVERRIDE T_FINAL;

	virtual void apply(
		const TransformChain& transformChain,
		const Vector2& cursorPosition,
		const Vector4& worldRayOrigin,
		const Vector4& worldRayDirection,
		const Vector4& screenDelta,
		const Vector4& viewDelta
	) T_OVERRIDE T_FINAL;

	virtual void end(const TransformChain& transformChain) T_OVERRIDE T_FINAL;

	//\}

	/*! \name Preview */
	//\{

	virtual void draw(render::PrimitiveRenderer* primitiveRenderer) const T_OVERRIDE T_FINAL;

	//\}

private:
	SceneEditorContext* m_context;
	RefArray< EntityAdapter > m_entityAdapters;
	AlignedVector< Transform > m_baseTransforms;
	Vector4 m_center;
	float m_baseHead;
	float m_basePitch;
	float m_baseBank;
	float m_deltaHead;
	float m_deltaPitch;
	float m_deltaBank;
	uint32_t m_axisEnable;
};

	}
}

#endif	// traktor_scene_RotateModifier_H
