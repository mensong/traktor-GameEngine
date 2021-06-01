#include "Core/RefArray.h"
#include "Core/Functor/Functor.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/String.h"
#include "Core/Thread/Thread.h"
#include "Core/Thread/ThreadManager.h"
#include "Core/Thread/ThreadPool.h"
#include "Spark/Action/ActionContext.h"
#include "Spark/Action/ActionFunctionNative.h"
#include "Spark/Action/Common/XML.h"
#include "Spark/Action/Common/XMLNode.h"
#include "Net/Url.h"
#include "Net/UrlConnection.h"
#include "Xml/XmlPullParser.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spark.XML", XML, XMLNode)

XML::XML(ActionContext* context)
:	XMLNode("XML", XMLNode::NtElement, L"", L"")
,	m_context(context)
,	m_thread(nullptr)
{
}

XML::~XML()
{
	if (m_thread)
	{
		ActionObject* self = getAsObject(m_context);
		T_ASSERT(self);

		m_context->removeFrameListener(self);

		ThreadPool::getInstance().stop(m_thread);
		m_thread = nullptr;
	}
}

bool XML::load(const std::wstring& url_)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT(self);

	if (m_thread)
		return false;

	ThreadPool::getInstance().spawn(makeFunctor< XML, std::wstring >(this, &XML::threadLoad, url_), m_thread);
	if (!m_thread)
		return false;

	self->setMember("onFrame", ActionValue(createNativeFunction(m_context, this, &XML::onFrame)));
	m_context->addFrameListener(self);

	return true;
}

void XML::onFrame(CallArgs& ca)
{
	ActionObject* self = getAsObject(m_context);
	T_ASSERT(self);

	if (m_thread->wait(0))
	{
		m_thread = nullptr;

		m_context->removeFrameListener(self);

		ActionValue memberOnLoad;
		if (self->getMember("onLoad", memberOnLoad))
		{
			Ref< ActionFunction > onLoad = memberOnLoad.getObject< ActionFunction >();
			if (onLoad)
				onLoad->call(self, ActionValueArray());
		}
	}
}

void XML::threadLoad(std::wstring url_)
{
	net::Url url(url_);
	if (!url.valid() || m_thread->stopped())
		return;

	Ref< net::UrlConnection > connection = net::UrlConnection::open(url);
	if (!connection || m_thread->stopped())
		return;

	IStream* stream = connection->getStream();
	if (!stream || m_thread->stopped())
		return;

	RefArray< XMLNode > elementStack;
	elementStack.push_back(this);

	xml::XmlPullParser parser(stream);
	while (!m_thread->stopped())
	{
		if (stream->available() <= 0)
		{
			m_thread->sleep(100);
			continue;
		}

		xml::XmlPullParser::EventType eventType = parser.next();

		if (eventType == xml::XmlPullParser::EventType::EndDocument)
			break;
		else if (eventType == xml::XmlPullParser::EventType::Invalid)
			break;

		if (eventType == xml::XmlPullParser::EventType::StartElement)
		{
			Ref< XMLNode > element = new XMLNode(
				"XMLNode",
				XMLNode::NtElement,
				parser.getEvent().value,
				L""
			);

			elementStack.push_back(element);
		}
		else if (eventType == xml::XmlPullParser::EventType::EndElement)
		{
			if (elementStack.empty())
				return;

			Ref< XMLNode > element = elementStack.back();
			elementStack.pop_back();

			if (elementStack.empty())
				return;

			elementStack.back()->appendChild(element);
		}
		else if (eventType == xml::XmlPullParser::EventType::Text)
		{
			if (elementStack.empty())
				return;

			std::wstring value = trim(parser.getEvent().value);
			if (!value.empty())
			{
				Ref< XMLNode > text = new XMLNode(
					"XMLNode",
					XMLNode::NtText,
					L"",
					value
				);
				elementStack.back()->appendChild(text);
			}
		}
	}
}

	}
}
