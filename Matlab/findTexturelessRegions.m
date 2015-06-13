% *************************************************************************
% Title: Function-Find Textureless regions of an image
% Notes: Textureless regions are defined as regions where the squared horizontal
% intensity gradient averaged over a square window of a given size 
% (windowSize) is below a given threshold (thresh);
% Author: Siddhant Ahuja
% Created: May 2008
% Copyright Siddhant Ahuja, 2008
% Inputs: Input Image (var: inputImage), Window Size (var: windowSize),
% Threshold (var: thresh) Typical value is 4
% Outputs: Textureless Map (var: texturelessImg) , Time taken (var: timeTaken)
% Example Usage of Function: [texturelessImg, timeTaken]=funcTexturelessRegions('imL.png', 9, 4);
% *************************************************************************
function [texturelessImg, timeTaken]=findTexturelessRegions(inputImage, windowSize, thresh)
% Read the input image
try
    % Read an image using imread function
    inputImage=imread(inputImage);
    % grab the number of rows, columns, and channels
    [nr, nc, nChannels]=size(inputImage);
     % Grab the image information (metadata) of input image using the function imfinfo
    inputImageInfo=imfinfo(inputImage);
    % Determine if input left image is already in grayscale or color
    if(getfield(inputImageInfo,'ColorType')=='truecolor')
        colored=1;
    else if(getfield(inputImageInfo,'ColorType')=='grayscale')
        colored=0;
        else
        error('The Color Type of Left Image is not acceptable. Acceptable color types are truecolor or grayscale.');
        end
    end
catch
    % if it is not an image but a variable
    % grab the number of channels
    [nr, nc, nChannels]=size(inputImage);
    if(nChannels)>1
        colored=1;
    else
        colored=0;
    end
end
% Check the size of window to see if it is an odd number.
if (mod(windowSize,2)==0)
    error('The window size must be an odd number.');
end
% Create an image of size nr and nc, fill it with zeros and assign
% it to variable texturelessImg
texturelessImg=zeros(nr, nc);
% Create an image of size nr and nc, fill it with zeros and assign
% it to variable sqGradImg
sqGradImg=zeros(nr,nc);
% Find out how many rows and columns are to the left/right/up/down of the
% central pixel based on the window size
win=(windowSize-1)/2;
inputImage=double(inputImage);
tic; % Initialize the timer to calculate the time consumed.
% Produce Squared Horizontal Gradient image sqGradImg
for (i=1:1:nr)
    for (j=1:1:nc-1)
        sum = 0.0;        
        for (k=1:1:nChannels)
            diff =  inputImage(i,j,k) - inputImage(i,j+1,k);
            sum = sum + (diff*diff); 
        end
        sum = sum / nChannels;
        sqGradImg(i,j+1) = sum;
        if (j==1)
            sqGradImg(i,j) = sum;
        end
        if (sum > sqGradImg(i,j))
            sqGradImg(i,j) = sum;
        end
    end
end
% Compute average within predefined box window of size windowSize x
% windowSize
for (i=1+win:nr-win)
    for (j=1+win:nc-win)       
        % go over the square window
        sum = 0.0;
        avg = 0.0;
        for (a=-win:1:win)
            for (b=-win:1:win)                
                sum = sum + sqGradImg(i+a,j+b);
            end
        end           
        % Compute the average
        avg = sum / (windowSize*windowSize);
        % Apply threshold
        if (avg < (thresh*thresh))
            texturelessImg(i,j) = 255; % mark detected textureless pixel as white
        end
    end
end
% Stop the timer to calculate the time consumed.
timeTaken=toc;