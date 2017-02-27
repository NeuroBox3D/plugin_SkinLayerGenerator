/*!
 * \file plugins/skin_layer_generator/skin_layer_generator.h
 * \brief
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
			 * \brief
			 */
			SkinLayerGenerator() : m_center(ug::vector3(0, 0, 0)),
								   m_centerInjection(ug::vector3(0, 0, 0)),
								   m_radius(1), m_radiusInjection(0.5),
								   m_numVertices(10), m_numVerticesInjection(10),
								   m_degTri(30), m_degTet(18) {
			}

           	/*!
			 * \brief generate the skin layers,
			 * \fn generate
			 */
			void generate();

			/*!
			 * \brief add a skin layer with given parameters
			 * \param[in] name
			 * \param[in] thickness
			 * \param[in] resolution
			 */
			void add_layer(const std::string& name, number thickness, number resolution) {
				m_layers.push_back(Layer(thickness, name, resolution));
			}

			/*!
			 * \brief add a skin layer with injection with given parameters
			 * \param[in] name
			 * \param[in] thickness
			 * \param[in] resolution
			 * \param[in] name2
			 * \param[in] thickness2
			 * \param[in] resolution2
			 * \param[in] with_inner
			 * \param[in] position relative position in layer
			 */
			void add_layer_with_injection(const std::string& name, number thickness, number resolution,
		  							     const std::string& name2, number thickness2, number resolution2, number position, bool with_inner) {
				Layer l(thickness, name, resolution);
				l.add_injection(name2, thickness2, resolution2, position, with_inner);
				m_layers.push_back(l);
			}

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
			 * \brief Injection
			 */
			struct Injection {
				std::string name;
				number thickness;
				number resolution;
				number position;
				bool with_inner;
				/*!
				 * \brief
				 */
				Injection(const std::string& name, number thickness, number resolution, number position, bool with_inner) : name(name), thickness(thickness), resolution(resolution), position(position), with_inner(with_inner) {
				}

				/*!
				 * \brief
				 */
				bool with_inner_neumann_boundary() {
					return with_inner;
				}
			};

			/*!
			 * \brief Layer
			 */
			struct Layer {
				std::string name;
				number thickness;
				number resolution;
				SmartPtr<Injection> injection;

				/*!
				 * \brief
				 */
				Layer(number thickness, const std::string& name, number resolution) : name(name), thickness(thickness), resolution(resolution) {}

				/*!
				 * \brief
				 */
				Layer(number thickness, const std::string& name) : name(name), thickness(thickness), resolution(0.5) {}

				/*!
				 * \brief adds an injection / depot into a layer
				 */
				void add_injection(const std::string& name, number thickness, number resolution, number position, bool with_inner) {
					UG_COND_THROW(thickness > this->thickness, "Thickness of injection layer may not be greater than layer itself");
					UG_COND_THROW( (this->thickness * position + thickness) > this->thickness, "Dimensions of injection too big");
					injection = make_sp(new Injection(name, thickness, resolution, position, with_inner));
				}

				/*!
				 *
				 */
				bool has_injection() const {
					return injection.get() != NULL;
				}

				/*!
				 *
				 */
				SmartPtr<Injection> get_injection() const {
					return injection;
				}

			};

			/// skin layers
			std::vector<Layer> m_layers;

			/// grid generation constants
			static const number REMOVE_DOUBLES_THRESHOLD;
			static const number SELECTION_THRESHOLD;
			static const bool FILL;
		};
	}
}

#endif // __H__UG__SKIN_LAYER_GENERATOR__SKIN_LAYER_GENERATOR__
