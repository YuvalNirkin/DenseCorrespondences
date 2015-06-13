addpath(fullfile(pwd,'mexDenseSIFT'));
addpath(fullfile(pwd,'mexDiscreteFlow'));

%% Load images
im1=imread('D:\Dev\Resources\AR\OpenMVG\SceauxCastleReduced4\images\100_7100.jpg');
im2=imread('D:\Dev\Resources\AR\OpenMVG\SceauxCastleReduced4\images\100_7101.jpg');
im1=im2double(im1);
im2=im2double(im2);
%figure;imshow(im1);figure;imshow(im2);

%% Calculate SIFT images
cellsize=3;
step=2;  % 4
sift1 = mexDenseSIFT(im1,cellsize,step);
sift2 = mexDenseSIFT(im2,cellsize,step);

%% Calculate SIFT flow
SIFTflowpara.alpha=0.01;   % 2*255
SIFTflowpara.d=0.2;  % 40*255
SIFTflowpara.gamma=0.001;   % 0.005*255
SIFTflowpara.nlevels=5;
SIFTflowpara.wsize=5;   % 2
SIFTflowpara.topwsize=10;
SIFTflowpara.nTopIterations = 200;   % 100 60
SIFTflowpara.nIterations= 80;   % 40 30

tic;[vx,vy,energylist]=SIFTflowc2f(sift1,sift2,SIFTflowpara);toc
tic;[vx_inv,vy_inv,energylist_inv]=SIFTflowc2f(sift2,sift1,SIFTflowpara);toc

%% match
dist = ones(size(vx))*10000;
[h w] = size(vx);
border = 20;
thresh = 0;

[X Y] = meshgrid(1:w,1:h);
p1 = [X(:) Y(:)];
p2 = p1 + [vx(:) vy(:)];
valid = p1(:,1) >= (1+border) & p1(:,1) <= (w-border) & p1(:,2) >= (1+border) & p1(:,2) <= (h-border);
valid = valid & p2(:,1) >= 1 & p2(:,1) <= w & p2(:,2) >= 1 & p2(:,2) <= h;
p1 = p1(valid,:);
p2 = p2(valid,:);
p2i = sub2ind(size(vx),p2(:,2),p2(:,1));
p1_inv = p2 + [vx_inv(p2i) vy_inv(p2i)];
delta = p1 - p1_inv;
dist = delta(:,1).*delta(:,1) + delta(:,2).*delta(:,2);
valid = dist <= thresh;
p1 = p1(valid,:).*step;
p2 = p2(valid,:).*step;

figure;showMatchedFeatures(im1,im2,p1,p2);

%{
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
            pts1 = [pts1;p1.*step];
            pts2 = [pts2;p2w.*step];
        end
    end
end


thresh = 0;
figure;imshow(dist <= thresh);
showMatchedFeatures(im1,im2,pts1,pts2);
%}