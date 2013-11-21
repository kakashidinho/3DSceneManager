3D Scene Management for efficiently drawing a large terrain and a lot of objects. 
====================================
1. Description
-------------------
- This project implements a "Loose" Octree Scene Manager to cull away a lot of objects that are not visible to the viewer. Thus, reducing unnecessary drawing operations because in the end, they will not be seen on the screen.
- Together with the Octree Scene Manager, there is also a Quad Tree Renderer for terrain's drawing. Similarly, this Renderer will cull away unseen parts of the terrain as well as reduce the details of some parts that are far away from viewer. 

- The project is implemented using Visual C++ 2008 & OpenGL (no shader).

2. Demo's shortcut keys
-------------------

1 – Enable/disable Terrain's Quadtree renderer. The renderer is enabled by default.
2 – Enable/disable Octree Scene Manager. The octree manager is enabled by default..
C – Enable/disable wire-frame drawing.
F – Increase terrain's level of detail error threshold.
G – Decrease terrain's level of detail error threshold.
R – Enable/disable terrain's crack filling. Enabled by default
V – Enable/disable geo-morphing technique.
Tab – Enable/disable bird-eye view.
~ or ` – Enable/disable information display.
W – Move camera forward.
S – Move camera backward.
A – Move camera left.
D – Move camera right.
Q – Increase camera's height.
E – Decrease camera's height.