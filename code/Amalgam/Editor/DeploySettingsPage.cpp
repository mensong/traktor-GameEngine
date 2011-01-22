#include "Amalgam/Editor/DeploySettingsPage.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/Settings.h"
#include "Ui/Container.h"
#include "Ui/NumericEditValidator.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.DeploySettingsPage", 0, DeploySettingsPage, editor::ISettingsPage)

bool DeploySettingsPage::create(ui::Container* parent, Settings* settings, const std::list< ui::Command >& shortcutCommands)
{
	Ref< ui::Container > container = new ui::Container();
	if (!container->create(parent, ui::WsNone, new ui::TableLayout(L"100%", L"*", 0, 4)))
		return false;

	Ref< ui::Container > containerInner = new ui::Container();
	if (!containerInner->create(container, ui::WsNone, new ui::TableLayout(L"*,100%", L"100%", 0, 4)))
		return false;

	Ref< ui::Static > staticRemoteDatabasePort = new ui::Static();
	staticRemoteDatabasePort->create(containerInner, L"Remote database port");

	m_editRemoteDatabasePort = new ui::Edit();
	m_editRemoteDatabasePort->create(containerInner, toString(c_remoteDatabasePort), ui::WsClientBorder, new ui::NumericEditValidator(false, 0, 65535, 0));

	Ref< ui::Static > staticTargetManagerPort = new ui::Static();
	staticTargetManagerPort->create(containerInner, L"Target manager port");

	m_editTargetManagerPort = new ui::Edit();
	m_editTargetManagerPort->create(containerInner, toString(c_targetConnectionPort), ui::WsClientBorder, new ui::NumericEditValidator(false, 0, 65535, 0));

	m_checkPublishActiveGuid = new ui::CheckBox();
	m_checkPublishActiveGuid->create(container, L"Publish active editor");

	int32_t remoteDatabasePort = settings->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort);
	m_editRemoteDatabasePort->setText(toString(remoteDatabasePort));

	int32_t targetManagerPort = settings->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort);
	m_editTargetManagerPort->setText(toString(targetManagerPort));

	bool publish = settings->getProperty< PropertyBoolean >(L"Amalgam.PublishActiveGuid", true);
	m_checkPublishActiveGuid->setChecked(publish);

	parent->setText(L"Deploy");
	return true;
}

void DeploySettingsPage::destroy()
{
}

bool DeploySettingsPage::apply(Settings* settings)
{
	int32_t remoteDatabasePort = parseString< int32_t >(m_editRemoteDatabasePort->getText());
	settings->setProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", remoteDatabasePort);

	int32_t targetManagerPort = parseString< int32_t >(m_editTargetManagerPort->getText());
	settings->setProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", targetManagerPort);

	bool publish = m_checkPublishActiveGuid->isChecked();
	settings->setProperty< PropertyBoolean >(L"Amalgam.PublishActiveGuid", publish);

	return true;
}

	}
}
