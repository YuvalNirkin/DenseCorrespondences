addpath '..\..\SIFTflow\mexDenseSIFT'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow\mexDiscreteFlow'

%% Create SIFT images
%im1=imread('Mars-1.jpg');
%im2=imread('Mars-2.jpg');
im1=imread('100_7100.JPG');
im2=imread('100_7101.JPG');

im1=imresize(imfilter(im1,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');
im2=imresize(imfilter(im2,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');

im1=im2double(im1);
im2=im2double(im2);

%figure;imshow(im1);figure;imshow(im2);

cellsize=3;
gridspacing=1;

addpath(fullfile(pwd,'mexDenseSIFT'));
addpath(fullfile(pwd,'mexDiscreteFlow'));

%sift1 = mexDenseSIFT(im1,cellsize,gridspacing);
%sift2 = mexDenseSIFT(im2,cellsize,gridspacing);
sift1 = create_sift_img(im1);
sift2 = create_sift_img(im2);

%% Do SIFT flow
SIFTflowpara.alpha=2*255;
SIFTflowpara.d=40*255;
SIFTflowpara.gamma=0.005*255;
SIFTflowpara.nlevels=4;
SIFTflowpara.wsize=2;
SIFTflowpara.topwsize=10;
SIFTflowpara.nTopIterations = 60;
SIFTflowpara.nIterations= 30;

[vx,vy,energylist] = SIFTflowc2f(single(sift1), single(sift2), SIFTflowpara);

%% Show results
clear flow;
flow(:,:,1) = vx;
flow(:,:,2) = vy;
figure;imshow(flowToColor(flow));

warpI2=warpImage(im2,vx,vy);
figure;imshow(im1);figure;imshow(warpI2);

rmpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow'
rmpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow\mexDiscreteFlow'