#ifndef traktor_drawing_ImageFormatGif_H
#define traktor_drawing_ImageFormatGif_H

#include "Drawing/IImageFormat.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DRAWING_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace drawing
	{

/*! \brief GIF format.
 * \ingroup Drawing
 */
class T_DLLCLASS ImageFormatGif : public IImageFormat
{
	T_RTTI_CLASS;

public:
	virtual Ref< Image > read(IStream* stream);

	virtual bool write(IStream* stream, Image* image);
};

	}
}

#endif	// traktor_drawing_ImageFormatGif_H
