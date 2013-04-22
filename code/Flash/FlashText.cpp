#include <sstream>
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberComposite.h"
#include "Flash/FlashText.h"
#include "Flash/FlashTextInstance.h"
#include "Flash/SwfMembers.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.flash.FlashText", 0, FlashText, FlashCharacter)

FlashText::FlashText()
:	m_textMatrix(Matrix33::identity())
{
}

FlashText::FlashText(uint16_t id, const SwfRect& textBounds, const Matrix33& textMatrix)
:	FlashCharacter(id)
,	m_textBounds(textBounds)
,	m_textMatrix(textMatrix)
{
}

bool FlashText::create(const AlignedVector< SwfTextRecord* >& textRecords)
{
	Character character = { 0, 0, 0, 0, { 255, 255, 255, 255 }, 0 };
	for (AlignedVector< SwfTextRecord* >::const_iterator i = textRecords.begin(); i != textRecords.end(); ++i)
	{
		const SwfTextRecord* textRecord = *i;
		if (textRecord->styleFlag)
		{
			if (textRecord->style.hasFont)
				character.fontId = textRecord->style.fontId;
			if (textRecord->style.hasColor)
				character.color = textRecord->style.color;
			if (textRecord->style.hasXOffset)
				character.offsetX = textRecord->style.XOffset;
			if (textRecord->style.hasYOffset)
				character.offsetY = textRecord->style.YOffset;
			if (textRecord->style.hasFont)
				character.height = textRecord->style.height;
		}
		else
		{
			for (uint8_t j = 0; j < textRecord->glyph.glyphCount; ++j)
			{
				character.glyphIndex = textRecord->glyph.glyphEntries[j]->glyphIndex;
				m_characters.push_back(character);
				character.offsetX += textRecord->glyph.glyphEntries[j]->glyphAdvance;
			}
		}
	}
	return true;
}

const AlignedVector< FlashText::Character >& FlashText::getCharacters() const
{
	return m_characters;
}

Ref< FlashCharacterInstance > FlashText::createInstance(
	ActionContext* context,
	FlashCharacterInstance* parent,
	const std::string& name,
	const ActionObject* initObject,
	const SmallMap< uint32_t, Ref< const IActionVMImage > >* events
) const
{
	return new FlashTextInstance(context, parent, this);
}

const SwfRect& FlashText::getTextBounds() const
{
	return m_textBounds;
}

const Matrix33& FlashText::getTextMatrix() const
{
	return m_textMatrix;
}

bool FlashText::serialize(ISerializer& s)
{
	if (!FlashCharacter::serialize(s))
		return false;

	s >> MemberSwfRect(L"textBounds", m_textBounds);
	s >> Member< Matrix33 >(L"textMatrix", m_textMatrix);
	s >> MemberAlignedVector< Character, MemberComposite< Character > >(L"characters", m_characters);

	return true;
}

bool FlashText::Character::serialize(ISerializer& s)
{
	s >> Member< uint16_t >(L"fontId", fontId);
	s >> Member< int16_t >(L"offsetX", offsetX);
	s >> Member< int16_t >(L"offsetY", offsetY);
	s >> Member< int16_t >(L"height", height);
	s >> MemberSwfColor(L"color", color);
	s >> Member< uint32_t >(L"glyphIndexOrCode", glyphIndex);
	return true;
}

	}
}
