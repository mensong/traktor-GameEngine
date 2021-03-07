#include <cstring>
#include "Core/Log/Log.h"
#include "Render/VertexElement.h"
#include "Render/OpenGL/ES/Platform.h"
#include "Render/OpenGL/ES/StateCache.h"
#include "Render/OpenGL/ES/VertexBufferDynamicOpenGLES.h"
#if defined(__ANDROID__)
#	include "Render/OpenGL/ES/Android/ContextOpenGLES.h"
#elif defined(__IOS__)
#	include "Render/OpenGL/ES/iOS/ContextOpenGLES.h"
#elif defined(__EMSCRIPTEN__)
#	include "Render/OpenGL/ES/Emscripten/ContextOpenGLES.h"
#elif defined(_WIN32)
#	include "Render/OpenGL/ES/Win32/ContextOpenGLES.h"
#elif defined(__LINUX__) || defined(__RPI__)
#	include "Render/OpenGL/ES/Linux/ContextOpenGLES.h"
#endif
#include "Render/OpenGL/ES/Editor/Glsl/GlslType.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

struct DeleteBufferCallback : public ContextOpenGLES::IDeleteCallback
{
	GLuint m_bufferName;

	DeleteBufferCallback(GLuint bufferName)
	:	m_bufferName(bufferName)
	{
	}

	virtual ~DeleteBufferCallback()
	{
	}

	virtual void deleteResource()
	{
		T_OGL_SAFE(glDeleteBuffers(1, &m_bufferName));
		delete this;
	}
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferDynamicOpenGLES", VertexBufferDynamicOpenGLES, VertexBufferOpenGLES)

VertexBufferDynamicOpenGLES::VertexBufferDynamicOpenGLES(ContextOpenGLES* context, const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBufferOpenGLES(bufferSize)
,	m_context(context)
,	m_arrayObject(0)
,	m_bufferObject(0)
,	m_vertexStride(0)
,	m_lockOffset(0)
,	m_lockSize(0)
,	m_dirty(false)
{
	m_vertexStride = getVertexSize(vertexElements);
	T_ASSERT(m_vertexStride > 0);

	for (size_t i = 0; i < vertexElements.size(); ++i)
	{
		if (vertexElements[i].getIndex() >= 4)
		{
			log::warning << L"Index out of bounds on vertex element " << uint32_t(i) << Endl;
			continue;
		}

		AttributeDesc desc;
		desc.location = VertexAttribute::getLocation(vertexElements[i].getDataUsage(), vertexElements[i].getIndex());

		switch (vertexElements[i].getDataType())
		{
		case DtFloat1:
			desc.size = 1;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat2:
			desc.size = 2;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat3:
			desc.size = 3;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtFloat4:
			desc.size = 4;
			desc.type = GL_FLOAT;
			desc.normalized = GL_FALSE;
			break;

		case DtByte4:
			desc.size = 4;
			desc.type = GL_UNSIGNED_BYTE;
			desc.normalized = GL_FALSE;
			break;

		case DtByte4N:
			desc.size = 4;
			desc.type = GL_UNSIGNED_BYTE;
			desc.normalized = GL_TRUE;
			break;

		case DtShort2:
			desc.size = 2;
			desc.type = GL_SHORT;
			desc.normalized = GL_FALSE;
			break;

		case DtShort4:
			desc.size = 4;
			desc.type = GL_SHORT;
			desc.normalized = GL_FALSE;
			break;

		case DtShort2N:
			desc.size = 2;
			desc.type = GL_SHORT;
			desc.normalized = GL_TRUE;
			break;

		case DtShort4N:
			desc.size = 4;
			desc.type = GL_SHORT;
			desc.normalized = GL_TRUE;
			break;

		case DtHalf2:
			desc.size = 2;
			desc.type = GL_HALF_FLOAT;
			desc.normalized = GL_TRUE;
			break;

		case DtHalf4:
			desc.size = 4;
			desc.type = GL_HALF_FLOAT;
			desc.normalized = GL_TRUE;
			break;

		default:
			log::warning << L"Unsupport vertex format" << Endl;
		}

		desc.offset = vertexElements[i].getOffset();

		m_attributes.push_back(desc);
	}
}

VertexBufferDynamicOpenGLES::~VertexBufferDynamicOpenGLES()
{
	destroy();
}

void VertexBufferDynamicOpenGLES::destroy()
{
	if (m_bufferObject)
	{
		if (m_context)
			m_context->deleteResource(new DeleteBufferCallback(m_bufferObject));
		m_bufferObject = 0;
	}
}

void* VertexBufferDynamicOpenGLES::lock()
{
	m_lockOffset = 0;
	m_lockSize = getBufferSize();

	if (!m_buffer.ptr())
	{
		m_buffer.reset((uint8_t*)Alloc::acquireAlign(getBufferSize(), 16, "VB"));
		std::memset(m_buffer.ptr(), 0, getBufferSize());
	}

	return m_buffer.ptr();
}
void VertexBufferDynamicOpenGLES::unlock()
{
	m_dirty = true;
}

void VertexBufferDynamicOpenGLES::activate(StateCache* stateCache)
{
	if (!m_bufferObject)
	{
		T_OGL_SAFE(glGenBuffers(1, &m_bufferObject));
		T_FATAL_ASSERT (m_bufferObject != 0);
	}

	stateCache->setArrayBuffer(m_bufferObject);

	if (m_dirty)
	{
		int32_t bufferSize = getBufferSize();

		if (m_lockOffset <= 0 && m_lockSize >= bufferSize)
		{
			T_OGL_SAFE(glBufferData(
				GL_ARRAY_BUFFER,
				bufferSize,
				m_buffer.ptr(),
				GL_DYNAMIC_DRAW
			));
		}
		else
		{
			T_OGL_SAFE(glBufferSubData(
				GL_ARRAY_BUFFER,
				m_lockOffset,
				m_lockSize,
				m_buffer.ptr()
			));
		}

		m_dirty = false;
	}

	stateCache->setVertexArrayObject(0);

	GLint maxAttributeIndex = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttributeIndex);
	for (GLint i = 0; i < maxAttributeIndex; ++i)
	{
		T_OGL_SAFE(glDisableVertexAttribArray(i));
	}

	for (AlignedVector< AttributeDesc >::const_iterator i = m_attributes.begin(); i != m_attributes.end(); ++i)
	{
		T_OGL_SAFE(glEnableVertexAttribArray(i->location));
		T_OGL_SAFE(glVertexAttribPointer(
			i->location,
			i->size,
			i->type,
			i->normalized,
			m_vertexStride,
			(const GLvoid*)i->offset
		));
	}
}

	}
}
