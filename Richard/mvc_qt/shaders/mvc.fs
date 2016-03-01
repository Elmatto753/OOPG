#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// The texture map
uniform sampler2D u_Texture;

// The texture coordinate output by the vertex shader
in vec2 o_TexCoord;

// This is no longer a built-in variable
out vec4 fragColor;

void main() {
    // This is the actual line we want, but it returns nothing
    fragColor = texture(u_Texture, o_TexCoord);

    // The result from the call below should be nice gradiated color from red to green
    //fragColor = vec4(o_TexCoord, 0.0, 1.0);

    // If the result of the call below is zero (black), we can assume that no texture was loaded
    //fragColor = vec4(textureSize(u_Texture,0),0.0,1.0);
}
