#include "particlesystem.h"
#include <time.h>

/**
  * Construct an empty system. Note that this is not ready to use until the init() function.
  * is called (i.e. draw() will do nothing.
  */
ParticleSystem::ParticleSystem() {
    // Specify our random seed
    srand( time(NULL) );
}

/**
  * Make sure to delete allocated memory.
  */
ParticleSystem::~ParticleSystem() {
}


void ParticleSystem::init(GLuint size)
{
  for(int i=0; i<size; i++)
  {
    Particle P;
    P.x = 1.0;
    P.y = 0.0;
    P.z = 0.0;

    P.r = frand();
    P.g = 1.0;
    P.b = frand();

    P.age = 0.0;
    P.lifespan = (2*frand()) + 0.5;

    particles.push_back(P);
  }

}

void ParticleSystem::draw(GLuint id, GLuint va, GLuint bo, GLuint size)
{

  GLint partAttribLoc = glGetAttribLocation(id, "a_ParticlePosition");
  GLint partAttribCol = glGetAttribLocation(id, "a_ParticleColour");
  GLint partAttribs = glGetAttribLocation(id, "a_ParticleAttribs");
  GLint mvLoc = glGetUniformLocation(id, "u_ModelViewMatrix");
  GLint projLoc = glGetUniformLocation(id, "u_ProjectionMatrix");

  glBindVertexArray(va);

  glBindBuffer(GL_ARRAY_BUFFER, bo);
  glEnableVertexAttribArray(partAttribLoc);
  glVertexAttribPointer(partAttribLoc, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (GLfloat *)NULL + 5);

  if(mvLoc != -1)
  {
    glm::mat4 mv(1.0f);

    glUniformMatrix4fv(mvLoc,
                       1,
                       GL_FALSE,
                       glm::value_ptr(mv));

  }

  glDrawArrays(GL_POINTS, 0, size);

  glDisableVertexAttribArray(partAttribLoc);

  glBindVertexArray(0);

}

void ParticleSystem::update() {
}
