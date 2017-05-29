/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/StringOutputStream.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Flash/FlashDictionary.h"
#include "Flash/FlashEdit.h"
#include "Flash/FlashEditInstance.h"
#include "Flash/FlashFont.h"
#include "Flash/FlashTextFormat.h"
#include "Flash/TextLayout.h"
#include "Flash/Action/ActionContext.h"
#include "Html/Attribute.h"
#include "Html/Document.h"
#include "Html/Element.h"
#include "Html/Text.h"

namespace traktor
{
	namespace flash
	{
		namespace
		{

const struct { const wchar_t* name; Color4f color; } c_colorTable[] =
{
	{ L"black",  Color4f(0.0f, 0.0f, 0.0f, 1.0f) },
	{ L"red",    Color4f(1.0f, 0.0f, 0.0f, 1.0f) },
	{ L"green",  Color4f(0.0f, 1.0f, 0.0f, 1.0f) },
	{ L"blue",   Color4f(0.0f, 0.0f, 1.0f, 1.0f) },
	{ L"yellow", Color4f(1.0f, 1.0f, 0.0f, 1.0f) },
	{ L"white",  Color4f(1.0f, 1.0f, 1.0f, 1.0f) },
	{ L"lime",   Color4f(0.3f, 0.5f, 1.0f, 1.0f) }
};

bool parseColor(const std::wstring& color, Color4f& outColor)
{
	if (startsWith< std::wstring >(color, L"#"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"#%02x%02x%02x", &red, &green, &blue);
		outColor = Color4f(red, green, blue, 255.0f) / Scalar(255.0f);
		return true;
	}
	else if (startsWith< std::wstring >(color, L"rgb"))
	{
		int32_t red, green, blue;
		swscanf(color.c_str(), L"rgb(%d,%d,%d)", &red, &green, &blue);
		outColor = Color4f(red, green, blue, 255.0f) / Scalar(255.0f);
		return true;
	}
	else if (toLower(color) == L"none")
		return false;

	for (int32_t i = 0; i < sizeof_array(c_colorTable); ++i)
	{
		if (toLower(color) == c_colorTable[i].name)
		{
			outColor = c_colorTable[i].color;
			return true;
		}
	}

	return false;
}

void traverseHtmlDOM(
	FlashEditInstance* editInstance,
	const html::Element* element,
	const FlashFont* font,
	const Color4f& textColor,
	TextLayout* layout,
	StringOutputStream& text
)
{
	Color4f color = textColor;

	if (element->getName() == L"font")
	{
		Ref< const html::Attribute > colorAttribute = element->getAttribute(L"color");
		if (colorAttribute)
		{
			if (!parseColor(colorAttribute->getValue(), color))
				color = textColor;
		}
	}
	else if (element->getName() == L"img")
	{
		Ref< const html::Attribute > sourceAttribute = element->getAttribute(L"src");
		if (sourceAttribute)
		{
			ActionContext* context = editInstance->getContext();
			FlashDictionary* dictionary = editInstance->getDictionary();

			uint16_t id;
			if (dictionary->getExportId(wstombs(sourceAttribute->getValue()), id))
			{
				const FlashCharacter* character = dictionary->getCharacter(id);
				T_FATAL_ASSERT (character);

				Ref< FlashCharacterInstance > instance = character->createInstance(
					context,
					dictionary,
					editInstance,
					"",
					Matrix33::identity(),
					0,
					0
				);
				if (instance)
				{
					layout->insertCharacter(instance);
				}
			}
		}
	}
	else if (element->getName() == L"br")
	{
		layout->newLine();
		text << Endl;
	}

	layout->setAttribute(font, color);

	for (const html::Node* child = element->getFirstChild(); child; child = child->getNextSibling())
	{
		if (const html::Element* childElement = dynamic_type_cast< const html::Element* >(child))
			traverseHtmlDOM(editInstance, childElement, font, color, layout, text);
		else
		{
			if (layout->getCursorX() > FUZZY_EPSILON)
				layout->insertText(L" ");
			layout->insertText(child->getValue());
			text << child->getValue();
		}
	}

	if (element->getName() == L"p")
		layout->newLine();

	layout->setAttribute(font, textColor);
}

Aabb2 adjustForGutter(const Aabb2& aabb)
{
	return Aabb2(
		Vector2(
			aabb.mn.x + 2.0f * 20.0f,
			aabb.mn.y + 2.0f * 20.0f
		),
		Vector2(
			aabb.mx.x - 2.0f * 20.0f,
			aabb.mx.y - 2.0f * 20.0f
		)
	);
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.FlashEditInstance", FlashEditInstance, FlashCharacterInstance)

FlashEditInstance::FlashEditInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html)
:	FlashCharacterInstance(context, "TextField", dictionary, parent)
,	m_edit(edit)
,	m_textBounds(edit->getTextBounds())
,	m_textColor(edit->getTextColor())
,	m_letterSpacing(0.0f)
,	m_align(edit->getAlign())
,	m_fontHeight(edit->getFontHeight())
,	m_html(false)
,	m_wordWrap(edit->wordWrap())
,	m_password(edit->password())
,	m_caret(0)
,	m_scroll(0)
,	m_layout(new TextLayout())
,	m_renderClipMask(true)
{
	if (m_edit->renderHtml())
		parseHtml(html);
	else
		parseText(html);
}

bool FlashEditInstance::parseText(const std::wstring& text)
{
	if (!internalParseText(text))
		return false;
	
	m_caret = 0;

	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();
		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const AlignedVector< TextLayout::Word >& words = lines.back().words;
			for (AlignedVector< TextLayout::Word >::const_iterator j = words.begin(); j != words.end(); ++j)
				m_caret += (int32_t)j->chars.size();
		}
	}

	return true;
}

bool FlashEditInstance::parseHtml(const std::wstring& html)
{
	if (!internalParseHtml(html))
		return false;

	m_caret = 0;

	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();
		for (AlignedVector< TextLayout::Line >::const_iterator i = lines.begin(); i != lines.end(); ++i)
		{
			const AlignedVector< TextLayout::Word >& words = lines.back().words;
			for (AlignedVector< TextLayout::Word >::const_iterator j = words.begin(); j != words.end(); ++j)
				m_caret += (int32_t)j->chars.size();
		}
	}

	return true;
}

Vector2 FlashEditInstance::measureText(const std::wstring& text) const
{
	const FlashDictionary* dictionary = getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	const float mxf = std::numeric_limits< float >::max();

	TextLayout layout;
	layout.begin();
	layout.setBounds(Aabb2(Vector2(0.0f, 0.0f), Vector2(mxf, mxf)));
	layout.setLeading(m_edit->getLeading());
	layout.setLetterSpacing(m_password ? 6 : m_letterSpacing);
	layout.setFontHeight(m_fontHeight);
	layout.setWordWrap(false);
	layout.setAlignment(m_align);
	layout.setAttribute(font, m_textColor);
	layout.insertText(text);
	layout.end();

	return Vector2(layout.getWidth(), layout.getHeight());
}

void FlashEditInstance::setTextBounds(const Aabb2& textBounds)
{
	m_textBounds = textBounds;
}

void FlashEditInstance::setTextColor(const Color4f& textColor)
{
	m_textColor = textColor;
	updateLayout();
}

void FlashEditInstance::setLetterSpacing(float letterSpacing)
{
	m_letterSpacing = letterSpacing;
	updateLayout();
}

void FlashEditInstance::setTextFormat(const FlashTextFormat* textFormat)
{
	m_letterSpacing = textFormat->getLetterSpacing();
	m_align = textFormat->getAlign();
	m_fontHeight = textFormat->getSize();
	updateLayout();
}

Ref< FlashTextFormat > FlashEditInstance::getTextFormat() const
{
	return new FlashTextFormat(m_letterSpacing, m_align, m_fontHeight);
}

void FlashEditInstance::setTextFormat(const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex)
{
	m_letterSpacing = textFormat->getLetterSpacing();
	m_align = textFormat->getAlign();
	m_fontHeight = textFormat->getSize();
	updateLayout();
}

Ref< FlashTextFormat > FlashEditInstance::getTextFormat(int32_t beginIndex, int32_t endIndex) const
{
	return new FlashTextFormat(m_letterSpacing, m_align, m_fontHeight);
}

const std::wstring& FlashEditInstance::getHtmlText() const
{
	return m_html ? m_htmlText : m_text;
}

void FlashEditInstance::setWordWrap(bool wordWrap)
{
	m_wordWrap = wordWrap;
	updateLayout();
}

void FlashEditInstance::setPassword(bool password)
{
	m_password = password;
	updateLayout();
}

void FlashEditInstance::setScroll(int32_t scroll)
{
	m_scroll = scroll;
}

int32_t FlashEditInstance::getMaxScroll() const
{
	if (m_layout)
	{
		const AlignedVector< TextLayout::Line >& lines = m_layout->getLines();

		float lineHeight = m_layout->getFontHeight() + m_layout->getLeading();
		float editHeight = m_textBounds.mx.y - m_textBounds.mn.y;

		int32_t maxScroll = int32_t(lines.size()) - int32_t(editHeight / lineHeight + 0.5f);
		return std::max(maxScroll, 0);
	}
	else
		return 0;
}

Ref< TextLayout > FlashEditInstance::prepareTextLayout() const
{
	const FlashDictionary* dictionary = getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	Ref< TextLayout > layout = new TextLayout();

	layout->begin();

	layout->setBounds(adjustForGutter(m_textBounds));
	layout->setLeading(m_edit->getLeading());
	layout->setLetterSpacing(m_password ? 6 : m_letterSpacing);
	layout->setFontHeight(m_fontHeight);
	layout->setWordWrap(m_wordWrap);
	layout->setAlignment(m_align);
	layout->setAttribute(font, m_textColor);

	return layout;
}

void FlashEditInstance::setTextLayout(TextLayout* layout)
{
	m_layout = layout;
}

void FlashEditInstance::setRenderClipMask(bool renderClipMask)
{
	m_renderClipMask = renderClipMask;
}

Aabb2 FlashEditInstance::getBounds() const
{
	return getTransform() * m_textBounds;
}

void FlashEditInstance::eventKey(wchar_t unicode)
{
	if (getContext()->getFocus() != this)
		return;

	if (unicode == L'\n' || unicode == L'\r' || unicode == L'\t')
		return;

	if (m_html)
		m_text = L"";

	if (unicode == L'\b')
	{
		if (!m_text.empty())
		{
			m_text = m_text.substr(0, m_text.length() - 1);
			m_caret--;
		}
	}
	else if (m_text.length() < m_edit->getMaxLength())
	{
		m_text += unicode;
		m_caret++;
	}
	
	internalParseText(m_text);

	executeScriptEvent(ActionContext::IdOnChanged, ActionValue(getAsObject()));
}

void FlashEditInstance::eventMouseDown(int32_t x, int32_t y, int32_t button)
{
	if (!m_edit->readOnly())
	{
		Vector2 xy = getFullTransform().inverse() * Vector2(float(x), float(y));
		if (m_textBounds.inside(xy))
			getContext()->setFocus(this);
	}

	FlashCharacterInstance::eventMouseDown(x, y, button);
}

void FlashEditInstance::setPosition(const Vector2& position)
{
	Matrix33 m = getTransform();
	m.e13 = position.x * 20.0f;
	m.e23 = position.y * 20.0f;
	setTransform(m);
}

Vector2 FlashEditInstance::getPosition() const
{
	const Matrix33& m = getTransform();
	return Vector2(m.e13 / 20.0f, m.e23 / 20.0f);
}

void FlashEditInstance::setX(float x)
{
	Matrix33 m = getTransform();
	m.e13 = x * 20.0f;
	setTransform(m);
}

float FlashEditInstance::getX() const
{
	const Matrix33& m = getTransform();
	return m.e13 / 20.0f;
}

void FlashEditInstance::setY(float y)
{
	Matrix33 m = getTransform();
	m.e23 = y * 20.0f;
	setTransform(m);
}

float FlashEditInstance::getY() const
{
	const Matrix33& m = getTransform();
	return m.e23 / 20.0f;
}

void FlashEditInstance::setSize(const Vector2& size)
{
	m_textBounds.mx.x = m_textBounds.mn.x + size.x * 20.0f;
	m_textBounds.mx.y = m_textBounds.mn.y + size.y * 20.0f;
}

Vector2 FlashEditInstance::getSize() const
{
	return Vector2(
		(m_textBounds.mx.x - m_textBounds.mn.x) / 20.0f,
		(m_textBounds.mx.y - m_textBounds.mn.y) / 20.0f
	);
}

void FlashEditInstance::setWidth(float width)
{
	m_textBounds.mx.x = m_textBounds.mn.x + width * 20.0f;
}

float FlashEditInstance::getWidth() const
{
	return (m_textBounds.mx.x - m_textBounds.mn.x) / 20.0f;
}

void FlashEditInstance::setHeight(float height)
{
	m_textBounds.mx.y = m_textBounds.mn.y + height * 20.0f;
}

float FlashEditInstance::getHeight() const
{
	return (m_textBounds.mx.y - m_textBounds.mn.y) / 20.0f;
}

void FlashEditInstance::setRotation(float rotation)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	R = deg2rad(rotation);
	setTransform(Matrix33::compose(T, S, R));
}

float FlashEditInstance::getRotation() const
{
	float R;
	getTransform().decompose(0, 0, &R);
	return rad2deg(R);
}

void FlashEditInstance::setScale(const Vector2& scale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = scale.x / 100.0f;
	S.y = scale.y / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

Vector2 FlashEditInstance::getScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return Vector2(
		S.x * 100.0f,
		S.y * 100.0f
	);
}

void FlashEditInstance::setXScale(float xscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.x = xscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float FlashEditInstance::getXScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.x * 100.0f;
}

void FlashEditInstance::setYScale(float yscale)
{
	Vector2 T, S;
	float R;

	getTransform().decompose(&T, &S, &R);
	S.y = yscale / 100.0f;
	setTransform(Matrix33::compose(T, S, R));
}

float FlashEditInstance::getYScale() const
{
	Vector2 S;
	getTransform().decompose(0, &S, 0);
	return S.y * 100.0f;
}

Vector2 FlashEditInstance::getTextSize() const
{
	return Vector2(m_layout->getWidth(), m_layout->getHeight());
}

float FlashEditInstance::getTextWidth() const
{
	return m_layout->getWidth();
}

float FlashEditInstance::getTextHeight() const
{
	return m_layout->getHeight();
}

void FlashEditInstance::trace(visitor_t visitor) const
{
	if (m_layout)
	{
		const RefArray< FlashCharacterInstance >& characters = m_layout->getCharacters();
		for (RefArray< FlashCharacterInstance >::const_iterator i = characters.begin(); i != characters.end(); ++i)
			visitor(*i);
	}
	FlashCharacterInstance::trace(visitor);
}

void FlashEditInstance::dereference()
{
	m_layout = 0;
	FlashCharacterInstance::dereference();
}

bool FlashEditInstance::internalParseText(const std::wstring& text)
{
	const FlashDictionary* dictionary = getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	m_layout->begin();

	m_layout->setBounds(adjustForGutter(m_textBounds));
	m_layout->setLeading(m_edit->getLeading());
	m_layout->setLetterSpacing(m_password ? 6 : m_letterSpacing);
	m_layout->setFontHeight(m_fontHeight);
	m_layout->setWordWrap(m_wordWrap);
	m_layout->setAlignment(m_align);
	m_layout->setAttribute(font, m_textColor);

	if (m_edit->multiLine())
	{
		StringSplit< std::wstring > ss(text, L"\n");
		for (StringSplit< std::wstring >::const_iterator i = ss.begin(); i != ss.end(); ++i)
		{
			m_layout->insertText(*i);
			m_layout->newLine();
		}
	}
	else if (m_password)
	{
		for (size_t i = 0; i < text.length(); ++i)
			m_layout->insertText(L"\u2022");
	}
	else
		m_layout->insertText(text);
	
	m_layout->end();

	m_text = text;
	m_htmlText.clear();
	m_html = false;

	return true;
}

bool FlashEditInstance::internalParseHtml(const std::wstring& html)
{
	html::Document document(false);
	if (!document.loadFromText(html))
		return false;

	const FlashDictionary* dictionary = getDictionary();
	T_ASSERT (dictionary);

	const FlashFont* font = dictionary->getFont(m_edit->getFontId());
	T_ASSERT (font);

	m_layout->begin();

	m_layout->setBounds(adjustForGutter(m_textBounds));
	m_layout->setLeading(m_edit->getLeading());
	m_layout->setLetterSpacing(m_letterSpacing);
	m_layout->setFontHeight(m_fontHeight);
	m_layout->setWordWrap(m_wordWrap);
	m_layout->setAlignment(m_align);
	m_layout->setAttribute(font, m_textColor);

	const html::Element* element = document.getDocumentElement();
	T_ASSERT (element);

	StringOutputStream text;
	traverseHtmlDOM(
		this,
		element,
		font,
		m_textColor,
		m_layout, 
		text
	);

	m_layout->end();

	m_text = text.str();
	m_htmlText = html;
	m_html = true;

	return true;
}

void FlashEditInstance::updateLayout()
{
	if (m_html)
		internalParseHtml(m_text);
	else
		internalParseText(m_text);
}

	}
}
