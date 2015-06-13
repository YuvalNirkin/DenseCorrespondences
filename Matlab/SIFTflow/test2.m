im1=imread('D:\Dev\Resources\AR\OpenMVG\SceauxCastleReduced8\images\100_7100.jpg');
im2=imread('D:\Dev\Resources\AR\OpenMVG\SceauxCastleReduced8\images\100_7101.jpg');

%im1=imresize(imfilter(im1,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');
%im2=imresize(imfilter(im2,fspecial('gaussian',7,1.),'same','replicate'),0.5,'bicubic');

im1=im2double(im1);
im2=im2double(im2);

%figure;imshow(im1);figure;imshow(im2);

cellsize=3;
gridspacing=1;  % 4

addpath(fullfile(pwd,'mexDenseSIFT'));
addpath(fullfile(pwd,'mexDiscreteFlow'));

sift1 = mexDenseSIFT(im1,cellsize,gridspacing);
sift2 = mexDenseSIFT(im2,cellsize,gridspacing);

SIFTflowpara.alpha=0.01;   % 2*255
SIFTflowpara.d=0.2;  % 40*255
SIFTflowpara.gamma=0.001;   % 0.005*255
SIFTflowpara.nlevels=5;
SIFTflowpara.wsize=5;   % 2
SIFTflowpara.topwsize=10;
SIFTflowpara.nTopIterations = 200;   % 100 60
SIFTflowpara.nIterations= 80;   % 40 30

%{
SIFTflowpara.alpha=0.01;
SIFTflowpara.d=40*0.01;
SIFTflowpara.gamma=0.005*255;
SIFTflowpara.nlevels=4;
SIFTflowpara.wsize=5;
SIFTflowpara.topwsize=10;
SIFTflowpara.nTopIterations = 100;
SIFTflowpara.nIterations= 50;
%}

tic;[vx,vy,energylist]=SIFTflowc2f(sift1,sift2,SIFTflowpara);toc
tic;[vx_inv,vy_inv,energylist_inv]=SIFTflowc2f(sift2,sift1,SIFTflowpara);toc

warpI2=warpImage(im2,vx,vy);
figure;imshow(im1);figure;imshow(warpI2);

% match
dist = ones(size(vx))*10000;
[h w] = size(vx);
border = 20;
pts1 = [];
pts2 = [];
for r = 1+border:h-border
    for c = 1+border:w-border
        p1 = [c r];
        p2w = p1 + [vx(r,c) vy(r,c)];
        if(p2w(1) < 1 || p2w(1) > w || p2w(2) < 1 || p2w(2) > h)
             continue;
        else
            p2 = p2w + [vx_inv(p2w(2),p2w(1)) vy_inv(p2w(2),p2w(1))];
            dist(r,c) = pdist2(p1, p2);
        end
        
        if(dist(r,c) <= 0)
            pts1 = [pts1;p1];
            pts2 = [pts2;p2w];
        end
    end
end

thresh = 0;
figure;imshow(dist <= thresh);
showMatchedFeatures(im1,im2,pts1,pts2);

%{
% display flow
clear flow;
flow(:,:,1)=vx;
flow(:,:,2)=vy;
figure;imshow(flowToColor(flow));

% display inverse flow
clear flow_inv;
flow_inv(:,:,1)=vx_inv;
flow_inv(:,:,2)=vy_inv;
figure;imshow(flowToColor(flow_inv));
%}

return;

% this is the code doing the brute force matching
%tic;[flow2,energylist2]=mexDiscreteFlow(Sift1,Sift2,[alpha,alpha*20,60,30]);toc
tic;[flow2,energylist2]=mexDiscreteFlow(sift1,sift2,...
    [SIFTflowpara.alpha SIFTflowpara.d SIFTflowpara.gamma SIFTflowpara.nIterations 2 SIFTflowpara.wsize]);toc
figure;imshow(flowToColor(flow2));
