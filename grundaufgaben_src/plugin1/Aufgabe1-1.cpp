#include <fantom/algorithm.hpp>
#include <fantom/dataset.hpp>
#include <fantom/register.hpp>

using namespace fantom;

namespace
{
	class StructuredGridAlgorithm : public DataAlgorithm
	{

		public:
			struct Options : public DataAlgorithm::Options
		{
			Options( fantom::Options::Control& control )
				: DataAlgorithm::Options( control )
			{
				add< long >( "nx", "", 10 );
				add< long >( "ny", "", 10 );
				add< long >( "nz", "", 10 );

				addSeparator();
				add< double >( "dx", "", 1.0 );
				add< double >( "dy", "", 1.0 );
				add< double >( "dz", "", 1.0 );
			}
		};

			struct DataOutputs : public DataAlgorithm::DataOutputs
		{
			DataOutputs( fantom::DataOutputs::Control& control )
				: DataAlgorithm::DataOutputs( control )
			{
				add< const Grid< 3 > >( "grid" );
			}
		};


			StructuredGridAlgorithm( InitData& data )
				: DataAlgorithm( data )
			{
			}

			virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
			{
				size_t extent[] = { (size_t)options.get< long >( "nx" ),
					(size_t)options.get< long >( "ny" ),
					(size_t)options.get< long >( "nz" ) };
				double origin[] = { -0.5 * options.get< double >( "dx" ) * ( options.get< long >( "nx" ) - 1 ),
					-0.5 * options.get< double >( "dy" ) * ( options.get< long >( "ny" ) - 1 ),
					-0.5 * options.get< double >( "dz" ) * ( options.get< long >( "nz" ) - 1 ) };
				double spacing[]
					= { options.get< double >( "dx" ), options.get< double >( "dy" ), options.get< double >( "dz" ) };

				std::shared_ptr< const Grid< 3 > > grid = DomainFactory::makeUniformGrid( extent, origin, spacing ); 
				setResult( "grid", grid );
			}
	};

	AlgorithmRegister< StructuredGridAlgorithm > dummy( "Grundaufgabe/Structured Grid", "Generates Structured Grid." );
}
