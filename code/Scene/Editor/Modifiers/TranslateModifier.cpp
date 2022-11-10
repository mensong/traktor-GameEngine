/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <numeric>
#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"
#include "Render/PrimitiveRenderer.h"
#include "Scene/Editor/EntityAdapter.h"
#include "Scene/Editor/SceneEditorContext.h"
#include "Scene/Editor/TransformChain.h"
#include "Scene/Editor/Modifiers/TranslateModifier.h"
#include "Ui/Application.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace scene
	{
		namespace
		{

const float c_guideThickness(0.02f);
const Scalar c_guideScale(0.15f);
const Scalar c_guideMinLength(1.0f);
const float c_infinite = 1e4f;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.scene.TranslateModifier", TranslateModifier, IModifier)

TranslateModifier::TranslateModifier(SceneEditorContext* context)
:	m_context(context)
,	m_axisEnable(0)
,	m_axisHot(0)
{
}

bool TranslateModifier::activate()
{
	return true;
}

void TranslateModifier::deactivate()
{
}

void TranslateModifier::selectionChanged()
{
	m_entityAdapters.clear();
	m_context->getEntities(m_entityAdapters, SceneEditorContext::GfDefault | SceneEditorContext::GfSelectedOnly | SceneEditorContext::GfNoExternalChild);

	m_baseTranslations.clear();
	for (auto entityAdapter : m_entityAdapters)
	{
		Transform T = entityAdapter->getTransform();
		m_baseTranslations.push_back(T.translation());
	}

	m_center = Vector4::origo();
	if (!m_baseTranslations.empty())
	{
		m_center = Vector4::zero();
		for (const auto& baseTranslation : m_baseTranslations)
			m_center += baseTranslation;

		m_center /= Scalar(float(m_baseTranslations.size()));
		m_center = snap(m_center.xyz1(), 1 | 2 | 4, false);
	}
}

bool TranslateModifier::cursorMoved(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection
)
{
	if (m_entityAdapters.empty())
		return false;

	Vector4 eye = transformChain.getView().inverse().translation();
	Scalar distance = (m_center - eye).xyz0().length();

	float axisLength = (distance / 4.0f) * m_context->getGuideSize();
	float arrowLength = axisLength / 8.0f;
	float squareLength = axisLength / 3.0f;

	TransformChain tc = transformChain;
	tc.pushWorld(translate(m_center));

	Vector4 viewDirection = m_center - eye;
	float sx = viewDirection.x() < 0.0f ? 1.0f : -1.0f;
	float sy = viewDirection.y() < 0.0f ? 1.0f : -1.0f;
	float sz = viewDirection.z() < 0.0f ? 1.0f : -1.0f;

	Vector2 center, axis0[3], axis1[3], square[6];
	tc.objectToScreen(Vector4(0.0f, 0.0f, 0.0f, 1.0f), center);
	tc.objectToScreen(Vector4(-axisLength, 0.0f, 0.0f, 1.0f), axis0[0]);
	tc.objectToScreen(Vector4(0.0f, -axisLength, 0.0f, 1.0f), axis0[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, -axisLength, 1.0f), axis0[2]);
	tc.objectToScreen(Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f), axis1[0]);
	tc.objectToScreen(Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f), axis1[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f), axis1[2]);
	tc.objectToScreen(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), square[0]);
	tc.objectToScreen(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), square[1]);
	tc.objectToScreen(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), square[2]);
	tc.objectToScreen(Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), square[3]);
	tc.objectToScreen(Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), square[4]);
	tc.objectToScreen(Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), square[5]);

	tc.popWorld();

	m_axisHot = 0;

	// Check drag circles.
	{
		// \FIXME
	}

	// Check combo squares.
	{
		Winding2 w(4);

		// XY
		w[0] = center;
		w[1] = square[0];
		w[2] = square[3];
		w[3] = square[1];
		if (w.inside(cursorPosition))
		{
			m_axisHot |= 1 | 2;
			goto hit;
		}

		// XZ
		w[0] = center;
		w[1] = square[0];
		w[2] = square[4];
		w[3] = square[2];
		if (w.inside(cursorPosition))
		{
			m_axisHot |= 1 | 4;
			goto hit;
		}

		// YZ
		w[0] = center;
		w[1] = square[1];
		w[2] = square[5];
		w[3] = square[2];
		if (w.inside(cursorPosition))
		{
			m_axisHot |= 2 | 4;
			goto hit;
		}

hit:;
	}

	// Check each line.
	if (m_axisHot == 0)
	{
		float guideThickness = ui::getSystemDPI() * c_guideThickness / 96.0f;
		if (Line2(axis0[0], axis1[0]).classify(cursorPosition, guideThickness))
			m_axisHot |= 1;
		if (Line2(axis0[1], axis1[1]).classify(cursorPosition, guideThickness))
			m_axisHot |= 2;
		if (Line2(axis0[2], axis1[2]).classify(cursorPosition, guideThickness))
			m_axisHot |= 4;
	}

	return m_axisHot != 0;
}

bool TranslateModifier::handleCommand(const ui::Command& command)
{
	if (command == L"Scene.Editor.Snap")
	{
		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			Transform T = m_entityAdapters[i]->getTransform0();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i], 1 | 2 | 4, false),
				T.rotation()
			));
		}
		selectionChanged();
		return true;
	}
	else
	{
		float offset = 0.0f;

		if (command == L"Scene.Editor.SnapToNext")
			offset = m_context->getSnapSpacing();
		else if (command == L"Scene.Editor.SnapToPrevious")
			offset = -m_context->getSnapSpacing();
		else
			return false;

		Vector4 delta(
			(m_axisHot & 1) ? offset : 0.0f,
			(m_axisHot & 2) ? offset : 0.0f,
			(m_axisHot & 4) ? offset : 0.0f,
			0.0f
		);

		for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
		{
			Transform T = m_entityAdapters[i]->getTransform0();
			m_entityAdapters[i]->setTransform(Transform(
				snap(m_baseTranslations[i] + delta, m_axisHot, false),
				T.rotation()
			));
		}

		selectionChanged();
		return true;
	}
}

bool TranslateModifier::begin(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	int32_t mouseButton
)
{
	m_axisEnable = m_axisHot;
	m_center0 = m_center;
	return true;
}

void TranslateModifier::apply(
	const TransformChain& transformChain,
	const Vector2& cursorPosition,
	const Vector4& worldRayOrigin,
	const Vector4& worldRayDirection,
	const Vector4& screenDelta,
	const Vector4& viewDelta,
	bool snapOverrideEnable
)
{
	Vector4 cp = transformChain.worldToClip(m_center);

	Vector4 worldDelta = transformChain.getView().inverse() * viewDelta * cp.w();
	if (!(m_axisEnable & 1))
		worldDelta *= Vector4(0.0f, 1.0f, 1.0f);
	if (!(m_axisEnable & 2))
		worldDelta *= Vector4(1.0f, 0.0f, 1.0f);
	if (!(m_axisEnable & 4))
		worldDelta *= Vector4(1.0f, 1.0f, 0.0f);
	m_center += worldDelta;

	Vector4 baseDelta = snap(m_center, 1 | 2 | 4, snapOverrideEnable) - m_center0;
	if (!(m_axisEnable & 1))
		baseDelta *= Vector4(0.0f, 1.0f, 1.0f);
	if (!(m_axisEnable & 2))
		baseDelta *= Vector4(1.0f, 0.0f, 1.0f);
	if (!(m_axisEnable & 4))
		baseDelta *= Vector4(1.0f, 1.0f, 0.0f);

	for (uint32_t i = 0; i < m_entityAdapters.size(); ++i)
	{
		Transform T = m_entityAdapters[i]->getTransform();
		m_entityAdapters[i]->setTransform(Transform(
			//m_baseTranslations[i] + baseDelta,	<< Snap in object space
			snap(m_baseTranslations[i] + baseDelta, m_axisEnable, snapOverrideEnable),	//< Snap in world space.
			T.rotation()
		));
	}

	m_axisHot = m_axisEnable;
}

void TranslateModifier::end(const TransformChain& transformChain)
{
	selectionChanged();
	m_axisEnable = 0;
}

void TranslateModifier::draw(render::PrimitiveRenderer* primitiveRenderer) const
{
	if (m_entityAdapters.empty())
		return;

	Vector4 eye = primitiveRenderer->getView().inverse().translation();
	Scalar distance = (m_center - eye).xyz0().length();

	float axisLength = (distance / 4.0f) * m_context->getGuideSize();
	float arrowLength = axisLength / 8.0f;
	float squareLength = axisLength / 3.0f;

	primitiveRenderer->pushWorld(translate(m_center));

	// Infinite "trace" lines.
	primitiveRenderer->pushDepthState(true, false, false);
	if (m_axisHot & 1)
		primitiveRenderer->drawLine(
			Vector4(-c_infinite, 0.0f, 0.0f, 1.0f),
			Vector4(c_infinite, 0.0f, 0.0f, 1.0f),
			1.0f,
			Color4ub(255, 0, 0, 100)
		);
	if (m_axisHot & 2)
		primitiveRenderer->drawLine(
			Vector4(0.0f, -c_infinite, 0.0f, 1.0f),
			Vector4(0.0f, c_infinite, 0.0f, 1.0f),
			1.0f,
			Color4ub(0, 255, 0, 100)
		);
	if (m_axisHot & 4)
		primitiveRenderer->drawLine(
			Vector4(0.0f, 0.0f, -c_infinite, 1.0f),
			Vector4(0.0f, 0.0f, c_infinite, 1.0f),
			1.0f,
			Color4ub(0, 0, 255, 100)
		);
	primitiveRenderer->popDepthState();

	primitiveRenderer->pushDepthState(false, false, false);

	// Drag circles.
	primitiveRenderer->drawWireCircle(
		Vector4(axisLength + arrowLength * 3.0f, 0.0f, 0.0f, 1.0f),
		Vector4(1.0f, 0.0f, 0.0f, 0.0f),
		0.2f,
		(float)ui::dpi96((m_axisHot & 1) ? 3 : 1),
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawWireCircle(
		Vector4(0.0f, axisLength + arrowLength * 3.0f, 0.0f, 1.0f),
		Vector4(0.0f, 1.0f, 0.0f, 0.0f),
		0.2f,
		(float)ui::dpi96((m_axisHot & 2) ? 3 : 1),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawWireCircle(
		Vector4(0.0f, 0.0f, axisLength + arrowLength * 3.0f, 1.0f),
		Vector4(0.0f, 0.0f, 1.0f, 0.0f),
		0.2f,
		(float)ui::dpi96((m_axisHot & 4) ? 3 : 1),
		Color4ub(0, 0, 255, 255)
	);

	Vector4 viewDirection = m_center - primitiveRenderer->getView().inverse().translation();
	float sx = viewDirection.x() < 0.0f ? 1.0f : -1.0f;
	float sy = viewDirection.y() < 0.0f ? 1.0f : -1.0f;
	float sz = viewDirection.z() < 0.0f ? 1.0f : -1.0f;

	// Guide fill squares.
	// XY
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f),
		Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f),
		Vector4(0.0f, sy * squareLength, 0.0f, 1.0f),
		Color4ub(255, 255, 0, m_axisHot == (1 | 2) ? 90 : 70)
	);
	// XZ
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f),
		Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f),
		Vector4(0.0f, 0.0f, sz * squareLength, 1.0f),
		Color4ub(255, 255, 0, m_axisHot == (1 | 4) ? 90 : 70)
	);
	// YZ
	primitiveRenderer->drawSolidQuad(
		Vector4(0.0f, 0.0f, 0.0f, 1.0f),
		Vector4(0.0f, sy * squareLength, 0.0f, 1.0f),
		Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f),
		Vector4(0.0f, 0.0f, sz * squareLength, 1.0f),
		Color4ub(255, 255, 0, m_axisHot == (2 | 4) ? 90 : 70)
	);

	// Guide square lines.
	// XY
	const float awxy = (float)ui::dpi96(((m_axisHot & (1 | 2)) == (1 | 2)) ? 3 : 1);
	primitiveRenderer->drawLine(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), awxy, Color4ub(255, 0, 0, 255));
	primitiveRenderer->drawLine(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), Vector4(sx * squareLength, sy * squareLength, 0.0f, 1.0f), awxy, Color4ub(0, 255, 0, 255));

	// XZ
	const float awxz = (float)ui::dpi96(((m_axisHot & (1 | 4)) == (1 | 4)) ? 3 : 1);
	primitiveRenderer->drawLine(Vector4(sx * squareLength, 0.0f, 0.0f, 1.0f), Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), awxz, Color4ub(255, 0, 0, 255));
	primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), Vector4(sx * squareLength, 0.0f, sz * squareLength, 1.0f), awxz, Color4ub(0, 0, 255, 255));

	// YZ
	const float awyz = (float)ui::dpi96(((m_axisHot & (2 | 4)) == (2 | 4)) ? 3 : 1);
	primitiveRenderer->drawLine(Vector4(0.0f, sy * squareLength, 0.0f, 1.0f), Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), awyz, Color4ub(0, 255, 0, 255));
	primitiveRenderer->drawLine(Vector4(0.0f, 0.0f, sz * squareLength, 1.0f), Vector4(0.0f, sy * squareLength, sz * squareLength, 1.0f), awyz, Color4ub(0, 0, 255, 255));

	// Guide axis lines.
	primitiveRenderer->drawLine(
		Vector4(-axisLength, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength, 0.0f, 0.0f, 1.0f),
		(float)ui::dpi96((m_axisHot & 1) ? 3 : 1),
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, -axisLength, 0.0f, 1.0f),
		Vector4(0.0f, axisLength, 0.0f, 1.0f),
		(float)ui::dpi96((m_axisHot & 2) ? 3 : 1),
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawLine(
		Vector4(0.0f, 0.0f, -axisLength, 1.0f),
		Vector4(0.0f, 0.0f, axisLength, 1.0f),
		(float)ui::dpi96((m_axisHot & 4) ? 3 : 1),
		Color4ub(0, 0, 255, 255)
	);

	// Guide arrows.
	primitiveRenderer->drawArrowHead(
		Vector4(axisLength, 0.0f, 0.0f, 1.0f),
		Vector4(axisLength + arrowLength, 0.0f, 0.0f, 1.0f),
		0.5f,
		Color4ub(255, 0, 0, 255)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, axisLength, 0.0f, 1.0f),
		Vector4(0.0f, axisLength + arrowLength, 0.0f, 1.0f),
		0.5f,
		Color4ub(0, 255, 0, 255)
	);
	primitiveRenderer->drawArrowHead(
		Vector4(0.0f, 0.0f, axisLength, 1.0f),
		Vector4(0.0f, 0.0f, axisLength + arrowLength, 1.0f),
		0.5f,
		Color4ub(0, 0, 255, 255)
	);

	primitiveRenderer->popDepthState();

	primitiveRenderer->popWorld();
}

Vector4 TranslateModifier::snap(const Vector4& position, uint32_t axisEnable, bool snapOverrideEnable) const
{
	if (m_context->getSnapMode() == SceneEditorContext::SmGrid && !snapOverrideEnable)
	{
		float spacing = m_context->getSnapSpacing();
		if (spacing > 0.0f)
		{
			return Vector4(
				(axisEnable & 1) ? floor(position[0] / spacing + 0.5f) * spacing : position[0],
				(axisEnable & 2) ? floor(position[1] / spacing + 0.5f) * spacing : position[1],
				(axisEnable & 4) ? floor(position[2] / spacing + 0.5f) * spacing : position[2],
				1.0f
			);
		}
	}
	return position;
}

	}
}
