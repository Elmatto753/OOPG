#include "Scene.h"
#include "Particle.h"
#include <boost/foreach.hpp>

Scene::Scene(ngl::Transformation *_t, ngl::Camera *_cam)
{
    m_numParticles = 0;
    m_transform=_t;
    m_cam=_cam;
}

void Scene::addParticle(ngl::Vec3 _pos, ngl::Vec3 _dir, ngl::Colour _c, GLfloat _r, GLfloat _ml)
{
    Particle *p = new Particle(_pos, _dir, _c, _r, _ml, "Phong", this);
    m_particles.push_back(p);
    ++m_numParticles;
}

Scene::~Scene()
{
    // we need to call the dtor for each of the particles as pointer
    // however the std::vector will clear it's self at the end
    for(std::list<Particle *>::iterator itr = m_particles.begin(); itr!= m_particles.end(); ++itr)
    {
        delete *itr;
    }
    deleteAllWalls();
}

void Scene::addNewWall(ngl::Vec3 _point, float _size, ngl::Vec3 _normal, bool _draw)
{
    Wall *newWall = new Wall;
    _normal.normalize();
    newWall->centre = _point;
    newWall->size = _size;
    newWall->a = _normal.m_x;
    newWall->b = _normal.m_y;
    newWall->c = _normal.m_z;
    newWall->d = -(newWall->a * _point.m_x + newWall->b * _point.m_y + newWall->c * _point.m_z);
    newWall->draw_flag = _draw;

    m_walls.push_back(newWall);
}

void Scene::deleteAllWalls()
{
    for(std::list<Wall *>::const_iterator itr=m_walls.begin(); itr!= m_walls.end(); ++itr)
    {
        delete *itr;
    }
    m_walls.clear();
}

ngl::Vec3 Scene::getRotationFromY(ngl::Vec3 _vec) const
{
    ngl::Vec3 rot;
    rot.m_z = 0.0;
    if(fabs(_vec.m_y)< 0.0001)
    {
        if (_vec.m_z>= 0.0)
            rot.m_x = -90;
        else
            rot.m_x = 90;
    }
    else
        rot.m_x = atan(_vec.m_z/_vec.m_y);
    if(fabs(_vec.m_y) + fabs(_vec.m_z) < 0.0001)
    {
        if(_vec.m_x > 0)
            rot.m_y = -90;
        else
            rot.m_y = 90;
    }
    else
        rot.m_z = atan(_vec.m_x/sqrt(_vec.m_y*_vec.m_y + _vec.m_z*_vec.m_z));

    return rot;
}

void Scene::draw(const ngl::Mat4 &_globalMouseTx) const
{
    BOOST_FOREACH(Particle *p, m_particles)
    {
        p->draw(_globalMouseTx);
    }
    // draw the walls
    ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["nglColourShader"]->use();
    BOOST_FOREACH(Wall *w, m_walls)
    {
        if(w->draw_flag)
        {
          m_transform->reset();
          {
              m_transform->setPosition(w->centre);
              m_transform->setScale(w->size, w->size, w->size);
              m_transform->setRotation(getRotationFromY(ngl::Vec3(w->a,w->b,w->c)));
              ngl::Mat4 MVP= m_transform->getMatrix()
                             *_globalMouseTx* m_cam->getVPMatrix();
              shader->setShaderParamFromMat4("MVP",MVP);
              prim->draw("wall");
          }
        }
    }
}

void Scene::update()
{
    // call the update method for each particle
    for(std::list<Particle *>::iterator itr = m_particles.begin(); itr!= m_particles.end(); ++itr)
    {
        (*itr)->update(); // update the position of each particles
        if((*itr)->died)    // if this particle died, delete it from the list
        {
            delete *itr;
            itr=m_particles.erase(itr);
            m_numParticles--;
        }
    }
    collisionWithBalls();
    collisionWithWalls();
}

void Scene::collisionWithBalls()
{
    ngl::Vec3 P, Q, Vp, Vq, A, B, Pt, Qt, N;
    ngl::Vec3 newP, newQ, newVp, newVq;
    float t, t1, t2, Rp, Rq;

    // call the update method for each particle
    for(std::list<Particle *>::iterator itr = m_particles.begin(); itr!= m_particles.end(); ++itr)
    {
        P = (*itr)->getCurrentPosition();
        Vp = (*itr)->getCurrentSpeed();
        Rp = (*itr)->getRadius();
        std::list<Particle *>::iterator itr1 = itr;
        for(++itr1; itr1!= m_particles.end(); ++itr1)
        {
            Q = (*itr1)->getCurrentPosition();
            Vq = (*itr1)->getCurrentSpeed();
            Rq = (*itr1)->getRadius();
            // Task 2: collision between two balls, set back the new position and new velocity
            //      if multiple collision happened, you need to decrease the simulation step.

            if((P-Q).length()<=(Rp+Rq))
            {

              A=(P-Vp)-(Q-Vq);
              B=Vp-Vq;
              float ab = A.dot(B);
              float aa = A.dot(A);
              float bb = B.dot(B);
              float squareRoot = sqrt(ab*ab-bb*bb*(aa-(Rp+Rq)*(Rp+Rq)));
              t1 = (-ab-squareRoot)/(bb*bb);
              t2 = (-ab+squareRoot)/(bb*bb);
              if(t1>=0 && t1<=1)
              {
                t=t1;
              }
              else if(t2>=0 && t2<=1)
              {
                t=t2;
              }
              else
              {
                std::cout<<"OUT OF CHEESE ERROR";
              }

              Pt=P-(1-t)*Vp;
              Qt=Q-(1-t)*Vq;
              N = Qt-Pt;
              N.normalize();

              newVp=(Vp-(Vp.dot(N))*N)+(Vq.dot(N)*N);
              newVq=(Vq-(Vq.dot(N))*N)+(Vp.dot(N)*N);
              newP=Pt+newVp*(1-t);
              newQ=Qt+newVq*(1-t);
              (*itr1)->setPosition(newQ);
              (*itr1)->setSpeed(newVq);
              (*itr)->setPosition(newP);
              (*itr)->setSpeed(newVp);

            }








        }
    }

}

void Scene::collisionWithWalls()
{
    ngl::Vec3 oldP, oldV, newP, newV, wallNormal;
    float radius;
    Wall *wp;
    float dist;
    // call the update method for each particle
    for(std::list<Particle *>::iterator itr = m_particles.begin(); itr!= m_particles.end(); ++itr)
    {
        oldP = (*itr)->getCurrentPosition();
        oldV = (*itr)->getCurrentSpeed();
        radius = (*itr)->getRadius();
        for(std::list<Wall *>::const_iterator itrw = m_walls.begin(); itrw!= m_walls.end(); ++itrw)
        {
            wp = *itrw;
            wallNormal.m_x = wp->a; wallNormal.m_y = wp->b; wallNormal.m_z = wp->c;
            // Task1: collision with walls, compute the new postion and velocity, set back to *itr
            dist = oldP.m_x * wp->a + oldP.m_y * wp->b + oldP.m_z * wp->c + wp->d;

            if(dist<radius)
            {
              newP = oldP - 2.0 * (dist-radius) * wallNormal;
              newV = -oldV.dot(wallNormal)*wallNormal + (oldV-oldV.dot(wallNormal)*wallNormal);
              (*itr)->setPosition(newP);
              (*itr)->setSpeed(newV);
            }
        }
    }
}

void Scene::clearParticles()
{
    // however the std::vector will clear it's self at the end
    for(std::list<Particle *>::iterator itr = m_particles.begin(); itr!= m_particles.end(); ++itr)
    {
        delete *itr;
    }
    // must remember to re-size the std::vector back to 0
    m_particles.clear();
    m_numParticles=0;
}
