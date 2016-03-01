#include "glinc.h"
#include "mvcdeform.h"
#include <fstream>
#include <iostream>
#include <time.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Need this for cross platform texture loading
#include <SOIL.h>

#include <algorithm>
#include <iterator>

// Windows can't decide a consistent way to include maths defines (sort it out people!) so
// we'll just define it manually.
#ifdef WIN32
#define M_PI 3.1415926535897932385
#endif

/**
  *
  */
MVCDeform::MVCDeform()  {
    m_init = false;
    m_cx = m_cy = 0.0f;
    m_lx = m_ly = 1.0f;
}

/**
  * Delete all used buffers and data.
  */
MVCDeform::~MVCDeform() {
    if (m_init) {
        delete [] m_cageVerts;
        glDeleteBuffers(1, &m_cageTBO);
        glDeleteBuffers(1, &m_weightTBO);
        glDeleteBuffers(1, &m_texVBO);
        glDeleteBuffers(1, &m_cageVBO);
        glDeleteBuffers(1, &m_cageEA);
        glDeleteBuffers(1, &m_meshEA);
        glDeleteTextures(1, &m_texID);
    }
}

/**
 * @brief MVCDeform::checkGLError
 * From http://www.glprogramming.com/red/chapter14.html
 */
void MVCDeform::checkGLError() {
    GLenum errCode;
    const GLubyte *errString;

    if ((errCode = glGetError()) != GL_NO_ERROR) {
        errString = gluErrorString(errCode);
        fprintf (stderr, "OpenGL Error: %s\n", errString);
        exit(0);
    }
}

/**
  * Data points for the cage and vertices are stored in a node file. For the format, see
  * http://www.cs.cmu.edu/~quake/triangle.html
  * \param filename Name of node file to load
  * \param pts Data points returned in a vector
  */
void MVCDeform::loadNodeFile(const std::string &filename, std::vector<Point2D> &pts) {
    std::ifstream infile;
    infile.open(filename.c_str(), std::ifstream::in);
    if (!infile.good()) {
        std::cerr << "Failed to open " << filename << "\n";
        exit(0);
    }
    int sz, i1, i2, i3, cnt = 0;
    infile >> sz >> i1 >> i2 >> i3;
    pts.resize(sz);
    while (infile.good() && !infile.eof() && cnt < sz) {
        infile >> i1 >> pts[cnt][0] >> pts[cnt][1] >> i2;
        // Triangle flips the coordinates for some reason
        pts[cnt][1] = 1.0f - pts[cnt][1];
        cnt++;
    }
    infile.close();
}

/**
  * Read a basic .ele file containing a triangle mesh. This is based on Shewchuk's
  * triangle package http://www.cs.cmu.edu/~quake/triangle.html.
  * \param filename The name of the ele file
  * \param ele The triangle elements output in a vector
  */
void MVCDeform::loadEleFile(const std::string &filename, std::vector<TriIndex> &ele) {
    std::ifstream infile;
    infile.open(filename.c_str(), std::ifstream::in);
    if (!infile.good()) {
        std::cerr << "Failed to open " << filename << "\n";
        exit(0);
    }
    int sz, i1, i2, cnt = 0;
    infile >> sz >> i1 >> i2;
    ele.resize(sz);
    while (infile.good() && !infile.eof() && cnt < sz) {
        infile >> i1 >> ele[cnt][0] >> ele[cnt][1] >> ele[cnt][2];
        cnt++;
    }
    infile.close();
}

/**
  * Simply compute the range and center for the vertex positions and resize the window.
  * \param verts Vertex list to use
  */
void MVCDeform::computeExtents(const std::vector<Point2D> &verts) {
    GLuint i;
    GLfloat tot_x=0.0f, tot_y=0.0f;
    for (i=0; i<verts.size(); ++i) {
        tot_x += verts[i][0];
        tot_y += verts[i][1];
    }
    m_cx = tot_x / GLfloat(verts.size());
    m_cy = tot_y / GLfloat(verts.size());
    GLfloat max_x, max_y, x, y;
    max_x = fabs(verts[0][0] - m_cx);
    max_y = fabs(verts[0][1] - m_cy);
    for (i = 1; i < verts.size(); ++i) {
        x = fabs(verts[i][0] - m_cx);
        y = fabs(verts[i][1] - m_cy);
        if (max_x < x) max_x = x;
        if (max_y < y) max_y = y;
    }
    if (max_x > max_y)
        m_lx = m_ly = max_x;
    else
        m_lx = m_ly = max_y;
}

/**
  * This function creates three aspects relating to the cage:
  * - The cage vertices are packed into a samplerBuffer for use in the MVC shader.
  * - The cage vertices are stored in a vertex attrib array, for use in the basic shader.
  * - An element array is generated for rendering the line loop of the basic shader.
  *
  * \param cageVerts The vertices of the cage.
  */
void MVCDeform::createCage(const std::vector<Point2D> &cageVerts) {
    // Copy the cage vertices to our trusty buffer
    m_cageVerts = new GLfloat[cageVerts.size()*2];
    GLfloat *cageVertTexture = new GLfloat[cageVerts.size()*4];
    GLuint *cageIndices = new GLuint[cageVerts.size()];
    GLuint i;
    for (i=0; i<cageVerts.size(); ++i) {
        // Copy our data into the buffer
        m_cageVerts[i*2+0] = cageVertTexture[i*4+0] = cageVerts[i][0];
        m_cageVerts[i*2+1] = cageVertTexture[i*4+1] = cageVerts[i][1];
        cageVertTexture[i*4+2] = cageVertTexture[i*4+3] = 0.0f;
        cageIndices[i] = i;
    }

    // Create a dynamic buffer to hold cage vertices
    glGenBuffers(1, &m_cageTBO); checkGLError();
    glBindBuffer(GL_TEXTURE_BUFFER, m_cageTBO); checkGLError();
    glBufferData(GL_TEXTURE_BUFFER,
                 /*size of buffer to allocate*/ cageVerts.size()*sizeof(GLfloat)*4,
                 /*null means we allocate space but don't copy to it*/ cageVertTexture,
                 /*this means that we need to change the data dynamically*/GL_DYNAMIC_DRAW);
    checkGLError();

    // Wrap this in a texture
    glGenTextures(1, &m_cageTex); checkGLError();
    glActiveTexture(GL_TEXTURE0); checkGLError();
    glBindTexture(GL_TEXTURE_BUFFER, m_cageTex); checkGLError();
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, m_cageTBO); checkGLError();

    // Set the cage parameters
    m_mvcProgram.bind();
    glUniform1i(glGetUniformLocation(m_mvcProgram.id(), "u_cageVerts"), 0);
    m_mvcProgram.unbind();
    checkGLError();

    // Generate some arrays and buffers
    glGenVertexArrays(1, &m_cageVA);checkGLError();
    glBindVertexArray(m_cageVA);checkGLError();
    glGenBuffers(1, &m_cageVBO);checkGLError();
    glBindBuffer(GL_ARRAY_BUFFER, m_cageVBO);checkGLError();
    glBufferData(/*type of data*/ GL_ARRAY_BUFFER,
                 /*size of data to allocate (bytes)*/ 4*cageVerts.size()*sizeof(GLfloat),
                 /*start of data to copy*/ (void*) m_cageVerts,
                 /*will be changed by client*/ GL_DYNAMIC_DRAW);
    checkGLError();

    m_basicProgram.bind();
    glEnableVertexAttribArray(glGetAttribLocation(m_basicProgram.id(),"a_Pos"));
    glVertexAttribPointer(/*index of attribute*/ glGetAttribLocation(m_basicProgram.id(),"a_Pos"),
                          /*number of components per vert (1-4)*/ 2,
                          /*type of data*/ GL_FLOAT,
                          /*should it be normalized?*/ GL_FALSE,
                          /*stride (0 means tightly packed)*/ 2*sizeof(GLfloat),
                          /*pointer to first element*/ 0);
    checkGLError();
    m_basicProgram.unbind();
    glBindVertexArray(0);

    // Create the buffer to store the elements (the indices of the line loop)
    glGenBuffers(1, &m_cageEA);checkGLError();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cageEA);checkGLError();
    glBufferData(/*type of buffer*/ GL_ELEMENT_ARRAY_BUFFER,
                 /*size of buffer (bytes)*/ sizeof(GLuint)*cageVerts.size(),
                 /*pointer to start of buffer*/ (void*) cageIndices,
                 /*will not be changed by client*/ GL_STATIC_DRAW);
    checkGLError();

    // Create away memory and unbind
    delete [] cageIndices;
    delete [] cageVertTexture;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

/**
  * This little function achieves animation, and should be linked to a timer.
  * It perturbs each cage vertex out from the center based on a simple sin wave
  * function.
  * \param t The time parameter, hopefully increasing steadily for nice effects.
  */
void MVCDeform::wobbleCage(const GLfloat t) {
    // Compute the radial deformation from the center of the cage
    GLfloat *wobbleVerts = new GLfloat[m_nCage*2];
    int i;
    GLfloat inv_n = 1.0f / GLfloat(m_nCage);
    GLfloat freq = 4.0f;        // Change the frequency here
    GLfloat amp = 0.4;          // Change the amplitude here
    GLfloat s;
    for (i=0; i<m_nCage; ++i) {
        s = amp*sin(t*M_PI + freq*M_PI*GLfloat(i)*inv_n);
        wobbleVerts[i*2+0] = s*(m_cageVerts[i*2+0] - m_cx) + m_cageVerts[i*2+0];
        wobbleVerts[i*2+1] = s*(m_cageVerts[i*2+1] - m_cx) + m_cageVerts[i*2+1];
    }

    // Update the data on the GPU for rendering
    updateCage(wobbleVerts);
    delete [] wobbleVerts;
}

/**
  * Update the cage in the buffer with a possibly different set of data. This is
  * used to animate the vertices if, for example, the vertices are perturbed through user
  * interaction. Note that this could be done with glMapBuffers! Performances may be
  * better: check this.
  * \param cageVerts The new positions to move the vertices to
  */
void MVCDeform::updateCage(const GLfloat *cageVerts) {
    glBindBuffer(GL_TEXTURE_BUFFER, m_cageTBO);
    glBufferData(/*type of data*/GL_TEXTURE_BUFFER,
                 /*size of data to allocate (bytes)*/ 2*m_nCage*sizeof(GLfloat),
                 /*start of data to copy*/ cageVerts,
                 /*will be changed by client*/GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER,0);
    glBindBuffer(GL_ARRAY_BUFFER, m_cageVBO);
    glBufferData(/*type of data*/ GL_ARRAY_BUFFER,
                 /*size of data to allocate (bytes)*/ 2*m_nCage*sizeof(GLfloat),
                 /*start of data to copy*/ (void*) cageVerts,
                 /*will be changed by client*/ GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER,0);
}


/**
  * This function determines the barycentric weights of each of the dataVerts within
  * the cage. The magic happens in a function called mean_value_coordinates(), which
  * defines the position of each dataVert in terms of the cage positions. The resulting
  * barycentric coordinates are then stored in a Buffer Object and then bound to a
  * texture for use in the MVC shader.
  * \param cageVerts The array of 2D cage positions
  * \param dataVerts The array of vertices
  */
void MVCDeform::createWeights(const std::vector<Point2D> &cageVerts,
                              const std::vector<Point2D> &dataVerts) {
    // TODO: Generate the weights using the mean_value_coordinates function, write it to a buffer, and
    // then wrap the buffer to a texture. The texture unit must then be written to the "u_vertWeights"
    // attribute to get this working.

  std::vector<Point2D>::const_iterator pit;
  std::vector<double> baryCoords(cageVerts.size());
  std::vector<float> allBaryCoords;
  allBaryCoords.reserve(cageVerts.size()*dataVerts.size());
  for (pit = dataVerts.begin(); pit != dataVerts.end(); ++pit)
  {
    mean_value_coordinates(cageVerts, *pit, baryCoords);
    allBaryCoords.insert(allBaryCoords.end(),
                         baryCoords.begin(),
                         baryCoords.end());

  }

  std::copy(baryCoords.begin(), baryCoords.end(),
            std::ostream_iterator<float>(std::cout, " "));


  // Create a dynamic buffer to hold cage vertices
  glGenBuffers(1, &m_weightTBO); checkGLError();
  glBindBuffer(GL_TEXTURE_BUFFER, m_weightTBO); checkGLError();
  glBufferData(GL_TEXTURE_BUFFER,
               cageVerts.size()*dataVerts.size()*sizeof(GLfloat),
               &allBaryCoords[0],
               GL_STATIC_DRAW);
  checkGLError();

  // Wrap this in a texture
  glGenTextures(1, &m_weightTex); checkGLError();
  glActiveTexture(GL_TEXTURE1); checkGLError();
  glBindTexture(GL_TEXTURE_BUFFER, m_weightTex); checkGLError();
  glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, m_weightTBO); checkGLError();

  // Set the cage parameters
  m_mvcProgram.bind();
  glUniform1i(glGetUniformLocation(m_mvcProgram.id(), "u_vertWeights"), 1);
  m_mvcProgram.unbind();
  checkGLError();
}

/**
  * Create the elements necessary to draw the mesh. This will reference the
  * vertex array data.
  * \param dataTri Triangle index data
  */
void MVCDeform::createElements(const std::vector<TriIndex> &dataTri) {
    // Create the element array to draw our mesh
    GLuint *idx = new GLuint[dataTri.size()*3];
    GLuint i;
    for (i=0; i<dataTri.size(); ++i) {
        idx[i*3+0] = dataTri[i][0];
        idx[i*3+1] = dataTri[i][1];
        idx[i*3+2] = dataTri[i][2];
    }
    glGenBuffers(1, &m_meshEA);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshEA);
    glBufferData(/*type of buffer*/ GL_ELEMENT_ARRAY_BUFFER,
                 /*size of buffer (bytes)*/ sizeof(GLuint)*dataTri.size()*3,
                 /*pointer to start of buffer*/ (void*) idx,
                 /*will not be changed by client*/ GL_STATIC_DRAW);

    delete [] idx;
}

/**
  * This function loads the data into a buffer object. Note that this
  * information is actually used to calculate texture coordinates, rather
  * than being used for rendering.
  * \param dataVerts The vertex data to be written to the vertexAttribArray
  */
void MVCDeform::createVertices(const std::vector<Point2D> &dataVerts) {
    // Create a dummy Vertex Buffer Object so that it renders something
    GLfloat *dataVertBuffer = new GLfloat[2*dataVerts.size()];
    GLuint i;
    for (i=0; i<dataVerts.size(); ++i) {
        dataVertBuffer[i*2+0] = dataVerts[i][0];
        dataVertBuffer[i*2+1] = dataVerts[i][1];
    }

    glGenVertexArrays(1, &m_texVA);
    glBindVertexArray(m_texVA);
    glGenBuffers(1, &m_texVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_texVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 2*dataVerts.size()*sizeof(GLfloat),
                 dataVertBuffer /*can I leave this null?*/,
                 GL_STATIC_DRAW);
    // Note that we have not allocated any data here
    m_mvcProgram.bind();
    glEnableVertexAttribArray(glGetAttribLocation(m_mvcProgram.id(),"a_Pos"));
    glVertexAttribPointer(/*index of attribute*/ 0,
                          /*number of components per vert (1-4)*/ 2,
                          /*type of data*/ GL_FLOAT,
                          /*should it be normalized?*/ GL_FALSE,
                          /*stride (0 means tightly packed)*/ 0,
                          /*pointer to first element (0 means current buffer)*/ 0);
    m_mvcProgram.unbind();

    // Unbind everything and free memory
    delete [] dataVertBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

/**
  * Given the texture file name, this function creates the texture and loads it into
  * memory and binds it to a fragment shader texture handle.
  * \param texfilename The name of the texture file
  */
void MVCDeform::createTexture(const std::string &texfilename) {
    // Load the image from a file
    m_texID = SOIL_load_OGL_texture(
                texfilename.c_str(),    /*filename*/
                SOIL_LOAD_AUTO,         /*channels (automatic)*/
                SOIL_CREATE_NEW_ID,     /*reuse id or request new one*/
                SOIL_FLAG_MIPMAPS |     /*other flags*/
                SOIL_FLAG_INVERT_Y |
                SOIL_FLAG_NTSC_SAFE_RGB
        );
    if( m_texID == 0 ) {
        printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }

    // Activate and bind our new texture to position 2
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, m_texID);

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

//    std::cerr<<"Texture: ["<<texData.width()<<","<<texData.height()<<"]\n";
//    glTexImage2D(/*target*/ GL_TEXTURE_2D,
//                 /*mipmap level*/ 0,
//                 /*internal format*/ GL_RGBA,
//                 /*texture width*/ texData.width(),
//                 /*texture height*/ texData.height(),
//                 /*border width*/ 0,
//                 /*external formal*/ GL_RGBA,
//                 /*type*/ GL_UNSIGNED_BYTE,
//                 /*data*/ texData.bits());

    // Set the uniform to know which texture to associate with the uniform
    m_mvcProgram.bind();
    glUniform1i(glGetUniformLocation(m_mvcProgram.id(), "u_Texture"), 2);
    m_mvcProgram.unbind();

    // Unbind our texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

/**
  * Generate a circular cage instead of the one in the node file.
  * \param pts The point vector to fill.
  * \param size The number of points to create.
  */
void MVCDeform::genRadialCage(std::vector<Point2D> &pts, const int size) {
    int i;
    GLfloat r=0.6f;
    pts.resize(size);
    GLfloat inc = (2.0f * M_PI) / GLfloat(size);
    for (i=0; i<size; ++i) {
        pts[i][0] = r*cos(inc*GLfloat(i)) + 0.5f;
        pts[i][1] = r*sin(inc*GLfloat(i)) + 0.5f;
    }
}

/**
  * Initialise the scene from the specified filenames. This does most of the work
  * in this class. The user specifies the file storing the cage vertices, mesh
  * vertices, mesh elements and then the texture filename. Currently it needs all
  * four for each example (although the cage can be generated automatically).
  * The node and ele files were generated using contour2node and triangle.
  * \param cagefilename A node file for the cage
  * \param datafilename A node file for the mesh vertices
  * \param elefilename A mesh data structure in a ele file
  * \param texfilename The name of the texture to be applied to the mesh (texture
  *        coordinates are assumed to be the same as the 2D coordinates.
  */
void MVCDeform::initData(const std::string &/*cagefilename*/,
                         const std::string &datafilename,
                         const std::string &elefilename,
                         const std::string &texfilename) {
    // Read the cage and data from the file
    std::vector<Point2D> cageVerts, dataVerts;
    std::vector<TriIndex> dataTri;

    genRadialCage(cageVerts, 50);
    //loadNodeFile(cagefilename, cageVerts);
    loadNodeFile(datafilename, dataVerts);
    loadEleFile(elefilename, dataTri);
    m_nCage = cageVerts.size();
    m_nVert = dataVerts.size();
    m_nMesh = dataTri.size();

    std::cerr<<"#cage="<<m_nCage<<
               ", #vert="<<m_nVert<<
               ", #tris="<<m_nMesh<<"\n";

    // Compute the new extents from the cage data
    computeExtents(cageVerts);

    // Fill a sample buffer with cage vertices
    createCage(cageVerts);

    // Fill a sample buffer with barycentric weights
    createWeights(cageVerts, dataVerts);

    // Create the triangle elements used to render
    createElements(dataTri);

    // Create the vertex array objects for rendering
    createVertices(dataVerts);

    // Create the image texture file to use with the shader
    createTexture(texfilename);

    // We're done and ready to render
    m_init = true;

    // Update the clock time
    m_startTime = high_resolution_clock::now();
}

/**
  * Built in Qt timer event handler
  */
void MVCDeform::update() {
    duration<float> elapsed = duration_cast<duration<float>>(high_resolution_clock::now() - m_startTime);
    wobbleCage(elapsed.count());
}

/**
  * Initialise the shaders and some basic uniform parameters like the MVP
  */
void MVCDeform::initShaders() {
    // Initialise the shaders
    m_mvcProgram.init("shaders/mvc.vs", "shaders/mvc.fs");
    m_basicProgram.init("shaders/basic.vs", "shaders/basic.fs");

    // Set up the default camera parameters
    //glViewport(0,0,640, 480);
    //glm::mat4 mvp = glm::ortho(0.0f, 1.0f, 0.0f, GLfloat(height())/GLfloat(width()), -1.0f, 1.0f);

    // Set the initial parameters for the MVC shader
    //m_mvcProgram.bind();
    //glUniformMatrix4fv(/*location*/glGetUniformLocation(m_mvcProgram.id(), "u_MVP"),
    //                   /*# of matrices*/ 1,
    //                   /*transpose?*/ GL_FALSE,
    //                   /*The matrix pointer*/ glm::value_ptr(mvp));
    // Initialise number of cage vertices to 0 until we have some data
    //m_mvcProgram.unbind();

    // Set the initial parameters for our basic shader
    //m_basicProgram.bind();
    //glUniformMatrix4fv(/*location*/glGetUniformLocation(m_basicProgram.id(), "u_MVP"),
    //                   /*# of matrices*/ 1,
    //                   /*transpose?*/ GL_FALSE,
    //                   /*The matrix pointer*/ glm::value_ptr(mvp));
    //glEnableVertexAttribArray(glGetAttribLocation(m_basicProgram.id(),"a_Pos"));
    //m_basicProgram.unbind();

    //m_mvcProgram.bind();
//    GLint nUniforms, nAttribs;
//    glGetProgramiv(m_mvcProgram.id(), GL_ACTIVE_UNIFORMS, &nUniforms);
//    char name[256]; GLsizei l; int i;
//    for (i=0; i<nUniforms; ++i) {
//        glGetActiveUniformName(m_mvcProgram.id(), i, 256, &l, name);
//        std::cerr << "Uniform: "<<name<<"\n";
//    }
//    glGetProgramiv(m_mvcProgram.id(), GL_ACTIVE_ATTRIBUTES, &nAttribs);
//    GLint size; GLenum type;
//    for (i=0; i<nAttribs; ++i) {
//        glGetActiveAttrib(m_mvcProgram.id(), i, 256, &l, &size, &type, name);
//        std::cerr << "Attribute: "<<name<<" Size: "<<size<<" Type: "<<type<<"\n";
//    }
    //m_mvcProgram.unbind();
//    m_mvcProgram.printProperties();
}

/**
  * This is called automatically when the GL window is created. It needs to make all
  * the necessary OpenGL initialisation calls.
  */
void MVCDeform::init() {
    glewInit();
    glEnable(GL_TEXTURE_2D);
    glFrontFace(GL_CW);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable( GL_POINT_SMOOTH );
    glEnable( GL_MULTISAMPLE_ARB);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    initShaders(); // Our function to create the shaders
    initData(std::string("data/jmacey_cage.node"),
             std::string("data/jmacey.node"),
             std::string("data/jmacey.ele"),
             std::string("data/jmacey.jpg"));
}

/**
  * Resize the window based on OS interaction. Must make sure aspect ratio is maintained.
  * I use an orthographic camera for the examples. This is set for each shader's MVP
  * uniform (ModelViewProjection) which is used to transform points.
  * \param w Width of window (actually I use Qt's width())
  * \param h Height of the window (I use Qt's height())
  */
void MVCDeform::resize(int w, int h) {
    // Use this function to ensure that the aspect ratio is maintained
    GLfloat r = 1.0f;
    glm::mat4 mvp;
    if (w > h) {
        r = GLfloat(w) / GLfloat(h);
        mvp = glm::ortho(m_cx - m_lx*r,
                         m_cx + m_lx*r,
                         m_cy - m_ly,
                         m_cy + m_ly,
                         -1.0f, 1.0f);
    } else {
        r = GLfloat(h) / GLfloat(w);
        mvp = glm::ortho(m_cx - m_lx,
                         m_cx + m_lx,
                         m_cy - m_ly*r,
                         m_cy + m_ly*r,
                         -1.0f, 1.0f);
    }
    // Specify the extents of our GL window
    glViewport(0,0,w, h);

    // Set up the default camera parameters
    m_mvcProgram.bind();
    glUniformMatrix4fv(/*location*/glGetUniformLocation(m_mvcProgram.id(), "u_MVP"),
                       /*# of matrices*/ 1,
                       /*transpose?*/ GL_FALSE,
                       /*The matrix pointer*/ glm::value_ptr(mvp));
    m_mvcProgram.unbind();

    m_basicProgram.bind();
    glUniformMatrix4fv(/*location*/glGetUniformLocation(m_basicProgram.id(), "u_MVP"),
                       /*# of matrices*/ 1,
                       /*transpose?*/ GL_FALSE,
                       /*The matrix pointer*/ glm::value_ptr(mvp));
    m_basicProgram.unbind();
}

/**
  * Draw only the MVC deformed points using the mvc shader. This is called from paintGL(),
  * and hopefully your data has been initialized or results will be undefined. Note that
  * it basically just binds all the data that we've created in other functions and calls
  * glDrawElements(), which simply creates the textured triangles that you see in the
  * demo.
  */
void MVCDeform::drawVerts() {
    // Draw the MVC coordinates
    m_mvcProgram.bind();

    // Bind the texture vertex array
    glBindVertexArray(m_texVA);

    // Bind the texture buffer object used to define the cage
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, m_cageTex);

    // Bind the texture buffer object used to define the barycentric coordinates of vertices in terms of the cage
    glActiveTexture(GL_TEXTURE0+1);
    glBindTexture(GL_TEXTURE_BUFFER, m_weightTex);

    // Bind the texture image which is used by the fragment buffer
    glActiveTexture(GL_TEXTURE0+2);
    glBindTexture(GL_TEXTURE_2D, m_texID);

    // Draw the array of points
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshEA);
    glDrawElements(GL_TRIANGLES, m_nMesh*3, GL_UNSIGNED_INT, 0);

    // Unbind everything
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_BUFFER,0);
    glBindTexture(GL_TEXTURE_2D, 0);
    m_mvcProgram.unbind();
}

/**
  * Draw the deformation cage. This is displayed as a line
  * loop with the vertices drawn as fat points.
  */
void MVCDeform::drawCage() {
    // Draw the cage
    m_basicProgram.bind();

    // Draw the vertices of the cage
    glBindVertexArray(m_cageVA);
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, m_nCage);
    glPointSize(1.0f);

    // Draw the line loop defining the bounds of the cage
    // Note: the last parameter of NULL indicates that it will look at the GL_ELEMENT_ARRAY_BUFFER pointer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cageEA);
    glDrawElements(GL_LINE_LOOP, m_nCage, GL_UNSIGNED_INT, 0);

    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    m_basicProgram.unbind();
}

/**
  * This is called automatically when the scene needs to be redrawn (for example,
  * when updateGL() is called). Note that you should only call drawVerts() or drawCage()
  * when the scene has been initialised. This is
  */
void MVCDeform::draw() {
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (m_init) {
        drawVerts();
        drawCage();
    }
}

