/*!
 * \file plugins/skin_layer_generator/skin_layer_generator.h
 * \brief
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
								   m_radius(1), m_numVertices(10) {
					m_subsetNames = boost::assign::map_list_of
			    		  ("Epidermis layer", EPIDERMIS)
						  ("Subcutan layer", SUBCUTAN)
						  ("Inection layer", INJECTION)
						  ("Injection boundary", INJECTION_BOUNDARY)
						  ("Top surface of skin layer", SURFACE_TOP)
						  ("Bottom surface of skin layer", SURFACE_BOTTOM)
						  ("Left and right surface of skin layer", SURFACE_LEFT_RIGHT);
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
				EPIDERMIS,
				SUBCUTAN,
				INJECTION,
				INJECTION_BOUNDARY,
				SURFACE_TOP,
				SURFACE_BOTTOM,
				SURFACE_LEFT_RIGHT
			};

			/// grid generation parameters
			ug::vector3 m_center;
			number m_radius;
			size_t m_numVertices;

			/// grid generation constants
			static const number REMOVE_DOUBLES_THRESHOLD;
			static const bool FILL;
		};
	}
}

#endif // __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
