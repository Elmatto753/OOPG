#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable
//#extension GL_ARB_shading_language_420pack: enable    // Use for GLSL versions before 420.

// The modelview and projection matrices are no longer given in OpenGL 4.2
uniform mat4 u_ModelViewMatrix;
uniform mat4 u_ProjectionMatrix;
uniform mat3 u_NormalMatrix; // This is the inverse transpose of the mv matrix

// The vertex position attribute
in vec3 a_VertexPosition;

// The vertex normal attribute
in vec3 a_VertexNormal;

// The texture coordinate attribute
in vec2 a_TexCoord;

// The output texture coordinate
smooth out vec2 o_TexCoord;
smooth out vec2 o_Normal;

/************************************************************************************/
/* The section below was plundered from OpenGL 4.0 Shading Language Cookbook, pg 62 */
/************************************************************************************/
// Passed onto the fragment shader
out vec3 LightIntensity;

// Structure for holding light parameters
struct LightInfo {
    vec4 Position; // Light position in eye coords.
    vec3 La; // Ambient light intensity
    vec3 Ld; // Diffuse light intensity
    vec3 Ls; // Specular light intensity
};

// We'll have a single light in the scene
uniform LightInfo u_Light;

// The material properties of our object
struct MaterialInfo {
    vec3 Ka; // Ambient reflectivity
    vec3 Kd; // Diffuse reflectivity
    vec3 Ks; // Specular reflectivity
    float Shininess; // Specular shininess factor
};

// The object has a material
uniform MaterialInfo u_Material;

// Determine the position of the
void getEyeSpace( out vec3 norm, out vec4 position )
{
    norm = normalize( u_NormalMatrix * a_VertexNormal);
    position = u_ModelViewMatrix * vec4(a_VertexPosition,1.0);
}

// Compute the light intensity using the phong model (Gouraud shading, as the normal isn't interpolated)
vec3 phongModel( vec4 position, vec3 norm )
{
    vec3 s = normalize(vec3(u_Light.Position - position));
    vec3 v = normalize(-position.xyz);
    vec3 r = reflect( -s, norm );
    vec3 ambient = u_Light.La * u_Material.Ka;
    float sDotN = max( dot(s,norm), 0.0 );
    vec3 diffuse = u_Light.Ld * u_Material.Kd * sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
      spec = u_Light.Ls * u_Material.Ks * pow( max( dot(r,v), 0.0 ), u_Material.Shininess );
    return ambient + diffuse + spec;
}
/************************************************************************************/

void main() {
    vec3 eyeNorm;
    vec4 eyePosition;

    // Get the position and normal in eye space
    getEyeSpace(eyeNorm, eyePosition);

    // Evaluate the lighting equation.
    LightIntensity = phongModel( eyePosition, eyeNorm );

    // Set the position of the current vertex
    gl_Position = u_ProjectionMatrix * u_ModelViewMatrix * vec4(a_VertexPosition, 1.0);
    o_TexCoord = a_TexCoord;
}
