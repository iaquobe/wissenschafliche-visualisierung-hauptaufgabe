
#include <fantom/algorithm.hpp>
#include <fantom/cells.hpp>
#include <fantom/dataset.hpp>
#include <fantom/datastructures/ValueArray.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/types.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <memory>
#include <utility>
#include <vector>

using namespace fantom;
using namespace std;

namespace
{
	class UnstructuredGridAlgorithm : public DataAlgorithm
	{

		public:
			struct DataOutputs : public DataAlgorithm::DataOutputs
		{
			DataOutputs( fantom::DataOutputs::Control& control )
				: DataAlgorithm::DataOutputs( control )
			{
				add< const Grid< 3 > >( "grid" );
			}
		};


			UnstructuredGridAlgorithm( InitData& data )
				: DataAlgorithm( data )
			{
			}

			virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
			{

				/*vector<Point3> vertices({{0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1}}); */

				vector<Point3> vertices({{0,-1, 0}, {0, 0, 0}, {0, 1, 0}, 
																 {1,-1, 0}, {1, 0, 0}, {1, 1, 0},
																 {0, 0, 1}, {0, 1, 1},
																 {1, 0, 1}, {1, 1, 1},
																 {0.5, 1.5, 0.5}});

				vector<pair<Cell::Type, size_t>> cell_type_num = {make_pair(Cell::Type::PRISM, 1),
												make_pair(Cell::Type::HEXAHEDRON, 1),
												make_pair(Cell::Type::PYRAMID, 1)};

				vector<size_t> cells({0, 6, 1, 4, 8, 3, 
						1, 4, 8, 6, 7, 9, 5, 2, 
						2, 5, 9, 7, 10});					

				shared_ptr< const Grid< 3 > > grid = DomainFactory::makeGrid(vertices, cell_type_num.size(), cell_type_num.data(), cells);
				setResult( "grid", grid );
			}
	};

	AlgorithmRegister< UnstructuredGridAlgorithm > dummy( "Grundaufgabe/Unstructured Grid", "Generates Grid." );
}
