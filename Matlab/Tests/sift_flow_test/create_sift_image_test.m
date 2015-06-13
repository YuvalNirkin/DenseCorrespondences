addpath 'D:\Dev\Windows Linux\DenseCorrespondences\CPP\SiftFlowMex\x64\Debug'
I=imread('Mars-1.jpg');
I=imresize(I,0.25,'bicubic');

%% Create SIFT image (original)
sift_orig = create_sift_img_t(I);

%% Create SIFT image
I = im2single(rgb2gray(I));
sift = createSIFTImage(I);

%% Results
desc_orig = sift_orig(103,13,:);
desc = sift(103,13,:);

desc_orig = desc_orig(:)';
desc = desc(:)';