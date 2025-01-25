// TODO: create detailed documentation of the multipass approach

#include <fantom/algorithm.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>

#include <fantom-plugins/utils/Graphics/Font.hpp>

using namespace fantom;
using namespace fantom::graphics;


//=========================================================================================================================
// local declarations
//=========================================================================================================================

namespace
{
    // ======================================== Drawable =======================================

    class LocalDrawable : public Drawable
    {
    public:
        LocalDrawable( std::shared_ptr< Drawable > child, unsigned int pixelSize );

        // Override: Drawable
        virtual const BoundingSphere& boundingSphere() const override;
        virtual bool update( const RenderInfo& info ) override;
        virtual void draw( RenderState& state ) const override;

    private:
        void updateGeometry( Size2D size );

        std::shared_ptr< Drawable > mChild;
        unsigned int mPixelSize;

        std::shared_ptr< Drawable const > mScreenQuad;
        std::shared_ptr< Texture2D const > mColorTexture;
        std::shared_ptr< Texture2D const > mDepthTexture;
        std::shared_ptr< FrameBuffer const > mFrameBuffer;
    };


    // ======================================== Algorithm =======================================

    class LocalAlgorithm : public VisAlgorithm
    {
    public:
        struct VisOutputs;
        struct Options;

        LocalAlgorithm( InitData& init );

        // Override: VisAlgorithm
        void execute( Algorithm::Options const&, volatile bool const& ) override;

    private:
        std::shared_ptr< Drawable > createChild( VectorF< 3 > const& center ) const;
    };


    struct LocalAlgorithm::VisOutputs : VisAlgorithm::VisOutputs
    {
        VisOutputs( Control& control )
            : VisAlgorithm::VisOutputs( control )
        {
            addGraphics( "Example" );
        }
    };


    struct LocalAlgorithm::Options : VisAlgorithm::Options
    {
        Options( fantom::Options::Control& control )
            : fantom::Options( control )
        {
            add< unsigned int >( "Pixel size", "", 16 );
            add< Vector< 3 > >( "Sphere center", "", Vector< 3 >{ 0.0f, 0.0f, 0.0f } );
        }
    };


    AlgorithmRegister< LocalAlgorithm > reg( "Tutorial/Postprocessing1", "" );
} // namespace


//=========================================================================================================================
// local definitions
//=========================================================================================================================

namespace
{
    LocalDrawable::LocalDrawable( std::shared_ptr< Drawable > child, unsigned int pixelSize )
        : mChild( std::move( child ) )
        , mPixelSize( pixelSize )
    {
        updateGeometry( { 1021, 1750 } );
    }

    const BoundingSphere& LocalDrawable::boundingSphere() const
    {
        static const BoundingSphere sphere;

        if( mChild )
        {
            return mChild->boundingSphere();
        }
        else
        {
            return sphere;
        }
    }

    bool LocalDrawable::update( const RenderInfo& info )
    {
        auto targetSize = info.target.size() / mPixelSize;

        targetSize( 0 ) = std::max< size_t >( 1, targetSize( 0 ) );
        targetSize( 1 ) = std::max< size_t >( 1, targetSize( 1 ) );

        if( !mFrameBuffer || targetSize != mFrameBuffer->size() )
        {
            updateGeometry( targetSize );
        }

        if( mChild )
        {
            return mChild->update( info );
        }
        else
        {
            return false;
        }
    }

    void LocalDrawable::draw( RenderState& state ) const
    {
        if( !mChild )
        {
            return;
        }

        {
            auto stateMod = state.modify();
            stateMod.target( mFrameBuffer );
            state.clear( Color( 0.0f, 0.0f, 0.0f, 0.0f ) );

            mChild->draw( state );

        } // stateMod

        mScreenQuad->draw( state );
    }

    void LocalDrawable::updateGeometry( Size2D size )
    {
        const auto& system = GraphicsSystem::instance();

        auto colorTexture = system.makeTexture( size, ColorChannel::RGBA );
        auto depthTexture = system.makeTexture( size, ColorChannel::Depth );

        // We are blurring in screen space, which causes weird artifacts when tiling the image as done in snapshot mode.
        // As a workaround, we clamp the textures to the edge to prevent black borders to show up. However, there are
        // still artifacts.
        colorTexture->wrapMode( WrapMode::CLAMP_TO_EDGE, WrapMode::CLAMP_TO_EDGE );
        depthTexture->wrapMode( WrapMode::CLAMP_TO_EDGE, WrapMode::CLAMP_TO_EDGE );

        auto frameBuffer = system.makeFrameBuffer( size );
        frameBuffer->colorAttachment( "out_color", colorTexture );
        frameBuffer->depthAttachment( depthTexture );

        mFrameBuffer = std::move( frameBuffer );

        static const std::string fragSource
            = "#version 330\n"
              "uniform sampler2D text_color;\n"
              "uniform sampler2D text_depth;\n"
              "out vec4 out_color;\n"
              "in vec2 frag_vertex;\n"
              "void main()\n"
              "{\n"
              "    out_color = texture( text_color, frag_vertex );\n"
              "    gl_FragDepth = texture( text_depth, frag_vertex ).r;\n"
              "}";

        static const std::string vertSource
            = "#version 330\n"
              "in vec2 in_vertex;\n"
              "out vec2 frag_vertex;\n"
              "void main()\n"
              "{\n"
              "    frag_vertex = in_vertex * 0.5 + 0.5;\n"
              "    gl_Position = vec4( in_vertex, 0.0, 1.0 );\n"
              "}";

        mScreenQuad = system.makePrimitive(
            graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLE_STRIP }
                .vertexBuffer( "in_vertex",
                               system.makeBuffer( std::vector< Tensor< float, 2 > >{
                                   { 1.0, -1.0 }, { -1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 } } ) )
                .texture( "text_color", colorTexture )
                .texture( "text_depth", depthTexture )
                .boundingSphere( graphics::BoundingSphere( { 0.0f, 0.0f, 0.0f }, 1.0f ) ),
            system.makeProgramFromSource( vertSource, fragSource ) );
    }
} // namespace


namespace
{
    LocalAlgorithm::LocalAlgorithm( InitData& init )
        : VisAlgorithm( init )
    {
    }

    void LocalAlgorithm::execute( const Algorithm::Options& options, volatile const bool& )
    {
        auto pixelSize = options.get< unsigned int >( "Pixel size" );
        auto sphereCenter = options.get< Vector< 3 > >( "Sphere center" );

        auto child = createChild( sphereCenter.toType< float >() );

        auto drawable = std::make_shared< LocalDrawable >( child, pixelSize );

        setGraphics( "Example", drawable );
    }

    std::shared_ptr< Drawable > LocalAlgorithm::createChild( const VectorF< 3 >& center ) const
    {
        const auto& system = GraphicsSystem::instance();

        static const std::string fragSourceSphere
            = "#version 330\n"
              "uniform mat4 view_inv;\n"
              "uniform mat4 proj_inv;\n"
              "uniform mat4 view;\n"
              "uniform mat4 proj;\n"
              "uniform vec3 u_center;\n"
              "out vec4 out_color;\n"
              "in vec2 frag_vertex;\n"
              "\n"
              "vec3 homog( vec4 v )\n"
              "{\n"
              "    return v.xyz / v.w;\n"
              "}\n"
              "\n"
              "float depth( vec3 point, mat4 proj )\n"
              "{\n"
              "    float far = gl_DepthRange.far;\n"
              "    float near = gl_DepthRange.near;\n"
              "    vec4 p = proj * vec4( point, 1.0 );\n"
              "    return ( ( gl_DepthRange.diff * p.z / p.w ) + near + far ) / 2.0;\n"
              "}\n"
              "\n"
              "void main()\n"
              "{\n"
              "    vec3 p1 = homog( view_inv * proj_inv * vec4( frag_vertex, -1.0, 1.0 ) );\n"
              "    vec3 p2 = homog( view_inv * proj_inv * vec4( frag_vertex, 1.0, 1.0 ) );\n"
              "\n"
              "    vec3 p3 = u_center;\n"
              "    float r = 1.0;\n"
              "\n"
              "    float x1 = p1.x; float y1 = p1.y; float z1 = p1.z;\n"
              "    float x2 = p2.x; float y2 = p2.y; float z2 = p2.z;\n"
              "    float x3 = p3.x; float y3 = p3.y; float z3 = p3.z;\n"
              "\n"
              "    float dx = x2 - x1;\n"
              "    float dy = y2 - y1;\n"
              "    float dz = z2 - z1;\n"
              "\n"
              "    float a = dx*dx + dy*dy + dz*dz;\n\n"
              "    float b = 2.0 * (dx * (x1 - x3) + dy * (y1 - y3) + dz * (z1 - z3));\n"
              "    float c = x3*x3 + y3*y3 + z3*z3 + x1*x1 + y1*y1 + z1*z1 - 2.0 * (x3*x1 + y3*y1 + z3*z1) - r*r;\n"
              "\n"
              "    float test = b*b - 4.0*a*c;\n"
              "\n"
              "    if (test >= 0.0) {\n"
              "        float u = (-b - sqrt(test)) / (2.0 * a);\n"
              "        vec3 hitp = p1 + u * (p2 - p1);\n"
              "        out_color = vec4( hitp, 1.0 );\n"
              "        gl_FragDepth = depth( hitp, proj * view );\n"
              "    }\n"
              "    else\n"
              "        discard;\n"
              "}";

        static const std::string vertSourceSphere
            = "#version 330\n"
              "in vec2 in_vertex;\n"
              "out vec2 frag_vertex;\n"
              "void main()\n"
              "{\n"
              "    frag_vertex = in_vertex;\n"
              "    gl_Position = vec4( in_vertex, 0.0, 1.0 );\n"
              "}";

        return system.makePrimitive(
            graphics::PrimitiveConfig{ graphics::RenderPrimitives::TRIANGLE_STRIP }
                .vertexBuffer( "in_vertex",
                               system.makeBuffer( std::vector< Tensor< float, 2 > >{
                                   { 1.0, -1.0 }, { -1.0, -1.0 }, { 1.0, 1.0 }, { -1.0, 1.0 } } ) )
                .boundingSphere( graphics::BoundingSphere( center, 1.0f ) )
                .uniform( "u_center", center ),
            system.makeProgramFromSource( vertSourceSphere, fragSourceSphere ) );
    }
} // namespace
