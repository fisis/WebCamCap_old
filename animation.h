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

#ifndef ANIMATION_H
#define ANIMATION_H

#include "frame.h"

class Animation
{
    float roomEpsilon;
    float frameRate;

    size_t pointCount;

    std::string name;
    std::vector<Frame> frames;
public:
    Animation(float Epsilon, std::string name = "Animation_def");
    Animation(float Epsilon, size_t pointCount, std::string name = "Animation_def" );

    void AddFrame(Frame k);
    void AddFrame(std::vector<Point> pts, std::vector<std::vector<Line> > lines, int elapsed);
    void Save(std::string file);
    void PostProcess();

    //getters
    std::string getName() const {return name;}
    float getFrameRate() const {return frameRate;}
    int getLength() const {return  frames.size();}

    //setters
    void setName(std::string name) {this->name = name;}

private:
    void SaveBVH(std::ofstream &outputFile);

    //postprocessing
    void handleBadFrames(std::vector<size_t> frames);
    void findStructure();
};

#endif // ANIMATION_H
