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

#include "modelstructure.h"

using namespace glm;

ModelStructure::ModelStructure()
{
    epsilon = 5;
}

ModelStructure::ModelStructure(QTreeWidget *item)
{
    for(size_t i = 0; i < item->topLevelItemCount(); i++)
    {
        MarkerPoint temp(item->topLevelItem(i), nullptr);

        AddPoint(temp);
        roots.push_back( &pointsStructure[i] );

        addChildren(item->topLevelItem(i), &pointsStructure[i]);
    }

    for(size_t i = 0; i < pointsStructure.size(); i++)
    {
        pointsStructure[i].setID(i);
    }
}

void ModelStructure::countDistances(vector <vec3> Points)
{
    distances.clear();
    vector<float>tempDistances;
    //init
    for(size_t i = 0; i < Points.size(); i++)
    {
        tempDistances.clear();
        for(size_t j = 0; j < Points.size(); j++)
        {
            tempDistances.push_back(0.0);
        }
        distances.push_back(tempDistances);
    }

    //count all distances from veryfying points to other points
    for(size_t i = 0; i < Points.size(); i++)
    {
        for(size_t j = i+1; j < Points.size(); j++)
        {
            distances[i][j] = distances[j][i] = MarkerPoint::PointDistance(Points[i], Points[j]);
        }
    }
}

vector<size_t> ModelStructure::getIDs(vector <vec3> Points)
{
    if(pointsStructure.size() != Points.size())
    {
        //cerr<< "zly pocet bodov" << endl;
        vector<size_t> t;
        return t;
    }
    vector<size_t> IDs;

    countDistances(Points);

    vector<bool> usedMarkers;
    //init
    for(size_t j = 0; j < pointsStructure.size(); j++)
    {
        usedMarkers.push_back(false);
    }

    //for every point get ID (verify distances of neighbours)
    for(size_t i = 0; i < Points.size(); i++)
    {
        for(size_t j = 0; j < pointsStructure.size(); j++)
        {
            if(!usedMarkers[j])
                if(verifyDistance(i,pointsStructure[j]))
                {
                    usedMarkers[j] = true;
                    IDs.push_back(j);
                }
        }
    }

    return IDs;
}

void ModelStructure::Save(ExportFormat format, ofstream &outputFile)
{

}

bool ModelStructure::verifyDistance(size_t PointIndex ,MarkerPoint marker)
{
    size_t counter = 0;
    size_t minEps;
    int minEpsIndex;
    vector<bool> used;
    for(size_t j = 0; j < distances[PointIndex].size(); j++)
    {
        used.push_back(false);
    }
/*
    //verify if there are points in same distance like in the structure
    for(size_t i = 0; i < marker.getDistances().size(); i++)
    {
        minEps = 10;//pozor pozor
        minEpsIndex = -1;
        for(size_t j = 0; j < marker.getDistances()[PointIndex].size(); j++)
        {
            if(!used[j])
                if(glm::abs(marker.getDistances()[i] - marker.getDistances()[PointIndex][j]) < minEps)
                {
                    minEps = glm::abs(marker.getDistances()[i] - marker.getDistances()[PointIndex][j]);
                    minEpsIndex = j;
                }
        }
        if(minEps < Epsilon && minEpsIndex != -1)
        {
            used[minEpsIndex] = true;
            counter++;
        }
    }

    if(counter == marker.getDistances().size())
        return true;
    else
        return false;
        */
}

void ModelStructure::addChildren(QTreeWidgetItem *parentItem, MarkerPoint *parent)
{
    for(int i = 0; i < parentItem->childCount(); i++ )
    {
        MarkerPoint temp(parentItem->child(i), parent);
        AddPoint(temp);
        parent->AddChild(&pointsStructure[pointsStructure.size() - 1]);

        addChildren(parentItem->child(i), &pointsStructure[pointsStructure.size() - 1]);
    }
}
