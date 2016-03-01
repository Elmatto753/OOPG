#ifndef MVCDEFORM_H
#define MVCDEFORM_H

// Our own simple shader management class
#include "shader.h"

// This is needed to determine the MVC weights
#include "mvc.h"

// Need this for clock time
#include <chrono>
using namespace std::chrono;

#include <string>

/**
  * The class for displaying the MVC widget.
  */
class MVCDeform
{    
public:
    MVCDeform();
    ~MVCDeform();

public:
    /// Load up the geometry and the cage
    void initData(const std::string &/*cagefilename*/,
                  const std::string &/*datafilename*/,
                  const std::string &/*elefilename*/,
                  const std::string &/*texfilename*/);
    void init();
    void resize(int w, int h);
    void draw();
    void update();

protected:

    /// Utility function to read points from the file
    void loadNodeFile(const std::string &filename, std::vector<Point2D> &pts);
    void loadEleFile(const std::string &filename, std::vector<TriIndex> &ele);

    /// Utility function to create and initialise the shaders
    void initShaders();

    /// Calculate the new extents from the input data
    void computeExtents(const std::vector<Point2D> &verts);

    /// Create the vertex structure for rendering
    void createVertices(const std::vector<Point2D> &/*dataVerts*/);

    /// Compute the weights and store them in a sampler buffer
    void createWeights(const std::vector<Point2D> &/*cageVerts*/,
                       const std::vector<Point2D> &/*dataVerts*/);

    /// Create mesh elements for rendering
    void createElements(const std::vector<TriIndex> &/*dataTri*/);

    /// Store the cage vertices in a sampler buffer
    void createCage(const std::vector<Point2D> &/*cageVerts*/);

    /// Create a simple radial cage
    void genRadialCage(std::vector<Point2D> &/*pts*/, const int /*size*/);

    /// Create the texture to draw with our object
    void createTexture(const std::string &/*texfilename*/);

    /// Draw the vertices defined by their generalized barycentric coordinates
    void drawVerts();

    /// Draw the cage around the MVC vertices
    void drawCage();

    /// Update the cage vertices according to a sin wobble function
    void wobbleCage(const GLfloat /*elapsed time*/);

    /// Update the cage render data with the specified result
    void updateCage(const GLfloat * /*cageVerts*/);

private:
    /// The shaders used in this scene
    Shader m_basicProgram, m_mvcProgram;

    GLfloat *m_cageVerts;       //< We need cage data to deform it with updateCage
    int m_nCage;                //< The size of the cage
    int m_nVert;                //< The number of mesh vertices
    int m_nMesh;                //< The number of triangles in the mesh

    GLuint m_cageTBO;           //< The texture buffer object holding our cage data
    GLuint m_weightTBO;         //< The texture buffer object storing vertex weights
    GLuint m_cageTex;           //< The sampler buffer texture for the cage
    GLuint m_weightTex;         //< The sampler buffer texture for the barycentric weights
    GLuint m_texVBO;            //< The texture coordinate buffer object
    GLuint m_texVA;             //< The texture coordinate vertex array
    GLuint m_cageVBO;           //< The vertex buffer object for the cage
    GLuint m_cageVA;            //< The cage vertex array
    GLuint m_cageEA;            //< The cage element array storing connectivity
    GLuint m_meshEA;            //< A mesh associated with our drawn nodes
    GLuint m_texID;             //< The texture id to do actual texture mapping
    bool m_init;                //< whether the class is ready to draw

    GLfloat m_cx, m_cy;         //< The center of the loaded data
    GLfloat m_lx, m_ly;         //< The extents of the data which is loaded in each direction

    void checkGLError();        //< A private utility function to look for GL errors

    /// Use this to determine the time accurately
    high_resolution_clock::time_point m_startTime;
};

#endif // GLWIDGET_H
