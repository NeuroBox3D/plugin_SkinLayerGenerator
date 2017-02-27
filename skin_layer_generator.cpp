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

	/// mesh operations - check for consistency first
	UG_COND_THROW(m_radiusInjection == 0, "Radius of injection layer has to be > 0.")
	CreateCircle(mesh, m_centerInjection, m_radiusInjection, m_numVerticesInjection, 0, false);

	UG_COND_THROW(m_radius == 0, "Radius of skin layer has to be > 0.")
	CreateCircle(mesh, m_center, m_radius, m_numVertices, 1, false);

	/// TODO: consistency check if depot fits in layer!

	/// position attachment for vertices
	AInt aInt;
	mesh->grid().attach_to_vertices(aInt);
	mesh->grid().attach_to_vertices(aPosition);
	mesh->grid().attach_to_all(aNormal);

	/// extrude layers
	number totalHeight = 0;
	mesh->selector().clear();
	SelectSubset(mesh, 0, true, true, true, true);
	SelectSubset(mesh, 1, true, true, true, true);
	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		if (it->has_injection()) {
			SmartPtr<Injection> inj = it->get_injection();
			number diff = it->thickness - inj->thickness - it->thickness*inj->position;
			ExtrudeAndMove(mesh, ug::vector3(0, 0, it->thickness*inj->position), (it->thickness*inj->position) / it->resolution, true, false);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
			TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());

			ExtrudeAndMove(mesh, ug::vector3(0, 0, inj->thickness), (inj->thickness) / inj->resolution, true, false);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
			TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());

			ExtrudeAndMove(mesh, ug::vector3(0, 0, diff), diff / it->resolution, true, false);
			FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
			totalHeight += it->thickness;

			TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());
		}
		else {
			ExtrudeAndMove(mesh, ug::vector3(0, 0, it->thickness), it->thickness / it->resolution, true, false);
			totalHeight += it->thickness;
			ug::promesh::TriangleFill(mesh, true, m_degTri, 1);
		}
		FixFaceOrientation(mesh->grid(), mesh->selector().begin<Face>(), mesh->selector().end<Face>());
	}
	AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), 1);
	AssignSubsetColors(mesh->subset_handler());
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step0.ugx");

	/// assign delaunay mesh
	mesh->selector().clear();
	number base_coord = 0;
	size_t si = 1;
	ug::vector3 bottom;
	ug::vector3 top_coord;
	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		/// TODO: check if works if depot coincidences with a layer boundary!
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

			/// TODO: below needs to be fixed (select correct inner neumann boundary)
			if (it->get_injection()->with_inner_neumann_boundary()) {
					ug::vector3 base2 = bottom2;
					ug::vector3 top2 = top_coord2;
					base2[2] = base2.z() + SELECTION_THRESHOLD;
					top2[2] = top2.z() - SELECTION_THRESHOLD;

					SelectElementsInCylinder<ug::Volume>(mesh, base2, top2, m_radiusInjection - SELECTION_THRESHOLD);
					SelectElementsInCylinder<ug::Vertex>(mesh, base2, top2, m_radiusInjection - SELECTION_THRESHOLD);
					SelectElementsInCylinder<ug::Edge>(mesh, base2, top2, m_radiusInjection - SELECTION_THRESHOLD);
					SelectElementsInCylinder<ug::Face>(mesh, base2, top2, m_radiusInjection - SELECTION_THRESHOLD);
					AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si+2);
					mesh->selector().clear();
			}

			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + SELECTION_THRESHOLD);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position));
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position);
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			si++; si++; si++;

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

	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step1.ugx");
	SelectBoundaryFaces(mesh); /// Note this has no effect if surface not closed... (VERIFY first TODO)
	SelectBoundaryVertices(mesh); /// dito
	SelectBoundaryEdges(mesh); /// dito
	AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
	mesh->selector().clear();
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// assign subset names (for all subsets except surface subsets)
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
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step2.ugx");

	/// triangulate (TODO: better way to select these parts)
	///// BOTTOM
	mesh->selector().clear();
	SelectElementsInCylinder<ug::Edge>(mesh, ug::vector3(0,0,-SELECTION_THRESHOLD), ug::vector3(0, 0, SELECTION_THRESHOLD), m_radius);
	CloseSelection(mesh);
	TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());
	SelectSubset(mesh, -1, true, true, true, true);
	Retriangulate(mesh, m_degTri);
	SelectSubset(mesh, -1, true, true, true, true);
	AssignSubset(mesh, si);
	mesh->selector().clear();
	mesh->subset_handler().subset_info(si).name = "Bottom Surface";

	////// TOP
	SelectElementsInCylinder<ug::Edge>(mesh, ug::vector3(0,0,totalHeight-SELECTION_THRESHOLD), ug::vector3(0, 0, totalHeight +SELECTION_THRESHOLD), m_radius);
	CloseSelection(mesh);
	TriangleFill_SweepLine(mesh->grid(), mesh->selector().edges_begin(), mesh->selector().edges_end(), aPosition, aInt, &mesh->subset_handler());
	SelectSubset(mesh, -1, true, true, true, true);
	Retriangulate(mesh, m_degTri);
	SelectSubset(mesh, -1, true, true, true, true);
	si++;
	AssignSubset(mesh, si);
	mesh->subset_handler().subset_info(si).name = "Top Surface";
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh);
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step3.ugx");

	/// tetrahedralize
	SelectAll(mesh);
	mesh->subset_handler().set_default_subset_index(-1);
	Tetrahedralize(mesh->grid(), mesh->subset_handler(), m_degTet, false, false, aPosition, 1);
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// save volume grid to file
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step4.ugx");

	/// assign generated volumes
	mesh->selector().clear();
	base_coord = 0;

	for (std::vector<Layer>::const_iterator it = m_layers.begin(); it != m_layers.end(); ++it) {
		if (it->has_injection()) {
			/// TODO this is not quite good way to select it
			si = mesh->subset_handler().get_subset_index(it->name.c_str());
			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->thickness * it->injection->position);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->thickness * it->injection->position;
			/// AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->injection->thickness);

			ug::vector3 bottom2 = ug::vector3(m_centerInjection.x(), m_centerInjection.y(), m_centerInjection.z() + base_coord);
			ug::vector3 top_coord2 = ug::vector3(m_centerInjection.x(), m_centerInjection.y(), m_centerInjection.z() + base_coord + it->injection->thickness);

			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->injection->thickness;
			/// AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();

			////////////// TODO replace by correct coordinates not 10.85, -0.6 and 8.9 //////////
			Grid::VertexAttachmentAccessor<APosition> aaPos(mesh->grid(), aPosition);
			Selector sel(mesh->grid());

			/// the depot
			SelectRegion<Volume>(sel, ug::vector3(0, 0, 10.85), aaPos, IsNotInSubset(mesh->subset_handler(), -1));
			for (VolumeIterator vIter = sel.begin<Volume>(); vIter != sel.end<Volume>(); ++vIter) {
				Volume* v = *vIter;
				mesh->subset_handler().assign_subset(v, 102);
			}
			sel.clear();

			/// around depot
			SelectRegion<Volume>(sel, ug::vector3(-0.6, 0, 10.85), aaPos, IsNotInSubset(mesh->subset_handler(), -1));
			for (VolumeIterator vIter = sel.begin<Volume>(); vIter != sel.end<Volume>(); ++vIter) {
				Volume* v = *vIter;
				mesh->subset_handler().assign_subset(v, si);
			}
			sel.clear();

			SelectRegion<Volume>(sel, ug::vector3(-0.6, 0, 8.9), aaPos, IsNotInSubset(mesh->subset_handler(), -1));
			for (VolumeIterator vIter = sel.begin<Volume>(); vIter != sel.end<Volume>(); ++vIter) {
				Volume* v = *vIter;
				mesh->subset_handler().assign_subset(v, si);
			}
			sel.clear();

			SelectRegion<Volume>(sel, ug::vector3(0, 0, 8.9), aaPos, IsNotInSubset(mesh->subset_handler(), -1));
			for (VolumeIterator vIter = sel.begin<Volume>(); vIter != sel.end<Volume>(); ++vIter) {
				Volume* v = *vIter;
				mesh->subset_handler().assign_subset(v, si);
			}
			sel.clear();

			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position));
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			si = mesh->subset_handler().get_subset_index(it->name.c_str());
			base_coord = base_coord + (it->thickness - it->injection->thickness - it->thickness * it->injection->position);
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();
		} else {
			si = mesh->subset_handler().get_subset_index(it->name.c_str());
			bottom = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord);
			top_coord = ug::vector3(m_center.x(), m_center.y(), m_center.z() + base_coord + it->thickness);
			SelectElementsInCylinder<ug::Volume>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Face>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Edge>(mesh, bottom, top_coord, m_radius);
			SelectElementsInCylinder<ug::Vertex>(mesh, bottom, top_coord, m_radius);
			base_coord = base_coord + it->thickness;
			AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
			mesh->selector().clear();
		}
	}

	/// reassign boundary
	si++;
	SelectBoundaryFaces(mesh);
	SelectBoundaryVertices(mesh);
	SelectBoundaryEdges(mesh);
	AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);
	mesh->subset_handler().subset_info(si).name = "Surface";
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());

	/// cleanup mesh
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step5.ugx");

	/// reassign unassociated elements
	Grid& grid = mesh->grid();
	SubsetHandler& sh = mesh->subset_handler();
	sh.assign_subset(grid.vertices_begin(), grid.vertices_end(), -1);
	sh.assign_subset(grid.edges_begin(), grid.edges_end(), -1);

	for (int i = 0; i < sh.num_subsets(); ++i){
		CopySubsetIndicesToSides(sh, sh.begin<Face>(i), sh.end<Face>(i), true);
		CopySubsetIndicesToSides(sh, sh.begin<Edge>(i), sh.end<Edge>(i), true);
		CopySubsetIndicesToSides(sh, sh.begin<Vertex>(i), sh.end<Vertex>(i), true);
	}

	for (int i = 0; i < sh.num_subsets(); ++i){
		CopySubsetIndicesToSides(sh, sh.begin<Volume>(i), sh.end<Volume>(i), true);
		CopySubsetIndicesToSides(sh, sh.begin<Face>(i), sh.end<Face>(i), true);
		CopySubsetIndicesToSides(sh, sh.begin<Edge>(i), sh.end<Edge>(i), true);
		CopySubsetIndicesToSides(sh, sh.begin<Vertex>(i), sh.end<Vertex>(i), true);
	}

	/// cleanup and save
	EraseEmptySubsets(mesh->subset_handler());
	AssignSubsetColors(mesh->subset_handler());
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step6.ugx");

	/// reassign boundary faces
	SelectBoundaryFaces(mesh);
	SelectBoundaryVertices(mesh);
	SelectBoundaryEdges(mesh);
	AssignSelectionToSubset(mesh->selector(), mesh->subset_handler(), si);

	/// final grid to be saved
	SaveGridToFile(mesh->grid(), mesh->subset_handler(), "skin_layer_generator_step7.ugx");

	/// delete mesh
	delete mesh;
}



/////////////////////////////////////////////////////////
/// constants
/////////////////////////////////////////////////////////
const number SkinLayerGenerator::REMOVE_DOUBLES_THRESHOLD = 1e-8;
const number SkinLayerGenerator::SELECTION_THRESHOLD = 0.1;
const bool SkinLayerGenerator::FILL = false;
