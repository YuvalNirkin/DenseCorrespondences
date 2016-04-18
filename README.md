# DenseCorrespodences
## Introduction
The objective of this project was to apply SIFT-flow into 3D reconstruction. The SIFT-flow and scale propagation algorithms were integrated into an existing 3D reconstruction pipeline, provided by OpenMVG. Part of the SIFT-flow code was available in C++ and the rest had to be converted from Matlab, in order to be fitted in the pipeline and for better performance.

If you find this code useful, please add reference to the following papers in your work:

[1] M. Tau and T. Hassner, Dense Correspondences Across Scenes and Scales. IEEE Trans. Pattern Anal. Mach. Intell. (TPAMI) 38(5): 875-888 (2016)
[2] C. Liu, J. Yuen, and A. Torralba, “SIFT flow: Dense correspondence across scenes and its applications,” Trans. Pattern Anal. Mach. Intell., vol. 33, no. 5, pp. 978–994, 2011.

For more information, please see project page for scale propagation here:
http://www.openu.ac.il/home/hassner/projects/scalemaps/

## The 3D Reconstruction Pipeline
The pipeline is based on OpenMVG’s pipeline and includes the following modules:

1. Sparse matching – Matching all possible image pairs using SIFT descriptors on sparse keypoints.
2. Dense matching – Matching all possible image pairs using SIFT images.
    * For each image pair find their scale maps using the matched sparse SIFT descriptor’s scales as seed.
    * Calculate the SIFT images of each image pair using their scale maps.
    * Do 2-sided SIFT-flow for every image pair and save pixel pairs that moved to each other as matches.
    * Optionally filter matches from pixels that are in textureless regions.
    * Apply geometric filtering. Using RANSAC on the matches of each image pair to find the corresponding Fundamental Matrix. A pair of matching points that is too far from the corresponding epipolar lines are than filtered out.
3.	Incremental SfM – Calculate the camera’s 3D poses and the scene’s 3D point cloud from the image matches.

## Dependencies
1. [OpenMVG](https://github.com/openMVG/openMVG/)
2. [Boost](http://www.boost.org/)
3. [OpenCV](http://opencv.org/)
4. [Eigen](http://eigen.tuxfamily.org/)

## Citations
1. Adaptive structure from motion with a contrario model estimation. Pierre Moulon, Pascal Monasse, and Renaud Marlet. In ACCV, 2012.
2. C. Liu, J. Yuen, and A. Torralba, “SIFT flow: Dense correspondence across scenes and its applications,” Trans. Pattern Anal. Mach. Intell., vol. 33, no. 5, pp. 978–994, 2011.
3. M. Tau and T. Hassner, Dense Correspondences Across Scenes and Scales. IEEE Trans. Pattern Anal. Mach. Intell. (TPAMI) 38(5): 875-888 (2016)

## Results
<a href="http://www.youtube.com/watch?feature=player_embedded&v=hPCFbbUbaYo
" target="_blank"><img src="http://img.youtube.com/vi/hPCFbbUbaYo/0.jpg" 
alt="OpenMVG 3D reconstruction sparse vs dense " width="640" height="480" border="10" /></a>
