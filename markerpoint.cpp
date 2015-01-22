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

#include "markerpoint.h"

using namespace glm;

MarkerPoint::MarkerPoint(QTreeWidgetItem *item, MarkerPoint *parent)
{
    vec3 offset = vec3( item->text(1).toFloat(), item->text(2).toFloat(), item->text(3).toFloat());

    offsetFromRoot = offset;
    name = item->text(0).toStdString();
    parent = parent;
    Id = -1;

}

MarkerPoint::MarkerPoint(vec3 offset, std::string name, MarkerPoint *parent, int ID)
{
    offsetFromRoot = offset;
    name = name;
    parent  = parent;
    this->Id = ID;
}

void MarkerPoint::AddChild(MarkerPoint *child)
{
    childs.push_back(child);
    distances.push_back(PointDistance(offsetFromRoot, child->getOffset()));
}

void MarkerPoint::SaveHierarchy(ExportFormat format, std::ofstream &outputFile)
{
    switch (format) {
    case PROJECT:

        break;
    default:
        break;
    }
}

void MarkerPoint::SaveMotion(ExportFormat format, std::ofstream &outputFile)
{

}

float MarkerPoint::PointDistance(vec3 point1, vec3 point2)
{
    vec3 vector = point2 - point1;

    return glm::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

float MarkerPoint::PointDistance(vec2 point1, vec2 point2)
{
    vec2 vector = point2 - point1;

    return glm::sqrt(vector.x * vector.x + vector.y * vector.y);
}
