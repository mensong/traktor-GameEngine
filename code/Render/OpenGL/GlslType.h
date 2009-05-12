#ifndef traktor_render_GlslType_H
#define traktor_render_GlslType_H

#include <string>
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

/*! \ingroup OGL */
//@{

enum GlslType
{
	GtVoid,
	GtFloat,
	GtFloat2,
	GtFloat3,
	GtFloat4,
	GtFloat4x4
};

std::wstring glsl_vertex_attr_name(DataUsage usage, int index);

std::wstring glsl_type_name(GlslType type);

GlslType glsl_from_data_type(DataType type);

//@}

	}
}

#endif	// traktor_render_GlslType_H
