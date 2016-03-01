#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// This is no longer a built-in variable
out vec4 fragColor;

void main() {
    // Set the output color of our current pixel
    fragColor = vec4(1.0);
}
