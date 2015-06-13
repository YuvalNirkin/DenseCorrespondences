function [I isDerivativeImage] = loadImage(imgPath)
%LOADIMAGE Summary of this function goes here
%   Detailed explanation goes here
fileID = fopen(imgPath);
type = fread(fileID,[1 16],'*char');
type=type(1:min(find(type == 0))-1);
width = fread(fileID,1,'*uint32');
height = fread(fileID,1,'*uint32');
channels = fread(fileID,1,'*uint32');
isDerivativeImage = fread(fileID,1,'*uint8');
I = zeros(height, width, channels);
for r = 1:height 
    for c = 1:width      
        I(r, c, :) = fread(fileID,channels,['*' type]);
    end
end
%I = fread(fileID,height*width*channels,['*' type]);
%I = reshape(I, [height width channels]);
%I = permute(I, [3 2 1]);
fclose(fileID);