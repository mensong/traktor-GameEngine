#ifndef traktor_render_CGLWindow_H
#define traktor_render_CGLWindow_H

#include "Core/Config.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{
	
uint32_t cglwGetDisplayModeCount();

bool cglwGetDisplayMode(uint32_t index, DisplayMode& outDisplayMode);

bool cglwSetDisplayMode(const DisplayMode& displayMode);

bool cglwGetCurrentDisplayMode(DisplayMode& outDisplayMode);

void* cglwCreateWindow(const std::wstring& title, const DisplayMode& displayMode, bool fullscreen);

void cglwDestroyWindow(void* windowHandle);

void cglwSetFullscreen(void* windowHandle, bool fullscreen);

bool cglwIsFullscreen(void* windowHandle);

void cglwUpdateWindow(void* windowHandle);

void* cglwGetWindowView(void* windowHandle);
	
	}
}

#endif	// traktor_render_CGLWindow_H
