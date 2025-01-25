#version 330 core
#define DEPTH_PEELING 0

uniform mat4 view_inv;
uniform mat4 proj_inv;
uniform mat4 view;
uniform mat4 proj;

#if DEPTH_PEELING
    uniform sampler2D u_depthPeelingTexture;
#endif

uniform vec3 u_center;
uniform float u_radius;
uniform vec4 u_color;

in vec2 frag_vertex;
in vec3 p1;
in vec3 p2;

out vec4 out_color;


float depth( vec3 point, mat4 proj )
{
    float far = gl_DepthRange.far;
    float near = gl_DepthRange.near;
    vec4 p = proj * vec4( point, 1.0 );
    return ( ( gl_DepthRange.diff * p.z / p.w ) + near + far ) / 2.0;
}

void main()
{
    // Retrieve components of each point for better readability
    float x1 = p1.x; float y1 = p1.y; float z1 = p1.z;
    float x2 = p2.x; float y2 = p2.y; float z2 = p2.z;
    float x3 = u_center.x; float y3 = u_center.y; float z3 = u_center.z;

    // Calculate differences between x, y and z
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;

    // Use the abc formula to calculate roots
    float a = dx * dx + dy * dy + dz * dz;
    float b = 2.0 * (dx * (x1 - x3) + dy * (y1 - y3) + dz * (z1 - z3));
    float c = x3*x3 + y3*y3 + z3*z3 + x1*x1 + y1*y1 + z1*z1 - 2.0 * (x3*x1 + y3*y1 + z3*z1) - u_radius * u_radius;

    float test = b * b - 4.0 * a * c;

    // Test if solution for quadratic equation is real
    if (test < 0)
    {
        discard;
    }

    // Calculate the root
    float u = (-b - sqrt(test)) / (2.0 * a);

    // Calculate the hitpoint on the screen
    vec3 hitp = p1 + u * (p2 - p1);
    out_color = u_color;

    // Calculate the depth of the current hitpoint
    float depth = depth(hitp, proj * view );

    #if DEPTH_PEELING
        // Retrieve texel from texture and compare it to the previous "peeled" layer
        if (depth <= texture(u_depthPeelingTexture, frag_vertex / 2.0 + 0.5).r)
        {
            // Discard the pixel, if it's further in the back or has the same
            // depth as a previous pixel
            discard;
        }
    #endif

    // Set the depth buffer for the current fragment
    gl_FragDepth = depth;
}
