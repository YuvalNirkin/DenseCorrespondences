% DEMO script for producing dense flow using scale maps [1]. The demo will
% compute flow from source image to target image, and then warp the target
% image back to the source.
% 
% Project webpage:
%   http://www.openu.ac.il/home/hassner/scalemaps
%
% Notes:
%   1. This code uses the vl_sift function included in the vlfeat
%       distribution [3]. It MUST be installed and available on your MATLAB
%       path in order for this demo to work. See README.txt for more
%       information.
%   2. To compute dense correspondences this code uses the SIFT-Flow
%       algorithm available from [2]. It MUST be installed and available on
%       yout MATLAB path in order for this demo to work. See README.txt for
%       more information.
%
% References:
%   [1] M. Tau and T. Hassner, "Dense Correspondences Across Scenes and 
%   Scales," arXiv preprint arXiv:1406.6323, 24 Jun. 2014. Please see
%   project webpage for information on more recent versions:
%   http://www.openu.ac.il/home/hassner/scalemaps
%
%   [2] C. Liu, J. Yuen, A. Torralba, J. Sivic, and W. Freeman. "Sift
%   flow: dense correspondence across different scenes." In European
%   Conf. Comput. Vision, pages 28-42, 2008.
%   http://people.csail.mit.edu/celiu/ECCV2008/
%
%   [3] A. Vedaldi and B. Fulkerson. Vlfeat: An open and portable
%   library of computer vision algorithms. In Proc. int. conf.
%   on Multimedia, pages 1469-1472, 2010.
%   http://www.vlfeat.org/
%
% ver 1.0s, August-2014
%
% Copyright 2014, Moria Tau and Tal Hassner
%
% The SOFTWARE ("demo.m") is provided "as is", without any
% guarantee made as to its suitability or fitness for any particular use.  
% It may contain bugs, so use of this tool is at your own risk. 
% We take no responsibility for any damage that may unintentionally be caused 
% through its use.
%

%% Initialization
addpath vlfeat-0.9.18/toolbox % Tested with vlfeat 0.9.18 [3]
addpath release % Tested with SIFT-Flow ECCV'08 release [2]
addpath release/mex % Tested with SIFT-Flow ECCV'08 release [2]
%addpth 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\ScaleMaps\x64\Debug'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\ScaleMaps\x64\Release'
vl_setup('quiet');

% read source and target images
source_name = 'target.JPG';
target_name = 'source.JPG';
source = imread(source_name);
target = imread(target_name);

%% Extract descriptors
tic
%[desc_source, desc_target] = get_descriptors_test(source, target, 'match-aware', 'linear');
[desc_source, desc_target] = get_descriptors_test(source, target, 'match-aware', 'exponential');
toc

%% SIFT-Flow

SIFTflowpara.alpha = 2 * 255;
SIFTflowpara.d = 40 * 255;
SIFTflowpara.gamma = 0.005 * 255;
SIFTflowpara.nlevels = 4;
SIFTflowpara.wsize = 5;
SIFTflowpara.topwsize = 20;
SIFTflowpara.nIterations = 60;

[vx, vy, ~] = SIFTflowc2f(double(desc_source), double(desc_target), SIFTflowpara);

flow(:, :, 1) = vx;
flow(:, :, 2) = vy;

clrflo = flowToColor(flow);

%% Figures
figure; imshow(clrflo); title('flow');

warp = warpImage(im2double(target),vx,vy);

figure; imshow(source); title('source');
figure; imshow(target); title('target');
figure; imshow(warp); title('warped');
