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

// Given a "randomly" (regular grid point with noise) created input sites,
// 0. Create big box to wrapping up all input sites, create two triangles from the box
// 1. Insert one sites each time, splitting the triangle where the new vertex falling in, incrementally get a initial triangle mesh
// 2. press space key to do one flip, step by step to achieve delaunay trianglation, until no flip can inprove the mesh
// 3. press "q" to delete the wrapping box
// 4. press "v" to draw the voronoi diagram
//
// Data structure: face - edge - vertex
//
// Workshop:
//  1. finish getTwoOppositeAngles function
//  2. finish computeCircumCircleForAllFaces function
//  3. finish insideTriangle function

//----------------------------------------------------------------------------------------------------------------------
/// @file Delaunay.cpp
/// @brief Using incremental insertion method to generate Delaunay triangulation
//----------------------------------------------------------------------------------------------------------------------
#include "Delaunay.h"

//----------------------------------------------------------------------------------------------------------------------
Delaunay::Delaunay(std::vector<ngl::Vec3> _site)
{
    m_vertices = _site;
    m_vaoDelaunay = NULL;
    m_vaoVoronoi = NULL;
    m_drawVoronoiFlag = false;
    computeDelaunay();
}

//----------------------------------------------------------------------------------------------------------------------
Delaunay::~Delaunay()
{
    if(m_vaoDelaunay!=0 && m_vaoVoronoi!=0)
    {
        m_vaoDelaunay->unbind();
        m_vaoDelaunay->removeVOA();
        delete m_vaoDelaunay;
        m_vaoVoronoi->unbind();
        m_vaoVoronoi->removeVOA();
        delete m_vaoVoronoi;
    }
    if(m_faces.size()>0)
    {
        // free the memory of all the faces and edges
        std::vector<DFace *>::iterator itr;
        for(itr = m_faces.begin();itr!=m_faces.end();++itr)
        {
            (*itr)->m_edges[0]->m_flag = true;
            (*itr)->m_edges[1]->m_flag = true;
            (*itr)->m_edges[2]->m_flag = true;
        }
        std::vector<DEdge *> edgeList;
        for(itr = m_faces.begin();itr!=m_faces.end();++itr)
        {
            if((*itr)->m_edges[0]->m_flag)
            {
                edgeList.push_back((*itr)->m_edges[0]);
                (*itr)->m_edges[0]->m_flag = false;
            }
            if((*itr)->m_edges[1]->m_flag)
            {
                edgeList.push_back((*itr)->m_edges[1]);
                (*itr)->m_edges[1]->m_flag = false;
            }
            if((*itr)->m_edges[2]->m_flag)
            {
                edgeList.push_back((*itr)->m_edges[2]);
                (*itr)->m_edges[2]->m_flag = false;
            }
            delete (*itr);
        }
        std::vector<DEdge *>::iterator itr1;
        for(itr1 = edgeList.begin();itr1!=edgeList.end(); ++itr1)
            delete (*itr1);
    }
}

void Delaunay::turnVoronoiDrawFlag()
{
    if(m_drawVoronoiFlag)
        m_drawVoronoiFlag = false;
    else
        m_drawVoronoiFlag = true;
}

//----------------------------------------------------------------------------------------------------------------------
void Delaunay::draw() const
{
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    (*shader)["nglColourShader"]->use();
    shader->setShaderParam4f("Colour",1,1,1,1);
    drawDelaunay();

    if(m_drawVoronoiFlag)
    {
        shader->setShaderParam4f("Colour",1,0,0,1);
        drawVoronoi();
    }

}

//----------------------------------------------------------------------------------------------------------------------
void Delaunay::drawVoronoi() const
{
    m_vaoVoronoi->bind();
    m_vaoVoronoi->draw(GL_LINES);
    m_vaoVoronoi->unbind();
}
//----------------------------------------------------------------------------------------------------------------------
void Delaunay::drawDelaunay() const
{
    m_vaoDelaunay->bind();
    m_vaoDelaunay->draw(GL_LINES);
    m_vaoDelaunay->unbind();
}

BoundBox Delaunay::computeBoundBox()
{
    BoundBox box;
    box.m_xmax = box.m_xmin = m_vertices[0].m_x;
    box.m_ymax = box.m_ymin = m_vertices[0].m_y;
    box.m_zmax = box.m_zmin = m_vertices[0].m_z;
    for(std::vector<ngl::Vec3>::iterator itr = m_vertices.begin();itr!=m_vertices.end();++itr)
    {
        if(itr->m_x < box.m_xmin) box.m_xmin = itr->m_x;
        if(itr->m_x > box.m_xmax) box.m_xmax = itr->m_x;
        if(itr->m_y < box.m_ymin) box.m_ymin = itr->m_y;
        if(itr->m_y > box.m_ymax) box.m_ymax = itr->m_y;
        if(itr->m_z < box.m_zmin) box.m_zmin = itr->m_z;
        if(itr->m_z > box.m_zmax) box.m_zmax = itr->m_z;
    }
    return box;
}

void Delaunay::swapForDelaunay()
{
    bool changed = true;
    std::vector<DFace *>::iterator itr;
    unsigned int i;
    DEdge   *tEdge;
    float   angle1, angle2;
//    while(changed)
    {
        changed = false;
        itr = m_faces.begin();
        while(itr!=m_faces.end() && !changed)
        {
            i=0;
            while(i<3 && !changed)
            {
                tEdge = (*itr)->m_edges[i];
                if(tEdge->m_faces[0]!=0 && tEdge->m_faces[1]!=0)
                {
                    getTwoOppositeAngles(tEdge, &angle1, &angle2);
                    if(angle1+angle2>180)
                    {
                        swapEdge(tEdge);
                        changed = true;
                    }
                }
                i++;
            }
            ++itr;
        }
    }
}

//          ve1              ve1
//         / | \            /   \
//        /  |  \          / f0  \
//      v2 f1|f0 v1       v2-----v1
//       \   |   /         \ f1  /
//        \  |  /           \   /
//         \ | /             \ /
//          ve2              ve2
void Delaunay::swapEdge(DEdge *_e)
{
    unsigned int v1, v2, ve1, ve2, i, vtmp;
    DFace *f0, *f1;
    getTwoOppositeVertices(_e, &v1, &v2);
    ve1 = _e->m_vertices[0];
    ve2 = _e->m_vertices[1];
    f0 = _e->m_faces[0];
    f1 = _e->m_faces[1];
    // make sure all other edges in the two neighbour triangles have the f0&f1 as second face
    // that means these edges don't need to change the vertices order
    // the only thing need to change is the m_faces[1]
    for(i=0;i<3;i++)
    {
        if(f0->m_edges[i] == _e)
            continue;
        if(f0->m_edges[i]->m_faces[0] == f0)
        {
            // swap the vertices and faces order
            vtmp = f0->m_edges[i]->m_vertices[0];
            f0->m_edges[i]->m_vertices[0] = f0->m_edges[i]->m_vertices[1];
            f0->m_edges[i]->m_vertices[1] = vtmp;
            f0->m_edges[i]->m_faces[0] = f0->m_edges[i]->m_faces[1];
            f0->m_edges[i]->m_faces[1] = f0;
        }
    }
    for(i=0;i<3;i++)
    {
        if(f1->m_edges[i] == _e)
            continue;
        if(f1->m_edges[i]->m_faces[0] == f1)
        {
            // swap the vertices and faces order
            vtmp = f1->m_edges[i]->m_vertices[0];
            f1->m_edges[i]->m_vertices[0] = f1->m_edges[i]->m_vertices[1];
            f1->m_edges[i]->m_vertices[1] = vtmp;
            f1->m_edges[i]->m_faces[0] = f1->m_edges[i]->m_faces[1];
            f1->m_edges[i]->m_faces[1] = f1;
        }
    }
    // find the neighbour four edges
    DEdge *e_v1_ve1, *e_v1_ve2, *e_v2_ve1, *e_v2_ve2;
    for(i=0;i<3;i++)
    {
        if(f0->m_edges[i] == _e)
            continue;
        if(f0->m_edges[i]->m_vertices[0] == ve1 && f0->m_edges[i]->m_vertices[1] == v1 )
            e_v1_ve1 = f0->m_edges[i];
        if(f0->m_edges[i]->m_vertices[0] == v1 && f0->m_edges[i]->m_vertices[1] == ve2 )
            e_v1_ve2 = f0->m_edges[i];
    }
    for(i=0;i<3;i++)
    {
        if(f1->m_edges[i] == _e)
            continue;
        if(f1->m_edges[i]->m_vertices[0] == v2 && f1->m_edges[i]->m_vertices[1] == ve1 )
            e_v2_ve1 = f1->m_edges[i];
        if(f1->m_edges[i]->m_vertices[0] == ve2 && f1->m_edges[i]->m_vertices[1] == v2 )
            e_v2_ve2 = f1->m_edges[i];
    }

    // change topology, swap
    e_v1_ve1->m_faces[1] = f0;
    e_v1_ve2->m_faces[1] = f1;
    e_v2_ve1->m_faces[1] = f0;
    e_v2_ve2->m_faces[1] = f1;

    _e->m_vertices[0] = v2;
    _e->m_vertices[1] = v1;
    _e->m_faces[0] = f0;
    _e->m_faces[1] = f1;

    f0->m_edges[0] = _e;
    f0->m_edges[1] = e_v1_ve1;
    f0->m_edges[2] = e_v2_ve1;

    f1->m_edges[0] = _e;
    f1->m_edges[1] = e_v1_ve2;
    f1->m_edges[2] = e_v2_ve2;
}

void Delaunay::getTwoOppositeVertices(DEdge *_e, unsigned int *_vp1, unsigned int *_vp2)
{
    unsigned int ve1, ve2, v1, v2, i, vtmp;
    ve1 = _e->m_vertices[0];
    ve2 = _e->m_vertices[1];
    for(i=0;i<3;i++)
    {
        vtmp = _e->m_faces[0]->m_edges[i]->m_vertices[0];
        if(vtmp!=ve1 && vtmp!=ve2)
        {
            v1 = vtmp;
            break;
        }
        vtmp = _e->m_faces[0]->m_edges[i]->m_vertices[1];
        if(vtmp!=ve1 && vtmp!=ve2)
        {
            v1 = vtmp;
            break;
        }
    }
    for(i=0;i<3;i++)
    {
        vtmp = _e->m_faces[1]->m_edges[i]->m_vertices[0];
        if(vtmp!=ve1 && vtmp!=ve2)
        {
            v2 = vtmp;
            break;
        }
        vtmp = _e->m_faces[1]->m_edges[i]->m_vertices[1];
        if(vtmp!=ve1 && vtmp!=ve2)
        {
            v2 = vtmp;
            break;
        }
    }
    *_vp1 = v1;
    *_vp2 = v2;
}

void Delaunay::getTwoOppositeAngles(DEdge *_e, float *_a1, float *_a2)
{
    unsigned int ve1, ve2, v1, v2;
    ngl::Vec3 e1, e2;
    getTwoOppositeVertices(_e, &v1, &v2);
    ve1 = _e->m_vertices[0];
    ve2 = _e->m_vertices[1];

    //          ve1              ve1
    //         / | \            /   \
    //        /  |  \          / f0  \
    //      v2 f1|f0 v1       v2-----v1
    //       \   |   /         \ f1  /
    //        \  |  /           \   /
    //         \ | /             \ /
    //          ve2              ve2
    // Task 1: compute the two opposite angles


    e1 = m_vertices[ve1] - m_vertices[v1];
    e2 = m_vertices[ve2] - m_vertices[v1];
    e1.normalize();
    e2.normalize();


    *_a1 = acos(e1.dot(e2)) /3.1415926*180.0;

    e1 = m_vertices[ve1] - m_vertices[v2];
    e2 = m_vertices[ve2] - m_vertices[v2];
    e1.normalize();
    e2.normalize();


    *_a2 = acos(e1.dot(e2))/3.1415926*180.0;
}

float Delaunay::matrix3x3Determinant(float _m00, float _m01, float _m02, float _m10, float _m11, float _m12, float _m20, float _m21, float _m22)
{
    return(_m00*(_m11*_m22-_m21*_m12) - _m01*(_m10*_m22-_m20*_m12) + _m02*(_m10*_m21-_m20*_m11));
}

float Delaunay::minimumAngle()
{
    std::vector<DFace *>::iterator itr;
    unsigned int v1, v2, v3;
    ngl::Vec3 p1, p2, p3, e1, e2;
    float   angle1, angle2, angle3, minAngle=180.0;
    for(itr=m_faces.begin();itr!=m_faces.end();++itr)
    {
        findThreeVertices(*itr, &v1, &v2, &v3);
        p1 = m_vertices[v1];
        p2 = m_vertices[v2];
        p3 = m_vertices[v3];
        e1 = p2-p1;
        e2 = p3-p1;
        e1.normalize();
        e2.normalize();
        angle1 = acos(e1.dot(e2))/3.1415926*180.0;
        if(angle1<minAngle)
            minAngle = angle1;
        e1 = p1-p2;
        e2 = p3-p2;
        e1.normalize();
        e2.normalize();
        angle2 = acos(e1.dot(e2))/3.1415926*180.0;
        if(angle2<minAngle)
            minAngle = angle2;
        e1 = p1-p3;
        e2 = p2-p3;
        e1.normalize();
        e2.normalize();
        angle3 = acos(e1.dot(e2))/3.1415926*180.0;
        if(angle3<minAngle)
            minAngle = angle3;
    }
    return minAngle;
}

void Delaunay::computeCircumCircleForAllFaces()
{
    std::vector<DFace *>::iterator itr;
    unsigned int v1, v2, v3;
    ngl::Vec3 p1, p2, p3;
    float   a, bx, by, c;
    for(itr=m_faces.begin();itr!=m_faces.end();++itr)
    {
        findThreeVertices(*itr, &v1, &v2, &v3);
        p1 = m_vertices[v1];
        p2 = m_vertices[v2];
        p3 = m_vertices[v3];


        // Task 2: compute the circumcircle centre and radius, use matrix3x3Determinant function
        a = matrix3x3Determinant(p1.m_x, p1.m_y, 1.0f,
                                 p2.m_x, p2.m_y, 1.0f,
                                 p3.m_x, p3.m_y, 1.0f);
        bx = - matrix3x3Determinant(((p1.m_x*p1.m_x)+(p1.m_y*p1.m_y)), p1.m_y, 1.0f,
                                  ((p2.m_x*p2.m_x)+(p2.m_y*p2.m_y)), p2.m_y, 1.0f,
                                  ((p3.m_x*p3.m_x)+(p3.m_y*p3.m_y)), p3.m_y, 1.0f);
        by = matrix3x3Determinant(((p1.m_x*p1.m_x)+(p1.m_y*p1.m_y)), p1.m_x, 1.0f,
                                  ((p2.m_x*p2.m_x)+(p2.m_y*p2.m_y)), p2.m_x, 1.0f,
                                  ((p3.m_x*p3.m_x)+(p3.m_y*p3.m_y)), p3.m_x, 1.0f);
        c = - matrix3x3Determinant(((p1.m_x*p1.m_x)+(p1.m_y*p1.m_y)), p1.m_x, p1.m_y,
                                 ((p2.m_x*p2.m_x)+(p2.m_y*p2.m_y)), p2.m_x, p2.m_y,
                                 ((p3.m_x*p3.m_x)+(p3.m_y*p3.m_y)), p3.m_x, p3.m_y);
        (*itr)->m_circum_centre_x = - bx/(2*a);
        (*itr)->m_circum_centre_y = - by/(2*a);
        (*itr)->m_circum_radius = sqrt((bx*bx)+(by*by)-(4*a*c))/(2*abs(a));
    }
}

//----------------------------------------------------------------------------------------------------------------------
void Delaunay::computeDelaunay()
{
    unsigned int size = m_vertices.size();

    // create a triangulation first
    computeTwoBoundTriangle();
    unsigned int i;
    for(i=0;i<size; i++)
        insertSite(i);

    // use swap method to create delaunay triangles
    //computeCircumCircleForAllFaces();
    //swapForDelaunay();

    // delete the wrapping triangle on the outside
    //deleteWrappingTriangle();
}

void Delaunay::insertSite(unsigned int _p)
{
    std::vector<DFace *>::iterator itr;
    unsigned int va, vb, vc;
    for(itr=m_faces.begin();itr!=m_faces.end();++itr)
    {
        findThreeVertices(*itr, &va, &vb, &vc);
        if(insideTriangle(_p, va, vb, vc)) // if the new vertex is lying inside this triangle, split this triangle into four triangles
        {
            splitTriangle(_p, *itr, va, vb, vc);
            return;
        }
    }
}
void Delaunay::deleteWrappingTriangle()
{
    unsigned int size = m_vertices.size();
    size = size-4;
    unsigned int numFaces = m_faces.size();
    unsigned int i=0;
    unsigned int va, vb, vc;
    while(i<numFaces)
    {
        findThreeVertices(m_faces[i], &va, &vb, &vc);
        if(va>=size || vb>=size || vc>=size) // this face related to the wrapping triangle, should be deleted
        {
            if(m_faces[i]->m_edges[0]->m_vertices[0]>=size || m_faces[i]->m_edges[0]->m_vertices[1]>=size) // this edge need to be deleted
            {
                if(m_faces[i]->m_edges[0]->m_faces[0] == NULL || m_faces[i]->m_edges[0]->m_faces[1] == NULL)
                    delete m_faces[i]->m_edges[0];
                else
                {
                    if(m_faces[i]->m_edges[0]->m_faces[0] == m_faces[i])
                        m_faces[i]->m_edges[0]->m_faces[0] = NULL;
                    else
                        m_faces[i]->m_edges[0]->m_faces[1] = NULL;
                }
            }
            else
            {
                if(m_faces[i]->m_edges[0]->m_faces[0] == m_faces[i])
                    m_faces[i]->m_edges[0]->m_faces[0] = NULL;
                else
                    m_faces[i]->m_edges[0]->m_faces[1] = NULL;
            }
            if(m_faces[i]->m_edges[1]->m_vertices[0]>=size || m_faces[i]->m_edges[1]->m_vertices[1]>=size) // this edge need to be deleted
            {
                if(m_faces[i]->m_edges[1]->m_faces[0] == NULL || m_faces[i]->m_edges[1]->m_faces[1] == NULL)
                    delete m_faces[i]->m_edges[1];
                else
                {
                    if(m_faces[i]->m_edges[1]->m_faces[0] == m_faces[i])
                        m_faces[i]->m_edges[1]->m_faces[0] = NULL;
                    else
                        m_faces[i]->m_edges[1]->m_faces[1] = NULL;
                }
            }
            else
            {
                if(m_faces[i]->m_edges[1]->m_faces[0] == m_faces[i])
                    m_faces[i]->m_edges[1]->m_faces[0] = NULL;
                else
                    m_faces[i]->m_edges[1]->m_faces[1] = NULL;
            }
            if(m_faces[i]->m_edges[2]->m_vertices[0]>=size || m_faces[i]->m_edges[2]->m_vertices[1]>=size) // this edge need to be deleted
            {
                if(m_faces[i]->m_edges[2]->m_faces[0] == NULL || m_faces[i]->m_edges[2]->m_faces[1] == NULL)
                    delete m_faces[i]->m_edges[2];
                else
                {
                    if(m_faces[i]->m_edges[2]->m_faces[0] == m_faces[i])
                        m_faces[i]->m_edges[2]->m_faces[0] = NULL;
                    else
                        m_faces[i]->m_edges[2]->m_faces[1] = NULL;
                }
            }
            else
            {
                if(m_faces[i]->m_edges[2]->m_faces[0] == m_faces[i])
                    m_faces[i]->m_edges[2]->m_faces[0] = NULL;
                else
                    m_faces[i]->m_edges[2]->m_faces[1] = NULL;
            }
            delete m_faces[i];
            m_faces.erase(m_faces.begin()+i);
            numFaces--;
        }
        else
            i++;
    }
    m_vertices.erase(m_vertices.begin()+size, m_vertices.end());
}

void Delaunay::findThreeVertices(DFace *_face, unsigned int *_pa, unsigned int *_pb, unsigned int *_pc)
{
    *_pa = (_face->m_edges[0]->m_faces[0]==_face)?(_face->m_edges[0]->m_vertices[0]):(_face->m_edges[0]->m_vertices[1]);
    *_pb = (_face->m_edges[1]->m_faces[0]==_face)?(_face->m_edges[1]->m_vertices[0]):(_face->m_edges[1]->m_vertices[1]);
    *_pc = (_face->m_edges[2]->m_faces[0]==_face)?(_face->m_edges[2]->m_vertices[0]):(_face->m_edges[2]->m_vertices[1]);
}

//          vc
//          /|\
//         / e3\
//        /  |  \
//       /f3 p f2\
//      /  /   \  \
//     / e1  f1  e2\
//   va-------------vb
void Delaunay::splitTriangle(unsigned int _p, DFace *_face, unsigned int _va, unsigned int _vb, unsigned _vc)
{
    // make a copy of the old face, then this allocated memory will be used for new face
    DFace oldFace;
    oldFace.m_edges[0] = _face->m_edges[0];
    oldFace.m_edges[1] = _face->m_edges[1];
    oldFace.m_edges[2] = _face->m_edges[2];
    DFace *f1 = _face;
    DFace *f2 = new DFace;
    DFace *f3 = new DFace;
    DEdge *e1 = new DEdge;
    DEdge *e2 = new DEdge;
    DEdge *e3 = new DEdge;

    f1->m_edges[0] = oldFace.m_edges[0];
    f1->m_edges[1] = e2;
    f1->m_edges[2] = e1;
    f2->m_edges[0] = oldFace.m_edges[1];
    f2->m_edges[1] = e3;
    f2->m_edges[2] = e2;
    f3->m_edges[0] = oldFace.m_edges[2];
    f3->m_edges[1] = e1;
    f3->m_edges[2] = e3;

    e1->m_vertices[0] = _p;
    e1->m_vertices[1] = _va;
    e1->m_faces[0] = f1;
    e1->m_faces[1] = f3;
    e2->m_vertices[0] = _p;
    e2->m_vertices[1] = _vb;
    e2->m_faces[0] = f2;
    e2->m_faces[1] = f1;
    e3->m_vertices[0] = _p;
    e3->m_vertices[1] = _vc;
    e3->m_faces[0] = f3;
    e3->m_faces[1] = f2;
    if(oldFace.m_edges[0]->m_faces[0] == _face)
        oldFace.m_edges[0]->m_faces[0] = f1;
    else
        oldFace.m_edges[0]->m_faces[1] = f1;
    if(oldFace.m_edges[1]->m_faces[0] == _face)
        oldFace.m_edges[1]->m_faces[0] = f2;
    else
        oldFace.m_edges[1]->m_faces[1] = f2;
    if(oldFace.m_edges[2]->m_faces[0] == _face)
        oldFace.m_edges[2]->m_faces[0] = f3;
    else
        oldFace.m_edges[2]->m_faces[1] = f3;
    m_faces.push_back(f2);
    m_faces.push_back(f3);
}

bool Delaunay::insideTriangle(unsigned int _p, unsigned int _va, unsigned int _vb, unsigned _vc)
{
    // find the tree vertices of the face, in the CCW order, assume original face's vertices are in CCW order
    ngl::Vec3 a, b, c, p, n;
    a = m_vertices[_va];
    b = m_vertices[_vb];
    c = m_vertices[_vc];
    p = m_vertices[_p];

    // Task 3: determine if point _p fall inside the triangle (_va, _vb, _vc)


    n=(b-a).cross((c-a));



    if(((p-a).dot((c-a).cross(n)))>0 &&
       ((p-b).dot((a-b).cross(n)))>0 &&
       ((p-c).dot((b-c).cross(n)))>0)
            return true;
    else
        return false;
}

//   v3---e3---v2
//    |       /|
//    |  f1  / |
//    |     /  |
//    e4  e2  e1
//    |   /    |
//    |  /  f0 |
//    | /      |
//    |/       |
//   v0---e0---v1

void Delaunay::computeTwoBoundTriangle()
{
    float xdim, ydim, zdim;

    // create the bounding two big triangles wrapping in all the input sites
    BoundBox box= computeBoundBox();
    xdim = box.m_xmax - box.m_xmin;
    ydim = box.m_ymax - box.m_ymin;
    zdim = box.m_zmax - box.m_zmin;

    // add the noise, to make sure no inputSite is lying exactly on edges
    ngl::Random *rnl=ngl::Random::instance();
    rnl->setSeed();
    ngl::Vec3 noise = rnl->getRandomVec3();
    xdim += xdim/1000.0*noise.m_x;
    ydim += ydim/1000.0*noise.m_y;
    zdim += zdim/1000.0*noise.m_z;

    // add vertices
    unsigned size = m_vertices.size();
    m_vertices.push_back(ngl::Vec3(box.m_xmin-0.5*xdim, box.m_ymin-0.5*ydim, 0.0));
    m_vertices.push_back(ngl::Vec3(box.m_xmax+0.5*xdim, box.m_ymin-0.5*ydim, 0.0));
    m_vertices.push_back(ngl::Vec3(box.m_xmax+0.5*xdim, box.m_ymax+0.5*ydim, 0.0));
    m_vertices.push_back(ngl::Vec3(box.m_xmin-0.5*xdim, box.m_ymax+0.5*ydim, 0.0));
    DFace   *face0 = new DFace;
    DFace   *face1 = new DFace;
    DEdge   *edge0 = new DEdge;
    DEdge   *edge1 = new DEdge;
    DEdge   *edge2 = new DEdge;
    DEdge   *edge3 = new DEdge;
    DEdge   *edge4 = new DEdge;
    face0->m_edges[0] = edge0;
    face0->m_edges[1] = edge1;
    face0->m_edges[2] = edge2;
    face1->m_edges[0] = edge2;
    face1->m_edges[1] = edge3;
    face1->m_edges[2] = edge4;
    edge0->m_faces[0] = face0;
    edge0->m_faces[1] = NULL;
    edge0->m_vertices[0] = size+0;
    edge0->m_vertices[1] = size+1;
    edge1->m_faces[0] = face0;
    edge1->m_faces[1] = NULL;
    edge1->m_vertices[0] = size+1;
    edge1->m_vertices[1] = size+2;
    edge2->m_faces[0] = face0;
    edge2->m_faces[1] = face1;
    edge2->m_vertices[0] = size+2;
    edge2->m_vertices[1] = size+0;
    edge3->m_faces[0] = face1;
    edge3->m_faces[1] = NULL;
    edge3->m_vertices[0] = size+2;
    edge3->m_vertices[1] = size+3;
    edge4->m_faces[0] = face1;
    edge4->m_faces[1] = NULL;
    edge4->m_vertices[0] = size+3;
    edge4->m_vertices[1] = size+0;

    m_faces.push_back(face0);
    m_faces.push_back(face1);
}

//----------------------------------------------------------------------------------------------------------------------

void Delaunay::meshQuality()
{
    // evaluate the mesh quality based on different criteria
    // such as:
    //  1. minimum angle
    //  2. maximum angle
    //  3. circumcircle radius/edge
    //  ...
}

//----------------------------------------------------------------------------------------------------------------------
void Delaunay::createVAO()
{
    if(m_vaoDelaunay!=0)
    {
        m_vaoDelaunay->unbind();
        m_vaoDelaunay->removeVOA();
        delete m_vaoDelaunay;
    }
    if(m_vaoVoronoi!=0)
    {
        m_vaoVoronoi->unbind();
        m_vaoVoronoi->removeVOA();
        delete m_vaoVoronoi;
    }
    // create Delaunay VAO
    m_vaoDelaunay=ngl::VertexArrayObject::createVOA(GL_LINES);
    m_vaoDelaunay->bind();

    std::vector<Data> lines;
    Data  tmp;
    std::vector<DFace *>::iterator itv;
    unsigned int count=0;
    for(itv=m_faces.begin(); itv!=m_faces.end();itv++)
    {
        tmp.m_x = m_vertices[(*itv)->m_edges[0]->m_vertices[0]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[0]->m_vertices[0]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[0]->m_vertices[0]].m_z;
        lines.push_back(tmp);
        tmp.m_x = m_vertices[(*itv)->m_edges[0]->m_vertices[1]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[0]->m_vertices[1]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[0]->m_vertices[1]].m_z;
        lines.push_back(tmp);
        tmp.m_x = m_vertices[(*itv)->m_edges[1]->m_vertices[0]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[1]->m_vertices[0]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[1]->m_vertices[0]].m_z;
        lines.push_back(tmp);
        tmp.m_x = m_vertices[(*itv)->m_edges[1]->m_vertices[1]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[1]->m_vertices[1]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[1]->m_vertices[1]].m_z;
        lines.push_back(tmp);
        tmp.m_x = m_vertices[(*itv)->m_edges[2]->m_vertices[0]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[2]->m_vertices[0]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[2]->m_vertices[0]].m_z;
        lines.push_back(tmp);
        tmp.m_x = m_vertices[(*itv)->m_edges[2]->m_vertices[1]].m_x;
        tmp.m_y = m_vertices[(*itv)->m_edges[2]->m_vertices[1]].m_y;
        tmp.m_z = m_vertices[(*itv)->m_edges[2]->m_vertices[1]].m_z;
        lines.push_back(tmp);
        count+=6;
    }
    m_vaoDelaunay->setData(count*sizeof(Data),lines[0].m_x);
    m_vaoDelaunay->setNumIndices(count);
    m_vaoDelaunay->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(Data),0);
    m_vaoDelaunay->unbind();

    lines.clear();

    // create Voronoi VAO
    m_vaoVoronoi=ngl::VertexArrayObject::createVOA(GL_LINES);
    m_vaoVoronoi->bind();

    count=0;
    ngl::Vec3 centre, vtmp;
    unsigned int va, vb, vc;
    for(itv=m_faces.begin(); itv!=m_faces.end();itv++)
    {
        findThreeVertices(*itv, &va, &vb, &vc);
        centre =(m_vertices[va]+m_vertices[vb]+m_vertices[vc])/3.0;

        tmp.m_x = centre.m_x;
        tmp.m_y = centre.m_y;
        tmp.m_z = centre.m_z;
        lines.push_back(tmp);
        vtmp = (m_vertices[(*itv)->m_edges[0]->m_vertices[0]]+m_vertices[(*itv)->m_edges[0]->m_vertices[1]])/2.0;
        if((*itv)->m_edges[0]->m_faces[0]== NULL || (*itv)->m_edges[0]->m_faces[1]== NULL)
        {
            // this is the boundary, the voronoi line should cross the edge
            vtmp = 5.0*(vtmp-centre)+centre;
        }
        tmp.m_x = vtmp.m_x;
        tmp.m_y = vtmp.m_y;
        tmp.m_z = vtmp.m_z;
        lines.push_back(tmp);

        tmp.m_x = centre.m_x;
        tmp.m_y = centre.m_y;
        tmp.m_z = centre.m_z;
        lines.push_back(tmp);
        vtmp = (m_vertices[(*itv)->m_edges[1]->m_vertices[0]]+m_vertices[(*itv)->m_edges[1]->m_vertices[1]])/2.0;
        if((*itv)->m_edges[1]->m_faces[0]== NULL || (*itv)->m_edges[1]->m_faces[1]== NULL)
        {
            // this is the boundary, the voronoi line should cross the edge
            vtmp = 5.0*(vtmp-centre)+centre;
        }
        tmp.m_x = vtmp.m_x;
        tmp.m_y = vtmp.m_y;
        tmp.m_z = vtmp.m_z;
        lines.push_back(tmp);

        tmp.m_x = centre.m_x;
        tmp.m_y = centre.m_y;
        tmp.m_z = centre.m_z;
        lines.push_back(tmp);
        vtmp = (m_vertices[(*itv)->m_edges[2]->m_vertices[0]]+m_vertices[(*itv)->m_edges[2]->m_vertices[1]])/2.0;
        if((*itv)->m_edges[2]->m_faces[0]== NULL || (*itv)->m_edges[2]->m_faces[1]== NULL)
        {
            // this is the boundary, the voronoi line should cross the edge
            vtmp = 5.0*(vtmp-centre)+centre;
        }
        tmp.m_x = vtmp.m_x;
        tmp.m_y = vtmp.m_y;
        tmp.m_z = vtmp.m_z;
        lines.push_back(tmp);

        count+=6;
    }
    m_vaoVoronoi->setData(count*sizeof(Data),lines[0].m_x);
    m_vaoVoronoi->setNumIndices(count);
    m_vaoVoronoi->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(Data),0);
    m_vaoVoronoi->unbind();

    lines.clear();
}

//----------------------------------------------------------------------------------------------------------------------
