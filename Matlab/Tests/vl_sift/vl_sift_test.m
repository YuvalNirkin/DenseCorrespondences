%% Load images
source = imread('..\scalemaps\source.jpg');
target = imread('..\scalemaps\target.jpg');

if size(source, 3) == 3
    source = rgb2gray(source);
end
if size(target, 3) == 3
    target = rgb2gray(target);
end

source = im2single(source);
target = im2single(target);

%% Calculate frames
[s1, s2] = size(source);
[x, y] = meshgrid(1:s1, 1:s2);
coord1 = [y(:) x(:)]';
coord1 = sortrows(coord1')';

scales = ones([s1 s2]).*3;
frames = [coord1; scales(:)'; zeros(1, numel(y))];
frames = sortrows(frames')';

%%
[f1, desc] = vl_sift(source', 'frames', frames, 'magnif', 1);