#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The combined modelviewprojection matrix (no longer given to you in GL)
uniform mat4 u_MVP;

// The vertex position attribute
in vec2 a_Pos;

void main() {
    // Set the position of the current vertex
    gl_Position = u_MVP * vec4(a_Pos.x, a_Pos.y, 0.0, 1.0);
}
