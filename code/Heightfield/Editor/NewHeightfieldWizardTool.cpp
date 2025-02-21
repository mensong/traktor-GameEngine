/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Log/Log.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "I18N/Text.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"
#include "Heightfield/Editor/NewHeightfieldDialog.h"
#include "Heightfield/Editor/NewHeightfieldWizardTool.h"
#include "Heightfield/Editor/HeightfieldAsset.h"

namespace traktor::hf
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.hf.NewHeightfieldWizardTool", 0, NewHeightfieldWizardTool, editor::IWizardTool)

std::wstring NewHeightfieldWizardTool::getDescription() const
{
	return i18n::Text(L"NEW_HEIGHTFIELD_WIZARDTOOL_DESCRIPTION");
}

const TypeInfoSet NewHeightfieldWizardTool::getSupportedTypes() const
{
	return TypeInfoSet();
}

uint32_t NewHeightfieldWizardTool::getFlags() const
{
	return WfGroup;
}

bool NewHeightfieldWizardTool::launch(ui::Widget* parent, editor::IEditor* editor, db::Group* group, db::Instance* instance)
{
	Ref< NewHeightfieldDialog > newHeightfieldDialog = new NewHeightfieldDialog();

	if (!newHeightfieldDialog->create(parent))
		return false;

	if (newHeightfieldDialog->showModal() != ui::DialogResult::Ok)
		return false;

	const int32_t gridSize = newHeightfieldDialog->getGridSize();
	const Vector4 worldExtent = newHeightfieldDialog->getWorldExtent();
	const Plane worldPlane = newHeightfieldDialog->getWorldPlane();

	Ref< db::Instance > heightfieldInstance = group->createInstance(L"Unnamed");
	if (!heightfieldInstance)
		return false;

	Ref< Heightfield > heightfield = new Heightfield(gridSize, worldExtent);

	for (int32_t z = 0; z < gridSize; ++z)
	{
		for (int32_t x = 0; x < gridSize; ++x)
		{
			float wx, wz;
			heightfield->gridToWorld(x, z, wx, wz);

			Scalar k;
			Vector4 p = Vector4::zero();

			worldPlane.intersectRay(Vector4(wx, -1e8f, wz, 1.0f), Vector4(0.0f, 1.0f, 0.0f, 0.0f), k, p);

			const float h = heightfield->worldToUnit(p.y());

			heightfield->setGridHeight(x, z, clamp(h, 0.0f, 1.0f));
			heightfield->setGridCut(x, z, true);
		}
	}

	Ref< IStream > file = heightfieldInstance->writeData(L"Data");
	if (!file)
		return false;

	HeightfieldFormat().write(file, heightfield);

	file->close();
	file = nullptr;

	Ref< HeightfieldAsset > heightfieldAsset = new HeightfieldAsset(worldExtent);
	heightfieldInstance->setObject(heightfieldAsset);
	heightfieldInstance->commit();

	return true;
}

}
