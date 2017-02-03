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
	CreateCircle(mesh, m_centerInjection, m_radiusInjection, m_numVerticesInjection, INJECTION, false);

	UG_COND_THROW(m_radius == 0, "Radius of skin layer has to be > 0.")
	CreateCircle(mesh, m_center, m_radius, m_numVertices, SUBCUTAN, false);

	/// position attachment for vertices
	AInt aInt;
	mesh->grid().attach_to_vertices(aInt);
	mesh->grid().attach_to_vertices(aPosition);
	mesh->grid().attach_to_all(aNormal);

	/// select all, then fill with triangles
	SelectAll(mesh);
	TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler(), 0);

	/// retriangulate (quality grid)
	SelectAll(mesh);
	Retriangulate(mesh, m_degTri);
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step1.ugx");

	/// extrude
	SelectAll(mesh);
	ExtrudeAlongNormal(mesh, m_injectionBase, m_numStepsExtrudeSubcutan, true, true);

    FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
	ExtrudeAlongNormal(mesh, m_injectionHeight, m_numStepsExtrudeInjection, true, true);

    FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
	ExtrudeAlongNormal(mesh, m_epidermisThickness, m_numStepsExtrudeEpidermis, true, true);

	/// rename subsets, erase empty subsets and assign colors
	EraseEmptySubsets(mesh->subset_handler());
	for (std::map<std::string, int>::const_iterator it = m_subsetNames.begin();
			it != m_subsetNames.end(); ++it) {
		mesh->subset_handler().subset_info(it->second).name = (it->first).c_str();
	}

	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// save grid to file
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator.ugx");

	/// delete mesh
	delete mesh;
}



/////////////////////////////////////////////////////////
/// constants
/////////////////////////////////////////////////////////
const number SkinLayerGenerator::REMOVE_DOUBLES_THRESHOLD = 1e-8;
const bool SkinLayerGenerator::FILL = false;
