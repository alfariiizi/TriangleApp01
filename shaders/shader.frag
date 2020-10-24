#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( location = 0 ) out vec4 outColor;


void main()
{
    vec4 redSolid = { 1.0, 0.0, 0.0, 1.0 };
    outColor = vec4( redSolid );
}

/*
 *  from :
 *      Shader modules -> Fragment shader
 *      Shader modules -> Pre-vertex colors
 *
 *  from udemy :
 *      vertex input
 * 
 */