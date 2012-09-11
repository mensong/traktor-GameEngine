#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashEdit", 0, FlashEdit, FlashCharacter)

FlashEdit::FlashEdit()
:	m_fontId(0)
,	m_fontHeight(0)
,	m_align(AnLeft)
,	m_leftMargin(0)
,	m_rightMargin(0)
,	m_wordWrap(false)
,	m_multiLine(false)
,	m_renderHtml(false)
{
}

FlashEdit::FlashEdit(
	uint16_t id,
	uint16_t fontId,
	uint16_t fontHeight,
	const SwfRect& textBounds,
	const SwfColor& textColor,
	const std::wstring& initialText,
	Align align,
	uint16_t leftMargin,
	uint16_t rightMargin,
	bool wordWrap,
	bool multiLine,
	bool renderHtml
)
:	FlashCharacter(id)
,	m_fontId(fontId)
,	m_fontHeight(fontHeight)
,	m_textBounds(textBounds)
,	m_textColor(textColor)
,	m_initialText(initialText)
,	m_align(align)
,	m_leftMargin(leftMargin)
,	m_rightMargin(rightMargin)
,	m_wordWrap(wordWrap)
,	m_multiLine(multiLine)
,	m_renderHtml(renderHtml)
{
}

Ref< FlashCharacterInstance > FlashEdit::createInstance(ActionContext* context, FlashCharacterInstance* parent, const std::string& name, const ActionObject* initObject) const
{
	return new FlashEditInstance(context, parent, this, m_initialText);
}

uint16_t FlashEdit::getFontId() const
{
	return m_fontId;
}

uint16_t FlashEdit::getFontHeight() const
{
	return m_fontHeight;
}

const SwfRect& FlashEdit::getTextBounds() const
{
	return m_textBounds;
}

const SwfColor& FlashEdit::getTextColor() const
{
	return m_textColor;
}

const std::wstring& FlashEdit::getInitialText() const
{
	return m_initialText;
}

FlashEdit::Align FlashEdit::getAlign() const
{
	return m_align;
}

uint16_t FlashEdit::getLeftMargin() const
{
	return m_leftMargin;
}

uint16_t FlashEdit::getRightMargin() const
{
	return m_rightMargin;
}

bool FlashEdit::wordWrap() const
{
	return m_wordWrap;
}

bool FlashEdit::multiLine() const
{
	return m_multiLine;
}

bool FlashEdit::renderHtml() const
{
	return m_renderHtml;
}

bool FlashEdit::serialize(ISerializer& s)
{
	const MemberEnum< Align >::Key kAlign[] =
	{
		{ L"AnLeft", AnLeft },
		{ L"AnRight", AnRight },
		{ L"AnCenter", AnCenter },
		{ L"AnJustify", AnJustify },
		{ 0, 0 }
	};

	if (!FlashCharacter::serialize(s))
		return false;

	s >> Member< uint16_t >(L"fontId", m_fontId);
	s >> Member< uint16_t >(L"fontHeight", m_fontHeight);
	s >> MemberSwfRect(L"textBounds", m_textBounds);
	s >> MemberSwfColor(L"textColor", m_textColor);
	s >> Member< std::wstring >(L"initialText", m_initialText);
	s >> MemberEnum< Align >(L"m_align", m_align, kAlign);
	s >> Member< uint16_t >(L"leftMargin", m_leftMargin);
	s >> Member< uint16_t >(L"rightMargin", m_rightMargin);
	s >> Member< bool >(L"wordWrap", m_wordWrap);
	s >> Member< bool >(L"multiLine", m_multiLine);
	s >> Member< bool >(L"renderHtml", m_renderHtml);

	return true;
}

	}
}
