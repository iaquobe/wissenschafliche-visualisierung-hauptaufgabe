#include <cmath>
#include <cstddef>
#include <cstdio>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/DomainFactory.hpp>
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
	class TensorAlgorithm : public DataAlgorithm {
		public:
		struct Options : public DataAlgorithm::Options
		{
			Options( fantom::Options::Control& control )
				: DataAlgorithm::Options( control )
			{
				add< Field< 3, Matrix3 > >( "Field", "A 3D Tensor field", definedOn< Grid< 3 > >( Grid< 3 >::Cells ) );
				add< double >("Step Size", "Threshold", 0.1);
				add< size_t >("Step Num", "Number of steps", 50);

				add< Point3 >( "Position", "position of Plane", Point3(0, 0, 0) );

				add< bool >( "Major", "Toggle surface mode, where starting points are on defined surface", true );
				add< bool >( "Minor", "Toggle surface mode, where starting points are on defined surface", false );
				add< bool >( "Median", "Toggle surface mode, where starting points are on defined surface", false );
			}

		};


		struct DataOutputs : public DataAlgorithm::DataOutputs
		{
			DataOutputs( fantom::DataOutputs::Control& control )
				: DataAlgorithm::DataOutputs( control )
			{
				add<LineSet<3>>( "Major" );
				add<LineSet<3>>( "Median" );
				add<LineSet<3>>( "Minor" );
			}
		};

		TensorAlgorithm( InitData& data )
			: DataAlgorithm( data )
		{
		}

		virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override {
			// load options
			auto position		= options.get< Point3 >( "Position" );
			auto step_size	= options.get<double>("Step Size");
			auto steps			= options.get<size_t>("Step Num");
			auto field			= options.get< Field< 3, Matrix3 > >( "Field" );
			auto major	= options.get<bool>("Major");
			auto median	= options.get<bool>("Median");
			auto minor	= options.get<bool>("Minor");

			std::vector<Mode> modes;
			if (major)  modes.push_back(MAJOR);
			if (minor)  modes.push_back(MINOR);
			if (median) modes.push_back(MEDIAN);

			for (auto mode: modes){
				std::vector<Point3> points{};
				std::vector<std::vector<size_t>> indices{};
				auto evaluator = field->makeEvaluator();
				if(!evaluator->reset(position))
					break;

				// get eigenvectors 
				auto tensor = evaluator->value();
				Eigen::Matrix3d stress{
					{tensor[0], tensor[1], tensor[2]},
					{tensor[3], tensor[4], tensor[5]}, 
					{tensor[6], tensor[7], tensor[8]}};
				Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(stress);
				auto eigenvectors = solver.eigenvectors();
				auto eigenvalues  = solver.eigenvalues();

				// get major (yes this is ugly)
				auto index = get_index(mode, eigenvalues);
				if(index == -1) break;
				auto m  = eigenvectors.col(index);
				auto ev = Point3(m[0], m[1], m[2]);

				// perform step and decide between two direcitons
				// take step or -step depending on which is further from previous point
				auto step = ev * step_size; 
				auto last_point = position;

				// take step in one direction 
				auto point      = position + step;
				perform_steps(std::move(evaluator), mode, indices, points, last_point, point, steps, step_size, true);

				// take step in the other direction 
				evaluator = field->makeEvaluator();
				point     = position - step;
				perform_steps(std::move(evaluator), mode, indices, points, last_point, point, steps, step_size, false);

				// set resulting line
				auto lines = DomainFactory::makeLineSet(points, indices);
				switch(mode){
					case Mode::MAJOR:
						setResult( "Major", lines );
						break;
					case Mode::MEDIAN:
						setResult( "Median", lines );
						break;
					case Mode::MINOR:
						setResult( "Minor", lines );
						break;
				}
			}
		}

		enum Mode { MAJOR, MEDIAN, MINOR };
		int get_index(Mode mode, const Eigen::VectorXd& eigenvalues){
			if(mode == MAJOR) {
				if(eigenvalues[0] > eigenvalues[1] && eigenvalues[0] > eigenvalues[2])
					return 0;
				if(eigenvalues[1] > eigenvalues[0] && eigenvalues[1] > eigenvalues[2])
					return 1;
				if(eigenvalues[2] > eigenvalues[1] && eigenvalues[2] > eigenvalues[0])
					return 2;
			}
			if(mode == MEDIAN) {
				std::cout << "eigenvalues" << eigenvalues << std::endl;
				if( (eigenvalues[1] > eigenvalues[0] && eigenvalues[0] > eigenvalues[2])
					||(eigenvalues[2] > eigenvalues[0] && eigenvalues[0] > eigenvalues[1]))
					return 0;
				if( (eigenvalues[0] > eigenvalues[1] && eigenvalues[1] > eigenvalues[2])
					||(eigenvalues[2] > eigenvalues[1] && eigenvalues[1] > eigenvalues[0]))
					return 1;
				if( (eigenvalues[1] > eigenvalues[2] && eigenvalues[2] > eigenvalues[2])
					||(eigenvalues[2] > eigenvalues[2] && eigenvalues[2] > eigenvalues[1]))
					return 1;
			}
			if(mode == MINOR) {
				/*std::cout << "eigenvalues" << eigenvalues << std::endl;*/

				if(eigenvalues[0] < eigenvalues[1] && eigenvalues[0] < eigenvalues[2])
					return 0;
				if(eigenvalues[1] < eigenvalues[0] && eigenvalues[1] < eigenvalues[2])
					return 1;
				if(eigenvalues[2] < eigenvalues[1] && eigenvalues[2] < eigenvalues[0])
					return 2;
			}
			return -1;
		}

		void perform_steps(std::unique_ptr<FieldEvaluator<3, Matrix3>> evaluator,
						Mode mode, 
						std::vector<std::vector<size_t>> &indices, 
						std::vector<Point3> &points,
						Point3 last_point,
						Point3 point,
						size_t steps,
						double step_size, 
						bool forward){

			// push existing line 
			points.push_back(last_point);
			std::vector<size_t> line{points.size() - 1, points.size()}; 
			points.push_back(point);
			size_t i = 0;

			while(evaluator->reset(point)) {
				// get eigenvectors 
				auto tensor = evaluator->value();
				Eigen::Matrix3d stress{
					{tensor[0], tensor[1], tensor[2]},
					{tensor[3], tensor[4], tensor[5]}, 
					{tensor[6], tensor[7], tensor[8]}};
				Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> solver(stress);
				auto eigenvectors = solver.eigenvectors();
				auto eigenvalues  = solver.eigenvalues();

				// get major (yes this is ugly)
				auto index = get_index(mode, eigenvalues);
				if(index == -1) {
					break;
				}
				auto m  = eigenvectors.col(index);
				auto ev = Point3(m[0], m[1], m[2]);

				// perform step and decide between two direcitons
				// take step or -step depending on which is further from previous point
				auto step = ev * step_size; 
				if(fantom::norm(point + step - last_point) > fantom::norm(point - step - last_point)) {
					last_point = point;
					point = point + step; 
				} else {
					last_point = point;
					point = point - step; 
				}
				
				// add to line
				line.push_back(points.size()-1);
				line.push_back(points.size());
				points.push_back(point);

				// increase
				if (++i >= steps)
					break;
			}
			indices.push_back(line);
		}

	};
	AlgorithmRegister< TensorAlgorithm >			dummy1( "Hauptaufgabe/TensorAlgorithm",	"Tensor Visualization " );
} 
