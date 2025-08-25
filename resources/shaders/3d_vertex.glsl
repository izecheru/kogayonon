#version 460 core

void main()
{
    // Define three positions based on gl_VertexID
    vec2 positions[3] = vec2[](
        vec2( 0.0,  0.5), // top
        vec2( 0.5, -0.5), // right
        vec2(-0.5, -0.5)  // left
    );

    gl_Position = vec4(positions[gl_VertexID], 0.0, 1.0);
}