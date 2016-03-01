#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// The texture to be mapped
uniform sampler2D u_RenderTexture;
uniform sampler2D u_DepthTexture;
uniform sampler2D u_NormalTexture;

// The depth at which we want to focus
uniform float u_Depth;

// The radius of blur
uniform float u_BlurRadius;

// This is no longer a built-in variable
out vec4 o_FragColor;

// Store the size of the window
uniform vec2 u_WindowSize;

// Gaussian coefficients
const float G5x5[25] = {0.0035,    0.0123,    0.0210,    0.0123,    0.0035,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0210,    0.0911,    0.2224,    0.0911,    0.0210,
                        0.0123,    0.0543,    0.0911,    0.0543,    0.0123,
                        0.0035,    0.0123,    0.0210,    0.0123,    0.0035};

const float G9x9[81] = {0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0273,    0.0430,    0.0469,    0.0430,    0.0273,    0.0117,    0.0039,
                        0.0039,    0.0117,    0.0234,    0.0352,    0.0430,    0.0352,    0.0234,    0.0117,    0.0039,
                        0.0039,    0.0078,    0.0117,    0.0234,    0.0273,    0.0234,    0.0117,    0.0078,    0.0039,
                        0,    0.0039,    0.0078,    0.0117,    0.0117,    0.0117,    0.0078,    0.0039,         0,
                        0,         0,    0.0039,    0.0039,    0.0039,    0.0039,    0.0039,         0,         0};

#define SZ 9
#define G  G9x9

void main() {
    // Calculate your depth of field effect here!
    vec2 texpos = gl_FragCoord.xy / u_WindowSize;
    float radius = abs(u_Depth - gl_FragCoord.z) * u_BlurRadius;
    //o_FragColor = texture(u_DepthTexture, texpos);
    vec4 colour = vec4(0);
    for( int i=0; i<5; i++ )
    {
        for( int j=0; j<5; j++ )
        {
            colour += G5x5[i*5+j]*texture(u_RenderTexture, vec2((j-2)*((2*radius)/5), (j-2)*((2*radius)/5)));
        }
    }
    o_FragColor = colour;
}
