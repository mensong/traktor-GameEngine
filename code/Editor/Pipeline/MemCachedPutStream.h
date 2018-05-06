/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_editor_MemCachedPutStream_H
#define traktor_editor_MemCachedPutStream_H

#include "Core/Ref.h"
#include "Core/Io/IStream.h"

namespace traktor
{
	namespace editor
	{

class MemCachedProto;

class MemCachedPutStream : public IStream
{
	T_RTTI_CLASS;

public:
	enum { MaxBlockSize = 65536 };

	MemCachedPutStream(MemCachedProto* proto, const std::string& key);

	virtual void close() T_OVERRIDE T_FINAL;

	virtual bool canRead() const T_OVERRIDE T_FINAL;

	virtual bool canWrite() const T_OVERRIDE T_FINAL;

	virtual bool canSeek() const T_OVERRIDE T_FINAL;

	virtual int64_t tell() const T_OVERRIDE T_FINAL;

	virtual int64_t available() const T_OVERRIDE T_FINAL;

	virtual int64_t seek(SeekOriginType origin, int64_t offset) T_OVERRIDE T_FINAL;

	virtual int64_t read(void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual int64_t write(const void* block, int64_t nbytes) T_OVERRIDE T_FINAL;

	virtual void flush() T_OVERRIDE T_FINAL;

private:
	Ref< MemCachedProto > m_proto;
	std::string m_key;
	uint8_t m_block[MaxBlockSize + 2];
	uint32_t m_inblock;
	uint32_t m_index;

	bool uploadBlock();
	
	void uploadEndBlock();
};

	}
}

#endif	// traktor_editor_MemCachedPutStream_H
