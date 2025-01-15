#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
 
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
                addGraphics( "x" );
                addGraphics( "y" );
                addGraphics( "z" );
                addGraphics( "ellipsoid" );
                addGraphics( "manySpheres" );
                addGraphics( "text" );
                addGraphics( "simpleDrawable" );
                addGraphics( "geometryDrawable" );
                addGraphics( "textureDrawable" );
            }
        };
 
        GraphicsTutorialAlgorithm( InitData& data )
            : VisAlgorithm( data )
        {
        }
 
        virtual void execute( const Algorithm::Options& /*options*/, const volatile bool& /*abortFlag*/ ) override
        {
            // example high-level primitive interface
            // The GraphicsSystem is needed to create Drawables, which represent the to be rendererd objects.
            auto const& system = graphics::GraphicsSystem::instance();
 
            // This is the path to the general resources folder, where you can find the shaders.
            std::string resourcePath = PluginRegistrationService::getInstance().getResourcePath( "utils/Graphics" );
 
            // It is also possible to address a "resources" folder within a plugin.
            // This method is used if the shaders are plugin specific and not used otherwise;
            std::string resourcePathLocal
                = PluginRegistrationService::getInstance().getResourcePath( "general/Tutorial" );
 
            //------------------------------------example high-level primitive interface---------------------------
            // The objectRenderer allows to conveniently render high-level primitves.
            auto xObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            auto yObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            auto zObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            auto eObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
 
            // Possible primitives are:
            // - volume and non-volume arrows
            xObjectRenderer->addArrow(
                Point3( 0.75, 0.0, 0.0 ), Vector3( 0.75, 0.0, 0.0 ), 0.2, Color( 1.0, 0.0, 0.0, 1.0 ), true );
            yObjectRenderer->addArrow(
                Point3( 0.0, 0.75, 0.0 ), Vector3( 0.0, 0.75, 0.0 ), 0.2, Color( 0.0, 1.0, 0.0, 1.0 ), false );
            // - cylinders and cones
            zObjectRenderer->addCylinder(
                Point3( 0.0, 0.0, 0.75 ), Vector3( 0.0, 0.0, 0.5 ), 0.1, Color( 0.0, 0.0, 1.0, 1.0 ) );
            zObjectRenderer->addCone(
                Point3( 0.0, 0.0, 1.0 ), Vector3( 0.0, 0.0, 0.25 ), 0.0, 0.2, Color( 0.0, 0.0, 1.0, 1.0 ) );
            // - spheres and ellipsoids
            xObjectRenderer->addSphere( Point3( -1.0, 0.0, 0.0 ), 0.25, Color( 1.0, 0.0, 0.0, 1.0 ) );
            yObjectRenderer->addSphere( Point3( 0.0, -1.0, 0.0 ), 0.25, Color( 0.0, 1.0, 0.0, 1.0 ) );
            zObjectRenderer->addSphere( Point3( 0.0, 0.0, -1.0 ), 0.25, Color( 0.0, 0.0, 1.0, 1.0 ) );
            eObjectRenderer->addEllipsoid( PointF< 3 >( 0.0, 0.0, 0.0 ),
                                           Tensor< float, 3 >( 0.6, 0.0, 0.0 ),
                                           Tensor< float, 3 >( 0.0, 0.3, 0.0 ),
                                           Tensor< float, 3 >( 0.0, 0.0, 0.1 ),
                                           Color( 1.0, 1.0, 0.0 ) );
 
            // You have to assign the objects to the graphic outputs.
            setGraphics( "x", xObjectRenderer->commit() );
            setGraphics( "y", yObjectRenderer->commit() );
            setGraphics( "z", zObjectRenderer->commit() );
            setGraphics( "ellipsoid", eObjectRenderer->commit() );
 
 
            // Optional: use ObjectRenderer::reserve(...) for an increase in performance if you draw large number of
            // objects
            auto performanceObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            performanceObjectRenderer->reserve( graphics::ObjectRenderer::ObjectType::SPHERE, 18 );
 
            performanceObjectRenderer->addSphere( Point3( 2.0, 0.0, 0.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, 2.0, 0.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
            performanceObjectRenderer->addSphere( Point3( -2.0, 0.0, 0.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, -2.0, 0.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, 0.0, 2.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, 0.0, -2.0 ), 0.25, Color( 0.6, 0.0, 0.0 ) );
 
            performanceObjectRenderer->addSphere( Point3( 1.0, 1.0, 0.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( -1.0, 1.0, 0.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( 1.0, -1.0, 0.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( -1.0, -1.0, 0.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
 
            performanceObjectRenderer->addSphere( Point3( 0.0, 1.0, 1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, -1.0, 1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, 1.0, -1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( 0.0, -1.0, -1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
 
            performanceObjectRenderer->addSphere( Point3( 1.0, 0.0, 1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( -1.0, 0.0, 1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( 1.0, 0.0, -1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
            performanceObjectRenderer->addSphere( Point3( -1.0, 0.0, -1.0 ), 0.25, Color( 0.4, 0.4, 0.7 ) );
 
            setGraphics( "manySpheres", performanceObjectRenderer->commit() );
 
 
            //--------------------------------------------example textlabel---------------------------------------
            // We simply use the objectRenderer again to draw the TextLabels.
            auto tObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
            tObjectRenderer->addTextLabel( Point3( 0.0, 1.0, 0.0 ), "Hello World !", 30., Color( 1.0, 0.0, 0.0 ) );
            setGraphics( "text", tObjectRenderer->commit() );
 
 
            //-----------------------------------example low-level primitive interface----------------------------
            // With vertex and fragment shader:
 
            std::vector< PointF< 3 > > tri( 3 );
            tri[0] = PointF< 3 >( 0.0, 0.0, 0.0 );
            tri[1] = PointF< 3 >( 1.0, 2.0, 0.3 );
            tri[2] = PointF< 3 >( 0.0, -1.0, -0.3 );
 
            // The BoundingSphere should contain all elements of the drawable and is needed fot its creation.
            auto bs = graphics::computeBoundingSphere( tri );
 
            // For the used Phong-Shading, the calculation of surface normals is necessary.
            std::vector< unsigned int > indices = { 0, 1, 2 };
            auto norm = graphics::computeNormals( tri, indices );
 
            // The Drawable object defines the input streams for the shaders.
            // Vertex- and IndexBuffers as well as Uniforms can be defined as seen below.
            std::shared_ptr< graphics::Drawable > simpleDrawable = system.makePrimitive(
                graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                    .vertexBuffer( "position", system.makeBuffer( tri ) )
                    .vertexBuffer( "normal", system.makeBuffer( norm ) )
                    .indexBuffer( system.makeIndexBuffer( indices ) )
                    .uniform( "color", Color( 0.5, 0.7, 0.1 ) )
                    .renderOption( graphics::RenderOption::Blend, true )
                    .boundingSphere( bs ),
                system.makeProgramFromFiles( resourcePath + "shader/surface/phong/singleColor/vertex.glsl",
                                             resourcePath + "shader/surface/phong/singleColor/fragment.glsl" ) );
 
            setGraphics( "simpleDrawable", simpleDrawable );
 
 
            //----------------------------
            // With vertex, fragment AND GEOMETRY shader:
            // Example geometry shader usage: each line is duplicated swapping x- and y-coordinates
 
            std::vector< PointF< 3 > > v( 2 );
            v[0] = PointF< 3 >( -1.5, -1.0, 0.0 );
            v[1] = PointF< 3 >( 1.5, 1.0, 0.0 );
 
            bs = graphics::computeBoundingSphere( v );
 
            std::shared_ptr< graphics::Drawable > geometryDrawable
                = system.makePrimitive( graphics::PrimitiveConfig{ graphics::RenderPrimitives::LINES }
                                            .vertexBuffer( "position", system.makeBuffer( v ) )
                                            .boundingSphere( bs ),
                                        system.makeProgramFromFiles( resourcePathLocal + "swizzle-vertex.glsl",
                                                                     resourcePathLocal + "swizzle-fragment.glsl",
                                                                     resourcePathLocal + "swizzle-geometry.glsl" ) );
 
            auto testObjectRenderer = std::make_shared< graphics::ObjectRenderer >( system );
 
            testObjectRenderer->addTextLabel( Point3( -1.5, -1.0, 0.0 ), "-1.5, -1.0", 20, Color( 1.0, 0.0, 0.0 ) );
            testObjectRenderer->addTextLabel( Point3( 1.5, 1.0, 0.0 ), "1.5, 1.0", 20, Color( 1.0, 0.0, 0.0 ) );
            testObjectRenderer->addTextLabel( Point3( -1.0, -1.5, 0.0 ), "-1.0, -1.5", 20, Color( 0.0, 1.0, 0.0 ) );
            testObjectRenderer->addTextLabel( Point3( 1.0, 1.5, 0.0 ), "1.0, 1.5", 20, Color( 0.0, 1.0, 0.0 ) );
 
            setGraphics( "geometryDrawable",
                         graphics::makeCompound( { geometryDrawable, testObjectRenderer->commit() } ) );
 
 
            //-----------------------------------------------example texture-----------------------------------------
            // This example loads a 2D-Texture from a picture and fills a 3D-Texture with it.
            // A plane through the 3D-Texture is then drawn in the scene.
 
            // Load the 2D-Texture from a picture.
            std::shared_ptr< graphics::Texture2D > texture2D
                = system.makeTextureFromFile( resourcePathLocal + "FAnToM-logo.jpg", graphics::ColorChannel::RGBA );
 
            // Create an empty 3D-Texture with the same width and height as the 2D-Texture.
            Size3D sizeTest( texture2D->width(), texture2D->height(), 32 );
            std::shared_ptr< graphics::Texture3D > texture3D
                = system.makeTexture( sizeTest, graphics::ColorChannel::RGBA );
 
            // Fill the 3D-Texture slice per slice with the color-data of the 2D-Texture.
            std::vector< Color > tempColors = texture2D->range( Pos2D( 0, 0 ), texture2D->size() );
            for( size_t k = 0; k != texture3D->depth(); ++k )
            {
                texture3D->range( Pos3D( 0, 0, k ), Size3D( texture2D->width(), texture2D->height(), 1 ), tempColors );
            }
 
            // These are the corners of the plane that lies diagonally in the 3D-Texture.
            std::vector< PointF< 3 > > verticesTex( 8 );
            verticesTex[0] = PointF< 3 >( -0.5, -0.5, 0.5 );
            verticesTex[1] = PointF< 3 >( 0.5, -0.5, 0.5 );
            verticesTex[2] = PointF< 3 >( -0.5, 0.5, -0.5 );
            verticesTex[3] = PointF< 3 >( 0.5, 0.5, -0.5 );
 
            // These are the 3D-TextureCoordinates describing the boarders of the 3D-Texture.
            std::vector< PointF< 3 > > texCoords( 8 );
            texCoords[0] = PointF< 3 >( 0.0, 0.0, 1.0 );
            texCoords[1] = PointF< 3 >( 1.0, 0.0, 1.0 );
            texCoords[2] = PointF< 3 >( 0.0, 1.0, 0.0 );
            texCoords[3] = PointF< 3 >( 1.0, 1.0, 0.0 );
 
            // Because we cannot draw QUADS in FAnToM, we have to draw two triangles.
            std::vector< unsigned int > indicesTex( 6 );
            indicesTex[0] = 0;
            indicesTex[1] = 1;
            indicesTex[2] = 2;
            indicesTex[3] = 2;
            indicesTex[4] = 1;
            indicesTex[5] = 3;
 
            // Now we can create a Drawable with the 3D-Texture and then set it as a graphical output.
            bs = graphics::computeBoundingSphere( verticesTex );
            std::shared_ptr< graphics::Drawable > textureDrawable
                = system.makePrimitive( graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLES }
                                            .vertexBuffer( "position", system.makeBuffer( verticesTex ) )
                                            .vertexBuffer( "texCoords", system.makeBuffer( texCoords ) )
                                            .indexBuffer( system.makeIndexBuffer( indicesTex ) )
                                            .texture( "inTexture", texture3D )
                                            .boundingSphere( bs ),
                                        system.makeProgramFromFiles( resourcePathLocal + "tex-vertex.glsl",
                                                                     resourcePathLocal + "tex-fragment.glsl" ) );
 
            setGraphics( "textureDrawable", textureDrawable );
        }
    };
 
    AlgorithmRegister< GraphicsTutorialAlgorithm > dummy( "Tutorial/Graphics", "Show some example graphics." );
} // namespace
