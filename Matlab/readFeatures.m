function feat = readFeatures(featuresFile)
%READFEATURES Read OpenMVG features from file
%   Output:
%       feat - Each row is a feature containing (x, y, scale, orientation)

%% Read data from file
fileID = fopen(featuresFile);
Data = fscanf(fileID,'%f');
fclose(fileID);

%% Parse data
feat = [Data(1:4:end) Data(2:4:end) Data(3:4:end) Data(4:4:end)];
