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

#include "line.h"
#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

using namespace glm;

Line::Line(vec3 pos, vec3 vec)
{
    found = false;
    position = pos;
    directionVector = vec;
    intersections = 0;
}

void Line::ClosestPointsOnTwoLines(Line l1, Line l2, vec3 &closestPointLine1, vec3 &closestPointLine2)
{
        float a = dot(l1.directionVector, l1.directionVector);
        float b = dot(l1.directionVector, l2.directionVector);
        float e = dot(l2.directionVector, l2.directionVector);

        float d = a*e - b*b;

        // if lines are not parallel
        if(d != 0){
            vec3 r = l1.position - l2.position;
            float c = dot(l1.directionVector, r);
            float f = dot(l2.directionVector, r);

            float s = (b*f - c*e) / d;
            float t = (a*f - c*b) / d;

            closestPointLine1 = l1.position + l1.directionVector * s;
            closestPointLine2 = l2.position + l2.directionVector * t;
/*
            std::cout  << l1.Position.x << " " << l1.Position.y << " " << l1.Position.z << std::endl
                       << "vector:" << l1.DirectionVector.x << " " << l1.DirectionVector.y << " " << l1.DirectionVector.z << std::endl;

            std::cout  << l2.Position.x << " " << l2.Position.y << " " << l2.Position.z << std::endl
                       << "vector2:" << l2.DirectionVector.x << " " << l2.DirectionVector.y << " " << l2.DirectionVector.z << std::endl << std::endl;
      */  }
}

vec3 Line::IntersectionLinePlane(Line L, Line Plane)
{
    float d = Plane.directionVector.x * -Plane.position.x + Plane.directionVector.y * -Plane.position.y  + Plane.directionVector.z * -Plane.position.z;

    float t =       -(Plane.directionVector.x * L.position.x + Plane.directionVector.y * L.position.y + Plane.directionVector.z * L.position.z + d)/
                       (Plane.directionVector.x * L.directionVector.x + Plane.directionVector.y * L.directionVector.y + Plane.directionVector.z * L.directionVector.z);

    return L.position + t * L.directionVector;
}

float Line::DistanceTwoPoints(vec3 point1, vec3 point2)
{
    vec3 vector = point2 - point1;

    //std::cout << vector.x << vector.y << vector.z << std::endl;

    float res = glm::sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);

    std::cout << res <<  std::endl;

    return res;
}

float Line::DistancePointPlane(vec3 Point, Line Plane)
{
    float    sb, sn, sd;

        sn = -dot( Plane.directionVector, (Point - Plane.position));
        sd = dot(Plane.directionVector, Plane.directionVector);
        sb = sn / sd;

        vec3 intersection = Point + sb * Plane.directionVector;
        return DistanceTwoPoints(Point, intersection);
}

vec3 Line::AveragePoint(vec3 point1, vec3 point2)
{
    return vec3((point1.x + point2.x)/2,(point1.y + point2.y)/2,(point1.z + point2.z)/2);
}

float Line::LineAngle(Line l1, Line l2)
{
    return acos( dot(l1.directionVector, l2.directionVector)/sqrt(glm::length2(l1.directionVector) * glm::length2(l2.directionVector)) );
}

float Line::LineAngle(vec2 v1, vec2 v2)
{
    return atan2(v2.y,v2.x) - atan2(v1.y, v1.x);
    //CCW is positive
}

vec3 Line::Intersection(Line &l1, Line &l2, float Epsilon)
{
    vec3 point1, point2;

    Line::ClosestPointsOnTwoLines(l1, l2, point1, point2);
    if(Epsilon > Line::DistanceTwoPoints(point1, point2))
    {
        l2.intersections += 1;
        l2.found = true;

        return Line::AveragePoint(point1, point2);
    }
    else
    {
        return vec3(0.0f,0.0f,0.0f);
    }
}

//operator

std::ostream& operator << (std::ostream &stream,const vec3 &position)
{
    stream << position.x << " " << position.y << " " << position.z << " ";

    return stream;
}

std::ostream& operator << (std::ostream &stream,const vec2 &position)
{
    stream << position.x << " " << position.y;

    return stream;
}

std::ostream& operator <<(std::ostream &stream, const Line &line)
{
    stream << "Line position: " << line.position << "direction: " << line.directionVector;

    return stream;
}

