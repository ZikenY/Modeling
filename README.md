## Geometry algorithms in 2D (Fall 2015)

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
