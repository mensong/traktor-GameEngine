/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/MemoryStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Singleton/SingletonManager.h"
#include "Drawing/Image.h"
#include "Ui/Application.h"
#include "Ui/Bitmap.h"
#include "Ui/Itf/ISystemBitmap.h"

namespace traktor
{
	namespace ui
	{
		namespace
		{

#pragma pack(1)
struct ImageEntry
{
	uint16_t dpi;
	uint32_t offset;
};
#pragma pack()

#pragma pack(1)
struct ImageHeader
{
	uint16_t count;
	ImageEntry entry[1];
};
#pragma pack()

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.Bitmap", Bitmap, IBitmap)

Bitmap::Bitmap()
:	m_bitmap(0)
{
}

Bitmap::Bitmap(uint32_t width, uint32_t height)
:	m_bitmap(0)
{
	create(width, height);
	T_ASSERT (m_bitmap);
}

Bitmap::Bitmap(drawing::Image* image)
:	m_bitmap(0)
{
	create(image);
	T_ASSERT (m_bitmap);
}

Bitmap::Bitmap(drawing::Image* image, const ui::Rect& srcRect)
:	m_bitmap(0)
{
	create(image, srcRect);
	T_ASSERT (m_bitmap);
}

Bitmap::~Bitmap()
{
	safeDestroy(m_bitmap);
}

bool Bitmap::create(uint32_t width, uint32_t height)
{
	T_ASSERT (!m_bitmap);

	if (!(m_bitmap = Application::getInstance()->getWidgetFactory()->createBitmap()))
	{
		log::error << L"Failed to create native widget peer (Bitmap)" << Endl;
		return false;
	}

	if (!m_bitmap->create(width, height))
	{
		m_bitmap->destroy();
		m_bitmap = 0;
		return false;
	}

	return true;
}

bool Bitmap::create(drawing::Image* image)
{
	T_ASSERT (!m_bitmap);

	if (!create(image->getWidth(), image->getHeight()))
		return false;

	m_bitmap->copySubImage(
		image,
		Rect(0, 0, image->getWidth(), image->getHeight()),
		Point(0, 0)
	);

	return true;
}

bool Bitmap::create(drawing::Image* image, const Rect& srcRect)
{
	T_ASSERT (!m_bitmap);

	if (!create(image->getWidth(), image->getHeight()))
		return false;

	m_bitmap->copySubImage(
		image,
		srcRect,
		Point(0, 0)
	);

	return true;
}

void Bitmap::destroy()
{
	safeDestroy(m_bitmap);
}

void Bitmap::copyImage(drawing::Image* image)
{
	if (m_bitmap)
		m_bitmap->copySubImage(
			image,
			Rect(0, 0, image->getWidth(), image->getHeight()),
			Point(0, 0)
		);
}

void Bitmap::copySubImage(drawing::Image* image, const Rect& srcRect, const Point& destPos)
{
	if (m_bitmap)
		m_bitmap->copySubImage(image, srcRect, destPos);
}

Ref< drawing::Image > Bitmap::getImage() const
{
	if (m_bitmap)
		return m_bitmap->getImage();
	else
		return nullptr;
}

Size Bitmap::getSize() const
{
	if (m_bitmap)
		return m_bitmap->getSize();
	else
		return Size(0, 0);
}

ISystemBitmap* Bitmap::getSystemBitmap() const
{
	return m_bitmap;
}

Ref< Bitmap > Bitmap::load(const std::wstring& fileName)
{
	Ref< Bitmap > bitmap;
	if (Path(fileName).getExtension() == L"image")
	{
		Ref< IStream > s = FileSystem::getInstance().open(fileName, File::FmRead);
		if (!s)
			return nullptr;

		DynamicMemoryStream dms(false, true);
		StreamCopy(&dms, s).execute();

		s->close();
		s = 0;

		const void* resource = &dms.getBuffer()[0];
		uint32_t size = dms.getBuffer().size();

		int32_t systemDPI = getSystemDPI();
		int32_t bestFit = std::numeric_limits< int32_t >::max();
		int32_t bestFitIndex = 0;

		const ImageHeader* h = static_cast< const ImageHeader* >(resource);
		for (uint32_t i = 0; i < h->count; ++i)
		{
			if (abs(systemDPI - h->entry[i].dpi) < bestFit)
			{
				bestFit = abs(systemDPI - h->entry[i].dpi);
				bestFitIndex = i;
			}
		}

		MemoryStream ms(
			static_cast< const uint8_t* >(resource) + h->entry[bestFitIndex].offset,
			size - h->entry[bestFitIndex].offset
		);
		Ref< drawing::Image > image = drawing::Image::load(&ms, L"png");
		if (!image)
			return nullptr;

		bitmap = new Bitmap();
		if (!bitmap->create(image))
			return nullptr;
	}
	else
	{
		Ref< drawing::Image > image = drawing::Image::load(fileName);
		if (!image)
			return nullptr;

		bitmap = new Bitmap();
		if (!bitmap->create(image))
			return nullptr;
	}
	return bitmap;
}

Ref< Bitmap > Bitmap::load(const void* resource, uint32_t size, const std::wstring& extension)
{
	Ref< Bitmap > bitmap = new Bitmap();
	if (extension == L"image")
	{
		int32_t systemDPI = getSystemDPI();
		int32_t bestFit = std::numeric_limits< int32_t >::max();
		int32_t bestFitIndex = 0;

		const ImageHeader* h = static_cast< const ImageHeader* >(resource);
		for (uint32_t i = 0; i < h->count; ++i)
		{
			if (abs(systemDPI - h->entry[i].dpi) < bestFit)
			{
				bestFit = abs(systemDPI - h->entry[i].dpi);
				bestFitIndex = i;
			}
		}

		MemoryStream ms(
			static_cast< const uint8_t* >(resource) + h->entry[bestFitIndex].offset,
			size - h->entry[bestFitIndex].offset
		);
		Ref< drawing::Image > image = drawing::Image::load(&ms, L"png");
		if (!image)
			return nullptr;

		if (!bitmap->create(image))
			return nullptr;
	}
	else
	{
		Ref< drawing::Image > image = drawing::Image::load(resource, size, extension);
		if (!image)
			return nullptr;

		if (!bitmap->create(image))
			return nullptr;
	}
	return bitmap;
}

	}
}
