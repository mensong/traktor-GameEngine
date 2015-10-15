#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Sound/Player/ISoundHandle.h"
#include "Spark/Canvas.h"
#include "Spark/Character.h"
#include "Spark/CharacterInstance.h"
#include "Spark/Context.h"
#include "Spark/IComponent.h"
#include "Spark/IComponentInstance.h"
#include "Spark/ScriptComponent.h"
#include "Spark/ScriptComponentInstance.h"
#include "Spark/Shape.h"
#include "Spark/SoundComponent.h"
#include "Spark/SoundComponentInstance.h"
#include "Spark/SparkClassFactory.h"
#include "Spark/Sprite.h"
#include "Spark/SpriteInstance.h"
#include "Spark/Text.h"
#include "Spark/TextInstance.h"

namespace traktor
{
	namespace spark
	{
		namespace
		{

void Canvas_quadricTo_4(Canvas* self, float x1, float y1, float x, float y)
{
	self->quadricTo(x1, y1, x, y);
}

void Canvas_quadricTo_2(Canvas* self, float x, float y)
{
	self->quadricTo(x, y);
}

void Canvas_cubicTo_6(Canvas* self, float x1, float y1, float x2, float y2, float x, float y)
{
	self->cubicTo(x1, y1, x2, y2, x, y);
}

void Canvas_cubicTo_4(Canvas* self, float x2, float y2, float x, float y)
{
	self->cubicTo(x2, y2, x, y);
}

void CharacterInstance_setPosition(CharacterInstance* self, float x, float y)
{
	self->setPosition(Vector2(x, y));
}

void CharacterInstance_setScale(CharacterInstance* self, float x, float y)
{
	self->setScale(Vector2(x, y));
}

Shape* SpriteInstance_getShape(SpriteInstance* self)
{
	return self->getShape().getResource();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.SparkClassFactory", 0, SparkClassFactory, IRuntimeClassFactory)

void SparkClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< Context > > classContext = new AutoRuntimeClass< Context >();
	registrar->registerClass(classContext);

	Ref< AutoRuntimeClass< IComponent > > classComponent = new AutoRuntimeClass< IComponent >();
	registrar->registerClass(classComponent);

	Ref< AutoRuntimeClass< IComponentInstance > > classComponentInstance = new AutoRuntimeClass< IComponentInstance >();
	registrar->registerClass(classComponentInstance);

	Ref< AutoRuntimeClass< ScriptComponent > > classScriptComponent = new AutoRuntimeClass< ScriptComponent >();
	registrar->registerClass(classScriptComponent);

	Ref< AutoRuntimeClass< ScriptComponentInstance > > classScriptComponentInstance = new AutoRuntimeClass< ScriptComponentInstance >();
	registrar->registerClass(classScriptComponentInstance);

	Ref< AutoRuntimeClass< SoundComponent > > classSoundComponent = new AutoRuntimeClass< SoundComponent >();
	registrar->registerClass(classSoundComponent);

	Ref< AutoRuntimeClass< SoundComponentInstance > > classSoundComponentInstance = new AutoRuntimeClass< SoundComponentInstance >();
	classSoundComponentInstance->addMethod("play", &SoundComponentInstance::play);
	registrar->registerClass(classSoundComponentInstance);

	Ref< AutoRuntimeClass< Character > > classCharacter = new AutoRuntimeClass< Character >();
	registrar->registerClass(classCharacter);

	Ref< AutoRuntimeClass< CharacterInstance > > classCharacterInstance = new AutoRuntimeClass< CharacterInstance >();
	classCharacterInstance->addMethod("setTransform", &CharacterInstance::setTransform);
	classCharacterInstance->addMethod("getTransform", &CharacterInstance::getTransform);
	classCharacterInstance->addMethod("setPosition", &CharacterInstance::setPosition);
	classCharacterInstance->addMethod("setPosition", &CharacterInstance_setPosition);
	classCharacterInstance->addMethod("getPosition", &CharacterInstance::getPosition);
	classCharacterInstance->addMethod("setScale", &CharacterInstance::setScale);
	classCharacterInstance->addMethod("setScale", &CharacterInstance_setScale);
	classCharacterInstance->addMethod("getScale", &CharacterInstance::getScale);
	classCharacterInstance->addMethod("setRotation", &CharacterInstance::setRotation);
	classCharacterInstance->addMethod("getRotation", &CharacterInstance::getRotation);
	classCharacterInstance->addMethod("setVisible", &CharacterInstance::setVisible);
	classCharacterInstance->addMethod("getVisible", &CharacterInstance::getVisible);
	classCharacterInstance->addMethod("getBounds", &CharacterInstance::getBounds);
	registrar->registerClass(classCharacterInstance);

	Ref< AutoRuntimeClass< Shape > > classShape = new AutoRuntimeClass< Shape >();
	registrar->registerClass(classShape);

	Ref< AutoRuntimeClass< Sprite > > classSprite = new AutoRuntimeClass< Sprite >();
	registrar->registerClass(classSprite);

	Ref< AutoRuntimeClass< SpriteInstance > > classSpriteInstance = new AutoRuntimeClass< SpriteInstance >();
	classSpriteInstance->addMethod("getContext", &SpriteInstance::getContext);
	classSpriteInstance->addMethod("setShape", &SpriteInstance::setShape);
	classSpriteInstance->addMethod("getShape", &SpriteInstance_getShape);
	classSpriteInstance->addMethod("create", &SpriteInstance::create);
	classSpriteInstance->addMethod("place", &SpriteInstance::place);
	classSpriteInstance->addMethod("remove", &SpriteInstance::remove);
	classSpriteInstance->addMethod("setAlpha", &SpriteInstance::setAlpha);
	classSpriteInstance->addMethod("getAlpha", &SpriteInstance::getAlpha);
	classSpriteInstance->addMethod("setComponent", &SpriteInstance::setComponent);
	classSpriteInstance->addMethod("getComponent", &SpriteInstance::getComponent);
	registrar->registerClass(classSpriteInstance);

	Ref< AutoRuntimeClass< Text > > classText = new AutoRuntimeClass< Text >();
	registrar->registerClass(classText);

	Ref< AutoRuntimeClass< TextInstance > > classTextInstance = new AutoRuntimeClass< TextInstance >();
	classTextInstance->addMethod("setText", &TextInstance::setText);
	classTextInstance->addMethod("getText", &TextInstance::getText);
	classTextInstance->addMethod("setHeight", &TextInstance::setHeight);
	classTextInstance->addMethod("getHeight", &TextInstance::getHeight);
	classTextInstance->addMethod("setBounds", &TextInstance::setBounds);
	classTextInstance->addMethod("setAlpha", &TextInstance::setAlpha);
	classTextInstance->addMethod("getAlpha", &TextInstance::getAlpha);
	registrar->registerClass(classTextInstance);

	Ref< AutoRuntimeClass< Canvas > > classCanvas = new AutoRuntimeClass< Canvas >();
	classCanvas->addConstructor();
	classCanvas->addMethod("clear", &Canvas::clear);
	classCanvas->addMethod("moveTo", &Canvas::moveTo);
	classCanvas->addMethod("lineTo", &Canvas::lineTo);
	classCanvas->addMethod("quadricTo", &Canvas_quadricTo_4);
	classCanvas->addMethod("quadricTo", &Canvas_quadricTo_2);
	classCanvas->addMethod("cubicTo", &Canvas_cubicTo_6);
	classCanvas->addMethod("cubicTo", &Canvas_cubicTo_4);
	classCanvas->addMethod("close", &Canvas::close);
	classCanvas->addMethod("rect", &Canvas::rect);
	classCanvas->addMethod("circle", &Canvas::circle);
	classCanvas->addMethod("fill", &Canvas::fill);
	classCanvas->addMethod("stroke", &Canvas::stroke);
	classCanvas->addMethod("createShape", &Canvas::createShape);
	registrar->registerClass(classCanvas);
}

	}
}
