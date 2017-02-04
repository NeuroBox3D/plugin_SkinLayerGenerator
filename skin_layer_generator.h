/*!
 * \file plugins/skin_layer_generator/skin_layer_generator.h
 * \brief
 *
 * TODO: make this an utilty class
 *
 *  Created on: January 30, 2017
 *      Author: Stephan Grein
 */

#ifndef __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
#define __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__

/// includes
#include <vector>
#include <string>
#include <algorithm>
#include "lib_grid/lib_grid.h"
#include <boost/assign/list_of.hpp>

namespace ug {
	namespace skin_layer_generator {
		/*!
		 * \brief SkinLayerGenerator
		 */
		class SkinLayerGenerator {
		public:
			/*!
			 * \brief
			 */
			SkinLayerGenerator() : m_center(ug::vector3(0, 0, 0)),
								   m_centerInjection(ug::vector3(0, 0, 0)),
								   m_injectionBase(1), m_injectionHeight(0.25),
								   m_numStepsExtrudeSubcutan(1), m_numStepsExtrudeEpidermis(1),
								   m_numStepsExtrudeInjection(1), m_epidermisThickness(0.5),
								   m_radius(1), m_radiusInjection(0.5),
								   m_numVertices(10), m_numVerticesInjection(10),
								   m_degTri(30), m_bWithInnerNeumannBoundary(false) {
					m_subsetNames["Epidermis layer"] = EPIDERMIS;
					m_subsetNames["Subcutan layer"] = SUBCUTAN;
					m_subsetNames["Injection layer"] = INJECTION;
					m_subsetNames["Injection boundary"] = INJECTION_BOUNDARY;
					m_subsetNames["Surface"] = SURFACE_ALL;
			}

           	/*!
			 * \brief generate the skin layer with given parameters
			 * \fn generate
			 */
			void generate();

		private:
			/// available subset names
			std::map<std::string, int> m_subsetNames;
			enum SUBSET_INDICES {
				SUBCUTAN,
				INJECTION,
				EPIDERMIS,
				SURFACE_ALL,
				INJECTION_BOUNDARY
			};

			/// grid generation parameters
			ug::vector3 m_center;
			ug::vector3 m_centerInjection;
			number m_injectionBase;
			number m_injectionHeight;
			number m_numStepsExtrudeSubcutan;
			number m_numStepsExtrudeEpidermis;
			number m_numStepsExtrudeInjection;
			number m_epidermisThickness;

			number m_radius;
			number m_radiusInjection;
			size_t m_numVertices;
			size_t m_numVerticesInjection;

			number m_degTri;

			bool m_bWithInnerNeumannBoundary;

			/// grid generation constants
			static const number REMOVE_DOUBLES_THRESHOLD;
			static const bool FILL;
		};
	}
}

#endif // __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
