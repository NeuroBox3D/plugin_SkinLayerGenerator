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
#include "../ugcore/ugbase/bridge/domain_bridges/selection_bridge.h"
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
	Mesh* mesh = new Mesh();
	CreateCircle(mesh, m_center, m_radius, m_numVertices, SUBCUTAN, false);
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator.ugx");
	delete mesh;
}



/////////////////////////////////////////////////////////
/// constants
/////////////////////////////////////////////////////////
const number SkinLayerGenerator::REMOVE_DOUBLES_THRESHOLD = 1e-8;
const bool SkinLayerGenerator::FILL = false;
