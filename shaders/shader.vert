#version 450
#extension GL_ARB_separate_shader_objects : enable

vec2 position[3] = vec2[](
    vec2( -0.5, -0.5 ), // bottom left
    vec2( 0.0, 0.5 ),   // top
    vec2( 0.5, -0.5)    // bottom right
);

vec3 colors[3] = vec3[](
    vec3( 1.0, 0.0, 0.0 ),  // red
    vec3( 0.0, 1.0, 0.0 ),  // green
    vec3( 0.0, 0.0, 1.0 )   // blue
);

layout( location = 0 ) out vec3 fragColors;

void main()
{
    int i = gl_VertexIndex;
    gl_Position = vec4( position[i].x, position[i].y * (-1.0), 0.0, 1.0 );
    fragColors = colors[i];
}

/*
 *  from :
 *      Shader modules -> Vertex shader
 *      Shader modules -> Pre-vertex colors
 * 
 */