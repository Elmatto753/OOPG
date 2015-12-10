#include "HalfEdgeMesh.h"

//----------------------------------------------------------------------------------------------------------------------
/// @file HalfEdgeMesh.cpp
/// @brief the basic Half Edge data structure
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
            newHEList[i].m_flag = false;
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

ngl::Vec3 HalfEdgeMesh::computeFaceCentre(HE_Face *_f)
{
    unsigned int i=1;
    ngl::Vec3 sum(0.0);
    // filling in task one
    HalfEdge *startEdge = _f->m_halfEdge;
    HalfEdge *newEdge=startEdge;
    sum+=m_verts[newEdge->m_toVertex].m_vert;
    newEdge=newEdge->m_next;

    while(newEdge != startEdge)
    {
      sum+=m_verts[newEdge->m_toVertex].m_vert;
      newEdge=newEdge->m_next;
      i++;
    }

    sum=sum/i;

    return sum;

}

void HalfEdgeMesh::CCSubdivision()
{
    // find the centre of each face, and add to the vertex list
    std::vector<HE_Face *> faceList = findAllFaces();
    HE_Vertex tmpV;
    for(std::vector<HE_Face *>::iterator itr=faceList.begin();itr!=faceList.end();++itr)
    {
        tmpV.m_vert = computeFaceCentre(*itr);
        m_verts.push_back(tmpV);
        (*itr)->m_centre = m_verts.size()-1;
    }

    // compute the new centre for each edge
    std::vector<HalfEdge *> edgeList; // each pair of dual half edge only store one in this list
    bool oldEdgeFlag = m_verts[0].m_outHalfEdge->m_flag;
    bool newEdgeFlag = oldEdgeFlag?false:true;
    HalfEdge *startHE, *tHE;
    for(std::vector<HE_Face *>::iterator itr=faceList.begin();itr!=faceList.end();++itr)
    {
        startHE = (*itr)->m_halfEdge;
        if(startHE->m_flag == oldEdgeFlag)
        {
            tmpV.m_vert = 0.25*((m_verts[startHE->m_toVertex].m_vert)+
                                (m_verts[startHE->m_dual->m_toVertex].m_vert)+
                                (m_verts[startHE->m_face->m_centre].m_vert)+
                                (m_verts[startHE->m_dual->m_face->m_centre].m_vert));  //// filling in task two
            m_verts.push_back(tmpV);
            startHE->m_centre = startHE->m_dual->m_centre = m_verts.size()-1;
            startHE->m_flag = startHE->m_dual->m_flag = newEdgeFlag;
            edgeList.push_back(startHE);
        }
        tHE = startHE->m_next;
        while(tHE!=startHE)
        {
            if(tHE->m_flag == oldEdgeFlag)
            {
                tmpV.m_vert = 0.25*((m_verts[tHE->m_toVertex].m_vert)+
                                    (m_verts[tHE->m_dual->m_toVertex].m_vert)+
                                    (m_verts[tHE->m_face->m_centre].m_vert)+
                                    (m_verts[tHE->m_dual->m_face->m_centre].m_vert));  //// filling in task two
                m_verts.push_back(tmpV);
                tHE->m_centre = tHE->m_dual->m_centre = m_verts.size()-1;
                tHE->m_flag = tHE->m_dual->m_flag = newEdgeFlag;
                edgeList.push_back(tHE);
            }
            tHE = tHE->m_next;
        }
    }

    // compute the new position for each old vertex

    // first make a copy of all the old vertices, not including new face-point and edge-point at the end after m_nVerts
    std::vector<ngl::Vec3> oldPos;
    unsigned int i=0;
    for(i=0;i<m_nVerts;i++)
    {
        oldPos.push_back(m_verts[i].m_vert);
    }

    // compute the new position
    unsigned int valence;

    for(i=0;i<m_nVerts;i++)
    {
        ngl::Vec3 tmp(0,0,0);
        valence = 1;

        startHE = m_verts[i].m_outHalfEdge;

        // filling in task three

        HalfEdge *newHE=startHE;
        tmp+=(oldPos[newHE->m_toVertex])+(m_verts[newHE->m_face->m_centre].m_vert);
        newHE=newHE->m_dual->m_next;
        while(newHE != startHE)
        {
          tmp+=(oldPos[newHE->m_toVertex])+(m_verts[newHE->m_face->m_centre].m_vert);
          newHE=newHE->m_dual->m_next;
          valence++;
        }

        tmp += oldPos[i] * valence * (valence - 2);
        m_verts[i].m_vert = tmp/(valence*valence);
    }

    // all new vertices has been created
    m_nVerts = m_verts.size();

    // change topology

    // 1. split each edge into two edges, now the edgeList only hold one of each pair of dual halfedges.
    for(std::vector<HalfEdge *>::iterator itr=edgeList.begin();itr!=edgeList.end();++itr)
        splitDualHalfEdge(*itr);

    for(std::vector<HE_Face *>::iterator itr=faceList.begin();itr!=faceList.end();++itr)
        splitFace(*itr);


    // clear up all memory allocation
    oldPos.erase(oldPos.begin(), oldPos.end());
    faceList.erase(faceList.begin(), faceList.end());
    edgeList.erase(edgeList.begin(), edgeList.end());
}

void HalfEdgeMesh::splitFace(HE_Face *_hf)
{
    HalfEdge * startHE = _hf->m_halfEdge;
    HalfEdge *tHE = startHE->m_next->m_next;

    std::vector<HalfEdge *> oldHalfEdgeListInFace; // start from the second halfedge of the fist side of the face
    oldHalfEdgeListInFace.push_back(startHE->m_next); // first edge for the first new face
    unsigned int numNewFaces = 1;
    while(tHE!=startHE)
    {
        oldHalfEdgeListInFace.push_back(tHE); // second edge for the new face
        oldHalfEdgeListInFace.push_back(tHE->m_next); // first edge for the next new face
        tHE=tHE->m_next->m_next;
        numNewFaces++;
    }
    oldHalfEdgeListInFace.push_back(startHE);

    HE_Face **newFaceList = new HE_Face*[numNewFaces];
    HalfEdge **newHEList = new HalfEdge*[2*numNewFaces];

    for(unsigned int i=0;i<numNewFaces;i++)
    {
        newFaceList[i] = new HE_Face;
        newHEList[2*i] = new HalfEdge;
        newHEList[2*i+1] = new HalfEdge;
    }
    // settle vertex data structure
    m_verts[_hf->m_centre].m_outHalfEdge = newHEList[1]; // newHEList[0] is the halfEdge pointing into vertex centre, newHEList[1] pointing out from centre

    // settle face data structre
    for(unsigned int i=0;i<numNewFaces;++i)
    {
        newFaceList[i]->flag = _hf->flag;
        newFaceList[i]->m_halfEdge = newHEList[i*2];
    }

    // settle Half Edge data structure
    for(unsigned int i=0;i<numNewFaces;++i)
    {
        oldHalfEdgeListInFace[2*i]->m_face = newFaceList[i];
        oldHalfEdgeListInFace[2*i+1]->m_face = newFaceList[i];
        oldHalfEdgeListInFace[2*i+1]->m_next = newHEList[2*i];

        newHEList[2*i]->m_dual = newHEList[(2*i+3)%(2*numNewFaces)];
        newHEList[2*i]->m_face = newFaceList[i];
        newHEList[2*i]->m_flag = oldHalfEdgeListInFace[2*i]->m_flag;
        newHEList[2*i]->m_next = newHEList[2*i+1];
        newHEList[2*i]->m_toVertex = _hf->m_centre;

        newHEList[2*i+1]->m_dual = newHEList[(2*i-2+2*numNewFaces)%(2*numNewFaces)];
        newHEList[2*i+1]->m_face = newFaceList[i];
        newHEList[2*i+1]->m_flag = oldHalfEdgeListInFace[2*i]->m_flag;
        newHEList[2*i+1]->m_next = oldHalfEdgeListInFace[2*i];
        newHEList[2*i+1]->m_toVertex = oldHalfEdgeListInFace[2*i]->m_dual->m_toVertex;
    }

    delete _hf; // delete the old face
}

void HalfEdgeMesh::splitDualHalfEdge(HalfEdge *_he)
{
    HalfEdge    *e1 = _he;
    HalfEdge    *e2 = _he->m_dual;
    HalfEdge    e1Copy = *e1;
    HalfEdge    e2Copy = *e2;
    HalfEdge    *e1Next = new HalfEdge;
    HalfEdge    *e2Next = new HalfEdge;

    // settle the vertex data structure
    unsigned int centre = e1->m_centre;
    m_verts[e1->m_centre].m_outHalfEdge = e1Next;

    // nothing needs to change for the face data structure

    // settle the halfEdge information

    // filling in task four
     e1->m_dual = e2Next;
     e1->m_next = e1Next;
     e1->m_toVertex = centre;

     e2->m_dual = e1Next;
     e2->m_next = e2Next;
     e2->m_toVertex = centre;

     e1Next->m_dual = e2;
     e1Next->m_face = e1->m_face;
     e1Next->m_flag = e1->m_flag;
     e1Next->m_next = e1Copy.m_next;
     e1Next->m_toVertex = e1Copy.m_toVertex;

     e2Next->m_dual = e1;
     e2Next->m_face = e2->m_face;
     e2Next->m_flag = e2->m_flag;
     e2Next->m_next = e2Copy.m_next;
     e2Next->m_toVertex = e2Copy.m_toVertex;

}

std::vector<HE_Face *> HalfEdgeMesh::findAllFaces()
{
    unsigned int i;
    std::vector<HE_Face *> faceList;

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


        std::queue<HE_Face *> workingList;
        workingList.push(tFace);
        tFace->flag = newFlag;

        HE_Face *front;
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
            faceList.push_back(front);
        }
    }

    vertexVisited.erase(vertexVisited.begin(), vertexVisited.end());
    return faceList;
}

void HalfEdgeMesh::deleteHalfEdgeDataStructure()
{
    std::vector<HE_Face *> faceList = findAllFaces();
    std::vector<HalfEdge *> HElist;

    HalfEdge *startHE, *tHE;
    for(std::vector<HE_Face *>::iterator itr= faceList.begin(); itr!=faceList.end(); itr++)
    {
        startHE = (*itr)->m_halfEdge;
        HElist.push_back(startHE);
        tHE = startHE->m_next;
        while(tHE!=startHE)
        {
            HElist.push_back(tHE);
            tHE = tHE->m_next;
        }
        delete (*itr);
    }
    for(std::vector<HalfEdge *>::iterator itr1 = HElist.begin(); itr1!=HElist.end(); itr1++)
        delete (*itr1);
    faceList.erase(faceList.begin(), faceList.end());
    HElist.erase(HElist.begin(),HElist.end());
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
        unsigned int i, next;
        for(i=0;i<numNeigh;i++)
        {
            next=((i==numNeigh-1)?0:i+1);
            tmpN.cross(m_verts[oneRingNeigh[i]].m_vert - itr->m_vert, m_verts[oneRingNeigh[next]].m_vert - itr->m_vert);
            tmpN.normalize();
            norm+=tmpN;
        }
        norm/=numNeigh;
        norm.normalize();
        itr->m_norm = norm;

        // free the memory
        oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
        j++;
    }
}

std::vector<unsigned int> HalfEdgeMesh::findOneRingNeighbours(unsigned int _indexOfVertex)
{
    HE_Vertex   centreVertex = m_verts[_indexOfVertex];

    // find all the one ring neighbours
    std::vector<unsigned int> oneRingNeigh;
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
    float area = 0.0, a, b, c;
    unsigned int i, next;
    for(i=0;i<numNeigh;i++)
    {
        next=((i==numNeigh-1)?0:i+1);
        a = (m_verts[oneRingNeigh[i]].m_vert - centreVertex.m_vert).length();
        b = (m_verts[oneRingNeigh[i]].m_vert - m_verts[oneRingNeigh[next]].m_vert).length();
        c = (m_verts[oneRingNeigh[next]].m_vert - centreVertex.m_vert).length();
        area += 0.25*sqrt((a+b+c)*(b+c-a)*(c+a-b)*(a+b-c));
    }

    // free the memory
    oneRingNeigh.erase(oneRingNeigh.begin(), oneRingNeigh.end());
    return area;
}

void HalfEdgeMesh::createVAO()
{
    // if we have already created a VBO just return.
    if(m_vao == true)
    {
        //glDeleteBuffers(1,&m_vboBuffers);
        if(m_vaoMesh!=0)
        {
            delete m_vaoMesh;
        }
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
                d.nx=-m_verts[startHE->m_dual->m_toVertex].m_norm.m_x;
                d.ny=-m_verts[startHE->m_dual->m_toVertex].m_norm.m_y;
                d.nz=-m_verts[startHE->m_dual->m_toVertex].m_norm.m_z;
                vboMesh.push_back(d);
                d.x=m_verts[tHE->m_dual->m_toVertex].m_vert.m_x;
                d.y=m_verts[tHE->m_dual->m_toVertex].m_vert.m_y;
                d.z=m_verts[tHE->m_dual->m_toVertex].m_vert.m_z;
                d.nx=-m_verts[tHE->m_dual->m_toVertex].m_norm.m_x;
                d.ny=-m_verts[tHE->m_dual->m_toVertex].m_norm.m_y;
                d.nz=-m_verts[tHE->m_dual->m_toVertex].m_norm.m_z;
                vboMesh.push_back(d);
                d.x=m_verts[tHE->m_toVertex].m_vert.m_x;
                d.y=m_verts[tHE->m_toVertex].m_vert.m_y;
                d.z=m_verts[tHE->m_toVertex].m_vert.m_z;
                d.nx=-m_verts[tHE->m_toVertex].m_norm.m_x;
                d.ny=-m_verts[tHE->m_toVertex].m_norm.m_y;
                d.nz=-m_verts[tHE->m_toVertex].m_norm.m_z;
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
    // nx,ny,nz,x,y,z
    // If you look at the shader we have the following attributes being used
    // attribute vec3 inVert; attribute 0
    // attribute vec3 inNormal; attribure 1
    // so we need to set the vertexAttributePointer so the correct size and type as follows
    // vertex is attribute 0 with x,y,z(3) parts of type GL_FLOAT, our complete packed data is
    // sizeof(vertData) and the offset into the data structure for the first x component is 5 (u,v,nx,ny,nz)..x
    m_vaoMesh->setVertexAttributePointer(0,3,GL_FLOAT,sizeof(VertData),3);
    // normal same as vertex only starts at position 0
    m_vaoMesh->setVertexAttributePointer(1,3,GL_FLOAT,sizeof(VertData),0);


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

