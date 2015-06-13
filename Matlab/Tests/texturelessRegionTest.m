%addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\mexFindTexturelessRegions\x64\Debug'
addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\mexFindTexturelessRegions\x64\Release'

%% Load images
%img=imread('D:\Dev\Resources\MSc\kermit\images\kermit001.jpg');
%img=imread('D:\Dev\Resources\MSc\SceauxCastleReduced2\images\100_7106.JPG');
img=imread('D:\Dev\Resources\MSc\ET\images\et001.jpg');

sImg = im2single(img);
if size(sImg, 3) == 3
    sImg = rgb2gray(sImg);
end

%% Process
winSize = 21;
t = 4;
out = findTexturelessRegions2(sImg, winSize, t/255);
%out = mexFindTexturelessRegions2(sImg, winSize, t);

%% Show results
figure;imshow(img);
figure;imshow(out*255);