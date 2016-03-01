Workshop 4: Depth of Field
--------------------------
Depth of field is a simple effect which demonstrates the application of offscreen rendering and the
combination of this with a depth buffer to do selective blur. In the demo app, use "+" and "-" to
change the current render depth.

This effect is achieved by rendering twice: the first time renders the scene, and writes it out
to two textures: one storing the colour and the other stores the fragment depth. In the second
render pass, a simple quad is rendered to the screen with these textures bound, and then the
output pixel colour is computed by blurring based on the fragment depth, retrieved from the
depth image.

You will have to write two sections of code to complete this effect: first you need to create the
frame buffer object with render targets for the colour and depth components. Then you'll need to
complete the dof.fs shader to combine these two texture buffers by blurring the fragment based on
depth.

In summary:
- Create the Frame Buffer Object and textures for storing colour and depth (glwidget.cpp)
- Complete the depth of field shader (dof.fs)
- Advanced: The depth map is the "gateway drug" for numerous effects. Take a look at shadow mapping.

Your intended learning outcomes:
- Frame buffer objects
- Off-screen rendering
- Applications of depth buffers

-----------------
Richard Southern (rsouthern@bournemouth.ac.uk)
October 2012
