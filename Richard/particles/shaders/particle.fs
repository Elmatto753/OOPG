#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// The texture which will be applied to each particle
uniform sampler2D u_ParticleSprite;

// These attributes are passed on from the vertex shader
in vec3 o_ParticleColour;
in vec3 o_ParticleAttribs;

// this is the output colour, written to the screen
out vec4 o_FragColour;

void main() {
    // Compute the colour by multiplying by this fragment colour
    o_FragColour = vec4(o_ParticleColour, 1.0) * texture(u_ParticleSprite, gl_PointCoord.st);

    // Annoying bug - I can't get the alpha from the point sprite texture! Something is
    // wrong with my blend function?
    o_FragColour.a = o_FragColour.r;

    // Fade out the particles when they near the end of their lives
    float ageRatio = o_ParticleAttribs.r / o_ParticleAttribs.g;
    if (ageRatio > 0.8)
        o_FragColour = (1.0 - 5.0 * (ageRatio - 0.8)) * o_FragColour;
}
