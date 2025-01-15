#include <cstddef>
#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>

// needed for BoundinSphere-Calculation and normal calculation
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>

using namespace fantom;

namespace
{

	class GraphicsTutorialAlgorithm : public VisAlgorithm
	{

		public:
		struct VisOutputs : public VisAlgorithm::VisOutputs {
			VisOutputs( fantom::VisOutputs::Control& control )
				: VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "spheres" );
			}
		};

		struct Options : public VisAlgorithm::Options {
			Options(Options::Control& control) 
				: VisAlgorithm::Options(control) {
				add<Field<3, Scalar>>( "Field", "Scalar field");
				add< Color >("Color", "color of lines/surface", Color(0, 1, 0));
				add< double >("Threshold", "Threshold", 0.1);
				add< double >("Radius", "Radius of Spheres", 0.1);
			}
		};

		GraphicsTutorialAlgorithm( InitData& data )
			: VisAlgorithm( data )
		{
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
		{
			// load options
			auto threshold = options.get<double>("Threshold");
			auto radius = options.get<double>("Radius");
			auto color = options.get<Color>("Color");
			auto function = options.get<Function<Scalar>>("Field");
			if(!function) return;

			// renderer
			auto const& system = graphics::GraphicsSystem::instance();
			std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
			auto ObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );

			//grid
			auto grid = std::dynamic_pointer_cast< const Grid< 3 > >( function->domain() );
			if( !grid ) throw std::logic_error( "Wrong type of grid!" );
			auto evaluator = function->makeDiscreteEvaluator();

			// loop through values
			for(size_t i = 0; i < evaluator->numValues(); i++){
				auto v = evaluator->value(i);
				if(v >= Scalar(threshold)){
					auto point = grid->points()[i];
					ObjectRenderer->addSphere( point, radius, color);
				}
			}

			setGraphics( "spheres", ObjectRenderer->commit() );
		}
	};

	AlgorithmRegister< GraphicsTutorialAlgorithm > dummy( "Grundaufgabe/Show Scalar", "Show scalar value over threshold" );
} 
