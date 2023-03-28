/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <fbxsdk.h>
#include "Core/Containers/SmallMap.h"

namespace traktor::model
{

class Model;

bool convertMaterials(Model& outModel, SmallMap< int32_t, int32_t >& outMaterialMap, FbxNode* meshNode);

void fixMaterialUvSets(Model& outModel);

}
