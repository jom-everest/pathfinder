#ifndef PATHSMAP_MATRIX_WA_H
#define PATHSMAP_MATRIX_WA_H

#include <vector>
#include "smatrix.h"

struct Node {
    int terrainFactor {1};
    float totalPathCost {0};
    std::vector<Coord> pathsFrom {};
};

SMatrix<Node> createPathsMap_v1(Coord startPoint, SMatrix<uint32_t>& sourceMap, bool dMode = false);

#endif // PATHSMAP_MATRIX_WA_H
