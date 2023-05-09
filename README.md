# Ray Tracing Fun

![Rendering May 2023](/Assets/full_render.png)

As mentioned in my [OpenGL project](https://github.com/uurriola/OpenGLFun), I learned some legacy OpenGL during engineering.
Decided to learn about ray tracing, it turns out my OpenGL lessons from engineering school already focused on this topic!
Unfortunaly, I lost the code of the project I developed back then. Anyway, I'll just learn ray tracing again!

I'll render here some results to observe my progress.

Based on [tutorials on Youtube by The Cherno](https://www.youtube.com/watch?v=gfW1Fhd9u9Q&list=PLlrATfBNZ98edc5GshdBtREv5asFW3yXl&ab_channel=TheCherno).

As of May 2023, these videos do not go as far as I'd like, so I switched to [_Ray Tracing in One Weekend_](https://raytracing.github.io/books/RayTracingInOneWeekend.html).

I then watched [this video, by Sebastian Lague](https://www.youtube.com/watch?v=Qz0KTGYJtUk&ab_channel=SebastianLague) and refactor the main part of my code based on his work.

## Progress

*Please note that the capture frequency is pretty low. The rendering itself is not that slow.*

### Simple scene with a moving light

![Sphere with simple moving light](/Assets/moving_light.gif)

### Transparency

![Transparent sphere](/Assets/transparency.png)

### Refraction

![Refracting sphere](/Assets/update_refraction_index.png)

### Roughness

![Rough sphere](/Assets/roughness.png)

### Metallic

![Metallic sphere](/Assets/metallic.png)

### Bigger scene

The app allows to dynamically add sphere and materials.

![Bigger scene](/Assets/bigger_scene.png)

## Improvements

I'm thinking about spending some more time on this project and focus on:
* handle triangles / 3D models (only spheres ATM),
* moving rendering computation from CPU to GPU,
* improve antialiasing / focus computation.