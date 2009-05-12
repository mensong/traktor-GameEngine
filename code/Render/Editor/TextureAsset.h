#ifndef traktor_render_TextureAsset_H
#define traktor_render_TextureAsset_H

#include "Editor/Asset.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class T_DLLCLASS TextureAsset : public editor::Asset
{
	T_RTTI_CLASS(TextureAsset)

public:
	bool m_generateNormalMap;
	float m_scaleDepth;
	bool m_generateMips;
	bool m_keepZeroAlpha;
	bool m_isCubeMap;
	bool m_hasAlpha;
	bool m_ignoreAlpha;
	bool m_scaleImage;
	int32_t m_scaleWidth;
	int32_t m_scaleHeight;
	bool m_enableCompression;
	bool m_linearGamma;
	bool m_generateSphereMap;

	TextureAsset();

	virtual const Type* getOutputType() const;

	virtual int getVersion() const;

	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_render_TextureAsset_H
