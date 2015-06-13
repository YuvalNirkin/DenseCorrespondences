function sift = create_sift_img(I)
%CREATE_SIFT_IMG Create SIFT image without scale map
%   Detailed explanation goes here
I = im2single(rgb2gray(I));
[h w] = size(I);
[X,Y] = meshgrid(1:w, 1:h);
frames = [X(:) Y(:) ones(h*w,1).*3 zeros(h*w,1)]';

[f1, sift] = vl_sift(I, 'frames', frames, 'magnif', 1);
[~, ind] = sortrows(f1');
sift = sift(:, ind)';
sift = reshape(sift(:), [h w 128]);

