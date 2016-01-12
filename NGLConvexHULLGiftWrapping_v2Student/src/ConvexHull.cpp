/*
  Copyright (C) 2012 Xiaosong Yang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
//----------------------------------------------------------------------------------------------------------------------
/// @file ConvexHull.cpp
/// @brief Using CGAL 2D ConvexHull algorithm
//----------------------------------------------------------------------------------------------------------------------
#include "ConvexHull.h"
#include <ngl/Random.h>

//----------------------------------------------------------------------------------------------------------------------
ConvexHull::ConvexHull()
{
    m_numP=20;
    ngl::Random *rng=ngl::Random::instance();
    rng->setSeed();
    ngl::Vec2 tmp;
    for(unsigned int i=0;i<m_numP;i++)
    {
        tmp = rng->getRandomVec2();
        tmp/=2.0;
        m_p.push_back(tmp);
    }
    m_vaoHull=0;
    m_vaoPoints=0;
}

//----------------------------------------------------------------------------------------------------------------------
ConvexHull::ConvexHull(
                         const ConvexHull &_c
                        )
{
    m_numP =_c.m_numP;
    m_p =_c.m_p;
    m_vaoHull=0;
    m_vaoPoints=0;

}

//----------------------------------------------------------------------------------------------------------------------
ConvexHull::ConvexHull(const ngl::Vec2 *_p,
                         int _nPoints
                        )
{
    m_numP=_nPoints;
    for(unsigned int i=0; i<m_numP; ++i)
    {
        m_p.push_back(_p[i]);
	}
    m_vaoHull=0;
    m_vaoPoints=0;
}

//----------------------------------------------------------------------------------------------------------------------
ConvexHull::~ConvexHull()
{
    m_p.clear();
    if(m_vaoHull!=0 && m_vaoPoints!=0)
    {
        m_vaoHull->unbind();
        m_vaoHull->removeVOA();
        delete m_vaoHull;
        m_vaoPoints->unbind();
        m_vaoPoints->removeVOA();
        delete m_vaoPoints;
    }
}

//----------------------------------------------------------------------------------------------------------------------
void ConvexHull::drawPoints() const
{
    glPointSize(4);
    m_vaoPoints->bind();
    m_vaoPoints->draw(GL_POINTS);
    m_vaoPoints->unbind();
}

//----------------------------------------------------------------------------------------------------------------------
void ConvexHull::drawConvexHull() const
{
    glPointSize(1);
    m_vaoHull->bind();
    m_vaoHull->draw(GL_LINE_LOOP);
    m_vaoHull->unbind();
}

//----------------------------------------------------------------------------------------------------------------------
void ConvexHull::computeConvexHull()
{
    m_ch.clear();
    // gift wrapping method
    std::vector<bool> vertexVisitFlag(m_numP, false);

    // 1. find the left most point
    float leftMostX = m_p[0].m_x;
    float leftMostID = 0;
    for(unsigned int i=1;i<m_numP;i++)
    {
        if(m_p[i].m_x < leftMostX)
        {
            leftMostX = m_p[i].m_x;
            leftMostID = i;
        }
    }
    m_ch.push_back(leftMostID);
    // 2. wrapping
    unsigned int currentID = leftMostID, nextID;
    ngl::Vec2 lastVertex(m_p[leftMostID].m_x, m_p[leftMostID].m_y - 1.0);
    bool finished = false;
    float tmpCos;

    ngl::Vec2 v1, v2;

    // Task: finish this gift wrapping method for convex hull generation. Check the algorithm in lecture note

    while(!finished)
    {
      float maxAngleCos = 1;
      for(unsigned int j=0; j<m_numP; j++)
      {
        if(vertexVisitFlag[j])
        {
          continue;
        }

        if(j == currentID)
        {
          continue;
        }
        v1 = lastVertex-m_p[currentID];
        v1.normalize();
        v2 = m_p[j] - m_p[currentID];
        v2.normalize();
        tmpCos = v1.m_x*v2.m_x + v1.m_y*v2.m_y;

        if(tmpCos<maxAngleCos)
        {
          maxAngleCos=tmpCos;
          nextID=j;
        }
      }

      lastVertex = m_p[currentID];
      vertexVisitFlag[nextID]=true;
      currentID = nextID;
      m_ch.push_back(nextID);
      if(currentID == leftMostID)
      {
        finished = true;
      }


    }
















}

//----------------------------------------------------------------------------------------------------------------------
void ConvexHull::createVAO()
{
  if(m_vaoHull!=0 && m_vaoPoints!=0)
  {
    m_vaoHull->unbind();
    m_vaoHull->removeVOA();
    delete m_vaoHull;
    m_vaoPoints->unbind();
    m_vaoPoints->removeVOA();
    delete m_vaoPoints;
  }

  computeConvexHull();

  m_vaoPoints=ngl::VertexArrayObject::createVOA(GL_POINTS);
  m_vaoPoints->bind();
  std::vector <ngl::Vec3> points(m_numP);
  std::vector<ngl::Vec2>::iterator it;
  unsigned int i=0;
  for (it= m_p.begin(); it!= m_p.end(); it++)
  {
      points[i++]=ngl::Vec3(it->m_x, it->m_y, 0.0);
  }
  m_vaoPoints->setData(m_numP*sizeof(ngl::Vec3),points[0].m_x);
  m_vaoPoints->setNumIndices(m_numP);
  m_vaoPoints->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vaoPoints->unbind();


  m_vaoHull=ngl::VertexArrayObject::createVOA(GL_LINE_LOOP);
  m_vaoHull->bind();

  unsigned int size = m_ch.size();
  std::vector <ngl::Vec3> lines(size);
  std::vector<unsigned int>::iterator itv;
  i=0;
  for(itv=m_ch.begin(); itv!=m_ch.end();itv++)
  {
      lines[i++].set(ngl::Vec3(m_p[*itv].m_x,m_p[*itv].m_y, 0.0));
  }
  m_vaoHull->setData(size*sizeof(ngl::Vec3),lines[0].m_x);
  m_vaoHull->setNumIndices(size);
  m_vaoHull->setVertexAttributePointer(0,3,GL_FLOAT,0,0);
  m_vaoHull->unbind();

}

//----------------------------------------------------------------------------------------------------------------------
