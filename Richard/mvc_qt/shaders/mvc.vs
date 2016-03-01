// Keeping you on the bleeding edge!
#version 420
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable  Use for GLSL versions before 420.

precision highp float;

// Pass the original vertex locations of the vertices for texture coordinates
in vec2 a_Pos;

// This is output to be scan converted and used by frag shader
out vec2 o_TexCoord;

// Contains the location of the cage coordinates, size nCage*2
uniform samplerBuffer u_cageVerts;

// Contains the barycentric coordinates of each vertex, size nCage*nVert
uniform samplerBuffer u_vertWeights;

// The combined modelviewprojection matrix (no longer given to you in GL)
uniform mat4 u_MVP;

void main() {
        // TODO: calculate the vertex position by blending the weights
        // retrieved from the the textures. Note that you want to use the
        // built-in variable gl_VertexID to retrieve the values for this
        // particular vertex
        int i;
        int nCage = textureSize(u_cageVerts);
        int nVerts = textureSize(u_vertWeights) / nCage;
        vec2 result = vec2(0,0);
        float beta;

        for ( i = 0; i < nCage; ++i)
        {
            vec2 cageVert = vec2(texelFetch(u_cageVerts, i));

            beta = texelFetch(u_vertWeights, gl_VertexID * nCage + i).x;
            result += beta * cageVert.xy;

        }
        // Set the position of the current vertex
        gl_Position = u_MVP * vec4(result,0.0,1.0);

        // Do a direct copy of the texture coordinates
        o_TexCoord = a_Pos;

}
