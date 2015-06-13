%addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Debug'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Release'

%% Load images
%im1=imread('target.jpg');
%im2=imread('source.jpg');
im1=imread('D:\Dev\Resources\MSc\kermit\images\kermit000.jpg');
im2=imread('D:\Dev\Resources\MSc\kermit\images\kermit001.jpg');

sImage1 = im2single(im1);
sImage2 = im2single(im2);
if size(sImage1, 3) == 3
    sImage1 = rgb2gray(sImage1);
end
if size(sImage2, 3) == 3
    sImage2 = rgb2gray(sImage2);
end

%% Compute matched features
[F1, D1] = vl_sift(sImage1);
[F2, D2] = vl_sift(sImage2);
[matches, ~] = vl_ubcmatch(D1, D2, 0.1);
F1 = F1(:, matches(1, :));
F2 = F2(:, matches(2, :));

%% Do SIFT flow
SIFTflowpara.alpha=2*255;
SIFTflowpara.d=40*255;
SIFTflowpara.gamma=0.005*255;
SIFTflowpara.nlevels=4;
SIFTflowpara.wsize=5;
SIFTflowpara.topwsize=20;
SIFTflowpara.nTopIterations = 100;
SIFTflowpara.nIterations= 60;

[vx, vy] = sift_flow_sm(sImage1, sImage2, F1, F2, SIFTflowpara);
%[vx, vy] = sift_flow(sImage1, sImage2, SIFTflowpara);

%% Show results
clear flow;
flow(:,:,1) = vx;
flow(:,:,2) = vy;
figure;imshow(flowToColor(flow));

warpI2=warpImage(im2double(im2),vx,vy);
figure;imshow(im1);figure;imshow(warpI2);

rmpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Debug'
