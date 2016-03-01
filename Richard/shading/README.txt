Workshop 2: Per-Pixel and Toon Shading
--------------------------------------
In this assignment you are given an animating scene and are required to write two new shaders.
- Per pixel shading: based on the original Phong model, you will pass an interpolated normal from the
  vertex shader onto the fragment shader, and then compute the light reflectance at the pixel level.
- Toon shading: Use your shader above, and then create a stepped function for the light intensity
  so you only have between 4-8 shading levels. Also, try and create a black boundary to the object.

For an advanced exercise, you could try and make the toon shader anti-aliased: i.e. smooth the
boundary between the step function levels.

In summary:
- Write two shaders: per-pixel (phong) lighting and toon shading.
- Advanced: Come up with a way to anti-alias the toon shader

Your intended learning outcomes:
- Shader design for simple rendering
- Some standard rendering effects used commonly in games

-----------------
Richard Southern (rsouthern@bournemouth.ac.uk)
October 2012
