#ifndef traktor_render_Edge_H
#define traktor_render_Edge_H

#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class OutputPin;
class InputPin;

/*! \brief Shader graph edge.
 * \ingroup Render
 */
class T_DLLCLASS Edge : public Serializable
{
	T_RTTI_CLASS(Edge)

public:
	Edge(const OutputPin* source = 0, const InputPin* destination = 0);

	const OutputPin* getSource() const;

	const InputPin* getDestination() const;

	bool serialize(Serializer& s);

private:
	Ref< const OutputPin > m_source;
	Ref< const InputPin > m_destination;
};

	}
}

#endif	// traktor_render_Edge_H
