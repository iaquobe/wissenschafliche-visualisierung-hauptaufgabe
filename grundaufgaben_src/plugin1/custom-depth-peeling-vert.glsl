#version 330 core

// Current billboard vertex
in vec2 in_vertex;

// Pass billboard vertex to fragment shader to calculate ray hitpoint
out vec2 frag_vertex;

uniform mat4 view_inv;
uniform mat4 proj_inv;

out vec3 p1;
out vec3 p2;


vec3 homog( vec4 v )
{
    return v.xyz / v.w;
}

void main()
{
    frag_vertex = in_vertex;
    gl_Position = vec4( in_vertex, 0.0, 1.0 );

    p1 = homog( view_inv * proj_inv * vec4( frag_vertex, -1.0, 1.0 ) );
    p2 = homog ( view_inv * proj_inv * vec4( frag_vertex, 1.0, 1.0 ) );
}