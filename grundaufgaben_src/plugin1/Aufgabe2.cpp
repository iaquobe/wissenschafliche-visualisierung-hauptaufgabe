#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/cells.hpp>
#include <fantom/dataset.hpp>
#include <fantom/datastructures/ValueArray.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/types.hpp>
#include <fantom/math.hpp>
#include <fantom/register.hpp>
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>
#include <memory>
#include <vector>

using namespace fantom;
using namespace std;

namespace
{
	class ShowGridCustom : public VisAlgorithm
	{
		public:

		struct Options : public VisAlgorithm::Options {
			Options(Options::Control& control) 
				: VisAlgorithm::Options(control) {
				add< Grid< 3 >>("Grid", "grid to visualize");
				add< Color >("Color", "color of lines/surface", Color(0, 1, 0));

				add< bool >( "Surface Mode", "Toggle Surface Mode", false );
				add< bool >( "Index Mode", "Toggle Single Cell Mode", false );
				add< size_t >( "Cell Index", "An invisible option.", 0 );
				setEnabled( "Cell Index", false );
			}

			void optionChanged( const std::string& name )
			{
				if( name == "Index Mode" )
				{
					bool value = get< bool >("Index Mode");
					setEnabled( "Cell Index", value );
				}
			}
		};

		struct VisOutputs : public VisAlgorithm::VisOutputs {
			// These are the graphic outputs which can be toggled on and off in the interface.
			VisOutputs( fantom::VisOutputs::Control& control )
				: VisAlgorithm::VisOutputs( control )
			{
				addGraphics( "Grid" );
			}
		};

			ShowGridCustom( InitData& data )
				: VisAlgorithm( data )
			{
			}

			virtual void execute( const Algorithm::Options& options, const volatile bool& /*abortFlag*/ ) override
			{
				auto surface_mode = options.get<bool>("Surface Mode");
				auto color = options.get<Color>("Color");
				auto grid = options.get<Grid<3>>("Grid"); 
				if(!grid)
					return;
				auto index_mode = options.get<bool>("Index Mode");
				size_t cell_index;
				if(index_mode)
					cell_index = options.get<size_t>("Cell Index");

				// transform all points to pointf
				std::vector<PointF<3>> vertices;
				auto& points = grid->points(); 
				for(size_t i = 0; i < points.size(); i++){ 
					PointF<3> pf(points[i]);
					vertices.push_back(pf);
				}

				//read all indices from cells
				std::vector<uint32_t> indices;
				if(index_mode){
					if(cell_index < grid->numCells()){
						Cell cell = grid->cell(cell_index);
						// split cell into faces, is either quad or triangle
						for(size_t f = 0; f < cell.numFaces(); f++){
							Cell face = cell.face(f);
							add_indices(indices, face, surface_mode);
						}
					}
				} else {
					for(size_t i = 0; i < grid->numCells(); i++){
						Cell cell = grid->cell(i);
						// split cell into faces, is either quad or triangle
						for(size_t f = 0; f < cell.numFaces(); f++){
							Cell face = cell.face(f);
							add_indices(indices, face, surface_mode);
						}
					}

				}


				auto const& system = graphics::GraphicsSystem::instance();
				std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
				auto bs = graphics::computeBoundingSphere(vertices);
				std::shared_ptr< graphics::Drawable > geometryDrawable;

				if(surface_mode) { 
					auto norm = graphics::computeNormals(vertices, indices);

					geometryDrawable
						= system.makePrimitive( graphics::PrimitiveConfig{graphics::RenderPrimitives::TRIANGLES}
								.vertexBuffer("position", system.makeBuffer(vertices))
								.vertexBuffer("normal", system.makeBuffer(norm))
								.indexBuffer(system.makeIndexBuffer(indices))
								.uniform("color", color)
								.boundingSphere(bs),
								system.makeProgramFromFiles( resourcePath + "shader/surface/basic/singleColor/vertex.glsl",
																						 resourcePath + "shader/surface/basic/singleColor/fragment.glsl") );
				} 
				else {
					geometryDrawable = system.makePrimitive( graphics::PrimitiveConfig{graphics::RenderPrimitives::LINES}
								.vertexBuffer( "in_vertex", system.makeBuffer(vertices))
								.indexBuffer(system.makeIndexBuffer(indices))
								.uniform("u_lineWidth", 1.0f)
								.uniform("u_color", color)
								.boundingSphere(bs),
								system.makeProgramFromFiles( resourcePath + "shader/line/noShading/singleColor/vertex.glsl",
																						 resourcePath + "shader/line/noShading/singleColor/fragment.glsl",
																						 resourcePath + "shader/line/noShading/singleColor/geometry.glsl" ) );
				}
				setGraphics("Grid", geometryDrawable);
			}

			void add_indices(vector<uint32_t>& indices, Cell& face, bool surface_mode){
				if(!surface_mode){
					// LINE MODE
					switch (face.type()) {
						case fantom::Cell::TRIANGLE:
							indices.push_back(face.index(0));
							indices.push_back(face.index(1));

							indices.push_back(face.index(1));
							indices.push_back(face.index(2));

							indices.push_back(face.index(2));
							indices.push_back(face.index(0));
							break;

						case fantom::Cell::QUAD:
							indices.push_back(face.index(0));
							indices.push_back(face.index(1));

							indices.push_back(face.index(1));
							indices.push_back(face.index(2));

							indices.push_back(face.index(2));
							indices.push_back(face.index(3));

							indices.push_back(face.index(3));
							indices.push_back(face.index(0));
							break;
						default:
							break;
					}
				} else {
					// SURFACE MODE
					switch (face.type()) {
						case fantom::Cell::TRIANGLE:
							indices.push_back(face.index(0));
							indices.push_back(face.index(2));
							indices.push_back(face.index(1));
							break;

						case fantom::Cell::QUAD:
							indices.push_back(face.index(0));
							indices.push_back(face.index(2));
							indices.push_back(face.index(1));

							indices.push_back(face.index(0));
							indices.push_back(face.index(3));
							indices.push_back(face.index(2));
							break;
						default:
							break;
					}
				}
			}

	};

	AlgorithmRegister< ShowGridCustom > dummy( "Grundaufgabe/Show Grid", "Shows Grid." );
}
