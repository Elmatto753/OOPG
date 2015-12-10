#include "HalfEdgeMesh.h"

//----------------------------------------------------------------------------------------------------------------------
/// @file HalfEdgeMesh.cpp
/// @brief the basic Half Edge data structure
///
/// Workshop:
/// 1. computeVertexNormal()
/// 2. findOneRingNeighbours()
/// 3. computeFirstRingArea()
/// 4. computeMeanCurvature()
//----------------------------------------------------------------------------------------------------------------------

HalfEdgeMesh::HalfEdgeMesh(ngl::Obj* _objMesh)
{
    unsigned int i, j;

    m_vbo=false;
    m_vao=false;
    m_ext=new ngl::BBox(_objMesh->getBBox());
    m_nVerts=_objMesh->getNumVerts();
    m_center = _objMesh->getCenter();

    // convert data from obj into HalfEdge
    // 1. copy the vertex data
    HE_Vertex tVertex;
    std::vector<ngl::Vec3> verts = _objMesh->getVertexList();

    for(i=0; i< m_nVerts; i++)
    {
        tVertex.m_vert = verts[i];
        tVertex.m_outHalfEdge = NULL;
        m_verts.push_back(tVertex);
    }

    // parsing through face list to create HE_Face, HalfEdge structure, set m_outHalfEdge in Vertex
    unsigned int numVertexInFace;
    // keep a temporary record of all HE_Face
    std::vector<HE_Face *> tFaceList;
    std::vector<ngl::Face> objFaceList = _objMesh->getFaceList();
    for(std::vector<ngl::Face>::iterator itr=objFaceList.begin(); itr!=objFaceList.end(); ++itr)
    {
        numVertexInFace = itr->m_vert.size();
        HE_Face *newFace = new HE_FACE;
        HalfEdge *newHEList = new HalfEdge[numVertexInFace];
        for(i=0;i<numVertexInFace;i++)
        {
            newHEList[i].m_face = newFace;
            newHEList[i].m_next = &newHEList[(i==numVertexInFace-1)?0:i+1];
            newHEList[i].m_dual = NULL;
            newHEList[i].m_toVertex = itr->m_vert[(i==numVertexInFace-1)?0:i+1];
            if(m_verts[itr->m_vert[i]].m_outHalfEdge==NULL)
                m_verts[itr->m_vert[i]].m_outHalfEdge = &newHEList[i];
        }
        newFace->m_halfEdge = &newHEList[0];
        newFace->flag = false;
        tFaceList.push_back(newFace);
    }

    // create the dual halfedge
    unsigned long int startV, endV;
    unsigned int numVertexInFace1;
    std::vector<HE_Face*>::iterator itrHE= tFaceList.begin();
    for(std::vector<ngl::Face>::iterator itr=objFaceList.begin(); itr!=objFaceList.end(); ++itr)
    {
        numVertexInFace = itr->m_vert.size();
        HalfEdge *tHE = (*itrHE)->m_halfEdge;
        for(i=0;i<numVertexInFace;i++)
        {
            if(tHE->m_dual!= NULL)
            {
                tHE = tHE->m_next;
                continue;
            }
            startV = itr->m_vert[i];
            endV = itr->m_vert[(i==numVertexInFace-1)?0:i+1];
            std::vector<HE_Face*>::iterator itrHE1= tFaceList.begin();
            for(std::vector<ngl::Face>::iterator itr1=objFaceList.begin(); itr1!=objFaceList.end(); ++itr1)
            {
                numVertexInFace1 = itr1->m_vert.size();
                HalfEdge *tHE1 = (*itrHE1)->m_halfEdge;
                for(j=0;j<numVertexInFace1;j++)
                {
                    if(endV == itr1->m_vert[j] && startV == itr1->m_vert[(j==numVertexInFace1-1)?0:j+1])
                    {
                        tHE->m_dual = tHE1;
                        tHE1->m_dual = tHE;
                    }
                    tHE1 = tHE1->m_next;
                }
                itrHE1++;
            }
            tHE = tHE->m_next;
        }
        itrHE++;
    }

    // loading data finished
    m_loaded=true;
    tFaceList.erase(tFaceList.begin(), tFaceList.end());
    objFaceList.erase(objFaceList.begin(), objFaceList.end());

    // compute the vertex normal
    computeVertexNormal();
    // compute curvature
    //computeGaussianCurvature();
    computeMeanCurvature();
    // map curvature to color
    //mapCurvaturetoColor();
}

void HalfEdgeMesh::drawBBox() const
{
    m_ext->draw();
}

//----------------------------------------------------------------------------------------------------------------------
HalfEdgeMesh::~HalfEdgeMesh()
{
    deleteHalfEdgeDataStructure();

    if(m_loaded == true)
    {
        m_verts.erase(m_verts.begin(),m_verts.end());

        if(m_vbo)
        {
            //glDeleteBuffers(1,&m_vboBuffers);
            if(m_vaoMesh!=0)
            {
                delete m_vaoMesh;
            }
        }
        if(m_ext !=0)
        {
            delete m_ext;
        }
    }
}

void HalfEdgeMesh::deleteHalfEdgeDataStructure()
{
    unsigned int i;

    std::vector<bool> vertexVisited(m_nVerts, false);
    unsigned int numVertexVisited = 0;
    while(numVertexVisited< m_nVerts)
    {
        // find the first vertex not visited yet
        std::vector<bool>::iterator itrVV = vertexVisited.begin();
        i=0;
        while(*itrVV)
        {
            itrVV++; // it must stop before m_nVerts
            i++;
        }
        HE_Face *tFace = m_verts[i].m_outHalfEdge->m_face;
        bool oldFlag = tFace->flag;
        bool newFlag = oldFlag?false:true;

        std::vector<HE_Face *> faceList;
        std::vector<HalfEdge *> HElist;

        std::queue<HE_Face *> workingList;
        workingList.push(tFace);
        tFace->flag = newFlag;

        HE_Face *front;
        HalfEdge    *tHE, *startHE;
        while(!workingList.empty())
        {
            front = workingList.front();
            startHE = front->m_halfEdge;
            HElist.push_back(startHE);
            if(startHE->m_dual->m_face->flag==oldFlag)
            {
                workingList.push(startHE->m_dual->m_face);
                startHE->m_dual->m_face->flag = newFlag;
            }
            printf("%d (%d)", startHE->m_toVertex, startHE->m_dual->m_toVertex);// debug purpose
            if(!vertexVisited[startHE->m_toVertex])
            {
                vertexVisited[startHE->m_toVertex]=true;
                numVertexVisited++;
            }
            tHE = startHE->m_next;
            while(tHE!=startHE)
            {
                if(tHE->m_dual->m_face->flag==oldFlag)
                {
                    workingList.push(tHE->m_dual->m_face);
                    tHE->m_dual->m_face->flag = newFlag;
                }
                printf("%d (%d)", tHE->m_toVertex, tHE->m_dual->m_toVertex); // debug purpose
                if(!vertexVisited[tHE->m_toVertex])
                {
                    vertexVisited[tHE->m_toVertex]=true;
                    numVertexVisited++;
                }
                HElist.push_back(tHE);
                tHE=tHE->m_next;
            }
            printf("\n");
            workingList.pop();
            faceList.push_back(front);
        }

        for(std::vector<HE_Face *>::iterator itr= faceList.begin(); itr!=faceList.end(); itr++)
            delete (*itr);
        for(std::vector<HalfEdge *>::iterator itr1 = HElist.begin(); itr1!=HElist.end(); itr1++)
            delete (*itr1);
        faceList.erase(faceList.begin(), faceList.end());
        HElist.erase(HElist.begin(),HElist.end());
    }

    vertexVisited.erase(vertexVisited.begin(), vertexVisited.end());
    m_verts.erase(m_verts.begin(), m_verts.end());
}

void HalfEdgeMesh::computeVertexNormal()
{
    unsigned int j=0;
    for(std::vector<HE_Vertex>::iterator itr = m_verts.begin(); itr!=m_verts.end(); ++itr)
    {
        // find all the one ring neighbours
        std::vector<unsigned int> oneRingNeigh=findOneRingNeighbours(j);

        // compute the equally weighted normal
        unsigned int numNeigh = oneRingNeigh.size();
        ngl::Vec3 norm(0.0,0.0,0.0), tmpN;

        // Task 1
        int x, next;
        for(x=0; x<numNeigh; x++)
        {
          next=((x==numNeigh-1)?0:x+1);

          tmpN.cross(m_verts[oneRingNeigh[x]].m_vert - itr->m_vert,
                     m_verts[oneRingNeigh[next]].m_vert - itr->m_vert);
          tmpN.normalize();

          norm+=tmpN;
        }


        itr->m_norm = -norm;

        // free the memory
        oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
        j++;
    }
}

void HalfEdgeMesh::mapCurvaturetoColor()
{
    float maxCurv, minCurv, tmpCurv;
    float r, g, b;
    std::vector<HE_Vertex>::iterator itr = m_verts.begin();
    maxCurv = minCurv = itr->m_curvature;
    itr++;
    for(; itr!=m_verts.end(); ++itr)
    {
        if(maxCurv<itr->m_curvature) maxCurv = itr->m_curvature;
        if(minCurv>itr->m_curvature) minCurv = itr->m_curvature;
    }
    if(fabs(maxCurv-minCurv)<0.0001) // all curvature are same for all vertices
    {
        for(itr=m_verts.begin(); itr!=m_verts.end(); ++itr)
        {
            itr->m_color = ngl::Vec3(0.5, 0.5, 0.5);
        }
    }
    else
    {
        for(itr=m_verts.begin(); itr!=m_verts.end(); ++itr)
        {
            tmpCurv = (itr->m_curvature-minCurv)/(maxCurv-minCurv);
            r = tmpCurv<0.333?1.0:(tmpCurv>0.666?0.0:(0.666-tmpCurv)/0.333);
            g = tmpCurv<0.333?tmpCurv/0.333:(tmpCurv<0.666?1.0:(1.0-tmpCurv)/0.334);
            b = tmpCurv<0.333?0.0:(tmpCurv<0.666?(tmpCurv-0.333)/0.333:1.0);
            itr->m_color = ngl::Vec3(0.5*r, 0.5*g, 0.5*b);
        }
    }
}

std::vector<unsigned int> HalfEdgeMesh::findOneRingNeighbours(unsigned int _indexOfVertex)
{
    HE_Vertex   centreVertex = m_verts[_indexOfVertex];

    // find all the one ring neighbours
    std::vector<unsigned int> oneRingNeigh;

    // Task 2:
    HalfEdge *startHE = centreVertex.m_outHalfEdge;
    oneRingNeigh.push_back(startHE->m_toVertex);
    HalfEdge *nextHE = startHE->m_dual->m_next;

    while(nextHE!=startHE)
    {
      oneRingNeigh.push_back(nextHE->m_toVertex);
      nextHE=nextHE->m_dual->m_next;
    }



    return oneRingNeigh;
}

float HalfEdgeMesh::computeFirstRingArea(unsigned int _indexOfVertex)
{
    HE_Vertex   centreVertex = m_verts[_indexOfVertex];

    // find all the one ring neighbours
    std::vector<unsigned int> oneRingNeigh=findOneRingNeighbours(_indexOfVertex);

    // compute the equally weighted normal
    unsigned int numNeigh = oneRingNeigh.size();
    float area = 0.0;
    ngl::Vec3 a, b, c;

    // Task 3
    unsigned int i, next;
    for(i=0;i<numNeigh;i++)
    {

      next=((i==numNeigh-1)?0:i+1);
      a = (m_verts[oneRingNeigh[i]].m_vert - centreVertex.m_vert);
      b = (m_verts[oneRingNeigh[next]].m_vert - centreVertex.m_vert);
      c.cross(a,b);
      area += 0.5 * c.length();
    }





    // free the memory
    oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
    return area;
}

void HalfEdgeMesh::computeGaussianCurvature()
{
    unsigned int j=0;
    for(std::vector<HE_Vertex>::iterator itr = m_verts.begin(); itr!=m_verts.end(); ++itr)
    {
        // find all the one ring neighbours
        std::vector<unsigned int> oneRingNeigh=findOneRingNeighbours(j);
        float area = computeFirstRingArea(j);

        // compute the equally weighted normal
        unsigned int numNeigh = oneRingNeigh.size();
        float alpha=0.0;
        ngl::Vec3 v1, v2, tmpA;
        unsigned int i, next;
        for(i=0;i<numNeigh;i++)
        {
            next=((i==numNeigh-1)?0:i+1);
            v1=m_verts[oneRingNeigh[i]].m_vert - itr->m_vert;
            v2=m_verts[oneRingNeigh[next]].m_vert - itr->m_vert;
            v1.normalize();
            v2.normalize();
            tmpA.cross(v1,v2);
            alpha += asin(tmpA.length());
        }
        itr->m_curvature = (2*pi-alpha)*3.0/area;

        // free the memory
        oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
        j++;
    }
}

void HalfEdgeMesh::computeMeanCurvature()
{
    unsigned int j=0;
    for(std::vector<HE_Vertex>::iterator itr = m_verts.begin(); itr!=m_verts.end(); ++itr)
    {
        // find all the one ring neighbours
        std::vector<unsigned int> oneRingNeigh=findOneRingNeighbours(j);
        float area = computeFirstRingArea(j);

        // compute the equally weighted normal
        unsigned int numNeigh = oneRingNeigh.size();
        std::vector<ngl::Vec3> norm(numNeigh);
        std::vector<float> edgeLen(numNeigh);
        float curv = 0.0;

        // Task 4
        ngl::Vec3 a, b, tmpN;
        unsigned int i, next;
        for(i=0; i<numNeigh; i++)
        {
          next=((i==numNeigh-1)?0:i+1);
          a = (m_verts[oneRingNeigh[i]].m_vert - itr->m_vert);
          b = (m_verts[oneRingNeigh[next]].m_vert - itr->m_vert);
          tmpN.cross(a, b);
          tmpN.normalize();
          norm[i]=tmpN;
          edgeLen[i]=b.length();

        }

        for(i=0;i<numNeigh;i++)
        {
          next=((i==numNeigh-1)?0:i+1);
          tmpN.cross(norm[i], norm[next]);
          curv+=edgeLen[i]*asin(tmpN.length());
        }


        itr->m_curvature = 0.75*curv/area;

        // free the memory
        oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
        j++;
    }
}

void HalfEdgeMesh::createVAO()
{
    // if we have already created a VBO just return.
    if(m_vao == true)
    {
        std::cout<<"VAO exist so returning\n";
        return;
    }
    // else allocate space as build our VAO
    m_dataPackType=GL_TRIANGLES;

    // now we are going to process and pack the mesh into an ngl::VertexArrayObject
    std::vector <VertData> vboMesh;
    VertData d;
    unsigned int    i;

    std::vector<bool> vertexVisited(m_nVerts, false);
    unsigned int numVertexVisited = 0;
    while(numVertexVisited< m_nVerts)
    {
        // find the first vertex not visited yet
        std::vector<bool>::iterator itrVV = vertexVisited.begin();
        i=0;
        while(*itrVV)
        {
            itrVV++; // it must stop before m_nVerts
            i++;
        }
        // find the first face not visited yet
        HE_Face *tFace = m_verts[i].m_outHalfEdge->m_face;
        bool oldFlag = tFace->flag;
        bool newFlag = oldFlag?false:true;

        std::queue<HE_Face *> workingList;
        workingList.push(tFace);
        tFace->flag = newFlag;

        HE_Face     *front;
        HalfEdge    *tHE, *startHE;
        while(!workingList.empty())
        {
            front = workingList.front();
            startHE = front->m_halfEdge;
            if(startHE->m_dual->m_face->flag==oldFlag)
            {
                workingList.push(startHE->m_dual->m_face);
                startHE->m_dual->m_face->flag = newFlag;
            }
            if(!vertexVisited[startHE->m_toVertex])
            {
                vertexVisited[startHE->m_toVertex]=true;
                numVertexVisited++;
            }
            tHE = startHE->m_next;
            while(tHE!=startHE)
            {
                d.x=m_verts[startHE->m_dual->m_toVertex].m_vert.m_x;
                d.y=m_verts[startHE->m_dual->m_toVertex].m_vert.m_y;
                d.z=m_verts[startHE->m_dual->m_toVertex].m_vert.m_z;
                d.nx=m_verts[startHE->m_dual->m_toVertex].m_norm.m_x;
                d.ny=m_verts[startHE->m_dual->m_toVertex].m_norm.m_y;
                d.nz=m_verts[startHE->m_dual->m_toVertex].m_norm.m_z;
                d.r=m_verts[startHE->m_dual->m_toVertex].m_color.m_x;
                d.g=m_verts[startHE->m_dual->m_toVertex].m_color.m_y;
                d.b=m_verts[startHE->m_dual->m_toVertex].m_color.m_z;
                vboMesh.push_back(d);
                d.x=m_verts[tHE->m_dual->m_toVertex].m_vert.m_x;
                d.y=m_verts[tHE->m_dual->m_toVertex].m_vert.m_y;
                d.z=m_verts[tHE->m_dual->m_toVertex].m_vert.m_z;
                d.nx=m_verts[tHE->m_dual->m_toVertex].m_norm.m_x;
                d.ny=m_verts[tHE->m_dual->m_toVertex].m_norm.m_y;
                d.nz=m_verts[tHE->m_dual->m_toVertex].m_norm.m_z;
                d.r=m_verts[tHE->m_dual->m_toVertex].m_color.m_x;
                d.g=m_verts[tHE->m_dual->m_toVertex].m_color.m_y;
                d.b=m_verts[tHE->m_dual->m_toVertex].m_color.m_z;
                vboMesh.push_back(d);
                d.x=m_verts[tHE->m_toVertex].m_vert.m_x;
                d.y=m_verts[tHE->m_toVertex].m_vert.m_y;
                d.z=m_verts[tHE->m_toVertex].m_vert.m_z;
                d.nx=m_verts[tHE->m_toVertex].m_norm.m_x;
                d.ny=m_verts[tHE->m_toVertex].m_norm.m_y;
                d.nz=m_verts[tHE->m_toVertex].m_norm.m_z;
                d.r=m_verts[tHE->m_toVertex].m_color.m_x;
                d.g=m_verts[tHE->m_toVertex].m_color.m_y;
                d.b=m_verts[tHE->m_toVertex].m_color.m_z;
                vboMesh.push_back(d);
                if(tHE->m_dual->m_face->flag==oldFlag)
                {
                    workingList.push(tHE->m_dual->m_face);
                    tHE->m_dual->m_face->flag = newFlag;
                }
                if(!vertexVisited[tHE->m_toVertex])
                {
                    vertexVisited[tHE->m_toVertex]=true;
                    numVertexVisited++;
                }
                tHE=tHE->m_next;
            }
            workingList.pop();
        }
    }

    // first we grab an instance of our VOA
    m_vaoMesh= ngl::VertexArrayObject::createVOA(m_dataPackType);
    // next we bind it so it's active for setting data
    m_vaoMesh->bind();
    m_meshSize=vboMesh.size();

    // now we have our data add it to the VAO, we need to tell the VAO the following
    // how much (in bytes) data we are copying
    // a pointer to the first element of data (in this case the address of the first element of the
    // std::vector
    m_vaoMesh->setData(m_meshSize*sizeof(VertData),vboMesh[0].nx);
    // in this case we have packed our data in interleaved format as follows
    // nx,ny,nz,x,y,z,r,g,b
    // If you look at the shader we have the following attributes being used
    // attribute vec3 inVert; attribute 0
    // attribute vec3 inNormal; attribure 1
    // attribute vec3 inColor; attribure 2
    // so we need to set the vertexAttributePointer so the correct size and type as follows
    // vertex is attribute 0 with x,y,z(3) parts of type GL_FLOAT, our complete packed data is
    // sizeof(vertData)
    m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(VertData),3);
    // normal same as vertex only starts at position 0
    m_vaoMesh->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(VertData),0);
    // color same as vertex only starts at position 6
    m_vaoMesh->setVertexAttributePointer(2,3,GL_FLOAT,sizeof(VertData),6);


    // now we have set the vertex attributes we tell the VAO class how many indices to draw when
    // glDrawArrays is called, in this case we use buffSize (but if we wished less of the sphere to be drawn we could
    // specify less (in steps of 3))
    m_vaoMesh->setNumIndices(m_meshSize);
    // finally we have finished for now so time to unbind the VAO
    m_vaoMesh->unbind();

    // indicate we have a vao now
    m_vao=true;
}



//----------------------------------------------------------------------------------------------------------------------
void HalfEdgeMesh::draw() const
{
  if(m_vao == true)
  {
    m_vaoMesh->bind();
    m_vaoMesh->draw();
    m_vaoMesh->unbind();
  }
}

