#ifndef traktor_ui_custom_Sequence_H
#define traktor_ui_custom_Sequence_H

#include "Core/RefArray.h"
#include "Ui/Command.h"
#include "Ui/Rect.h"
#include "Ui/Custom/Sequencer/SequenceItem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_CUSTOM_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

class Bitmap;
class Command;

		namespace custom
		{

class Key;

/*! \brief Sequence of keys.
 * \ingroup UIC
 */
class T_DLLCLASS Sequence : public SequenceItem
{
	T_RTTI_CLASS;

public:
	Sequence(const std::wstring& name);

	int32_t addButton(Bitmap* imageUp, Bitmap* imageDown, const Command& command);

	bool getButtonState(int32_t buttonIndex) const;

	void addKey(Key* key);
	
	void removeKey(Key* key);

	void removeAllKeys();

	bool containsKey(Key* key) const;

	const RefArray< Key >& getKeys() const;

	Ref< Key > getSelectedKey() const;

	int clientFromTime(int time) const;

	int timeFromClient(int client) const;

	virtual void mouseDown(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset);

	virtual void mouseUp(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset);

	virtual void mouseMove(SequencerControl* sequencer, const Point& at, const Rect& rc, int button, int separator, int scrollOffset);

	virtual void paint(SequencerControl* sequencer, Canvas& canvas, const Rect& rc, int separator, int scrollOffset);

private:
	struct Button
	{
		Ref< Bitmap > imageUp;
		Ref< Bitmap > imageDown;
		Command command;
		bool state;
		Rect rc;
	};

	RefArray< Key > m_keys;
	std::vector< Button > m_buttons;
	Ref< Key > m_selectedKey;
	Ref< Key > m_trackKey;
	int32_t m_previousPosition;
	int32_t m_timeScale;
};

		}
	}
}

#endif	// traktor_ui_custom_Sequence_H
