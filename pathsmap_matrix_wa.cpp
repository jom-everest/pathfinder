#include "pathsmap_matrix_wa.h"
#include <cmath>

const float epsilon = 0.1;

SMatrix<Node> createPathsMap_v1(Coord startPoint, SMatrix<uint32_t>& sourceMap, bool dMode)
{
    uint32_t dimX = sourceMap.getDimx();
    uint32_t dimY = sourceMap.getDimy();

    if (startPoint.x >= dimX || startPoint.y >= dimY) {
//        throw
        return SMatrix<Node>();
    }

    // create new empty PathsMap with size of SourceMap;
    SMatrix<Node> pathsMap(dimX, dimY);

    // fill terrain factor with data from SourceMap
    for (size_t x = 0; x < dimX; ++x) {
        for (size_t y = 0; y < dimY; ++y) {
            auto f = sourceMap.data(x, y);
            if (f == 0) f = 1;
            pathsMap.data(x, y).terrainFactor = f;
        }
    }

    std::vector<std::pair<Coord, Node&>> queueWavefront;
    queueWavefront.reserve(dimX*dimY);

    Node& startNode = pathsMap.data(startPoint);
    startNode.totalPathCost = 1;
    startNode.pathsFrom = {};

    queueWavefront.push_back({startPoint, startNode});

    auto calcNextNode = [&](Coord nextNodeCoord, Coord coord, Node& node, const float diagonalFactor = 1) {
        Node& nextNode = pathsMap.data(nextNodeCoord);

        if (nextNode.terrainFactor >= 1000) return;

        float pathCost = nextNode.terrainFactor*diagonalFactor + node.totalPathCost;
        if (nextNode.totalPathCost == 0) {
            nextNode.totalPathCost = pathCost;
            nextNode.pathsFrom.push_back(coord);
            queueWavefront.push_back({nextNodeCoord, nextNode});
        }
        else if (std::abs(nextNode.totalPathCost - pathCost) < epsilon) {
            nextNode.pathsFrom.push_back(coord);
        }
        else if (nextNode.totalPathCost > pathCost) {
            nextNode.totalPathCost = pathCost;
            nextNode.pathsFrom.clear();
            nextNode.pathsFrom.push_back(coord);
        }
    };

    for (auto it = queueWavefront.begin(); it != queueWavefront.end(); ++it) {
        Coord coord = it->first;
        Node& node = it->second;

        calcNextNode({coord.x, coord.y-1}, coord, node, 1);

        if (dMode) calcNextNode({coord.x-1, coord.y-1}, coord, node, 1.5);
        calcNextNode({coord.x-1, coord.y}, coord, node, 1);
        if (dMode) calcNextNode({coord.x-1, coord.y+1}, coord, node, 1.5);

        calcNextNode({coord.x, coord.y+1}, coord, node, 1);

        if (dMode) calcNextNode({coord.x+1, coord.y+1}, coord, node, 1.5);
        calcNextNode({coord.x+1, coord.y}, coord, node, 1);
        if (dMode) calcNextNode({coord.x+1, coord.y-1}, coord, node, 1.5);
    }

    return pathsMap;
}

