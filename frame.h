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

#ifndef FRAME_H
#define FRAME_H

#include <QTime>

#include "line.h"

using namespace glm;

class Frame
{
    int elapsedTime;

    std::vector<vec3> points;
    std::vector<std::vector<Line> > lines;
    std::vector<Line> bones;

public:
    Frame(std::vector<vec3> pts, std::vector<std::vector<Line> > lines, int elapsed);

    std::vector<vec3> getPoints() const {return points;}
    std::vector<std::vector<Line> > getLines() const {return lines;}
    int getElapsedTime() const {return elapsedTime;}

    void setPoints(std::vector<vec3> pts) {points = pts;}
};

#endif // FRAME_H
