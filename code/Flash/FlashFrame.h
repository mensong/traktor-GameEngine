#ifndef traktor_flash_FlashFrame_H
#define traktor_flash_FlashFrame_H

#include <map>
#include "Core/RefArray.h"
#include "Core/Math/Matrix33.h"
#include "Flash/Action/ActionObject.h"
#include "Flash/SwfTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_FLASH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace flash
	{

class ActionScript;

/*! \brief SWF frame actions.
 * \ingroup Flash
 *
 * Keep display list actions for a single
 * frame.
 */
class T_DLLCLASS FlashFrame : public ActionObject
{
	T_RTTI_CLASS;

public:
	struct PlaceAction
	{
		uint32_t eventMask;
		Ref< ActionScript > script;
	};

	struct PlaceObject
	{
		union
		{
			struct  
			{
				unsigned hasBitmapCaching : 1;
				unsigned hasBlendMode : 1;
				unsigned hasFilters : 1;
				unsigned hasActions : 1;
				unsigned hasClipDepth : 1;
				unsigned hasName : 1;
				unsigned hasRatio : 1;
				unsigned hasCxTransform : 1;
				unsigned hasMatrix : 1;
				unsigned hasCharacterId : 1;
				unsigned hasMove : 1;
			};
			uint16_t hasFlags;
		};

		uint16_t depth;
		uint8_t bitmapCaching;
		uint8_t blendMode;
		std::vector< PlaceAction > actions;
		uint16_t clipDepth;
		std::wstring name;
		uint16_t ratio;
		SwfCxTransform cxTransform;
		Matrix33 matrix;
		uint16_t characterId;

		PlaceObject()
		:	hasFlags(0)
		,	depth(0)
		{
		}
	};

	struct RemoveObject
	{
		unsigned hasCharacterId : 1;
		uint16_t depth;
		uint16_t characterId;

		RemoveObject()
		:	hasCharacterId(0)
		,	depth(0)
		{
		}
	};

	FlashFrame();

	/*! \brief Set frame label.
	 *
	 * \param label Frame label.
	 */
	void setLabel(const std::wstring& label);

	/*! \brief Get frame label.
	 *
	 * \return Frame label.
	 */
	const std::wstring& getLabel() const;

	/*! \brief Change background color used to clear frame.
	 *
	 * \param backgroundColor Background color.
	 */
	void changeBackgroundColor(const SwfColor& backgroundColor);

	/*! \brief Check if this frame changes background color.
	 *
	 * \return True if background color changed.
	 */
	bool hasBackgroundColorChanged() const;

	/*! \brief Get background color.
	 *
	 * \return Background color.
	 */
	const SwfColor& getBackgroundColor() const;

	/*! \brief Place object onto display list.
	 *
	 * \param placeObject Place object description.
	 */
	void placeObject(const PlaceObject& placeObject);

	/*! \brief Remove object from display list.
	 *
	 * \param removeObject Remove object description.
	 */
	void removeObject(const RemoveObject& removeObject);

	/*! \brief Add frame action script.
	 *
	 * \param actionScript Frame action script.
	 */
	void addActionScript(ActionScript* actionScript);

	/*! \brief Get place object descriptions by this frame.
	 *
	 * \return Place object descriptions.
	 */
	const std::map< uint16_t, PlaceObject >& getPlaceObjects() const;

	/*! \brief Get remove object descriptions by this frame.
	 *
	 * \return Remove object descriptions.
	 */
	const std::map< uint16_t, RemoveObject >& getRemoveObjects() const;

	/*! \brief Get action scripts associated with this frame.
	 *
	 * \return Action scripts.
	 */
	const RefArray< ActionScript >& getActionScripts() const;

private:
	std::wstring m_label;
	bool m_backgroundColorChange;
	SwfColor m_backgroundColor;
	std::map< uint16_t, PlaceObject > m_placeObjects;
	std::map< uint16_t, RemoveObject > m_removeObjects;
	RefArray< ActionScript > m_actionScripts;
};

	}
}

#endif	// traktor_flash_FlashFrame_H
