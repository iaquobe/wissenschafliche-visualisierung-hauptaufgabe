#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <random>

// needed for BoundinSphere-Calculation and normal calculation
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>
#include <fantom-plugins/utils/Graphics/ObjectRenderer.hpp>

using namespace fantom;

namespace
{

class GraphicsTutorialAlgorithm : public VisAlgorithm
{

public:
    struct VisOutputs : public VisAlgorithm::VisOutputs
    {
        // These are the graphic outputs which can be toggled on and off in the interface.
        VisOutputs( fantom::VisOutputs::Control& control )
            : VisAlgorithm::VisOutputs( control )
        {
            addGraphics( "textureDrawable" );            
        }
    };

    GraphicsTutorialAlgorithm( InitData& data )
        : VisAlgorithm( data )
    {
    }

    virtual void execute( const Algorithm::Options& /*options*/, const volatile bool& /*abortFlag*/ ) override
    {
        auto const& system = graphics::GraphicsSystem::instance();

        std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
        std::string resourcePathLocal
                = PluginRegistrationService::getInstance().getResourcePath( "general/Tutorial" );

        std::shared_ptr< graphics::Texture2D > texture2D
                = system.makeTextureFromFile( resourcePathLocal + "PerlinNoise.png", graphics::ColorChannel::RGBA );


        // Updated to use 2D texture only
        std::vector< PointF< 3 > > verticesTex( 4 );
        verticesTex[0] = PointF< 3 >( -0.5, -0.5, 0.0 );
        verticesTex[1] = PointF< 3 >( 0.5, -0.5, 0.0 );
        verticesTex[2] = PointF< 3 >( -0.5, 0.5, 0.0 );
        verticesTex[3] = PointF< 3 >( 0.5, 0.5, 0.0 );

        std::vector< PointF< 2 > > texCoords( 4 );
        texCoords[0] = PointF< 2 >( 0.0, 0.0 );
        texCoords[1] = PointF< 2 >( 1.0, 0.0 );
        texCoords[2] = PointF< 2 >( 0.0, 1.0 );
        texCoords[3] = PointF< 2 >( 1.0, 1.0 );

        std::vector< unsigned int > indicesTex( 6 );
        indicesTex[0] = 0;
        indicesTex[1] = 1;
        indicesTex[2] = 2;
        indicesTex[3] = 2;
        indicesTex[4] = 1;
        indicesTex[5] = 3;

        std::shared_ptr< graphics::ShaderProgram > LICshader = system.makeProgramFromFiles( resourcePathLocal + "tex-vertex.glsl",resourcePathLocal + "LIC-fragment.glsl");
        std::shared_ptr<graphics::Texture2D> firstPassTexture =
            system.makeTexture(Size2D(texture2D->width(), texture2D->height()), graphics::ColorChannel::RGBA);

        auto bs = graphics::computeBoundingSphere( verticesTex );
        std::shared_ptr< graphics::Drawable > texture
                = system.makePrimitive( graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                                        .vertexBuffer( "position", system.makeBuffer( verticesTex ) )
                                        .vertexBuffer( "texCoords", system.makeBuffer( texCoords ) )
                                        .indexBuffer( system.makeIndexBuffer( indicesTex ) )
                                        .texture( "inTexture", texture2D )
                                        .boundingSphere( bs ),
                                        LICshader );


        auto frameBuffer = system.makeFrameBuffer(firstPassTexture->size());
        frameBuffer->colorAttachment("inTexture", firstPassTexture);
        //texture->draw(graphics::RenderTarget::);

        bs = graphics::computeBoundingSphere( verticesTex );
        std::shared_ptr< graphics::Drawable > textureDrawable
                = system.makePrimitive( graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                                        .vertexBuffer( "position", system.makeBuffer( verticesTex ) )
                                        .vertexBuffer( "texCoords", system.makeBuffer( texCoords ) )
                                        .indexBuffer( system.makeIndexBuffer( indicesTex ) )
                                        .texture( "inTexture", texture2D )
                                        .boundingSphere( bs ),
                                        system.makeProgramFromFiles( resourcePathLocal + "tex-vertex.glsl",
                                                                     resourcePathLocal + "tex-fragment.glsl" ) );
        setGraphics( "textureDrawable", textureDrawable );
    }    

};
AlgorithmRegister< GraphicsTutorialAlgorithm > dummy( "MyAlgorithms/Hauptaufgabe2", "Show some example graphics." );
} // namespace
