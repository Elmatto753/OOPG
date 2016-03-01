#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The modelview and projection matrices are no longer given in OpenGL 4.2
//uniform mat4 u_ModelViewMatrix;
//uniform mat4 u_ProjectionMatrix;

// The vertex position attribute
in vec3 a_VertexPosition;

void main() {
    // Set the position of the current vertex
    //gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * vec4(a_VertexPosition, 1.0);
    gl_Position = vec4(a_VertexPosition, 1.0);
}
