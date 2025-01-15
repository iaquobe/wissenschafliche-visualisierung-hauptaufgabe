#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fantom/algorithm.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom/graphics.hpp>
#include <fantom/dataset.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <memory>
#include <vector>
#include <Eigen/Dense>

using namespace fantom;

namespace
{
	class Integrator : public DataAlgorithm {
		public:
		struct Options : public DataAlgorithm::Options
		{
			Options( fantom::Options::Control& control )
				: DataAlgorithm::Options( control )
			{
				add< Field< 3, Vector3 > >( "Field", "A 3D Tensor field", definedOn< Grid< 3 > >( Grid< 3 >::Cells ) );
				add< double >("Step Size", "Threshold", 0.1);
				add< size_t >("Step Num", "Radius of Spheres", 50);

				add< bool >( "Surface Mode", "Toggle surface mode, where starting points are on defined surface", false );

				add< size_t >( "Point Number", "An invisible option.", 10 );
				add< Point3 >( "Position", "position of Plane", Point3(0, 0, 0) );
				add< Vector3 >( "Vector1", "vector 1 of plane", Vector3(1, 0, 0) );
				add< Vector3 >( "Vector2", "vector 1 of plane", Vector3(0, 1, 0) );
				setEnabled( "Point Number", false );
				setEnabled( "Position", false );
				setEnabled( "Vector1", false );
				setEnabled( "Vector2", false );
			}

			void optionChanged( const std::string& name )
			{
				if( name == "Surface Mode" )
				{
					bool value = get< bool >("Surface Mode");
					setEnabled( "Point Number", value );
					setEnabled( "Position", value );
					setEnabled( "Vector1", value );
					setEnabled( "Vector2", value );
				}
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

		Integrator( InitData& data )
			: DataAlgorithm( data )
		{
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override {
			// load options
			auto step_size	= options.get<double>("Step Size");
			auto steps			= options.get<size_t>("Step Num");
			auto field			= options.get< Field< 3, Matrix3 > >( "Field" );
			auto function		= options.get< Function< Matrix3 > >( "Field" );
			if(!field) return;



			// switch between surface mode and grid mode
			std::vector<Point3> starting_points; 
			auto surface_mode = options.get<bool>("Surface Mode");
			if(surface_mode){
				size_t point_number	= options.get< size_t >( "Point Number");
				Point3 position			= options.get< Point3 >( "Position" );
				Vector3 vec1				= options.get< Vector3 >( "Vector1" );
				Vector3 vec2 				= options.get< Vector3 >( "Vector2" );

				size_t max = std::sqrt(point_number);
				for (size_t i = 0; i < max; i++) {
					for (size_t j = 0; j < max; j++) {
						Point3 starting_point = position + (j * vec1 / max) + (i * vec2 / max); 
						starting_points.push_back(starting_point);
					}
				}
			}
			else {
				// cast to grid to get the points and get evaluator
				std::shared_ptr<const Grid<3>> grid = 
					std::dynamic_pointer_cast<const Grid<3>>(function->domain());
				for (size_t i = 0; i < grid->points().size(); i++)
					starting_points.push_back(grid->points()[i]);
			}


			// linesSet outputs
			std::vector<Point3> points;
			std::vector<std::vector<size_t>> indices; 
			
			// perform steps for each point in the grid 
			for (auto point : starting_points) {
				auto evaluator = field->makeEvaluator();
				perform_steps(std::move(evaluator), points, indices, point, steps, step_size);
			}

			auto lineSet = DomainFactory::makeLineSet(points, indices);
			setResult( "lines", lineSet );
		}

		virtual void perform_steps(
				std::unique_ptr<FieldEvaluator<3, Tensor<double, 3, 3>>> evaluator, 
				std::vector<Point3> &points,
				std::vector<std::vector<size_t>> &indices,
				Point3 point,
				size_t steps,
				double step_size) = 0;

	};

	class TensorAlgorithm : public Integrator {
		public: 
		TensorAlgorithm( InitData& data ) : Integrator( data ) { }

		void perform_steps(std::unique_ptr<FieldEvaluator<3, Tensor<double,3, 3>>> evaluator,
				std::vector<Point3> &points,
				std::vector<std::vector<size_t>> &indices,
				Point3 point,
				size_t steps,
				double step_size) 
		{
			points.push_back(point);
			std::vector<size_t> line_indices; 

			// perform `steps` steps
			for (size_t s = 0; s < steps; s++) { 
				if (evaluator->reset(point)){
					// go into direction
					auto tensor = evaluator->value();
					Eigen::Matrix3d stress(tensor);
					Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(stress);
					auto eigenvectors = solver.eigenvectors();

					/*point		 = tensor * step_size + point;*/

					// add to output vectors
					line_indices.push_back(points.size() - 1);
					line_indices.push_back(points.size());
					points.push_back(point);
				}
			}

			// add line to results
			indices.push_back(line_indices);
		}
	};

	AlgorithmRegister< TensorAlgorithm >			dummy1( "Hauptaufgabe/TensorAlgorithm",	"Tensor Visualization " );
} 
