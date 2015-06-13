function matches = readMatches(matchesDir)
%READMATCHES Read OpenMVG matches from directory
%   Detailed explanation goes here

%% Get file names
featDescs = dir(fullfile(matchesDir, '*.feat'));
featFiles = {featDescs.name};
putativeMatchesFile = fullfile(matchesDir, 'matches.putative.txt');
filteredMatchesFile = fullfile(matchesDir, 'matches.f.txt');

%% Read features
for i = 1:length(featFiles)
    featFilePath = fullfile(matchesDir, featFiles{i});
    features{i} = readFeatures(featFilePath);
end

%% Read matches and combine them with the features
matches.putative = combineMatches(putativeMatchesFile);
matches.filtered = combineMatches(filteredMatchesFile);

    function combinedMatches = combineMatches(matchesFile)
        combinedMatches = readImagePairMatches(matchesFile);
        for m = 1:length(combinedMatches)
            feat1 = features{combinedMatches{m}.imgIndex1};
            feat2 = features{combinedMatches{m}.imgIndex2};
            combinedMatches{m}.pts1 = ...
                feat1(combinedMatches{m}.pairInd(:,1),1:2);
            combinedMatches{m}.pts2 = ...
                feat2(combinedMatches{m}.pairInd(:,2),1:2);
        end
    end
end