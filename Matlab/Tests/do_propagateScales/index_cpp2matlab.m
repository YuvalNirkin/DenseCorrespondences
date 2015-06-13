function mati = index_cpp2matlab(size, cppi)
%ROWI2COLI Convert row index 2 column index

% cppi = r*cols + c
% mati = c*rows + r
c = mod(cppi, size(2)) + 1;
r = floor(cppi / size(2)) + 1;
mati = sub2ind(size, r, c);

