#ifndef traktor_mesh_MeshAssetEditor_H
#define traktor_mesh_MeshAssetEditor_H

#include "Editor/IObjectEditor.h"
#include "Core/Heap/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EDITOR_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace ui
	{

class Container;
class DropDown;
class Edit;
class Event;
class ListView;

	}

	namespace model
	{

class Model;

	}

	namespace mesh
	{

class MeshAsset;

class T_DLLCLASS MeshAssetEditor : public editor::IObjectEditor
{
	T_RTTI_CLASS(MeshAssetEditor)

public:
	MeshAssetEditor(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, db::Instance* instance, Serializable* object);

	virtual void destroy();

	virtual void apply();

private:
	editor::IEditor* m_editor;
	Ref< db::Instance > m_instance;
	Ref< MeshAsset > m_asset;
	Ref< model::Model > m_model;
	Ref< ui::Edit > m_editFileName;
	Ref< ui::Container > m_containerMaterials;
	Ref< ui::DropDown > m_dropMeshType;
	Ref< ui::ListView > m_materialList;

	void updateModel();

	void updateFile();

	void updateMaterialList();

	void createMaterialShader();

	void browseMaterialShader();
	
	void removeMaterialShader();

	void eventBrowseClick(ui::Event* event);

	void eventMaterialToolClick(ui::Event* event);
};

	}
}

#endif	// traktor_mesh_MeshAssetEditor_H
