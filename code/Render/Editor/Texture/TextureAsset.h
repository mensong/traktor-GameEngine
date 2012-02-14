#ifndef traktor_render_TextureAsset_H
#define traktor_render_TextureAsset_H

#include "Render/Editor/Texture/TextureAssetBase.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureAsset : public TextureAssetBase
{
	T_RTTI_CLASS;

public:
	virtual Ref< drawing::Image > load(const std::wstring& assetPath) const;
};

	}
}

#endif	// traktor_render_TextureAsset_H
