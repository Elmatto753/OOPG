#include "Particle.h"

Particle::Particle(ngl::Vec3 _pos, ngl::Vec3 _dir, ngl::Colour _c, GLfloat _r, GLfloat _ml, const std::string &_shaderName, Scene *_parent)
{
    // set the particles position on direction
    m_pos=_pos;
    m_dir=_dir;
    m_colour=_c;

    m_radius=_r;
    m_maxLife=_ml;
    m_life=0.0;
    m_shaderName=_shaderName;
    m_parent=_parent;

    died = false;

    //GLfloat m_emitAngle=360.0;
    //ngl::Random *rng=ngl::Random::instance();
    // get the angles for emission
    //GLfloat theta=ngl::radians(rng->randomNumber(m_emitAngle));
    //GLfloat phi=ngl::radians(rng->randomNumber(m_emitAngle));
    // set the
    //m_dir.m_x=sin(theta)*cos(phi);
    //m_dir.m_y=sin(theta)*sin(phi);
    //m_dir.m_z=cos(theta);
    //m_dir.normalize();
}

Particle::~Particle()
{
    //std::cout<<"particle  dtor\n";
}

void Particle::draw(const ngl::Mat4 &_globalMouseTx) const
{
    if(died)
        return;

    ngl::Mat3 normalMatrix;

      ngl::ShaderLib *shader=ngl::ShaderLib::instance();
      (*shader)["nglDiffuseShader"]->use();
      shader->setShaderParam4f("Colour",m_colour.m_r,m_colour.m_g,m_colour.m_b,m_colour.m_a);
      ngl::Transformation t;
      t.setPosition(m_pos);
      t.setScale(m_radius,m_radius,m_radius);
      ngl::Mat4 MVP=t.getMatrix() * _globalMouseTx * m_parent->getCamera()->getVPMatrix();
      normalMatrix=t.getMatrix()* _globalMouseTx*m_parent->getCamera()->getViewMatrix();
      normalMatrix.inverse();
      shader->setRegisteredUniform("MVP",MVP);
      shader->setRegisteredUniform("normalMatrix",normalMatrix);


      // get an instance of the VBO primitives for drawing
      ngl::VAOPrimitives::instance()->draw("sphere");
}


void Particle::update()
{
    // more the particle by adding the Direction vector to the position
    m_pos+=m_dir;
    ++m_life;
//    if(m_life > m_maxLife)
//        died = true;
}
