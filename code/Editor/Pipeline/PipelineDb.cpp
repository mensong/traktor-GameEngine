#include "Core/Io/StringOutputStream.h"
#include "Editor/Pipeline/PipelineDb.h"
#include "Editor/Pipeline/PipelineDbReport.h"
#include "Sql/IResultSet.h"
#include "Sql/Sqlite3/ConnectionSqlite3.h"

namespace traktor
{
	namespace editor
	{
		namespace
		{

const int32_t c_version = 2;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.editor.PipelineDb", PipelineDb, IPipelineDb)

bool PipelineDb::open(const std::wstring& connectionString)
{
	Ref< sql::ConnectionSqlite3 > connection = new sql::ConnectionSqlite3();
	if (!connection->connect(connectionString))
		return false;

	bool haveTable = connection->tableExists(L"PipelineHash");

	// Ensure db latest version.
	if (haveTable)
	{
		haveTable = false;

		if (connection->tableExists(L"Version"))
		{
			Ref< sql::IResultSet > rs;
			rs = connection->executeQuery(L"select * from Version");
			if (rs && rs->next())
			{
				if (rs->getInt32(0) == c_version)
					haveTable = true;
			}
		}

		// Drop all tables if incorrect version.
		if (!haveTable)
		{
			connection->executeUpdate(L"drop table PipelineHash");
			connection->executeUpdate(L"drop table PipelineFile");
			connection->executeUpdate(L"drop table Version");
		}
	}

	// Create tables if they doesn't exist.
	if (!haveTable)
	{
		if (connection->executeUpdate(
			L"create table PipelineHash ("
			L"id integer primary key,"
			L"guid char(37) unique,"
			L"pipelineVersion integer,"
			L"hash integer"
			L")"
		) < 0)
			return false;

		if (connection->executeUpdate(
			L"create table PipelineFile ("
			L"id integer primary key,"
			L"path varchar(1024),"
			L"size integer,"
			L"lastWriteTime integer,"
			L"hash integer"
			L")"
		) < 0)
			return false;

		if (connection->executeUpdate(L"create table Version (major integer)") < 0)
			return false;

		if (connection->executeUpdate(L"insert into Version (major) values (" + toString(c_version) + L")") <= 0)
			return false;
	}

	m_connection = connection;
	return true;
}

void PipelineDb::close()
{
	if (m_connection)
	{
		m_connection->disconnect();
		m_connection = 0;
	}
}

void PipelineDb::setDependency(const Guid& guid, const DependencyHash& hash)
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Remove existing records.
	ss << L"select id from PipelineHash where guid='" << guid.format() << L"'";
	rs = m_connection->executeQuery(ss.str());
	if (rs && rs->next())
	{
		int32_t hashId = rs->getInt32(0);

		ss.reset();
		ss << L"delete from PipelineHash where id='" << hashId << L"'";
		m_connection->executeUpdate(ss.str());
	}

	// Insert hash.
	ss.reset();
	ss <<
		L"insert into PipelineHash (guid, pipelineVersion, hash) "
		L"values (" <<
		L"'" << guid.format() << L"'," <<
		hash.pipelineVersion << L"," <<
		hash.hash <<
		L")";
	m_connection->executeUpdate(ss.str());
}

bool PipelineDb::getDependency(const Guid& guid, DependencyHash& outHash) const
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Get hash record.
	ss << L"select * from PipelineHash where guid='" << guid.format() << L"'";
	rs = m_connection->executeQuery(ss.str());
	if (!rs || !rs->next())
		return false;

	int32_t id = rs->getInt32(L"id");

	outHash.pipelineVersion = rs->getInt32(L"pipelineVersion");
	outHash.hash = rs->getInt32(L"hash");

	return true;
}

void PipelineDb::setFile(const Path& path, const FileHash& file)
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Delete existing records.
	ss << L"delete from PipelineFile where path='" << toLower(path.getPathName()) << L"'";
	m_connection->executeUpdate(ss.str());

	// Insert new record.
	ss.reset();
	ss <<
		L"insert into PipelineFile (path, size, lastWriteTime, hash) "
		L"values (" <<
		L"'" << toLower(path.getPathName()) << L"'," <<
		file.size << L"," <<
		file.lastWriteTime.getSecondsSinceEpoch() << L"," <<
		file.hash <<
		L")";
	m_connection->executeUpdate(ss.str());
}

bool PipelineDb::getFile(const Path& path, FileHash& outFile)
{
	Ref< sql::IResultSet > rs;
	StringOutputStream ss;

	// Get file record.
	ss << L"select * from PipelineFile where path='" << toLower(path.getPathName()) << L"'";
	rs = m_connection->executeQuery(ss.str());
	if (!rs || !rs->next())
		return false;

	outFile.size = rs->getInt64(L"size"),
	outFile.lastWriteTime = DateTime(rs->getInt64(L"lastWriteTime"));
	outFile.hash = rs->getInt32(L"hash");

	return true;
}

Ref< IPipelineReport > PipelineDb::createReport(const std::wstring& name, const Guid& guid)
{
	return new PipelineDbReport(m_connection, L"Report_" + name, guid);
}

	}
}
