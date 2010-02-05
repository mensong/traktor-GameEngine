#include "Core/Io/FileSystem.h"
#include "Core/Misc/String.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Database/Local/Perforce/PerforceChangeList.h"
#include "Database/Local/Perforce/PerforceClient.h"
#include "Database/Local/Perforce/PerforceFileStore.h"

namespace traktor
{
	namespace db
	{
		namespace
		{

const wchar_t* c_changeListDescription = L"** CREATED **";

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.PerforceFileStore", 0, PerforceFileStore, IFileStore)

bool PerforceFileStore::create(const ConnectionString& connectionString)
{
	PerforceClientDesc desc;

	desc.m_host = OS::getInstance().getComputerName();
	desc.m_port = connectionString.get(L"p4port");
	desc.m_user = connectionString.get(L"p4user");
	desc.m_password= connectionString.get(L"p4pwd");
	desc.m_client = connectionString.get(L"p4client");
	desc.m_securityLevel = PerforceClientDesc::SlLow;

	m_p4client = new PerforceClient(desc);

	RefArray< PerforceChangeList > changeLists;
	if (m_p4client->getChangeLists(changeLists))
	{
		for (RefArray< PerforceChangeList >::iterator i = changeLists.begin(); i != changeLists.end(); ++i)
		{
			if (compareIgnoreCase((*i)->getDescription(), c_changeListDescription) == 0)
			{
				m_p4changeList = *i;
				break;
			}
		}
	}

	if (!m_p4changeList)
	{
		m_p4changeList = m_p4client->createChangeList(c_changeListDescription);
		if (!m_p4changeList)
			return false;
	}

	return true;
}

void PerforceFileStore::destroy()
{
	m_p4changeList = 0;
	m_p4client = 0;
}

bool PerforceFileStore::add(const Path& filePath)
{
	return m_p4client->addFile(m_p4changeList, filePath.getPathName());
}

bool PerforceFileStore::remove(const Path& filePath)
{
	std::wstring localFile = filePath.getPathName();

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == AtAdd)
			return m_p4client->revertFile(m_p4changeList, localFile);
		else if (pa == AtEdit)
		{
			if (!m_p4client->revertFile(m_p4changeList, localFile))
				return false;
			// Fall through to open-for-delete.
		}
		else if (pa == AtDelete)
			return true;

		return m_p4client->openForDelete(m_p4changeList, localFile);
	}
	else
	{
		// File doesn't exist in P4; delete local file.
		return FileSystem::getInstance().remove(localFile);
	}
}

bool PerforceFileStore::edit(const Path& filePath)
{
	std::wstring localFile = filePath.getPathName();

	PerforceAction pa;
	if (m_p4client->isOpened(localFile, pa))
	{
		if (pa == AtAdd || pa == AtEdit)
			return true;

		if (m_p4client->openForEdit(m_p4changeList, filePath.getPathName()))
			return true;
	}
	else
	{
		// File doesn't exist in P4; add local file.
		if (m_p4client->addFile(m_p4changeList, filePath.getPathName()))
			return true;
	}

	return false;
}

bool PerforceFileStore::rollback(const Path& filePath)
{
	return m_p4client->revertFile(m_p4changeList, filePath.getPathName());
}

bool PerforceFileStore::clean(const Path& filePath)
{
	return true;
}

	}
}
