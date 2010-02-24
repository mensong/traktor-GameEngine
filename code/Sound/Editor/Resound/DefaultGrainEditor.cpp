#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/ITypedAsset.h"
#include "Editor/TypeBrowseFilter.h"
#include "I18N/Text.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Editor/Resound/DefaultGrainEditor.h"
#include "Ui/MethodHandler.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Custom/PropertyList/ArrayPropertyItem.h"
#include "Ui/Custom/PropertyList/BrowsePropertyItem.h"
#include "Ui/Custom/PropertyList/ObjectPropertyItem.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.DefaultGrainEditor", DefaultGrainEditor, IGrainEditor)

DefaultGrainEditor::DefaultGrainEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool DefaultGrainEditor::create(ui::Widget* parent)
{
	m_grainPropertyList = new ui::custom::AutoPropertyList();
	m_grainPropertyList->create(parent, ui::WsDoubleBuffer | ui::custom::AutoPropertyList::WsColumnHeader, this);
	m_grainPropertyList->addCommandEventHandler(ui::createMethodHandler(this, &DefaultGrainEditor::eventPropertyCommand));
	m_grainPropertyList->setSeparator(200);
	m_grainPropertyList->setColumnName(0, i18n::Text(L"PROPERTY_COLUMN_NAME"));
	m_grainPropertyList->setColumnName(1, i18n::Text(L"PROPERTY_COLUMN_VALUE"));
	m_grainPropertyList->setVisible(false);
	return true;
}

void DefaultGrainEditor::destroy()
{
	m_grainPropertyList->destroy();
}

void DefaultGrainEditor::show(IGrain* grain)
{
	m_grainPropertyList->bind(grain, 0);
	m_grainPropertyList->setVisible(true);
}

void DefaultGrainEditor::hide()
{
	m_grainPropertyList->apply();
	m_grainPropertyList->setVisible(false);
}

bool DefaultGrainEditor::resolvePropertyGuid(const Guid& guid, std::wstring& resolved) const
{
	Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(guid);
	if (!instance)
		return false;

	resolved = instance->getName();
	return true;
}

void DefaultGrainEditor::eventPropertyCommand(ui::Event* event)
{
	const ui::CommandEvent* cmdEvent = checked_type_cast< const ui::CommandEvent* >(event);
	const ui::Command& cmd = cmdEvent->getCommand();

	Ref< ui::custom::BrowsePropertyItem > browseItem = dynamic_type_cast< ui::custom::BrowsePropertyItem* >(event->getItem());
	if (browseItem)
	{
		if (cmd == L"Property.Browse")
		{
			if (browseItem->getValue().isNull())
			{
				Ref< db::Instance > instance;
				if (browseItem->getFilterType())
				{
					const TypeInfo* filterType = browseItem->getFilterType();

					// Check if filter type is actually a result of a asset; in such case we should
					// browse for the asset and not the final result.
					TypeInfoSet filterTypes;

					std::vector< const TypeInfo* > assetTypes;
					type_of< editor::ITypedAsset >().findAllOf(assetTypes);
					for (std::vector< const TypeInfo* >::iterator i = assetTypes.begin(); i != assetTypes.end(); ++i)
					{
						Ref< editor::ITypedAsset > asset = dynamic_type_cast< editor::ITypedAsset* >((*i)->createInstance());
						if (asset && asset->getOutputType())
						{
							if (is_type_of(*asset->getOutputType(), *filterType))
								filterTypes.insert(*i);
						}
					}

					if (filterTypes.empty())
						filterTypes.insert(filterType);

					editor::TypeBrowseFilter filter(filterTypes);
					instance = m_editor->browseInstance(&filter);
				}
				else
					instance = m_editor->browseInstance();

				if (instance)
				{
					browseItem->setValue(instance->getGuid());
					m_grainPropertyList->apply();
				}
			}
			else
			{
				browseItem->setValue(Guid());
				m_grainPropertyList->apply();
			}
		}
		else if (cmd == L"Property.Edit")
		{
			Guid instanceGuid = browseItem->getValue();
			if (instanceGuid.isNull() || !instanceGuid.isValid())
				return;

			Ref< db::Instance > instance = m_editor->getSourceDatabase()->getInstance(instanceGuid);
			if (!instance)
				return;

			m_editor->openEditor(instance);
		}
	}

	Ref< ui::custom::ObjectPropertyItem > objectItem = dynamic_type_cast< ui::custom::ObjectPropertyItem* >(event->getItem());
	if (objectItem)
	{
		const TypeInfo* objectType = objectItem->getObjectType();
		if (!objectType)
			objectType = &type_of< ISerializable >();

		if (!objectItem->getObject())
		{
			objectType = m_editor->browseType(objectType);
			if (objectType)
			{
				Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
				if (object)
				{
					objectItem->setObject(object);

					m_grainPropertyList->refresh(objectItem, object);
					m_grainPropertyList->apply();
				}
			}
		}
		else
		{
			objectItem->setObject(0);

			m_grainPropertyList->refresh(objectItem, 0);
			m_grainPropertyList->apply();
		}
	}

	Ref< ui::custom::ArrayPropertyItem > arrayItem = dynamic_type_cast< ui::custom::ArrayPropertyItem* >(event->getItem());
	if (arrayItem)
	{
		if (arrayItem->getElementType())
		{
			const TypeInfo* objectType = m_editor->browseType(arrayItem->getElementType());
			if (objectType)
			{
				Ref< ISerializable > object = dynamic_type_cast< ISerializable* >(objectType->createInstance());
				if (object)
				{
					m_grainPropertyList->addObject(arrayItem, object);
					m_grainPropertyList->apply();
					m_grainPropertyList->refresh();
				}
			}
		}
		else	// Non-complex array; just apply and refresh.
		{
			m_grainPropertyList->apply();
			m_grainPropertyList->refresh();
		}
	}

	m_grainPropertyList->update();
}

	}
}
