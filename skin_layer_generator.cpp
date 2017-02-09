/*!
 * \file plugins/skin_layer_generator/skin_layer_generator.cpp
 * \brief
 *
 *  Created on: January 30, 2017
 *      Author: Stephan Grein
 */

/// includes
#include "skin_layer_generator.h"
#include "lib_grid/lib_grid.h"
#include "lib_grid/algorithms/remove_duplicates_util.h"
#include "bridge/domain_bridges/selection_bridge.h"
#include "../ProMesh/mesh.h"
#include "../ProMesh/tools/grid_generation_tools.h"
#include "../ProMesh/tools/remeshing_tools.h"
#include "../ProMesh/tools/selection_tools.h"
#include "../ProMesh/tools/new_tools.h"
#include "../ProMesh/tools/subset_tools.h"
#include "../ProMesh/tools/refinement_tools.h"
#include "../ProMesh/tools/coordinate_transform_tools.h"
#include "../ProMesh/tools/topology_tools.h"
 
/// usings
using namespace ug::skin_layer_generator;

/// enable warnings and set debug ids
#define UG_ENABLE_WARNINGS
ug::DebugID SLGGenerateMesh("SLG_DID.GenerateMesh");

/////////////////////////////////////////////////////////
/// GENERATE
/////////////////////////////////////////////////////////
void SkinLayerGenerator::generate() {
	using namespace promesh;
	UG_DLOG(SLGGenerateMesh, 0, "Generating mesh.")
	/// empty mesh
	Mesh* mesh = new Mesh();

	/// mesh operations
	UG_COND_THROW(m_radiusInjection == 0, "Radius of injection layer has to be > 0.")
	CreateCircle(mesh, m_centerInjection, m_radiusInjection, m_numVerticesInjection, -1, false);

	UG_COND_THROW(m_radius == 0, "Radius of skin layer has to be > 0.")
	CreateCircle(mesh, m_center, m_radius, m_numVertices, -1, false);

	/// position attachment for vertices
	AInt aInt;
	mesh->grid().attach_to_vertices(aInt);
	mesh->grid().attach_to_vertices(aPosition);
	mesh->grid().attach_to_all(aNormal);

	/// select all, then fill with triangles
	SelectAll(mesh);
	TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());

	/// retriangulate (quality grid)
	SelectAll(mesh);
	Retriangulate(mesh, m_degTri);

	/// extrude layers
	SelectAll(mesh);
	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		if (it->has_injection()) {
			SmartPtr<Injection> inj = it->get_injection();
			number diff = it->thickness - inj->thickness - it->thickness*inj->position;
			ExtrudeAlongNormal(mesh, it->thickness*inj->position, (it->thickness*inj->position) / it->resolution, true, true);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
			ExtrudeAlongNormal(mesh, inj->thickness, inj->thickness / inj->resolution, true, true);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
			ExtrudeAlongNormal(mesh, diff, diff / it->resolution, true, true);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
		} else {
			ExtrudeAlongNormal(mesh, it->thickness, it->thickness / it->resolution, true, true);
		}
		FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
	}

	mesh->selector().clear();
	number base_coord = 0;
	size_t si = 1;
	ug::vector3 bottom;
	ug::vector3 top_coord;
	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		if (it->has_injection()) {
			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->thickness * it->injection->position);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->thickness * it->injection->position;
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->injection->thickness);

			ug::vector3 bottom2 = ug::vector3(m_centerInjection.x(), m_centerInjection.y(), m_centerInjection.z() + base_coord);
			ug::vector3 top_coord2 = ug::vector3(m_centerInjection.x(), m_centerInjection.y(), m_centerInjection.z() + base_coord + it->injection->thickness);

			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->injection->thickness;
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			SelectElementsInCylinder<ug::Face>(mesh, bottom2, top_coord2, m_radiusInjection);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom2, top_coord2, m_radiusInjection);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom2, top_coord2, m_radiusInjection);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom2, top_coord2, m_radiusInjection);
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si+1);
			mesh->selector().clear();

			/// TODO: handle injection subset boundary, i.e. separate boundary from volume
			if (it->get_injection()->with_inner_neumann_boundary()) {

			}

			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position));
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position);
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			si++; si++;

		} else {
			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->thickness);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->thickness;
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();
			si++;
		}
	}

	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step0.ugx");
	SelectBoundaryFaces(mesh);
	SelectBoundaryVertices(mesh);
	SelectBoundaryEdges(mesh);
	AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
	mesh->selector().clear();
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// assign subset names
	si = 0;
	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		mesh->subset_handler().subset_info(si).name = it->name;
		if (it->has_injection()) {
			si++;
			mesh->subset_handler().subset_info(si).name = it->injection->name;
		}
		si++;
	}
	mesh->subset_handler().subset_info(si).name = "Surface";
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step1.ugx");

	/// tetrahedralize
	Tetrahedralize(mesh->grid(), mesh->subset_handler(), m_degTet, true, true, aPosition, 1);
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// TODO separate volume subset after tetrahedralizing
	/// ...

	/// save volume grid to file
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step2.ugx");

	/// delete mesh
	delete mesh;
}



/////////////////////////////////////////////////////////
/// constants
/////////////////////////////////////////////////////////
const number SkinLayerGenerator::REMOVE_DOUBLES_THRESHOLD = 1e-8;
const number SkinLayerGenerator::SELECTION_THRESHOLD = 0.1;
const bool SkinLayerGenerator::FILL = false;
