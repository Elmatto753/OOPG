#ifndef Delaunay_H__
#define Delaunay_H__
/// @file Delaunay.h
/// @brief Using incremental insertion method to generate the Delaunay triangulation
#include <ngl/Types.h>
#include <ngl/Vec3.h>
#include <ngl/Vec2.h>
#include <ngl/Random.h>
#include <vector>
#include <ngl/VertexArrayObject.h>
#include <ngl/ShaderLib.h>
#include <list>
#include <algorithm>

//----------------------------------------------------------------------------------------------------------------------
/// @class Delaunay  "include/Delaunay.h"
/// @brief Construct 2D delaunay mesh/Voronoi diagram for the input set of 2D points
/// @author Xiaosong Yang
/// @version 1.0
/// @date Last Revision 20/01/2013
/// @todo update this class to use more consistent datatype with NGL
//----------------------------------------------------------------------------------------------------------------------

struct DEDGE;

typedef struct DFACE {
    struct DEDGE    *m_edges[3];  // the edge should follow the CCW order
    float           m_circum_centre_x;
    float           m_circum_centre_y;
    float           m_circum_radius;
} DFace;

typedef struct DEDGE {
    DFace           *m_faces[2];    // two neighbour faces
    unsigned int    m_vertices[2];  // the vertices order should follow the CCW direction of faces[0]
    bool            m_flag;
} DEdge;

typedef struct BOUNDBOX {
    float   m_xmin;
    float   m_xmax;
    float   m_ymin;
    float   m_ymax;
    float   m_zmin;
    float   m_zmax;
} BoundBox;

typedef struct DATA {
    GLfloat m_x;
    GLfloat m_y;
    GLfloat m_z;
}Data;

class Delaunay
{
public :
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief default Ctor
    //----------------------------------------------------------------------------------------------------------------------
    Delaunay();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief Ctor passing in an Array of 3D points
    ///  @param[in] _p an array of 3D points
	///  @param[in] _nPoints the size of the Point Array
    //----------------------------------------------------------------------------------------------------------------------
    Delaunay(std::vector<ngl::Vec3> _site);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief Compute the bounding box of the input points set
    //----------------------------------------------------------------------------------------------------------------------
    BoundBox computeBoundBox();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief create two big triangle to wrap up all input points
    //----------------------------------------------------------------------------------------------------------------------
    void    computeTwoBoundTriangle();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief each time only insert on point, split up the triangle with this point into three sub-triangles
    ///  @param[in] _p the index of this point in the m_vertices
    //----------------------------------------------------------------------------------------------------------------------
    void    insertSite(unsigned int _p);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief determine if the _p point lying inside the triangle
    ///  @param[in] _p the input point
    ///  @param[in] _va, _vb, _vc the index of the three vertices of the triangle
    //----------------------------------------------------------------------------------------------------------------------
    bool    insideTriangle(unsigned int _p, unsigned int _va, unsigned int _vb, unsigned int _vc);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief split the triangle into three sub-triangles
    ///  @param[in] _p the middle point
    ///  @param[in] _face the face to split
    ///  @param[in] _va, _vb, _vc the index of the three vertices of the triangle
    //----------------------------------------------------------------------------------------------------------------------
    void    splitTriangle(unsigned int _p, DFace *_face, unsigned int _va, unsigned int _vb, unsigned _vc);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief find the index of the three vertices of the triangle
    ///  @param[in] _face the face to split
    ///  @param[out] _pa, _pb, _pc the index of the three vertices of the triangle
    //----------------------------------------------------------------------------------------------------------------------
    void    findThreeVertices(DFace *_face, unsigned int *_pa, unsigned int *_pb, unsigned int *_pc);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief delete the wrapping triangle at the outside
    //----------------------------------------------------------------------------------------------------------------------
    void    deleteWrappingTriangle();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief use the swap edge algorithm to turn an arbitrary triangulation into delaunay mesh
    //----------------------------------------------------------------------------------------------------------------------
    void    swapForDelaunay();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief get the two opposite vertices to the specified edge
    //----------------------------------------------------------------------------------------------------------------------
    void    getTwoOppositeVertices(DEdge *_e, unsigned int *_v1, unsigned int *_v2);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief get the two opposite angles to the specified edge
    //----------------------------------------------------------------------------------------------------------------------
    void    getTwoOppositeAngles(DEdge *_e, float *_a1, float *_a2);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief swap the edge in the neighbour two faces
    //----------------------------------------------------------------------------------------------------------------------
    void    swapEdge(DEdge *_e);
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief compute the center and radius of the circumcribe circle of the face
    //----------------------------------------------------------------------------------------------------------------------
    void    computeCircumCircleForAllFaces();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief compute the minimum angle of the mesh
    //----------------------------------------------------------------------------------------------------------------------
    float   minimumAngle();
    //----------------------------------------------------------------------------------------------------------------------
    ///  @brief compute the determinant of a 3x3 matrix
    //----------------------------------------------------------------------------------------------------------------------
    float   matrix3x3Determinant(float _m00, float _m01, float _m02, float _m10, float _m11, float _m12, float _m20, float _m21, float _m22);
    //----------------------------------------------------------------------------------------------------------------------
	/// @brief destructor
    //----------------------------------------------------------------------------------------------------------------------
    ~Delaunay();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Draw the convex hull and the input point set
    //----------------------------------------------------------------------------------------------------------------------
	void draw() const;
    void drawDelaunay() const;
    void drawVoronoi() const;

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Creat the VAO objects
    //----------------------------------------------------------------------------------------------------------------------
    void createVAO();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief Creat the Delaunay
    //----------------------------------------------------------------------------------------------------------------------
    void computeDelaunay();
    /// @brief calculate mesh quality
    void meshQuality();
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief on or off switch for draw voronoi
    //----------------------------------------------------------------------------------------------------------------------
    void turnVoronoiDrawFlag();

protected :

    //----------------------------------------------------------------------------------------------------------------------
    /// @brief The vertex list of Delaunay triangulation
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<ngl::Vec3>  	m_vertices;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief The face list of Delaunay triangulation
    //----------------------------------------------------------------------------------------------------------------------
    std::vector<DFace *>  		m_faces;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a vertex array object for our Voronoi diagram drawing
    //----------------------------------------------------------------------------------------------------------------------
    ngl::VertexArrayObject *m_vaoVoronoi;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief a vertex array object for our Delaunay triangulation drawing
    //----------------------------------------------------------------------------------------------------------------------
    ngl::VertexArrayObject *m_vaoDelaunay;
    //----------------------------------------------------------------------------------------------------------------------
    /// @brief the on or off switch for drawing voronoi
    //----------------------------------------------------------------------------------------------------------------------
    bool    m_drawVoronoiFlag;

}; // end class Delaunay

#endif // end header file

//----------------------------------------------------------------------------------------------------------------------
