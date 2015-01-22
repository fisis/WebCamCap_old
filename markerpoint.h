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

#ifndef MARKERPOINT_H
#define MARKERPOINT_H

#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#include <QTreeWidget>

enum ExportFormat
{
    BVH,
    CSM,
    SDL,
    PROJECT
};

class MarkerPoint
{
    int Id;
    std::string name;
    glm::vec3 offsetFromRoot;

    MarkerPoint* parent;

    std::vector<MarkerPoint*> childs;
    std::vector<float> distances;
public:
    MarkerPoint(QTreeWidgetItem *item, MarkerPoint *parent);
    MarkerPoint(glm::vec3 offset, std::string name, MarkerPoint* parent = nullptr, int Id = -1);

    void AddChild(MarkerPoint *child);
    void SaveHierarchy(ExportFormat format, std::ofstream &outputFile);
    void SaveMotion(ExportFormat format,  std::ofstream &outputFile);

    glm::vec3 getOffset() const {return offsetFromRoot;}
    int getID() const {return Id;}
    std::vector<float> getDistances() const {return distances;}

    void setID(int id) {Id = id;}

    static float PointDistance(glm::vec3 point1, glm::vec3 point2);
    static float PointDistance(glm::vec2 point1, glm::vec2 point2);
private:

};

#endif // MARKERPOINT_H
