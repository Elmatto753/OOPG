Workshop 3: Particle Systems
----------------------------
Particle systems are universally used in many applications due to their simplicity and efficiency.

In this assignment you need to write a class to manage a particle system. My completed example is
of a simple fountain, where each particle has a starting position, velocity, colour and attributes
like age and lifespan. Note that these properties that are expected by the shaders that are provided.

Feel free to create any type of particle system you like: for example, fireworks, blood splatter,
smoke, fire or even try making a drawing program (where a stroke is made up of particles).

If you want a challenging advanced problem, look into moving the calculation of the vertex positions
onto the shader. With my simple example (where each particle is independent) this is reasonably
straightforward. In more complex examples, like the fireworks, this is not so straightforward.

In summary:
- Write a particle system class which manages the particles and displays them using the provided shader.
- Modify the shader to support your particle system if it is different to mine.
- Advanced: Compute particle positions directly on the shader (hardcore)

Your intended learning outcomes:
- Particle system design, management and rendering.

-----------------
Richard Southern (rsouthern@bournemouth.ac.uk)
October 2012
