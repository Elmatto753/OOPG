#version 420                                            // Keeping you on the bleeding edge!
#extension GL_EXT_gpu_shader4 : enable

// The texture coordinates
smooth in vec2 o_TexCoord;
smooth in vec3 vertexPos;
smooth in vec3 vertNorm;
//smooth in vec3 refl;

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

// This is passed on from the vertex shader
in vec3 LightIntensity;

// The texture to be mapped
uniform sampler2D u_Texture;

// This is no longer a built-in variable
out vec4 o_FragColor;

vec3 phongModel()
{
    vec3 l = normalize(u_Light.Position.xyz-vertexPos);
    vec3 e = normalize(-vertexPos);
    vec3 n = normalize (vertNorm);
    vec3 r = reflect(-l,n);
    vec3 ambient = u_Light.La * u_Material.Ka;
    float sDotN = max( dot(l,vertNorm), 0.0 );
    vec3 diffuse = u_Light.Ld * u_Material.Kd *sDotN;
    vec3 spec = vec3(0.0);
    if( sDotN > 0.0 )
      spec = u_Light.Ls * u_Material.Ks * pow( max( dot(r,e), 0.0 ), u_Material.Shininess );
    return ambient + diffuse + spec;
}

void main() {
    // Set the output color of our current pixel
    int i=0;
    while(true)
    {
        o_FragColor = vec4(phongModel(),1.0) * texture(u_Texture, o_TexCoord)*sin(i);
        i++;
    }
}
