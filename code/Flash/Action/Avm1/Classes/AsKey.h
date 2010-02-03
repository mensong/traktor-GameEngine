#ifndef traktor_flash_AsKey_H
#define traktor_flash_AsKey_H

#include "Core/RefArray.h"
#include "Flash/Action/Avm1/ActionClass.h"

namespace traktor
{
	namespace flash
	{

class ActionContext;
class IActionVM;
struct CallArgs;

/*! \brief Key class.
 * \ingroup Flash
 */
class AsKey : public ActionClass
{
	T_RTTI_CLASS;

public:
	enum AsKeyEnum
	{
		AkBackspace = 8,
		AkCapsLock = 20,
		AkControl = 17,
		AkDeleteKey = 46,
		AkDown = 40,
		AkEnd = 35,
		AkEnter = 13,
		AkEscape = 27,
		AkHome = 36,
		AkInsert = 45,
		AkLeft = 37,
		AkPgDn = 34,
		AkPgUp = 33,
		AkRight = 39,
		AkShift = 16,
		AkSpace = 32,
		AkTab = 9,
		AkUp = 38
	};

	static Ref< AsKey > createInstance();

	AsKey();

	virtual ~AsKey();

	void eventKeyDown(IActionVM* actionVM, ActionContext* context, int keyCode);

	void eventKeyUp(IActionVM* actionVM, ActionContext* context, int keyCode);

private:
	RefArray< ActionObject > m_listeners;
	bool m_keyState[256];
	int m_lastKeyCode;

	void createPrototype();

	virtual ActionValue construct(ActionContext* context, const args_t& args);

	void Key_addListener(CallArgs& ca);

	void Key_getAscii(CallArgs& ca);

	void Key_getCode(CallArgs& ca);

	void Key_isAccessible(CallArgs& ca);

	void Key_isDown(CallArgs& ca);

	void Key_isToggled(CallArgs& ca);

	void Key_removeListener(CallArgs& ca);
};

	}
}

#endif	// traktor_flash_AsKey_H
