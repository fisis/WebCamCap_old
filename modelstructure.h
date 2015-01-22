/*
 *
 * Copyright (C) 2014  Miroslav Krajicek, Faculty of Informatics Masaryk University (https://github.com/kaajo).
 * All Rights Reserved.
 *
 * This file is part of WebCamCap.
 *
 * WebCamCap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU LGPL version 3 as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * WebCamCap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU LGPL version 3
 * along with WebCamCap. If not, see <http://www.gnu.org/licenses/lgpl-3.0.txt>.
 *
 */

#ifndef MODELSTRUCTURE_H
#define MODELSTRUCTURE_H

#include "markerpoint.h"

using namespace std;

class ModelStructure
{
    size_t size, epsilon;

    std::vector<MarkerPoint*> roots;
    std::vector<MarkerPoint> pointsStructure;

    vector<vector<float> > distances;
public:
    ModelStructure();
    ModelStructure(QTreeWidget *item);
    void AddPoint(MarkerPoint point){pointsStructure.push_back(point); size++;}
    void AddRoot(MarkerPoint *root){roots.push_back(root);}
    void AddPoint(vector<MarkerPoint> points){pointsStructure.insert(pointsStructure.end(), points.begin(), points.end()); size += points.size();}

    vector<size_t> getIDs(vector <glm::vec3> Points);
    size_t getSize() const {return size;}

    void Save(ExportFormat format, std::ofstream &outputFile);

private:
    void countDistances(vector <glm::vec3> Points);
    bool verifyDistance(size_t PointIndex, MarkerPoint marker);

    void addChildren(QTreeWidgetItem *parentItem, MarkerPoint* parent);
};

#endif // MODELSTRUCTURE_H
