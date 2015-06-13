function listFrames(inDir, calibFile, step)
%LISTFRAMES List frames in input directory with the specified step
%   Input:
%       inDir - Path to the input directory
%       calibFile - Path to the calibration file
%       step - Step size from frame to frame

%% Parse directory
fileDescs = dir(inDir);
fileNames = {fileDescs.name};
fileNames = fileNames(~[fileDescs.isdir]);

%% Read calibration file
fileID = fopen(calibFile,'r');
Data = fscanf(fileID, '%f');
fclose(fileID);
fx = Data(1);
fy = Data(2);
cx = Data(3);
cy = Data(4);
w = int32(Data(end - 1));
h = int32(Data(end));

%% Write file list
fileID = fopen(fullfile(inDir,'lists.txt'),'w');

for i = 1:step:length(fileNames)
   fprintf(fileID,'%s;%d;%d;%4.4f;0;%4.4f;0;%4.4f;%4.4f;0;0;1\n',...
       fileNames{i},w,h,fx,cx,fy,cy);
end

fclose(fileID);

