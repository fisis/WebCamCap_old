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

#include "animation.h"
#include <fstream>

Animation::Animation(float Epsilon, std::string name)
{
    roomEpsilon = Epsilon;
    this->name = name;
}

Animation::Animation(float Epsilon, size_t pointCount, std::string name)
{
    roomEpsilon = Epsilon;
    this->name = name;
    this->pointCount = pointCount;
}

void Animation::AddFrame(Frame k)
{
    frames.push_back(k);
}

void Animation::AddFrame(std::vector<Point> pts, std::vector<std::vector<Line> > lines, int elapsed)
{
    frames.push_back({elapsed, pts, lines});
}

void Animation::Save(std::string file)
{
    std::ofstream outputFile;
    outputFile.open(file, std::ios_base::out);

}

void Animation::PostProcess()
{
    //count framerate
    int sum = 0;

    for(size_t i = 1; i < frames.size(); i++)
    {
        sum += frames[i].getElapsedTime();
    }

    frameRate = 1000 / ( sum/ (float) frames.size() );

    //vodorovna zamena bodov



    //smooth positions, if some jumps from frame to frame



    //if not nuf pts try to find them, if not nuf intersections aproximate position
    std::vector<size_t> FramesWithMissingPoints;

    handleBadFrames(FramesWithMissingPoints);

    //smooth framerate (splines)



    //for every frame get structure
    findStructure();
}

void Animation::SaveBVH(std::ofstream &outputFile)
{

}

void Animation::handleBadFrames(std::vector<size_t> Frames)
{

}

void Animation::findStructure()
{

}
