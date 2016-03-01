#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The particle attributes
in vec3 a_ParticlePosition;
in vec3 a_ParticleColour;
in vec3 a_ParticleAttribs;

// The attributes and colour are passed onto the fragment shader
out vec3 o_ParticleColour;
out vec3 o_ParticleAttribs;

// The transformations
uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ModelViewMatrix;

void main() {
    // Set the position of the current vertex
    gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * vec4(a_ParticlePosition, 1.0);

    // Pass on the colour and attribs
    o_ParticleColour = a_ParticleColour;
    o_ParticleAttribs = a_ParticleAttribs;

    // You can do clever stuff here like change the size of the particle based on age or something
    gl_PointSize = 20.0;
}
