/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Skeleton.h"
#include "Animation/Editor/SkeletonAsset.h"
#include "Animation/Editor/SkeletonEditorPage.h"
#include "Animation/Editor/SkeletonEditorPageFactory.h"
#include "Ui/Command.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.animation.SkeletonEditorPageFactory", 0, SkeletonEditorPageFactory, editor::IEditorPageFactory)

const TypeInfoSet SkeletonEditorPageFactory::getEditableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< Skeleton >());
	typeSet.insert(&type_of< SkeletonAsset >());
	return typeSet;
}

bool SkeletonEditorPageFactory::needOutputResources(const TypeInfo& typeInfo, std::set< Guid >& outDependencies) const
{
	outDependencies.insert(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));	// System/Primitive/Shaders/Primitive
	outDependencies.insert(Guid(L"{123602E4-BC6F-874D-92E8-A20852D140A3}"));	// System/Primitive/Textures/SmallFont
	return false;
}

Ref< editor::IEditorPage > SkeletonEditorPageFactory::createEditorPage(editor::IEditor* editor, editor::IEditorPageSite* site, editor::IDocument* document) const
{
	return new SkeletonEditorPage(editor, site, document);
}

void SkeletonEditorPageFactory::getCommands(std::list< ui::Command >& outCommands) const
{
	outCommands.push_back(ui::Command(L"Skeleton.Editor.AddJoint"));
}

	}
}
