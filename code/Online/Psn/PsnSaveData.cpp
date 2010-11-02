#include <sysutil/sysutil_gamecontent.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/TString.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/PsnLogError.h"
#include "Online/Psn/PsnSaveData.h"

namespace traktor
{
	namespace online
	{

		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;
const char c_secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] = {
	'C', 'L', 'E', 'A', 
	'R', 'H', 'E', 'A', 
	'D', 'G', 'A', 'M', 
	'E', 'S', ' ', ' ', 
};

char s_dirNamePrefix[] = "NPEB00401-";
char s_loadIndicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Loading...";
char s_saveIndicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Saving...";

CellSaveDataAutoIndicator s_loadIndicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_loadIndicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

CellSaveDataAutoIndicator s_saveIndicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_saveIndicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

struct EnumerateData
{
	EnumerateData(std::set< std::wstring >& outSaveDataIds, PsnSaveData *this_)
		:	m_saveDataIds(outSaveDataIds)
		,	m_this(this_)
	{
		m_saveDataIds.clear();
	}

	std::set< std::wstring >& m_saveDataIds;
	PsnSaveData *m_this;
};

struct LoadData
{
	LoadData(PsnSaveData *this_)
		:	m_bufferPending(false)
		,	m_this(this_)
	{
	}

	std::vector< uint8_t > m_buffer;
	bool m_bufferPending;
	PsnSaveData *m_this;
};

struct SaveData
{
	SaveData(PsnSaveData *this_, const std::vector< uint8_t >& buffer)
		:	m_buffer(buffer)	
		,	m_bufferPending(false)
		,	m_this(this_)
	{
	}

	const std::vector< uint8_t >& m_buffer;
	bool m_bufferPending;
	PsnSaveData *m_this;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.PsnSaveData", PsnSaveData, ISaveDataProvider)

PsnSaveData::PsnSaveData(int32_t excessSpaceNeededKB)
:	m_excessSpaceNeededKB(excessSpaceNeededKB)
,	m_hddFreeSpaceKB(0)
,	m_spaceNeededKB(0)
{
}

bool PsnSaveData::enumerate(std::set< std::wstring >& outSaveDataIds)
{
	EnumerateData enumerateData(outSaveDataIds, this);

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
		);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	CellSaveDataSetList list;
	std::memset(&list, 0, sizeof(list));
	list.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
	list.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
	list.dirNamePrefix = s_dirNamePrefix;

	m_currentSavedataSizeKB = 0;

	int32_t err = cellSaveDataListAutoLoad(
		CELL_SAVEDATA_VERSION_CURRENT,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&list,
		&buf,
		&callbackEnumFixed,
		&callbackEnumStat,
		&callbackEnumFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*) &enumerateData
		);

	int32_t neededGameDataSizeKB = m_currentSavedataSizeKB > 0? 4 : 100;

	const int32_t NEED_SIZEKB = this->m_hddFreeSpaceKB - this->m_excessSpaceNeededKB - neededGameDataSizeKB;
	if (NEED_SIZEKB < 0)
	{
		cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, -NEED_SIZEKB, NULL); 
		log::error << L"Unable to create session manager. Not enough space on HDD to save trophies and savegames" << Endl;
		return false;
	}

	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		// EXIT APP
		return false;
	}

	return true;
}

void PsnSaveData::callbackEnumFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set)
{
	EnumerateData* enumerateData =	static_cast< EnumerateData* >(cbResult->userdata);
	T_ASSERT (enumerateData);
	PsnSaveData* this_ = enumerateData->m_this;
	T_ASSERT (this_);

	int32_t dirCount = get->dirListNum;
	int32_t prefixLength = strlen(s_dirNamePrefix);
	enumerateData->m_saveDataIds.clear();
	for (int i = 0; i< dirCount; i++)
	{
		const char* saveDataId = &(get->dirList[i].dirName[prefixLength]);
		enumerateData->m_saveDataIds.insert(mbstows(saveDataId));
	}

	set->dirName = s_dirNamePrefix; // Dummy to force next step
	set->newIcon = 0;
	set->option = CELL_SAVEDATA_OPTION_NONE;
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackEnumStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	EnumerateData* enumerateData =	static_cast< EnumerateData* >(cbResult->userdata);
	T_ASSERT (enumerateData);
	PsnSaveData* this_ = enumerateData->m_this;
	T_ASSERT (this_);

	this_->m_hddFreeSpaceKB = get->hddFreeSizeKB;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

void PsnSaveData::callbackEnumFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
}

bool PsnSaveData::get(const std::wstring& saveDataId, Ref< ISerializable >& outAttachment)
{
	LoadData loadData(this); 

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
		);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	m_currentSavedataSizeKB = 0;

	char dirName[CELL_SAVEDATA_DIRNAME_SIZE];
	strcpy(dirName, s_dirNamePrefix);
	strcat(dirName, wstombs(toUpper(saveDataId)).c_str());

	int32_t err = cellSaveDataAutoLoad2(
		CELL_SAVEDATA_VERSION_CURRENT,
		dirName,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&buf,
		&callbackLoadStat,
		&callbackLoadFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*) &loadData
		);

	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		// EXIT APP
		return false;
	}

	DynamicMemoryStream dms(loadData.m_buffer, true, false);
	outAttachment = BinarySerializer(&dms).readObject();
	if (outAttachment)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void PsnSaveData::callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	LoadData* loadData = static_cast< LoadData* >(cbResult->userdata);
	T_ASSERT (loadData);
	PsnSaveData* this_ = loadData->m_this;
	T_ASSERT (this_);

	this_->m_hddFreeSpaceKB = get->hddFreeSizeKB;

	if (get->bind != 0)
	{
		log::debug << PsnLogError::getSaveDataBindErrorString(get->bind) << Endl;
	}

	if (get->isNewData)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		return;
	}

	if (get->fileNum > get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	int32_t i = 0;
	for (; i < get->fileListNum; ++i)
	{
		if (strcmp(get->fileList[i].fileName, "ATT.BIN") == 0)
		{
			loadData->m_buffer.resize(get->fileList[i].st_size);
			loadData->m_bufferPending = true;
			break;
		}
	}
	if (i >= get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;
	set->setParam = 0;
	set->indicator = &s_loadIndicator;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	LoadData* loadData = static_cast< LoadData* >(cbResult->userdata);
	T_ASSERT (loadData);
	PsnSaveData* this_ = loadData->m_this;
	T_ASSERT (this_);

	cbResult->progressBarInc = 50;

	if (loadData->m_bufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = loadData->m_buffer.size();
		set->fileBufSize = loadData->m_buffer.size();
		set->fileBuf = &loadData->m_buffer[0];
		set->reserved = 0;
		memcpy(set->secureFileId, c_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);

		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		loadData->m_bufferPending = false;
	}
	else
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
}

bool PsnSaveData::set(const std::wstring& saveDataId, const ISerializable* attachment, bool replace)
{
	DynamicMemoryStream dms(false, true);
	BinarySerializer(&dms).writeObject(attachment);
	SaveData saveData(this, dms.getBuffer()); 

	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
		);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);


	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	char dirName[CELL_SAVEDATA_DIRNAME_SIZE];
	strcpy(dirName, s_dirNamePrefix);
	strcat(dirName, wstombs(toUpper(saveDataId)).c_str());

	int32_t err = cellSaveDataAutoSave2(
		CELL_SAVEDATA_VERSION_CURRENT,
		dirName,
		CELL_SAVEDATA_ERRDIALOG_ALWAYS,
		&buf,
		&callbackSaveStat,
		&callbackSaveFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*) &saveData
		);
	if (err == CELL_SAVEDATA_ERROR_CBRESULT && m_spaceNeededKB)
	{
		cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, m_spaceNeededKB, NULL); 
		log::error << L"Not enough space to save" << Endl;
	}
	if (err != CELL_SAVEDATA_RET_OK)
	{
		log::error << PsnLogError::getSaveDataErrorString(err) << Endl;
		return false;
	}
	return true;
}

void PsnSaveData::callbackSaveStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	SaveData* saveData = static_cast< SaveData* >(cbResult->userdata);
	T_ASSERT (saveData );
	PsnSaveData* this_ = saveData ->m_this;
	T_ASSERT (this_);

	set->setParam = &get->getParam;
	set->reCreateMode = CELL_SAVEDATA_RECREATE_YES;
	set->indicator = &s_saveIndicator;

	if (get->isNewData)
	{
		strncpy(set->setParam->title, "Puzzle Dimension", CELL_SAVEDATA_SYSP_TITLE_SIZE);
		strncpy(set->setParam->subTitle, "", CELL_SAVEDATA_SYSP_SUBTITLE_SIZE);
		strncpy(set->setParam->detail, "", CELL_SAVEDATA_SYSP_DETAIL_SIZE);
		strncpy(set->setParam->listParam, "", CELL_SAVEDATA_SYSP_LPARAM_SIZE);

		set->setParam->attribute = CELL_SAVEDATA_ATTR_NORMAL;

		std::memset(set->setParam->reserved, 0, sizeof(set->setParam->reserved));
		std::memset(set->setParam->reserved2, 0, sizeof(set->setParam->reserved2));	

		const int32_t CONTENT_SIZEKB = (saveData->m_buffer.size() + 1023) / 1024; // TODO Add size of "content information files": ICON0.PNG, ICON1.PAM, PIC1.PNG, SND0.AT3
		const int32_t NEW_SIZEKB = CONTENT_SIZEKB + get->sysSizeKB;
		const int32_t NEED_SIZEKB = get->hddFreeSizeKB - NEW_SIZEKB;
		if (NEED_SIZEKB < 0)
		{
			this_->m_spaceNeededKB = -NEED_SIZEKB;
			cbResult->errNeedSizeKB = -NEED_SIZEKB;
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_NOSPACE;
			return;
		}
	}
	saveData->m_bufferPending = true;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void PsnSaveData::callbackSaveFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	SaveData* saveData = static_cast< SaveData* >(cbResult->userdata);
	T_ASSERT (saveData );
	PsnSaveData* this_ = saveData ->m_this;
	T_ASSERT (this_);

	if (saveData->m_bufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_WRITE;
		set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = saveData->m_buffer.size();
		set->fileBufSize = saveData->m_buffer.size();
		set->fileBuf = (void*) &saveData->m_buffer[0];
		memcpy(set->secureFileId, c_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);
		set->reserved = 0;

		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		saveData->m_bufferPending = false;
	}
	else
	{
		cbResult->progressBarInc = 50;
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
	}
}


	}
}
