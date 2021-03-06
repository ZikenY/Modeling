## Geometry algorithms (Fall 2015)

### Delaunay Triangulation algorithm<br />
DelaunayTriangulation:  // A tree-based divide and conquer solution<br />
  Input: A given set of points P<br />
  Output: A Delaunay triangulation of P<br />
  1. Initialize a big enough triangle T consisting of the single triangle p_0 p_(-1) p_(-2)  that contains all points in P. p_0 is the rightmost among the points with largest y-coordinate; p_(-1) 〖,p〗_(-2) are two points in R^2 far away;<br />
  2. Randomly choose a point from P;<br />
  3. Find then triangle that point lies in by tree-based data structure, subdivide that triangle into smaller ones that has point as a vertex. ( A tree-based data structure where internal nodes are triangles that have been deleted or subdivided at some point in the construction, and the current triangulation is stored at the leaves);<br />
  4. Flip edges until all current edges are legal;<br />
  5. Repeat until all points have been added to T;<br />
  6. Discard p_(-1)and p_(-2) with all their incident edges from T;<br />
  7. Return T.<br />

Time Complexity: <br />
Locate the site that point lies in is O(nlog⁡n);<br />
Update the illegal edges takes O(n), because the expected number of triangles created by algorithm DelaunayTriangulation is at most 9n+1. The whole algorithm is O(nlog⁡n).<br />

sample source dataset:<br />
![Alt text]( DelaunayTriangulation/screenshot02.png?raw=true "")<br />

delaunay triangulation result:<br />
![Alt text]( DelaunayTriangulation/screenshot01.png?raw=true "")<br />


### Sweep line algorithm for circle crossing<br />
![Alt text]( sweepcircles_semi/screenshot01.jpg?raw=true "")<br />


### 3D Mesh Subdivision<br />
![Alt text]( Morphing/subdivision0.png?raw=true "")<br />
![Alt text]( Morphing/subdivision1.png?raw=true "")<br />


### 3D Mesh Morphing<br />
Goal:<br />
Morph smile.m to sad.m based on discrete harmonic mapping and G3dogl tool.<br />

Input:<br />
Two mesh file: smile.m, sad.m; identical vertices stored in a text file.<br />

Output:<br />
Smile_merged.m with Opos and Onormal fields for each vertex. These two fields are morphing results from smile.m to sad.m.<br />

Processes:<br />
Step1. Map sad mesh to a unit disk (2D) with radius 1 by discrete harmonic method;<br />
Step2. Get identical vertices’ 2D coordinates for sad mesh, and fix them in the unit disk for smile mesh;<br />
Step3. Map smile mesh to the unit disk with fixed identical vertices;<br />
Step4. Compute new 3D coordinates and new normal vectors for each vertex in smile mesh based on Barycentric method;<br />
Step5. Combine new 3D coordinates and new normal vectors with the original smile mesh, and export a combined mesh: smile_merged.m.<br />

![Alt text]( Morphing/morphing.png?raw=true "")<br />
