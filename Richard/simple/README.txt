Workshop 1: Setting up a simple scene
-------------------------------------
The goal of this assignment is to create a scene class, derived from the Scene class.
Then, minor adjustments need to be made to the GLWidget class to display this new scene.

For data files, take a look at the ../shared/src/*.h files, where * is "buddah", "dragon"
and "teapot". When these files are included, the data will be loaded into a large float
array with the default name (see these header files for more).

In order to render this data, you will need to set up a vertex array and set the attribute
pointers for vertices, normals and possibly texture coordinates. Take a look at the vertex shader
gouraud.vs for the names of the attributes and uniforms that you want to bind the data to.
Note that the data is not indexed, so there is repetition of all the vertices.
Fortunately for you, the shaders in this example are written and initialised for you.

Feel free to generate any scene you like. As an advanced exercise, try and do some sort
of animation on these objects (the Scene class is a QObject, so can receive signals?).
Also, you might try some texture mapping (only the teapot model has texture coordinates).

In summary:
- Create a scene by loading in data files from ../shared/src/*.h and creating a buffer objects
- Set up the rendering by binding uniforms, attributes etc to these new buffers
- Draw the scene!
- Advanced: do some sort of animation! Also try texturing!

Your intended learning outcomes:
- Learn about Buffer Objects as efficient storage for data
- The use of Vertex Arrays and Vertex Pointers to bind to shader attributes
- Binding of buffer object data to shader uniforms
- Rendering a scene

-----------------
Richard Southern (rsouthern@bournemouth.ac.uk)
October 2012
