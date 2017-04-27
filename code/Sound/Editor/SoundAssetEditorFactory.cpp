/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundAssetEditor.h"
#include "Sound/Editor/SoundAssetEditorFactory.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.sound.SoundAssetEditorFactory", 0, SoundAssetEditorFactory, editor::IObjectEditorFactory)

const TypeInfoSet SoundAssetEditorFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< SoundAsset >());
	return typeSet;
}

bool SoundAssetEditorFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	return true;
}

Ref< editor::IObjectEditor > SoundAssetEditorFactory::createObjectEditor(editor::IEditor* editor) const
{
	return new SoundAssetEditor(editor);
}

void SoundAssetEditorFactory::getCommands(std::list< ui::Command >& outCommands) const
{
}

	}
}
