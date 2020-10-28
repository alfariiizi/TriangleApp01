#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) in vec3 pos;
layout( location = 1 ) in vec3 col;

layout( location = 0 ) out vec3 fragmentColor;

void main()
{
    gl_Position = vec4( pos.x, pos.y * (-1.0), pos.z, 1.0 );
    fragmentColor = col;
}

/*
 *  from :
 *      Shader modules -> Vertex shader
 *      Shader modules -> Pre-vertex colors
 *
 *  from udemy :
 *      vertex input
 * 
 */