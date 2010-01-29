#include <algorithm>
#include <locale>
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/Std/Extensions.h"
#include "Render/OpenGL/Std/RenderSystemOpenGL.h"
#include "Render/OpenGL/Std/RenderViewOpenGL.h"
#include "Render/OpenGL/Std/ProgramCompilerOpenGL.h"
#include "Render/OpenGL/Std/ProgramOpenGL.h"
#include "Render/OpenGL/Std/VertexBufferVAR.h"
#include "Render/OpenGL/Std/VertexBufferVBO.h"
#include "Render/OpenGL/Std/IndexBufferIAR.h"
#include "Render/OpenGL/Std/IndexBufferIBO.h"
#include "Render/OpenGL/Std/SimpleTextureOpenGL.h"
#include "Render/OpenGL/Std/CubeTextureOpenGL.h"
#include "Render/OpenGL/Std/VolumeTextureOpenGL.h"
#include "Render/OpenGL/Std/RenderTargetSetOpenGL.h"
#include "Render/DisplayMode.h"
#include "Render/VertexElement.h"
#include "Core/Serialization/ISerializable.h"
#include "Core/Log/Log.h"

#if defined(__APPLE__)
#	include "Render/OpenGL/Std/OsX/CGLWrapper.h"
#endif

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.RenderSystemOpenGL", 0, RenderSystemOpenGL, IRenderSystem)

RenderSystemOpenGL::RenderSystemOpenGL()
#if defined(_WIN32)
:	m_hWndShared(NULL)
,	m_hWnd(NULL)
#endif
{
}

bool RenderSystemOpenGL::create(const RenderSystemCreateDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		0,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	WNDCLASS wc;

	// Create shared context class.
	wc.style         = 0;
	wc.lpfnWndProc   = DefWindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("RenderSystemOpenGL_GlobalContext");

	RegisterClass(&wc);

	// Create full screen window class.
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)wndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(this);
	wc.hInstance     = (HINSTANCE)GetModuleHandle(NULL);
	wc.hIcon         = NULL;
	wc.hCursor       = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = _T("RenderSystemOpenGL_FullScreen");

	RegisterClass(&wc);

	// Create window for shared context.
	m_hWndShared = CreateWindow(
		_T("RenderSystemOpenGL_GlobalContext"),
		_T(""),
		0,
		-1,
		-1,
		0,
		0,
		NULL,
		NULL,
		(HINSTANCE)GetModuleHandle(NULL),
		NULL
	);

	HDC hSharedDC = GetDC(m_hWndShared);
	if (!hSharedDC)
		return false;

	int pixelFormat = ChoosePixelFormat(hSharedDC, &pfd);
	if (!pixelFormat)
		return false;

	if (!SetPixelFormat(hSharedDC, pixelFormat, &pfd))
		return false;

	HGLRC hSharedRC = wglCreateContext(hSharedDC);
	T_ASSERT (hSharedRC);

	m_globalContext = new ContextOpenGL(m_hWndShared, hSharedDC, hSharedRC);
	m_globalContext->enter();

	if (!opengl_initialize_extensions())
		return false;

	m_globalContext->leave();

#elif defined(__APPLE__)

	void* globalContext = cglwCreateContext(0, 0, 0, 0, 0);
	m_globalContext = new ContextOpenGL(globalContext);

#else	// LINUX

	/*
	log::info << L"Open X display..." << Endl;

	Display* display = XOpenDisplay(0);
	T_ASSERT (display);

	if (!display)
		log::error << L"Unable to open X display!" << Endl;

	log::info << L"Choose X visual..." << Endl;
	
	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, None };
	XVisualInfo* visual = glXChooseVisual(display, DefaultScreen(display), attribs);
	T_ASSERT (visual);

	if (!visual)
		log::error << L"Unable to choose X visual!" << Endl;

	log::info << L"Create GLX context..." << Endl;

	GLXContext globalContext = glXCreateContext(display, visual, NULL, GL_TRUE);
	T_ASSERT (globalContext);

	if (!globalContext)
		log::error << L"Unable to create GLX context!" << Endl;

	log::info << L"Create global context..." << Endl;

	// @hack Need to have some GLXVisual ready, need to create a dummy window.
	m_globalContext = new ContextOpenGL(display, 0, globalContext);
	m_globalContext->enter();

	if (!opengl_initialize_extensions())
		T_BREAKPOINT;
	*/

	m_globalContext->leave();

#endif

	return true;
}

void RenderSystemOpenGL::destroy()
{
	if (m_globalContext)
	{
		m_globalContext->destroy();
		m_globalContext = 0;
	}
}

int RenderSystemOpenGL::getDisplayModeCount() const
{
#if defined(_WIN32)

	int count = 0;

	DEVMODE dm;
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);

	while (EnumDisplaySettings(NULL, count, &dm))
		++count;

	return count;

#else
	return 0;
#endif
}

Ref< DisplayMode > RenderSystemOpenGL::getDisplayMode(int index)
{
#if defined(_WIN32)

	DEVMODE dm;
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettings(NULL, index, &dm))
		return 0;

	return new DisplayMode(
		index,
		dm.dmPelsWidth,
		dm.dmPelsHeight,
		dm.dmBitsPerPel
	);

#else
	return 0;
#endif
}

Ref< DisplayMode > RenderSystemOpenGL::getCurrentDisplayMode()
{
#if defined(_WIN32)

	DEVMODE dm;
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
		return 0;

	return new DisplayMode(
		ENUM_CURRENT_SETTINGS,
		dm.dmPelsWidth,
		dm.dmPelsHeight,
		dm.dmBitsPerPel
	);

#else
	return 0;
#endif
}

bool RenderSystemOpenGL::handleMessages()
{
#if defined(_WIN32)

	bool going = true;
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret <= 0 || msg.message == WM_QUIT)
			going = false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return going;

#else
	return true;
#endif
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
#if defined(_WIN32)

	m_hWnd = CreateWindow(
		_T("RenderSystemOpenGL_FullScreen"),
		_T("Traktor 2.0 OpenGL Renderer"),
		WS_POPUPWINDOW,
		0,
		0,
		0,
		0,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return 0;

	SetWindowPos(m_hWnd, HWND_TOPMOST, 0, 0, displayMode->getWidth(), displayMode->getHeight(), SWP_SHOWWINDOW);
	ShowWindow(m_hWnd, SW_MAXIMIZE);
	UpdateWindow(m_hWnd);

	DEVMODE dm;
	std::memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);

	if (!EnumDisplaySettings(NULL, displayMode->getIndex(), &dm))
		return 0;

	if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		return 0;

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC(m_hWnd);
	if (!hDC)
		return 0;

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
		return 0;

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return 0;

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL(m_hWnd, hDC, hRC);
	m_globalContext->share(context);

	return new RenderViewOpenGL(context, m_globalContext, m_hWnd);

#else
	return 0;
#endif
}

Ref< IRenderView > RenderSystemOpenGL::createRenderView(void* windowHandle, const RenderViewCreateDesc& desc)
{
#if defined(_WIN32)

	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		(BYTE)desc.depthBits,
		(BYTE)desc.stencilBits,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC((HWND)windowHandle);
	if (!hDC)
		return 0;

	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (!pixelFormat)
		return 0;

	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
		return 0;

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC)
		return 0;

	Ref< ContextOpenGL > context = new ContextOpenGL((HWND)windowHandle, hDC, hRC);
	m_globalContext->share(context);

	context->enter();

	if (!opengl_initialize_extensions())
		return 0;

	context->leave();

	return new RenderViewOpenGL(context, m_globalContext, (HWND)windowHandle);

#elif defined(__APPLE__)

	void* glcontext = cglwCreateContext(
		windowHandle,
		m_globalContext->getGLContext(),
		desc.depthBits,
		desc.stencilBits,
		desc.multiSample
	);
	if (!glcontext)
		return 0;
		
	Ref< ContextOpenGL > context = new ContextOpenGL(glcontext);

	return new RenderViewOpenGL(context, m_globalContext);

#else	// LINUX
	
	struct Handle { Display* display; Window window; }* handle = reinterpret_cast< Handle* >(windowHandle);

	int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, None };
	XVisualInfo* visual = glXChooseVisual(handle->display, DefaultScreen(handle->display), attribs);
	if (!visual)
	{
		log::error << L"Unable to create renderview, glXChooseVisual failed" << Endl;
		return 0;
	}

	// @hack Need to have global context ready.
	GLXContext ctx = glXCreateContext(handle->display, visual, /*m_globalContext->getGLXContext()*/NULL, GL_TRUE);
	if (!ctx)
	{
		log::error << L"Unable to create renderview, glXCreateContext failed" << Endl;
		return 0;
	}

	log::info << L"GLX context created successfully" << Endl;

	Ref< ContextOpenGL > context = new ContextOpenGL(handle->display, handle->window, ctx);
	context->setCurrent();

	log::info << L"Loading OpenGL extensions..." << Endl;

	if (!opengl_initialize_extensions())
	{
		log::error << L"Unable to create renderview, failed to initialize required OpenGL extensions" << Endl;
		return 0;
	}

	log::info << L"OpenGL extensions loaded successfully" << Endl;
	
	return new RenderViewOpenGL(context, m_globalContext);

#endif
}

Ref< VertexBuffer > RenderSystemOpenGL::createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic)
{
	T_CONTEXT_SCOPE(m_globalContext)

#if defined(_WIN32)
	if (glGenBuffersARB)
		return new VertexBufferVBO(m_globalContext, vertexElements, bufferSize, dynamic);
	else
		return new VertexBufferVAR(m_globalContext, vertexElements, bufferSize, dynamic);
#elif defined(__APPLE__)
	return new VertexBufferVBO(m_globalContext, vertexElements, bufferSize, dynamic);
#else
	return new VertexBufferVAR(m_globalContext, vertexElements, bufferSize, dynamic);
#endif
}

Ref< IndexBuffer > RenderSystemOpenGL::createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic)
{
	T_CONTEXT_SCOPE(m_globalContext)

#if defined(_WIN32)
	if (glGenBuffersARB)
		return new IndexBufferIBO(m_globalContext, indexType, bufferSize, dynamic);
	else
		return new IndexBufferIAR(m_globalContext, indexType, bufferSize);
#elif defined(__APPLE__)
	return new IndexBufferIBO(m_globalContext, indexType, bufferSize, dynamic);
#else
	return new IndexBufferIAR(m_globalContext, indexType, bufferSize);
#endif
}

Ref< ISimpleTexture > RenderSystemOpenGL::createSimpleTexture(const SimpleTextureCreateDesc& desc)
{
	T_CONTEXT_SCOPE(m_globalContext)

	Ref< SimpleTextureOpenGL > texture = new SimpleTextureOpenGL(m_globalContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< ICubeTexture > RenderSystemOpenGL::createCubeTexture(const CubeTextureCreateDesc& desc)
{
	T_CONTEXT_SCOPE(m_globalContext)

	Ref< CubeTextureOpenGL > texture = new CubeTextureOpenGL(m_globalContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< IVolumeTexture > RenderSystemOpenGL::createVolumeTexture(const VolumeTextureCreateDesc& desc)
{
	T_CONTEXT_SCOPE(m_globalContext)

	Ref< VolumeTextureOpenGL > texture = new VolumeTextureOpenGL(m_globalContext);
	if (!texture->create(desc))
		return 0;

	return texture;
}

Ref< RenderTargetSet > RenderSystemOpenGL::createRenderTargetSet(const RenderTargetSetCreateDesc& desc)
{
	T_CONTEXT_SCOPE(m_globalContext)

	Ref< RenderTargetSetOpenGL > renderTargetSet = new RenderTargetSetOpenGL(m_globalContext);
	if (!renderTargetSet->create(desc))
		return 0;

	return renderTargetSet;
}

Ref< IProgram > RenderSystemOpenGL::createProgram(const ProgramResource* programResource)
{
	T_CONTEXT_SCOPE(m_globalContext)

	Ref< ProgramOpenGL > shader = new ProgramOpenGL(m_globalContext);
	if (!shader->create(programResource))
		return 0;

	return shader;
}

Ref< IProgramCompiler > RenderSystemOpenGL::createProgramCompiler() const
{
	return new ProgramCompilerOpenGL();
}

#if defined(_WIN32)

LRESULT RenderSystemOpenGL::wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RenderSystemOpenGL* renderSystem = reinterpret_cast< RenderSystemOpenGL* >(GetWindowLongPtr(hWnd, 0));
	LPCREATESTRUCT createStruct;
	LRESULT result = TRUE;

	switch (uMsg)
	{
	case WM_CREATE:
		createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		renderSystem = reinterpret_cast< RenderSystemOpenGL* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(renderSystem));
		break;

	case WM_KEYDOWN:
		if (wParam != VK_ESCAPE)
			break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ERASEBKGND:
		break;

	case WM_SETCURSOR:
		SetCursor(NULL);
		break;

	default:
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);
		break;
	}

	return result;
}

#endif

	}
}
