%I1 = imread('D:\Dev\Resources\MSc\SceauxCastleReduced4\images\100_7100.JPG');
%I2 = imread('D:\Dev\Resources\MSc\SceauxCastleReduced4\images\100_7102.JPG');
%matchesDir = 'D:\Dev\Resources\MSc\SceauxCastleReduced4\Archive\denseMatches';
%I1 = imread('D:\Dev\Resources\MSc\kermit\images\kermit000.jpg');
%I2 = imread('D:\Dev\Resources\MSc\kermit\images\kermit001.jpg');
%matchesDir = 'D:\Dev\Resources\MSc\kermit\denseMatches_sm';

imgDir = 'D:\Dev\Resources\MSc\ET\images';
matchesDir = 'D:\Dev\Resources\MSc\ET\denseMatches_sm';

%% Load images
fileDescs = dir(imgDir);
fileNames = {fileDescs.name};
fileNames = fileNames(~[fileDescs.isdir]);

%% Read matches
matches = readMatches(matchesDir);

%% Plot
mp = 1; mf = 1;
while(mp <= length(matches.putative))
    i = matches.putative{mp}.imgIndex1;
    j = matches.putative{mp}.imgIndex2;
    I1 = imread(fullfile(imgDir, fileNames{i}));
    I2 = imread(fullfile(imgDir, fileNames{j}));
    
    figure(1);
    showMatchedFeatures(I1,I2,matches.putative{mp}.pts1,...
        matches.putative{mp}.pts2);
    
    if(mf <= length(matches.filtered) && ...
        i == matches.putative{mf}.imgIndex1 && ...
        j == matches.putative{mf}.imgIndex2)
        figure(2);
        showMatchedFeatures(I1,I2,matches.filtered{mf}.pts1,...
            matches.filtered{mf}.pts2);
        mf = mf + 1;
    end
    mp = mp + 1;
    w = waitforbuttonpress;
end
%{
figure;
showMatchedFeatures(I1,I2,matches.putative{2}.pts1,matches.putative{2}.pts2);
figure;
showMatchedFeatures(I1,I2,matches.filtered{2}.pts1,matches.filtered{2}.pts2);
%}