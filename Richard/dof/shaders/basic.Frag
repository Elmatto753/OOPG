#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// The texture coordinates
smooth in vec2 o_TexCoord;

smooth in vec3 o_Normal;

// This is passed on from the vertex shader
in vec3 LightIntensity;

// The texture to be mapped
uniform sampler2D u_Texture;

// This is no longer a built-in variable
out vec4 o_FragColor;
out vec3 o_NormalColor;
//out vec4 o_NormalColor;

void main() {
    // Set the output color of our current pixel
    o_FragColor = vec4(LightIntensity,1.0) * texture(u_Texture, o_TexCoord);

    o_NormalColor = normalise(o_Normal);
}
