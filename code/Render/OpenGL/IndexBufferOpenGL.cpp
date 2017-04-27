/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/OpenGL/IndexBufferOpenGL.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.IndexBufferOpenGL", IndexBufferOpenGL, IndexBuffer)

IndexBufferOpenGL::IndexBufferOpenGL(IndexType indexType, uint32_t bufferSize)
:	IndexBuffer(indexType, bufferSize)
{
}

	}
}
