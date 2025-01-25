#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>

#include <random>


using namespace fantom;
using namespace fantom::graphics;

namespace
{
    /**
     * The CustomDepthPeelingShader class implements a custom shader to render a single sphere with Depth Peeling.
     * Depth Peeling is an algorithm to render transparent objects in correct order. This class creates visual objects
     * which use the custom shader, the actual implementation of the Depth Peeling algorithm is done in the shader files
     * in the resources folder of this algorithm.
     *
     * @note
     * For further information on how Fantom implements Depth Peeling and how it works see the documentation for the
     * DepthPeeling class.
     */
    class CustomDepthPeelingShader : public VisAlgorithm
    {
    public:

        /**
         * VisOutputs defines the graphic outputs which can be toggled on and off in the interface.
         */
        struct VisOutputs;

        /**
         * Options defines the options that can be set for the algorithm in the options view.
         */
        struct Options;

        explicit CustomDepthPeelingShader( InitData& init );

    private:

        /**
         * Creates a DrawablePrimitive wrapped in a shared pointer rendering a sphere using a custom shader.
         * @param center A Vector3 defining the center of the sphere.
         * @param radius A float defining the radius of the sphere.
         * @param color A Color defining the color of the sphere.
         * @return A DrawablePrimitive.
         */
        static std::shared_ptr< Drawable >
        createSphere( fantom::Tensor< float, 3 > const& center, float radius, Color const& color );

        /**
         * Executes the algorithm and creates the all the geometry of the spheres based on the options set for the
         * algorithm.
         * @param options The options set for the algorithm.
         */
        void execute( Algorithm::Options const&, volatile bool const& ) override;
    };


    struct CustomDepthPeelingShader::VisOutputs : VisAlgorithm::VisOutputs
    {
        explicit VisOutputs( Control& control )
            : VisAlgorithm::VisOutputs( control )
        {
            addGraphics( "Geometry" );
        }
    };


    struct CustomDepthPeelingShader::Options : VisAlgorithm::Options
    {
        explicit Options( fantom::Options::Control& control )
            : fantom::Options( control )
        {
            add< InputSlider >( "Opacity", "Slider for geometry opacity", 4096, 2048 );
            add< unsigned int >( "Sphere count", "Number of spheres", 3 );
        }
    };


    AlgorithmRegister< CustomDepthPeelingShader > reg(
      "Tutorial/CustomDepthPeelingShader1",
      "An example on how to create a custom sphere shader which implements Depth Peeling." );
} // namespace


namespace
{
    CustomDepthPeelingShader::CustomDepthPeelingShader( InitData& init )
        : VisAlgorithm( init )
    {
    }

    void CustomDepthPeelingShader::execute( Algorithm::Options const& options, volatile bool const& )
    {
        auto in_opacity = options.get< InputSlider >( "Opacity" ) / 4096.0f;
        auto in_sphereCount = options.get< unsigned int >( "Sphere count" );

        // Initialize random number generation
        std::default_random_engine gen( std::random_device{}() );
        std::uniform_real_distribution< float > dist( 0.0f, 1.0f );

        // Create two transparent spheres
        std::vector< std::shared_ptr< Drawable > > drawables;
        drawables.reserve(in_sphereCount);

        for( size_t i = 0; i < in_sphereCount; ++i )
        {
            drawables.push_back( createSphere(
              { i * 2.0f, 0.0f, 0.0f },
              1.0f,
              { dist( gen ), dist( gen ), dist( gen ), in_opacity }
            ) );
        }

        setGraphics( "Geometry", makeCompound( drawables ) );
    }

    std::shared_ptr< Drawable >
    CustomDepthPeelingShader::createSphere( fantom::Tensor< float, 3 > const& center, float radius, Color const& color )
    {
        auto const& system = GraphicsSystem::instance();
        auto resPath = PluginRegistrationService::getInstance().getResourcePath( "general/Tutorial" );

        auto config = graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLE_STRIP }
          .vertexBuffer( "in_vertex",
                         system.makeBuffer( std::vector< Tensor< float, 2 > >{
                           { 1.0, -1.0 }, { -1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 } } ) )
          .boundingSphere( graphics::BoundingSphere( center, radius ) )
          .uniform( "u_center", center )
          .uniform( "u_radius", radius )
          .uniform( "u_color", color )
          .renderBin( color.a() < 1 ? graphics::RenderBin::Transparent : graphics::RenderBin::Opaque );

        return system.makePrimitive(
          std::move( config ),
          system.makeProgramFromFiles(
            resPath + "custom-depth-peeling-vert.glsl",
            resPath + "custom-depth-peeling-frag.glsl" ) );
    }
} // namespace
