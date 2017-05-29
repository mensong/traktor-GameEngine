/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_flash_FlashEditInstance_H
#define traktor_flash_FlashEditInstance_H

#include "Flash/FlashCharacterInstance.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class FlashEdit;
class FlashTextFormat;
class TextLayout;

/*! \brief Dynamic text character instance.
 * \ingroup Flash
 */
class T_DLLCLASS FlashEditInstance : public FlashCharacterInstance
{
	T_RTTI_CLASS;

public:
	FlashEditInstance(ActionContext* context, FlashDictionary* dictionary, FlashCharacterInstance* parent, const FlashEdit* edit, const std::wstring& html);

	const FlashEdit* getEdit() const { return m_edit; }

	bool parseText(const std::wstring& text);

	bool parseHtml(const std::wstring& html);

	Vector2 measureText(const std::wstring& text) const;

	void setTextBounds(const Aabb2& textBounds);

	const Aabb2& getTextBounds() const { return m_textBounds; }

	void setTextColor(const Color4f& textColor);

	const Color4f& getTextColor() const { return m_textColor; }

	void setLetterSpacing(float letterSpacing);

	float getLetterSpacing() const { return m_letterSpacing; }

	void setTextFormat(const FlashTextFormat* textFormat);

	Ref< FlashTextFormat > getTextFormat() const;

	void setTextFormat(const FlashTextFormat* textFormat, int32_t beginIndex, int32_t endIndex);

	Ref< FlashTextFormat > getTextFormat(int32_t beginIndex, int32_t endIndex) const;

	const std::wstring& getText() const { return m_text; }

	const std::wstring& getHtmlText() const;

	void setWordWrap(bool wordWrap);

	bool getWordWrap() const { return m_wordWrap; }

	void setPassword(bool password);

	bool getPassword() const { return m_password; }

	int32_t getCaret() const { return m_caret; }

	void setScroll(int32_t scroll);

	int32_t getScroll() const { return m_scroll; }

	int32_t getMaxScroll() const;

	/*! \brief Prepare a text layout.
	 *
	 * Preparing a text layout with edit field's font and attributes.
	 * Useful for custom layout of field's content.
	 *
	 * \return Prepared text layout.
	 */
	Ref< TextLayout > prepareTextLayout() const;

	/*! \brief Override text layout.
	 *
	 * \note
	 * Layout is modified if text is changed in this field.
	 *
	 * \param layout Text layout.
	 */
	void setTextLayout(TextLayout* layout);

	const TextLayout* getTextLayout() const { return m_layout; }

	void setRenderClipMask(bool renderClipMask);

	bool getRenderClipMask() const { return m_renderClipMask; }

	virtual Aabb2 getBounds() const T_OVERRIDE T_FINAL;

	virtual void eventKey(wchar_t unicode) T_OVERRIDE T_FINAL;

	virtual void eventMouseDown(int32_t x, int32_t y, int32_t button) T_OVERRIDE T_FINAL;

	/*! \group Convenience methods, commonly used by scripting. */
	//@{

	void setPosition(const Vector2& position);

	Vector2 getPosition() const;

	void setX(float x);

	float getX() const;

	void setY(float y);

	float getY() const;

	void setSize(const Vector2& size);

	Vector2 getSize() const;

	void setWidth(float width);

	float getWidth() const;

	void setHeight(float height);

	float getHeight() const;

	void setRotation(float rotation);

	float getRotation() const;

	void setScale(const Vector2& scale);

	Vector2 getScale() const;

	void setXScale(float xscale);

	float getXScale() const;

	void setYScale(float yscale);

	float getYScale() const;

	Vector2 getTextSize() const;

	float getTextWidth() const;

	float getTextHeight() const;

	//@}

protected:
	virtual void trace(visitor_t visitor) const T_OVERRIDE;

	virtual void dereference() T_OVERRIDE;

private:
	Ref< const FlashEdit > m_edit;
	Aabb2 m_textBounds;
	Color4f m_textColor;
	float m_letterSpacing;
	SwfTextAlignType m_align;
	float m_fontHeight;
	std::wstring m_text;
	std::wstring m_htmlText;
	bool m_html;
	bool m_wordWrap;
	bool m_password;
	int32_t m_caret;
	int32_t m_scroll;
	Ref< TextLayout > m_layout;
	bool m_renderClipMask;

	bool internalParseText(const std::wstring& text);

	bool internalParseHtml(const std::wstring& html);

	void updateLayout();
};

	}
}

#endif	// traktor_flash_FlashEditInstance_H
