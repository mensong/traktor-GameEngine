#include <limits>
#include "Flash/FlashButtonInstance.h"
#include "Flash/FlashButton.h"
#include "Flash/FlashMovie.h"
#include "Flash/Action/Classes/AsButton.h"
#include "Flash/Action/ActionVM.h"
#include "Flash/Action/ActionFrame.h"
#include "Flash/Action/ActionContext.h"
#include "Flash/Action/ActionScript.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashButtonInstance", FlashButtonInstance, FlashCharacterInstance)

FlashButtonInstance::FlashButtonInstance(ActionContext* context, FlashCharacterInstance* parent, const FlashButton* button)
:	FlashCharacterInstance(context, AsButton::getInstance(), parent)
,	m_button(button)
,	m_state(FlashButton::SmUp)
,	m_inside(false)
,	m_pushed(false)
{
	// Create character instances of each character in button's layer list.
	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< const FlashCharacter > character = context->getMovie()->getCharacter(i->characterId);
		if (!character)
			continue;

		Ref< FlashCharacterInstance > characterInstance = character->createInstance(context, this);
		T_ASSERT (characterInstance);

		m_characterInstances[i->characterId] = characterInstance;
	}
}

const FlashButton* FlashButtonInstance::getButton() const
{
	return m_button;
}

uint8_t FlashButtonInstance::getState() const
{
	return m_state;
}

FlashCharacterInstance* FlashButtonInstance::getCharacterInstance(uint16_t referenceId) const
{
	std::map< uint16_t, Ref< FlashCharacterInstance > >::const_iterator i = m_characterInstances.find(referenceId);
	return i != m_characterInstances.end() ? i->second.getPtr() : 0;
}

void FlashButtonInstance::eventMouseDown(ActionVM* actionVM, int x, int y, int button)
{
	if (m_inside && !m_pushed)
	{
		executeCondition(actionVM, FlashButton::CmOverUpToOverDown);
		executeScriptEvent(actionVM, L"onPress");
		m_state = FlashButton::SmDown;
		m_pushed = true;
	}
}

void FlashButtonInstance::eventMouseUp(ActionVM* actionVM, int x, int y, int button)
{
	if (m_inside && m_pushed)
	{
		executeCondition(actionVM, FlashButton::CmOverDownToOverUp);
		executeScriptEvent(actionVM, L"onRelease");
		m_state = FlashButton::SmOver;
		m_pushed = false;
	}
	else if (!m_inside && m_pushed)
	{
		executeCondition(actionVM, FlashButton::CmOutDownToIdle);
		executeScriptEvent(actionVM, L"onReleaseOutside");
		m_state = FlashButton::SmUp;
		m_pushed = false;
	}
}

void FlashButtonInstance::eventMouseMove(ActionVM* actionVM, int x, int y, int button)
{
	SwfRect bounds = getBounds();
	bool inside = (x >= bounds.min.x && y >= bounds.min.y && x <= bounds.max.x && y <= bounds.max.y);
	if (inside != m_inside)
	{
		if (inside)
		{
			if (button == 0)
			{
				executeCondition(actionVM, FlashButton::CmIdleToOverUp);
				executeScriptEvent(actionVM, L"onRollOver");
				m_state = FlashButton::SmOver;
			}
			else
			{
				executeCondition(actionVM, FlashButton::CmOutDownToOverDown);
				m_state = FlashButton::SmDown;
			}
		}
		else
		{
			if (button == 0)
			{
				executeCondition(actionVM, FlashButton::CmOverUpToIdle);
				executeScriptEvent(actionVM, L"onRollOut");
				m_state = FlashButton::SmUp;
			}
			else
			{
				executeCondition(actionVM, FlashButton::CmOverDownToOutDown);
				m_state = FlashButton::SmOver;
			}
		}
		m_inside = inside;
	}
}

SwfRect FlashButtonInstance::getBounds() const
{
	SwfRect bounds =
	{
		Vector2( std::numeric_limits< float >::max(),  std::numeric_limits< float >::max()),
		Vector2(-std::numeric_limits< float >::max(), -std::numeric_limits< float >::max())
	};

	const FlashButton::button_layers_t& layers = m_button->getButtonLayers();
	for (FlashButton::button_layers_t::const_iterator i = layers.begin(); i != layers.end(); ++i)
	{
		Ref< FlashCharacterInstance > characterInstance = getCharacterInstance(i->characterId);
		if (!characterInstance)
			continue;

		SwfRect characterBounds = characterInstance->getBounds();
		bounds.min.x = std::min(bounds.min.x, characterBounds.min.x);
		bounds.min.y = std::min(bounds.min.y, characterBounds.min.y);
		bounds.max.x = std::max(bounds.max.x, characterBounds.max.x);
		bounds.max.y = std::max(bounds.max.y, characterBounds.max.y);
	}

	bounds.min = getTransform() * bounds.min;
	bounds.max = getTransform() * bounds.max;

	return bounds;
}

void FlashButtonInstance::executeCondition(ActionVM* actionVM, uint32_t conditionMask)
{
	const FlashButton::button_conditions_t& conditions = m_button->getButtonConditions();
	for (FlashButton::button_conditions_t::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
	{
		if ((i->mask & conditionMask) == 0)
			continue;

		ActionFrame callFrame(getContext(), this, i->script->getCode(), i->script->getCodeSize(), 4, 0, 0);
		actionVM->execute(&callFrame);
	}
}

void FlashButtonInstance::executeScriptEvent(ActionVM* actionVM, const std::wstring& eventName)
{
	ActionValue memberValue;
	if (!getLocalMember(eventName, memberValue))
		return;

	Ref< ActionFunction > eventFunction = memberValue.getObject< ActionFunction >();
	if (!eventFunction)
		return;

	ActionFrame callFrame(getContext(), this, 0, 0, 4, 0, 0);
	eventFunction->call(actionVM, &callFrame, this);
}

	}
}
