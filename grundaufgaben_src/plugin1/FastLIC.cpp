#include <cstddef>
#include <fantom/algorithm.hpp>
#include <fantom/datastructures/domains/Grid.hpp>
#include <fantom/datastructures/interfaces/Field.hpp>
#include <fantom/graphics.hpp>
#include <fantom/register.hpp>
#include <iostream>
#include <memory>
#include <random>

#include <fantom-plugins/utils/Graphics/Font.hpp>
#include <fantom-plugins/utils/Graphics/HelperFunctions.hpp>

using namespace fantom;
using namespace fantom::graphics;




// Sample the texture at the current fragment coordinates
//vec4 texColor = texture(inNoise, gl_FragCoord.xy / vec2(textureSize(inNoise, 0)));

// Modify the texture color (example: just add some value to the red channel)
// texColor.r = texColor.r * 0.5 + 0.5;  // Example operation

// Write the modified color to the output
// out_color = texColor;

// vec4 res = texture(inNoise, gl_FragCoord.xy / vec2(textureSize(inNoise, 0))) / (2 * step_size);
// for(int i=1; i<step_num; ++i) {

// }

namespace {
// ======================================== Geometry Data =======================================
const std::string LICVertexShader = R"(
	#version 330 core
	layout(location = 0) in vec3 position;
	layout(location = 1) in vec2 texCoords;

	out vec2 fragTexCoords;

	void main() {
		fragTexCoords = texCoords;
		gl_Position = vec4(position, 1.0);
	}
	)";

const std::string LICFragmentShader = R"(
	#version 330 core

	uniform sampler2D inField;  // vector texture
	uniform int   step_num;			// step num
	uniform float step_size;		// step size
	out vec4 out_color;         // Output color

	void main()
	{
		float acc = 0.0;
		vec2 pos_forw = gl_FragCoord.xy / textureSize(inField, 0);
		vec2 pos_back = pos_forw;

		for(int i = 1; i < step_num; ++i) {
			// forward
			vec4 field = texture(inField, pos_forw);
			acc      += field.z;
			vec2 dir  = (field.xy - 0.5) * 2;
			pos_forw += dir * step_size; 
			pos_forw  = clamp(pos_forw, vec2(0.0), vec2(1.0));

			// backward
			field		  = texture(inField, pos_back);
			acc      += field.z;
			dir			  = (field.xy - 0.5) * 2;
			pos_back += dir * step_size; 
			pos_back  = clamp(pos_back, vec2(0.0), vec2(1.0));
		}
		
		float nacc = acc / (step_num * 2);
		out_color  = vec4(nacc, nacc, nacc, 1.0);
	}
	)";

const std::string texVertexShader = R"(
	#version 330 core

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 proj;

	in vec3 position;
	in vec2 texCoords;
	out vec2 fragTexCoords;

	void main()
	{
		gl_Position = proj * view * model * vec4( position, 1.0 );
		fragTexCoords = texCoords;
	}
	)";

const std::string texFragmentShader = R"(
	#version 330 core

	in vec2 fragTexCoords;

	uniform sampler2D inTexture;

	out vec4 out_color;

	void main()
	{
		out_color = texture( inTexture, fragTexCoords );
	}
	)";


// ======================================== Geometry Data =======================================
struct GeometryData {
	std::vector<PointF<3>> verticesTex;
	std::vector<PointF<2>> texCoords;
	std::vector<unsigned int> indicesTex;
	Point2 v1, v2;

	GeometryData(Point2 v1, Point2 v2) : v1(v1), v2(v2) {
		verticesTex = {PointF<3>(v1[0], v1[1], 0.0), PointF<3>(v2[0], v1[1], 0.0),
									 PointF<3>(v1[0], v2[1], 0.0), PointF<3>(v2[0], v2[1], 0.0)};

		texCoords = {PointF<2>(0.0, 0.0), PointF<2>(1.0, 0.0), PointF<2>(0.0, 1.0),
								 PointF<2>(1.0, 1.0)};

		indicesTex = {0, 1, 2, 2, 1, 3};
	}
};

// ======================================== Drawable =======================================
class LocalDrawable : public Drawable {
	public:
		LocalDrawable(std::shared_ptr<Drawable> child, Size2D size, std::shared_ptr<GeometryData> shared_geometry)
			: mChild(std::move(child)), mSize(size), shared_geometry(shared_geometry) 
			{ updateGeometry(size); }

		// Override: Drawable
		virtual const BoundingSphere& boundingSphere() const override {
			static const BoundingSphere sphere;
			return mChild ? mChild->boundingSphere() : sphere;
		}

		virtual bool update(const RenderInfo &info) override {
			if (!mFrameBuffer) 
				updateGeometry(mSize);
			return mChild ? mChild->update(info) : false;
		}

		virtual void draw(RenderState &state) const override {
			if (!mChild) 
				return;

			{
				auto stateMod = state.modify();
				stateMod.target(mFrameBuffer);
				state.clear(Color(0.0f, 0.0f, 0.0f, 0.0f));
				mChild->draw(state);
			}
			mScreenQuad->draw(state);
		}

	private:
		void updateGeometry(Size2D size){
			const auto &system = GraphicsSystem::instance();

			auto colorTexture = system.makeTexture(size, ColorChannel::RGBA);
			auto frameBuffer = system.makeFrameBuffer(size);
			frameBuffer->colorAttachment("out_color", colorTexture);

			mFrameBuffer = std::move(frameBuffer);

			auto bs = graphics::computeBoundingSphere(shared_geometry->verticesTex);

			mScreenQuad = system.makePrimitive(
					graphics::PrimitiveConfig{graphics::RenderPrimitives::TRIANGLES}
							.vertexBuffer("position", system.makeBuffer(shared_geometry->verticesTex))
							.vertexBuffer("texCoords", system.makeBuffer(shared_geometry->texCoords))
							.indexBuffer(system.makeIndexBuffer(shared_geometry->indicesTex))
							.texture("inTexture", colorTexture)
							.boundingSphere(bs),
					system.makeProgramFromSource(texVertexShader, texFragmentShader));
		}

			std::shared_ptr<Drawable> mChild;
			Size2D mSize;

			std::shared_ptr<Drawable const> mScreenQuad;
			std::shared_ptr<FrameBuffer const> mFrameBuffer;
			std::shared_ptr<GeometryData> shared_geometry;
	};


// ======================================== Algorithm =======================================
class LocalAlgorithm : public VisAlgorithm {
	public:
		struct VisOutputs : VisAlgorithm::VisOutputs {
			VisOutputs(Control &control) : VisAlgorithm::VisOutputs(control) {
				addGraphics("Example");
			}
		};

		struct Options : VisAlgorithm::Options {
			Options(fantom::Options::Control &control) : fantom::Options(control) {
				add<Field<2, Vector2>>("Field", "A 2D vector field", definedOn<Grid<2>>(Grid<2>::Points));
				add<size_t>("Resolution", "Image resolution", 1000);
				add<size_t>("Step Num", "Step Number", 100);
				add<float>("Step Size", "Stepsize", 0.1);
				add<Vector2>("Vec1", "vector 1 of plane", Vector2(-10, -10));
				add<Vector2>("Vec2", "vector 1 of plane", Vector2(10, 10));
			}
		};

		LocalAlgorithm(InitData &init) : VisAlgorithm(init) {}

		void execute(Algorithm::Options const& options, volatile bool const &) override {
			// parse options 
			std::cout << "parsing options" << std::endl;
			auto step_size = options.get<float>("Step Size");
			auto step_num  = options.get<size_t>("Step Num");
			auto res       = options.get<size_t>("Resolution");
			Vector2 vec1   = options.get<Vector2>("Vec1");
			Vector2 vec2   = options.get<Vector2>("Vec2");
			auto field     = options.get<Field<2, Vector2>>("Field");
			if (!field) return;


			// generate textures
			std::cout << "generating textures" << std::endl;
			Size2D size{res, res};
			shared_geometry = std::make_shared<GeometryData>(vec1, vec2);
			auto evaluator    = field->makeEvaluator();
			auto vecTexture   = generateFieldTexture(size, std::move(evaluator));
			auto noiseTexture = generateRandomNoiseTexture(size);

			// do drawable 
			auto child = createChild(step_num, step_size, noiseTexture, vecTexture);
			auto drawable = std::make_shared<LocalDrawable>(child, noiseTexture->size(), shared_geometry);
			setGraphics("Example", drawable);
		}

	private:
		std::shared_ptr<GeometryData> shared_geometry;

		std::shared_ptr<Drawable> createChild(size_t step_num, float step_size, 
				std::shared_ptr<graphics::Texture2D> noiseTexture, 
				std::shared_ptr<graphics::Texture2D> vecTexture) const 
		{
			const auto &system = GraphicsSystem::instance();
			auto bs = graphics::computeBoundingSphere(shared_geometry->verticesTex);

			return system.makePrimitive(
				graphics::PrimitiveConfig{graphics::RenderPrimitives::TRIANGLES}
						.vertexBuffer("position", system.makeBuffer(shared_geometry->verticesTex))
						.vertexBuffer("texCoords", system.makeBuffer(shared_geometry->texCoords))
						.indexBuffer(system.makeIndexBuffer(shared_geometry->indicesTex))
						.texture("inField", vecTexture)
						.uniform("step_num", (int)step_num)
						.uniform("step_size", step_size)
						.boundingSphere(bs),
				system.makeProgramFromSource(LICVertexShader, LICFragmentShader));
		}

		std::shared_ptr<Texture2D> generateRandomNoiseTexture(Size2D size) {
			// Dimensions of the texture
			const size_t width = size[0];
			const size_t height = size[1];

			// Create a buffer to store random noise data
			std::vector<float> noiseData(width * height *
																	 4); // 4 floats per pixel for RGBA

			// Initialize random number generator
			std::random_device rd;
			std::mt19937 generator(rd());
			std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

			// Fill the buffer with random noise
			for (size_t i = 0; i < noiseData.size(); i += 4) {
				float value = distribution(generator); // Random grayscale value
				noiseData[i] = value;                  // Red channel
				noiseData[i + 1] = value;              // Green channel
				noiseData[i + 2] = value;              // Blue channel
				noiseData[i + 3] = 1.0f;               // Alpha channel (opaque)
			}

			// generate texture
			const auto &system = GraphicsSystem::instance();
			auto texture = system.makeTexture(size, graphics::ColorChannel::RGBA);
			texture->rangeData({0, 0}, size, noiseData);
			return texture;
		}

		std::shared_ptr<Texture2D> generateFieldTexture(Size2D size, std::unique_ptr<FieldEvaluator<2, Point2>> evaluator) {
			// Init field variables
			std::cout << "init field" << std::endl;
			const size_t width = size[0];
			const size_t height = size[1];
			std::vector<float> vecData(width * height * 4); 
			auto step = shared_geometry->v2 - shared_geometry->v1; 
			auto step_x = step[0] / width;
			auto step_y = step[1] / height;

			// Initialize random number generator
			std::random_device rd;
			std::mt19937 generator(rd());
			std::uniform_real_distribution<float> distribution(0.0f, 1.0f);

			// interpolate stream vec for each pixel
			std::cout << "fill field data" << std::endl;
			for (size_t y = 0; y < height; y++ ) {
				auto y_pos = shared_geometry->v1[1] + step_y * y;

				for (size_t x = 0; x < width; x++ ) {
					auto x_pos = shared_geometry->v1[0] + step_x * x;

					// default to no direction when not found
					auto  offset  = 4 * (y * width + x);
					float random  = distribution(generator); 
					auto n_stream = Point2(0,0);
					if(evaluator->reset(Vector2(x_pos, y_pos))){
						// transform the vectors so they are in [0.0f,1.0f]
						// v = norm(v) * 0.5 + 0.5
						auto stream   = evaluator->value();
						n_stream = (normalized(stream) * 0.5) + Point2(0.5, 0.5);
					}

					vecData[offset + 0] = n_stream[0];		// x
					vecData[offset + 1] = n_stream[1];    // y
					vecData[offset + 2] = random;         // random
					vecData[offset + 3] = 1.0f;           // alpha
				}
			}

			// generate texture
			const auto &system = GraphicsSystem::instance();
			auto texture = system.makeTexture(size, graphics::ColorChannel::RGBA);
			texture->rangeData({0, 0}, size, vecData);
			return texture;
		}
		



};

AlgorithmRegister<LocalAlgorithm> reg("Hauptaufgabe/FastLIC", "");
} // namespace
