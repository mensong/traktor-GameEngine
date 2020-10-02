#include "Render/Image2/ImageTargetSet.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageTargetSet", ImageTargetSet, Object)

ImageTargetSet::ImageTargetSet(const std::wstring& name, handle_t persistentHandle, handle_t textureIds[RenderGraphTargetSetDesc::MaxColorTargets], const RenderGraphTargetSetDesc& targetSetDesc)
:	m_name(name)
,	m_persistentHandle(persistentHandle)
,   m_targetSetDesc(targetSetDesc)
{
	for (int32_t i = 0; i < RenderGraphTargetSetDesc::MaxColorTargets; ++i)
		m_textureIds[i] = textureIds[i];
}

const std::wstring& ImageTargetSet::getName() const
{
	return m_name;
}

handle_t ImageTargetSet::getPersistentHandle() const
{
	return m_persistentHandle;
}

handle_t ImageTargetSet::getTextureId(int32_t colorIndex) const
{
	return m_textureIds[colorIndex];
}

const RenderGraphTargetSetDesc& ImageTargetSet::getTargetSetDesc() const
{
    return m_targetSetDesc;
}

	}
}
