#ifndef traktor_render_ContextOpenGLES2_H
#define traktor_render_ContextOpenGLES2_H

#include <map>
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/ThreadLocal.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

#if !defined(T_OFFLINE_ONLY)

class EAGLContextWrapper;
class PPContextWrapper;
class Window;

/*! \brief OpenGL ES2 context.
 * \ingroup OGL
 */
class ContextOpenGLES2 : public Object
{
	T_RTTI_CLASS;

public:
	/*! \brief Scoped enter/leave helper.
	 * \ingroup OGL
	 */
	struct Scope
	{
		ContextOpenGLES2* m_context;

		Scope(ContextOpenGLES2* context)
		:	m_context(context)
		{
			bool result = m_context->enter();
			T_FATAL_ASSERT_M (result, L"Unable to set OpenGL ES2 context!");
		}

		~Scope()
		{
			m_context->leave();
		}
	};

	/*! \brief Delete callback. 
	 * \ingroup OGL
	 *
	 * These are enqueued in the context
	 * and are invoked as soon as it's
	 * safe to actually delete resources.
	 */
	struct IDeleteCallback
	{
		virtual void deleteResource() = 0;
	};

	static Ref< ContextOpenGLES2 > createContext(void* nativeHandle, const RenderViewDefaultDesc& desc);

	static Ref< ContextOpenGLES2 > createContext(void* nativeHandle, const RenderViewEmbeddedDesc& desc);

	bool enter();

	void leave();

	void deleteResource(IDeleteCallback* callback);

	void deleteResources();

	GLuint createShaderObject(const char* shader, GLenum shaderType);

	bool resize(int32_t width, int32_t height);

	int32_t getWidth() const;

	int32_t getHeight() const;

	bool getLandscape() const;

	void swapBuffers();

	Semaphore& lock();
	
	void bindPrimary();

	GLuint getPrimaryDepth() const;

#if defined(_WIN32)
	Window* getWindow() const { return m_window; }
#endif

private:
	static ThreadLocal ms_contextStack;
#if defined(TARGET_OS_IPHONE)
	EAGLContextWrapper* m_context;
#elif defined(__PNACL__)
	Ref< PPContextWrapper > m_context;
#elif defined(T_OPENGL_ES2_HAVE_EGL)
#	if defined(_WIN32)
	Ref< Window > m_window;
	EGLint m_width;
	EGLint m_height;
#	endif
	EGLDisplay m_display;
	EGLConfig m_config;
	EGLSurface m_surface;
	EGLContext m_context;
	GLuint m_primaryDepth;
#endif
	Semaphore m_lock;
	std::vector< IDeleteCallback* > m_deleteResources;
	std::map< uint32_t, GLuint > m_shaderObjects;

	ContextOpenGLES2();
};

#endif

	}
}

#endif	// traktor_render_ContextOpenGLES2_H
