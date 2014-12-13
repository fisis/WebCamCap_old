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

#ifndef LINE_H
#define LINE_H

#include <iostream>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

using namespace glm;

class Line
{
public:
    bool found;
    vec3 position;
    vec3 directionVector;

    size_t intersections;

    Line(vec3 pos, vec3 vec);

    bool getFound() const {return found;}

    void setFound(bool F) {found = F;}

    static void ClosestPointsOnTwoLines(Line l1, Line l2, vec3 &closestPointLine1, vec3 &closestPointLine2);
    static vec3 IntersectionLinePlane(Line l1, Line Plane);
    static float DistanceTwoPoints(vec3 point1, vec3 point2);
    static float DistancePointPlane(vec3 point, Line Plane);
    static vec3 AveragePoint(vec3 point1, vec3 point2);
    static float LineAngle(Line l1, Line l2);
    static float LineAngle(vec2 v1, vec2 v2);
    static vec3 Intersection(Line &l1, Line &l2, float Epsilon);

};

std::ostream& operator << (std::ostream &stream,const vec3 &position);
std::ostream& operator << (std::ostream &stream,const vec2 &position);
std::ostream& operator << (std::ostream &stream, const Line &line);

#endif // LINE_H
