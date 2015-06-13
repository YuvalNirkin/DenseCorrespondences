function [varargout] = propagateScales_test(varargin)
% This method is used for propagating scales from sparse interest points
% to neighboring pixels.
% This method contains three different means for scale propagation:
% (1) using only the scales at detected interest points ('geometric'),
% (2) using the underlying image information to guide the propagation of
% this information across each image, separately ('image-aware'), and
% (3) using both images simultaneously ('match-aware').
% 
% Project webpage:
%   http://www.openu.ac.il/home/hassner/scalemaps
%
% Note: This code uses functions form the vlfeat distribution [3].
%
% Usage: [output] = propagateScales(proptype, optionalparams)
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
% The parameters that need to be specified depend on the proptype.
%
% Examples of use and associated default values:
%
%  newF = propagateScales('geometric', F, sze)
%       F being the frames returned by the detector,
%       sze could be 2-vector,
%       newF containing the frames for ALL the pixels
%
%  newF = propagateScales('image-aware', F, I)
%       Where I is the image for Image-Aware propagation
%
%  [newF1 newF2] = propagateScales('match-aware', F1, I1, D1, F2, I2, D2)
%       For Match Aware propagation. D1 and D2 are the descriptors extracted at frames 
%		F1 and F2 in images I1 and I2 respectively. 
%  Format for frame and descriptor matrices is the same as the format defined by vlfeat functions.
%
% Also, you can choose between two types of weighting function in the last argument.
% weight_func can be
%   'exponential'       - Used exponential weights.
%   'linear' (default)  - Used the linear weights.
%  
%
% References:
%   [1] M. Tau and T. Hassner, "Dense Correspondences Across Scenes and Scales", CoRR abs/1406.6323 (2014)
%	Please see project webpage for newer versions
%	http://www.openu.ac.il/home/hassner/projects/scalemaps/
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
% The SOFTWARE ("propagateScales.m") is provided "as is", without any
% guarantee made as to its suitability or fitness for any particular use.  
% It may contain bugs, so use of this tool is at your own risk. 
% We take no responsibility for any damage that may unintentionally be caused 
% through its use.
%
% ver 1.0s, August-2014
%

[proptype, F1, Im1, D1, F2, Im2, D2, weight_func] = checkargs(varargin(:));

if strcmpi(proptype, 'geometric') || ...
        strcmpi(proptype, 'image-aware')
    varargout{1} = do_propagateScales(F1, Im1, weight_func);

else if strcmpi(proptype, 'match-aware')
        
        % matches the two sets of SIFT descriptors descr1 and descr2
        [matches, ~] = vl_ubcmatch(D1, D2, 0.1);
        
        varargout{1} = do_propagateScales(F1(:, matches(1, :)), Im1, weight_func);
        varargout{2} = do_propagateScales(F2(:, matches(2, :)), Im2, weight_func);
        
    else
        error('unknown propagation type.');
    end
end

end

%% ----------------------------------------------------------------------
%  checkargs:
%  sort out input arguments, setting defaults and checking errors.

function [proptype, F1, Im1, D1, F2, Im2, D2, weight_func] = checkargs(arg)

% set default
sze1 = [50 50];
sze2 = [50 50];
Im1 = zeros(sze1);
Im2 = zeros(sze2);
F1 = [];
D1 = [];
F2 = [];
D2 = [];

% number of arguments
narg = length(arg);

% propagation type
proptype = arg{1};
if ~ischar(proptype)
    error('proptype must be specified as a string.');
end

if narg == 1
    error(['missing ' proptype 'parameters.']);
end

F1 = arg{2};

idx = 3;    % index in the argument list
if strcmpi(proptype, 'geometric')
    if narg >= 3
        sze1 = arg{idx};
        idx = idx + 1;
        if length(sze1) ~= 2
            error('image size must be a 2-vector.');
        end
        Im1 = zeros(sze1);
    end
end

if strcmpi(proptype, 'image-aware') || ...
        strcmpi(proptype, 'match-aware')
    if narg >= 3
        Im1 = arg{idx};
        idx = idx + 1;
    end
    Im1 = double(Im1);
end

if strcmpi(proptype, 'match-aware')
    if narg < 7
        error(['missing ' proptype 'parameters.']);
    else
        D1 = arg{idx};
        F2 = arg{idx + 1};
        Im2 = arg{idx + 2};
        Im2 = double(Im2);
        D2 = arg{idx + 3};
        idx = idx + 4;
    end
end

% get weighting function
if narg < idx
    weight_func = 'linear';
else
    weight_func = arg{idx};
end

end

%% ----------------------------------------------------------------------
%  do_propagateScales:
%  spropagae scale method.
%{
function new_frames = do_propagateScales(frames, image, weight_func)

% initialization
[s1, s2] = size(image);
image = double((image + 1) ./ (256 / 8));

scales = zeros([s1 s2]);
scales(sub2ind([s1 s2], floor(frames(2, :)), floor(frames(1, :)))) = 1;

indices = reshape(1:s1 * s2, s1, s2);

index = 0;
len = 0;
weights = zeros(1, 3 * 3);
values = zeros(s1 * s2 * 3 * 3, 1);
row_idx = zeros(s1 * s2 * 3 * 3, 1);
col_idx = zeros(s1 * s2 * 3 * 3, 1);
% loop over all image pixels p
for j = 1:s2 % s2 = width (columns)
    for i = 1:s1 % s1 = height (rows)
        len = len + 1;
        if ~scales(i, j)
            n = 0;          % number of neighborhoods
            % loop over 3x3 neighborhoods matrix
            % and calulate the variance of the intensities
            for ii = max(1, i - 1) : min(i + 1, s1)
                for jj = max(1, j - 1) : min(j + 1, s2)
                    if (ii ~= i) || (jj ~= j)
                        n = n + 1;
                        index = index + 1;
                        row_idx(index) = len;
                        col_idx(index) = indices(ii, jj);
                        weights(n) = image(ii, jj);
                    end
                end
            end
            weights(n + 1) = image(i, j);
            v = var(weights(1:n + 1));
            m = 0.6 * mean(weights(1:n + 1));
            if v < 0.0000001
                v = 0.0000001;
            end
            
            if strcmpi(weight_func, 'exponential')
                weights(1:n) = exp(-(weights(1:n) - image(i, j)) .^ 2 / (0.6 * v));
            else    % linear
                weights(1:n) = 1 + (1 / v) * (weights(1:n) - m) * (image(i, j) - m);
            end
            weights(1:n) = weights(1:n) / sum(weights(1:n)); % normalize
            
            values(index - n + 1:index) = -weights(1:n);            
        end
        index = index + 1;
        row_idx(index) = len;
        col_idx(index) = indices(i, j);
        values(index) = 1;
    end
end

values = values(1:index);
col_idx = col_idx(1:index);
row_idx = row_idx(1:index);

A = sparse(row_idx, col_idx, values, len, s1 * s2);
scales = zeros(size(A, 1), 1);
scales(sub2ind([s1 s2], floor(frames(2, :)), floor(frames(1, :)))) = ...
    frames(3, :);
eq = A \ scales;
scales = reshape(eq, s1, s2, 1);

% new frames using scale map
[x, y] = meshgrid(1:s1, 1:s2);
coord1 = [y(:) x(:)]';
coord1 = sortrows(coord1')';
new_frames = [coord1; scales(:)'; zeros(1, numel(y))];
new_frames = sortrows(new_frames')';

end
%}