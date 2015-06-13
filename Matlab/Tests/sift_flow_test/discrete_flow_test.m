addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Debug'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\Matlab\SIFTflow\mexDiscreteFlow'

im1=imread('Mars-1.jpg');
im2=imread('Mars-2.jpg');

im1=imresize(imfilter(im1,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');
im2=imresize(imfilter(im2,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');

im1=im2double(im1);
im2=im2double(im2);

%sift1 = mexDenseSIFT(im1,cellsize,gridspacing);
%sift2 = mexDenseSIFT(im2,cellsize,gridspacing);
sift1 = create_sift_img(im1);
sift2 = create_sift_img(im2);

% SIFT flow parameters
SIFTflowpara.alpha=2*255;
SIFTflowpara.d=40*255;
SIFTflowpara.gamma=0.005*255;
SIFTflowpara.nlevels=4;
SIFTflowpara.wsize=2;
SIFTflowpara.topwsize=10;
SIFTflowpara.nTopIterations = 60;
SIFTflowpara.nIterations= 30;

% Discrete flow original
[flow,foo]=mexDiscreteFlow(double(sift1),double(sift2),[SIFTflowpara.alpha,SIFTflowpara.d,SIFTflowpara.gamma,SIFTflowpara.nIterations,2,SIFTflowpara.wsize]);
vx=flow(:,:,1);
vy=flow(:,:,2);

figure;imshow(flowToColor(flow));

warpI2=warpImage(im2,vx,vy);
figure;imshow(im1);figure;imshow(warpI2);

% Discrete flow new
flow=discreteFlow(double(sift1),double(sift2), SIFTflowpara);
vx=flow(:,:,1);
vy=flow(:,:,2);

figure;imshow(flowToColor(flow));

warpI2=warpImage(im2,vx,vy);
figure;imshow(im1);figure;imshow(warpI2);