/*!
 * \file plugins/skin_layer_generator/unit_tests/src/tests.cpp
 * \brief registry for SkinLayerGenerator
 */

/// includes
#include <string>
#include <map>
#include <vector>
#include "skin_layer_generator.h"
#include <bridge/util.h>
#include <bridge/util_domain_dependent.h>
#include <common/error.h>
#include <common/ug_config.h>
#include <common/log.h>
#include <registry/registry.h>
#include <registry/error.h>
#include <lib_disc/domain.h>
#include <lib_grid/lib_grid.h>

namespace ug {
	namespace skin_layer_generator {
		/// usings
		using namespace ug::bridge;
		using namespace std;

		/*!
		 * \brief functionality which should be registered
		 */
		struct Functionality {
			static void Common(Registry& reg, string& parentGroup) {
				// group membership
				std::string grp("/UG4/Plugins/Neuro/SkinLayerGenerator/");

				// typedefs
				typedef skin_layer_generator::SkinLayerGenerator TSLG;

				/// registry of SkinLayerGenerator
				reg.add_class_<TSLG>("SkinLayerGenerator", grp)
						.add_constructor<void (*)()>("")
						.add_method("generate", (void (TSLG::*)())(&TSLG::generate), "", "", "generate the mesh", "")
						.add_method("add_layer", (void (TSLG::*)(number, number, const std::string&))(&TSLG::add_layer), "", "", "add skin layer", "")
						.add_method("add_layer_with_injection", (void (TSLG::*)(number, number, const std::string&, const std::string&, number, number, number))(&TSLG::add_layer_with_injection), "", "", "add skin layer with injection", "");
			}
		};
	}

	/*!
	 * \brief initializing routine for plugin
	 */
	extern "C" void
	InitUGPlugin_SkinLayerGenerator(bridge::Registry& reg, std::string& parentGroup) {
		std::string grp("/UG4/Plugins/Neuro/SkinLayerGenerator/");
		typedef skin_layer_generator::Functionality Functionality;
		try {
			bridge::RegisterCommon<Functionality>(reg, grp);
		} UG_REGISTRY_CATCH_THROW(grp);
	}
}
