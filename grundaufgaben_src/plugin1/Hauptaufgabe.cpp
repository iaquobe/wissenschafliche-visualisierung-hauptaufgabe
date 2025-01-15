#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fantom/algorithm.hpp>
#include <fantom/graphics/Texture.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/dataset.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <memory>
#include <vector>

using namespace fantom;

namespace
{
	class FastLIC : public DataAlgorithm {
		public:
		struct Options : public DataAlgorithm::Options
		{
			Options( fantom::Options::Control& control )
				: DataAlgorithm::Options( control )
			{
				add< Field< 2, Vector2 > >( "Field", "A 2D vector field", definedOn< Grid< 2 > >( Grid< 2 >::Points ) );
				add< double >("Step Size", "Threshold", 0.1);
				add< size_t >("Step Num", "Threshold", 10);
				add< size_t >("Min Hits", "Threshold", 2);
			}
		};


		struct DataOutputs : public DataAlgorithm::DataOutputs
		{
			DataOutputs( fantom::DataOutputs::Control& control )
				: DataAlgorithm::DataOutputs( control )
			{
				add<LineSet<3>>( "lines" );
			}
		};

		FastLIC( InitData& data )
			: DataAlgorithm( data )
		{
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override {
			// load options
			auto step_size	= options.get<double>("Step Size");
			auto steps			= options.get<size_t>("Step Num");
			auto min_hits		= options.get<size_t>("Min Hits");
			auto field			= options.get< Field< 2, Vector2 > >( "Field" );
			if(!field) return;

			// create buffer for hits and convolution

			// for each pixel p
			// 	if (numHits(p) < minNumHits) then
			// 		initiate stream line computation with x 0 = center of p
			// 		compute convolution I (x 0)
			// 		add result to pixel p
			// 		set m = 1
			// 		while m < some limit M
			// 			update convolution to obtain I (x m) and I (x -m)
			// 			add results to pixels containing x m and x -m
			// 			set m = m + 1
			// for each pixel p
			// 	normalize intensity according to numHits

		}
	};

	AlgorithmRegister< FastLIC >			dummy1( "Hauptaufgabe/FastLIC",			 "FastLIC implementation" );
} 
