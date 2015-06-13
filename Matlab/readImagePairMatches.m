function imgPairMatches = readImagePairMatches(matchesFile)
%READIMAGEPAIRMATCHES Read OpenMVG image pair matches from file
%   Detailed explanation goes here

%% Read data from file
fileID = fopen(matchesFile);
Data = fscanf(fileID,'%f');
fclose(fileID);

%% Parse data
pairCounter = 1;
dataPos = 1;
while(dataPos < length(Data))
    imgPairMatches{pairCounter}.imgIndex1 = Data(dataPos)+1;
    imgPairMatches{pairCounter}.imgIndex2 = Data(dataPos+1)+1;
    matchesNum = Data(dataPos+2);
    pairMatches = Data(dataPos+3:dataPos+2+matchesNum*2);
    pairMatches = [pairMatches(1:2:end) pairMatches(2:2:end)];
    pairMatches = pairMatches + 1;
    imgPairMatches{pairCounter}.pairInd = pairMatches;
    
    % Adjust data position and pair counter
    dataPos = dataPos+3+matchesNum*2;
    pairCounter = pairCounter + 1;
end
