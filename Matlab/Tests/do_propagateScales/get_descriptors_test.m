function [desc1, desc2] = get_descriptors_test(image1, image2, proptype, weight_func)
% Extract dense descriptors [1] for an image pair. These can then be used
% to establish dense correspondences between the images using the SIFT-Flow
% algorithm [2].
% 
% Project webpage:
%   http://www.openu.ac.il/home/hassner/scalemaps
%
% Note: This code uses the vl_sift function included in the vlfeat
% distribution [3].
%
% Usage: [desc1 desc2] = get_descriptors(image1, image2, proptype, weight_func)
%
% proptype can be
%
%   'geometric'     - propagate scale information from detected interest
%       points by considering only the spatial locations where scales were
%       detected.
%   'image-aware'   - scales are propagated as above, but using image
%       intensities in order to guide scale propagation.
%   'match-aware'   - consider the two images, propagating only the scales
%       of pixels that were selected as (sparse) key-points in both images.
%   see [1] for more details.
%
% weight_func can be
%   'exponential'       - Used exponential weights.
%   'linear' (default)  - Used the linear weights.
%
%
% References:
%   [1] "Dense Correspondences Across Scenes and Scales"
%
%   [2] C. Liu, J. Yuen, A. Torralba, J. Sivic, and W. Freeman. "Sift
%   flow: dense correspondence across different scenes." In European
%   Conf. Comput. Vision, pages 28-42, 2008.
%   http://people.csail.mit.edu/celiu/ECCV2008/
%
%   [3] A. Vedaldi and B. Fulkerson. Vlfeat: An open and portable
%   library of computer vision algorithms. In Proc. int. conf.
%   on Multimedia, pages 1469-1472, 2010.
%   http://www.vlfeat.org/
%
% Copyright 2014, Moria Tau and Tal Hassner
%
% The SOFTWARE ("get_descriptors.m") is provided "as is", without any
% guarantee made as to its suitability or fitness for any particular use.  
% It may contain bugs, so use of this tool is at your own risk. 
% We take no responsibility for any damage that may unintentionally be caused 
% through its use.
%
% ver 1.0s, August-2014
%

% compute SIFT frames of the input images
sImage1 = im2single(image1);
sImage2 = im2single(image2);
if size(sImage1, 3) == 3
    sImage1 = rgb2gray(sImage1);
end
if size(sImage2, 3) == 3
    sImage2 = rgb2gray(sImage2);
end
[F1, D1] = vl_sift(sImage1);
[F2, D2] = vl_sift(sImage2);

% scale propagation
if strcmp(proptype, 'geometric')
    newF1 = propagateScales_test(proptype, F1, size(sImage1), weight_func);
    newF2 = propagateScales_test(proptype, F2, size(sImage2), weight_func);
else if strcmp(proptype, 'image-aware')
        newF1 = propagateScales_test(proptype, F1, sImage1, weight_func);
        newF2 = propagateScales_test(proptype, F2, sImage2, weight_func);
    else if strcmp(proptype, 'match-aware')
            [newF1, newF2] = propagateScales_test(proptype, F1, sImage1, D1, F2, sImage2, D2, weight_func);
            clear F1 D1 F2 D2
        end
    end
end

% dense sift
desc1 = dense_sift(sImage1, newF1);
desc2 = dense_sift(sImage2, newF2);

end


function desc = dense_sift(image, frames)

pad_size = 2 * 16;
image_padded = padarray(image, [pad_size pad_size], 'replicate');
frames(1, :) = frames(1, :) + pad_size;
frames(2, :) = frames(2, :) + pad_size;

[f1, desc] = vl_sift(image_padded, 'frames', frames, 'magnif', 1);
[~, ind] = sortrows(f1');
desc = desc(:, ind)';
desc = reshape(desc(:), [size(image, 1) size(image, 2) 128]);

end