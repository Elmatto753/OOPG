#ifndef __PARTICLE__H_
#define __PARTICLE__H_

#include <ngl/Camera.h>
#include <ngl/Vec3.h>
#include <ngl/Colour.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include "Scene.h"

/// @file Particle.h
/// @brief Revised from Jon's ParticleFactory
/// @author Jonathan Macey, Xiaosong Yang
/// @version 1.0
/// @date 24/1/13
/// @class Particle
/// @brief this is the class for particles


// predeclare the scene manager - collision detection
class Scene;


class Particle
{
public:
    /// @brief constructor must be called by the child classes
    /// @param[in] _pos the position of the particle
    /// @param[in] _dir the direction of the particle
    /// @param[in] _c the colour of the particle
    /// @param[in] _shaderName the name of the shader to use
    /// @param[in] _parent the parent (Emitter used to query global values)
    Particle(ngl::Vec3 _pos, ngl::Vec3 _dir, ngl::Colour _c, GLfloat _r, GLfloat _ml, const std::string &_shaderName, Scene *_parent);
    /// @brief  brief destructor

    ~Particle();

    /// @brief draw the sphere
    void draw(const ngl::Mat4 &_globalMouseTx) const;
    /// @brief update the sphere
    void update();

    /// @brief set the maximum life of the particle
    inline void setMaxLife(int _l){m_maxLife=_l;}

    /// @brief test the life of this particle
    bool    died;
    /// @brief get current position
    ngl::Vec3   getCurrentPosition(){return m_pos;};
    /// @brief get current speed
    ngl::Vec3   getCurrentSpeed(){return m_dir;};
    /// @brief get radius
    GLfloat     getRadius(){return m_radius;};
    /// @brief set new position after collision
    void        setPosition(ngl::Vec3 _newPos){m_pos = _newPos;};
    /// @brief set new speed after collision
    void        setSpeed(ngl::Vec3 _newSpeed){m_dir = _newSpeed;};

protected :
    /// @brief  Position of the Particle
    ngl::Vec3 m_pos;
    /// @brief  Direction of the Particle
    ngl::Vec3 m_dir;
    /// @brief   Colour of the Particle
    ngl::Colour m_colour;
    /// @brief the current life of the particle
    GLfloat m_life;
    /// @brief the max life of the partice
    GLfloat m_maxLife;
    /// @brief the name of the shader to use
    std::string m_shaderName;
    /// @brief the parent Emitter
    Scene *m_parent;
    /// @brief the current radius for the sphere
    GLfloat m_radius;
};


#endif

