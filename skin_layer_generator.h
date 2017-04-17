/*!
 * \file plugins/skin_layer_generator/skin_layer_generator.h
 * \brief Generates a vertical column of Skinlayers
 *
 *  Created on: January 30, 2017
 *      Author: Stephan Grein
 */
#ifndef __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
#define __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__

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
			 * \brief default ctor
			 */
			SkinLayerGenerator() : m_center(ug::vector3(0, 0, 0)),
								   m_centerInjection(ug::vector3(0, 0, 0)),
								   m_radius(1), m_radiusInjection(0.5),
								   m_numVertices(10), m_numVerticesInjection(10),
								   m_degTri(30), m_degTet(18),
								   m_bStraightenSubsetNamesForLua(false) {
			}

           	/*!
			 * \brief generate the skin layer column out of added layers
			 *
			 * \fn generate
			 */
			void generate();

			/*!
			 * \brief add a skin layer with given parameters
			 *
			 * \param[in] name layer's name
			 * \param[in] thickness layer's thickness
			 * \param[in] resolution layer's resolution
			 */
			void add_layer(const std::string& name, number thickness, number resolution);

			/*!
			 * \brief add a skin layer with injection with given parameters
			 *
			 * \param[in] layerName layer's name
			 * \param[in] thicknessLayer layer's thickness
			 * \param[in] resLayer resolution of surrounding layer
			 * \param[in] injectionName name of injection
			 * \param[in] thicknessInjection thickness of injection
			 * \param[in] resInjection resolution of injection
			 * \param[in] relPosition relative position in layer
			 */
			void add_layer_with_injection(const std::string& layerName, number thicknessLayer,
									     number resLayer, const std::string& injectionName,
									     number thicknessInjection, number resInjection,
									     number relPosition);

			/*!
			 * \brief returns the number of injection sides
			 */
			size_t number_of_injections() const;

			/*!
			 * \brief enables straightening of subset names for Lua
			 * \param[in] straighten
			 */
			void set_straighten_subset_names_for_lua(bool straighten);

			/*!
			 * \brief check if straighening of subset names for Lua enabled
			 */
			bool is_straighten_subset_names_for_lua() const;


		private:
			/// grid generation parameters
			ug::vector3 m_center;
			ug::vector3 m_centerInjection;

			number m_radius;
			number m_radiusInjection;

			size_t m_numVertices;
			size_t m_numVerticesInjection;

			number m_degTri;
			number m_degTet;

			/*!
			 * \brief encapsulates all injection parameters
			 */
			struct Injection {
				std::string name;
				number thickness;
				number resolution;
				number position;

				/*!
				 * \brief constructs an injection in a given layer
				 *
				 * \param[in] name injection's name
				 * \param[in] thickness injection's thickness
				 * \param[in] resolution injection's resolution
				 * \param[in] relPosition injection's relative position
				 */
				Injection(const std::string& name, number thickness,
						  number resolution, number relPosition) :
						name(name), thickness(thickness),
						resolution(resolution),
						position(relPosition) {
				}
			};

			/*!
			 * \brief encapsulates all layer parameters
			 */
			struct Layer {
				std::string name;
				number thickness;
				number resolution;
				SmartPtr<Injection> injection;

				/*!
				 * \brief construct a layer with given resolution
				 *
				 * \param[in] thickness
				 * \param[in] name
				 * \param[in] resolution
				 */
				Layer(number thickness, const std::string& name, number resolution) :
					name(name), thickness(thickness), resolution(resolution) {
				}

				/*!
				 * \brief construct a layer
				 *
				 * \param[in] thickness
				 * \param[in] name
				 */
				Layer(number thickness, const std::string& name) :
					name(name),
					thickness(thickness),
					resolution(0.5) {
				}

				/*!
				 * \brief adds an injection to a given layer
				 *
				 * \param[in] name injection's name
				 * \param[in] thicness injection's thickness
				 * \param[in] resolution injection's resolution
				 * \param[in] relPosition injection's relative position in layer
				 */
				void add_injection(const std::string& name, number thickness,
								   number resolution, number relPosition) {
					UG_COND_THROW(thickness > this->thickness, "Thickness of"
							" injection layer may not be greater than layer itself");
					UG_COND_THROW( (this->thickness * relPosition + thickness) > this->thickness,
							"Dimensions of injection too big");
					injection = make_sp(new Injection(name, thickness, resolution, relPosition));
				}

				/*!
				 * \brief check if injection exists
				 */
				bool has_injection() const {
					return injection.get() != NULL;
				}

				/*!
				 * \brief return the injection member
				 */
				SmartPtr<Injection> get_injection() const {
					return injection;
				}

			};

			/// skin layers
			std::vector<Layer> m_layers;

			/// grid generation constants
			static const number SELECTION_THRESHOLD;

			/// output parameters
			bool m_bStraightenSubsetNamesForLua;
		};
	}
}

#endif // __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
