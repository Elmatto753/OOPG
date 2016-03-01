#ifndef PARTICLESYSTEM_H
#define PARTICLESYSTEM_H

#include <cstdlib>

#include "glinc.h"

/// This is 1.0f / (float) RAND_MAX (hopefully improves performance a smidgen)
#define INV_RAND_MAX 4.65661e-10

/// I couldn't find a better place to put this. Returns a random float between 0 and 1.
static float frand() {return (float)rand() * INV_RAND_MAX;}

/// Should probably use GLuint, but trying to make this portable.
typedef unsigned int uint;

typedef struct PARTICLE
{
  GLfloat x, y, z;
  GLfloat vX, vY, vZ;
  GLfloat r, g, b;
  GLfloat age, lifespan;

}Particle;

//in vec3 a_ParticlePosition;
//in vec3 a_ParticleColour;
//in vec3 a_ParticleAttribs;

class ParticleSystem
{
public:
    /// Construct an empty particle system
    ParticleSystem();

    /// Destroy the particle system: clear memory etc.
    virtual ~ParticleSystem();

    /// Initialise the particle system with the specified parameters
    void init(uint size);

    /// Draw the particle system in the current shader
    void draw(GLuint id, GLuint va, GLuint bo, GLuint size);

    /// Update the particle array (probably best to do this based on a timer)
    void update();

    std::vector<Particle> particles;


};

#endif // PARTICLESYSTEM_H
