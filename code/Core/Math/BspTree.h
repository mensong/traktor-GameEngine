#ifndef traktor_mesh_BspTree_H
#define traktor_mesh_BspTree_H

#include "Core/Heap/Ref.h"
#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Winding.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! \brief Binary space partitioning tree.
 * \ingroup Core
 */
class T_DLLCLASS BspTree
{
public:
	/*! \brief Build BSP from a set of polygons.
	 *
	 * \param polygons Polygon set.
	 * \return True if built successfully.
	 */
	bool build(const AlignedVector< Winding >& polygons);

	/*! \brief Check if point is inside "solid" space.
	 *
	 * \return True if point inside "solid" space.
	 */
	bool inside(const Vector4& pt) const;

	/*! \brief Check if all points of a winding is inside "solid" space.
	 *
	 * \return True if all points inside "solid" space.
	 */
	bool inside(const Winding& w) const;

	/*! \brief Clip polygon to BSP.
	 *
	 * \param polygon Polygon
	 * \param outClipped Clipped polygons.
	 */
	template < typename PolygonType >
	void clip(const PolygonType& polygon, AlignedVector< PolygonType >& outClipped) const
	{
		T_ASSERT (m_root);
		clip< PolygonType >(m_root, polygon, outClipped);
	}

private:
	struct BspNode : public Object
	{
		Plane plane;
		Ref< BspNode > front;
		Ref< BspNode > back;
	};

	Ref< BspNode > m_root;

	BspNode* recursiveBuild(const AlignedVector< Winding >& polygons, const AlignedVector< Plane >& planes);

	bool inside(const BspNode* node, const Vector4& pt) const;

	bool inside(const BspNode* node, const Winding& w) const;

	template < typename PolygonType >
	void clip(const BspNode* node, const PolygonType& polygon, AlignedVector< PolygonType >& outClipped) const
	{
		Winding w = polygon.winding();

		int cf = w.classify(node->plane);
		if (cf == Winding::CfCoplanar)
		{
			Plane polygonPlane;
			if (w.getPlane(polygonPlane))
				cf = dot3(node->plane.normal(), polygonPlane.normal()) >= 0.0f ? Winding::CfFront : Winding::CfBack;
			else
				cf = Winding::CfFront;
		}

		if (cf == Winding::CfFront)
		{
			if (node->front)
				clip(node->front, polygon, outClipped);
			else if (polygon.valid())
				outClipped.push_back(polygon);
		}
		else if (cf == Winding::CfBack)
		{
			if (node->back)
				clip(node->back, polygon, outClipped);
		}
		else if (cf == Winding::CfSpan)
		{
			PolygonType f, b;
			polygon.split(node->plane, f, b);
			if (f.valid())
			{
				if (node->front)
					clip(node->front, f, outClipped);
				else
					outClipped.push_back(f);
			}
			if (b.valid())
			{
				if (node->back)
					clip(node->back, b, outClipped);
			}
		}
	}
};

}

#endif	// traktor_mesh_BspTree_H
