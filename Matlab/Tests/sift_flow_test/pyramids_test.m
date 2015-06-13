addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Debug';
w = 20;
h = 20;
chnl = 128;
img = reshape(1:h*w*chnl, [h w chnl]);

%% Perform bicubic interpolation
img2 = imfilter(img,fspecial('gaussian',5,0.67),'same','replicate');
img3=imresize(img,0.5,'bicubic');

%% Call pyramids module
[img2_cpp img3_cpp] = Pyramids(img);

%% Compare
img2_diff = abs(img2 - img2_cpp);
img3_diff = abs(img3 - img3_cpp);