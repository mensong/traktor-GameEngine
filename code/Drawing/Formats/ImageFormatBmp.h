#ifndef traktor_drawing_ImageFormatBmp_H
#define traktor_drawing_ImageFormatBmp_H

#include "Drawing/ImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief BMP format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatBmp : public ImageFormat
{
	T_RTTI_CLASS(ImageFormatBmp)

public:
	virtual Image* read(Stream* stream);

	virtual bool write(Stream* stream, Image* image);
};

	}
}

#endif	// traktor_drawing_ImageFormatBmp_H
