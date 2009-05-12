#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/UtilitiesOpenGL.h"

namespace traktor
{
	namespace render
	{

bool convertTextureFormat(TextureFormat textureFormat, int& outPixelSize, GLint& outComponents, GLenum& outFormat, GLenum& outType)
{
	switch (textureFormat)
	{
	case TfR8:
		outPixelSize = 1;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfR8G8B8A8:
		outPixelSize = 4;
		outComponents = GL_RGBA8;
#if defined(T_BIG_ENDIAN)
		outFormat = GL_BGRA;
		outType = GL_UNSIGNED_INT_8_8_8_8;
#else	// T_LITTLE_ENDIAN
		outFormat = GL_BGRA;
		outType = GL_UNSIGNED_INT_8_8_8_8_REV;
#endif
		break;

	//case TfR16G16B16A16F:
	//	break;

	case TfR32G32B32A32F:
		outPixelSize = 16;
		outComponents = 4;
		outFormat = GL_RGBA;
		outType = GL_FLOAT;
		break;

#if !defined(__APPLE__)
	case TfR16F:
		outPixelSize = 2;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_HALF_FLOAT_ARB;
		break;
#endif

	case TfR32F:
		outPixelSize = 4;
		outComponents = 1;
		outFormat = GL_RED;
		outType = GL_FLOAT;
		break;

	case TfDXT1:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	case TfDXT3:
		outPixelSize = 0;
		outComponents = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		outFormat = GL_RGBA;
		outType = GL_UNSIGNED_BYTE;
		break;

	default:
		return false;
	}

	return true;
}

	}
}
