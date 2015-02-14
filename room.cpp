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

#include "room.h"
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

using glm::vec2;
using glm::vec3;
using namespace cv;

Room::Room(OpenGLWindow *opengl, vec3 dimensions, float eps, std::string name)
{
    if(dimensions == vec3(0.0f, 0.0f, 0.0f) && eps == 0.5 &&  name == "Default Project")
    {
        saved = true;
    }
    else
    {
        saved = false;
    }

    this->opengl = opengl;
    this->name = name;
    roomDimensions = dimensions;

    std::cout << roomDimensions << std::endl;

    Pipe = captureAnimation = record = false;

    epsilon  = eps;

    activeCams = 0;
    activeCamIndex = 0;

    server = new QLocalServer();
    server->setMaxPendingConnections(1);
}

Room::Room(std::string file)
{
    std::ifstream inputFile;
    inputFile.open(file);

    std::getline(inputFile, name);

    std::string line;
    std::stringstream linestream;

    std::getline(inputFile, line);
    linestream << line;

    size_t numOfPts;

    linestream >> roomDimensions.x;
    linestream >> roomDimensions.y;
    linestream >> roomDimensions.z;
    linestream >> epsilon;
    linestream >> numOfPts;
    linestream.flush();

    checker.setNumOfPoints(numOfPts);

    size_t numOfCams;
    std::stringstream linestream2;
    std::getline(inputFile, line);
    linestream2 << line;

    linestream2 >> numOfCams;

    for(size_t i = 0; i < numOfCams; i++)
    {
        std::stringstream linestream3;
        std::getline(inputFile,line);
        linestream3 << line;

        std::string tname;
        linestream3 >>  tname;

        vec3 tposition;
        linestream3 >> tposition.x;
        linestream3 >> tposition.y;
        linestream3 >> tposition.z;

        int tID;
        linestream3 >> tID;

        float tAngle;
        linestream3 >> tAngle;

        vec2 resolution;
        linestream3 >> resolution.x;
        linestream3 >> resolution.y;

        size_t thresholdValue;
        linestream3 >> thresholdValue;

        CaptureCamera* temp = new CaptureCamera(tposition, roomDimensions, tname, tID, tAngle);

        temp->resolution = resolution;
        temp->setThreshold(thresholdValue);

        AddCamera(temp);
        TurnOnCamera(cameras.size()-1);
        ShowCameraVideo(cameras.size()-1);
        cameras[i]->CalibNoMarkers();

    }

    inputFile.close();

    activeCams = 0;
    activeCamIndex = 0;

    saved = true;
}

Room::~Room()
{
    RecordingStop();

    for(size_t i = 0; i < cameras.size(); i++)
    {
        delete(workers[i]);
        if(!workerthreads[i]->wait(3000))
        {
            workerthreads[i]->quit();
        }
        delete(workerthreads[i]);

        cameras[i]->Hide();
        cameras[i]->TurnOff();
        delete(cameras[i]);
    }

    for(size_t i = 0; i < animations.size(); i++)
    {
        delete(animations[i]);
    }

    if(Pipe)
    {
        setPipe(false);
    }
}

void Room::AddCamera(CaptureCamera *cam)
{
    cameras.push_back(cam);

    if(cam->getTurnedOn())
    {
        activeCams++;
        activeCamIndex = cameras.size()-1;
    }

    saved = false;

    haveResults.push_back(false);
    results.push_back(std::vector<Line>());

    workers.push_back(new worker(&allLines, cam));
    workerthreads.push_back(new QThread);

    workers[workers.size()-1]->moveToThread(workerthreads[workerthreads.size()-1]);


    connect( this, SIGNAL(startWork2D()), this, SLOT(record2D()));
    connect( this, SIGNAL(startWork()), workers[workers.size()-1], SLOT(StartWork()));
    connect( this, SIGNAL(stopWork()), workers[workers.size()-1], SLOT(StopWork()));
    connect(workers[workers.size()-1],SIGNAL(ResultReady(std::vector<Line>)),this,SLOT(ResultReady(std::vector<Line>)), Qt::QueuedConnection);

    connect(workers[workers.size()-1], SIGNAL(finished()), workerthreads[workers.size()-1], SLOT(quit()));
    connect(workers[workers.size()-1], SIGNAL(finished()), workers[workers.size()-1], SLOT(deleteLater()));
    connect(workerthreads[workers.size()-1], SIGNAL(finished()), workerthreads[workers.size()-1], SLOT(deleteLater()));

    workerthreads[workerthreads.size()-1]->start();

    MakeTopology();
}

void Room::AddCamera(vec3 pos, std::string name, int ID, int angle)
{
    CaptureCamera* temp = new CaptureCamera(pos, roomDimensions, name, ID, angle);

    AddCamera(temp);
}

void Room::setDimensions(vec3 dims)
{
    roomDimensions = dims;

    for(size_t i = 0; i < cameras.size(); i++)
    {
        cameras[i]->setDimensions(dims);
    }

    saved = false;
}

void Room::setEpsilon(float size)
{
    epsilon = size;
    saved = false;
}

void Room::setNumberOfPoints(size_t nOfPts)
{
    checker.setNumOfPoints(nOfPts);
}

void Room::RemoveCamera(size_t index)
{
    if(cameras[index]->getTurnedOn())
    {
        activeCams--;
        if(activeCams == 1)
        {
            for(size_t i = 0; i < cameras.size(); i++)
            {
                if(cameras[i]->getTurnedOn())
                {
                    activeCamIndex = i;
                }
            }
        }
    }

    HideCameraVideo(index);
    TurnOffCamera(index);
    cameras.erase(cameras.begin()+index);
    saved = false;

    MakeTopology();
}

void Room::MakeTopology()
{
    camTopology.clear();

    vec3 pos1, pos2;
    vec3 dir1, dir2;

    float min = 181.0f, temp_angle;
    int min_index = -1;

    //get
    for(size_t i = 0; i < cameras.size(); i++)
    {

        pos1 = cameras[i]->getPosition();
        dir1 = cameras[i]->getDirVector();

        for(size_t j = i+1; j < cameras.size(); j++)
        {

            pos2 = cameras[j]->getPosition();
            dir2 = cameras[j]->getDirVector();

            temp_angle = Line::LineAngle(vec2(dir1.x, dir1.y),vec2(dir2.x, dir2.y));

            if(glm::abs(temp_angle) < min)
            {
                min = temp_angle;
                min_index = j;
            }
        }

        if(min_index != -1)
        {
            camTopology.push_back(Edge(i,min_index, epsilon));

        }
    }

    resolveTopologyDuplicates();

    std::cout << "new camtopology size:" << camTopology.size() << std::endl;
}

void Room::resolveTopologyDuplicates()
{
    for(size_t i = 0; i < camTopology.size(); i++)
    {
        int index_a = camTopology[i].index_a;
        int index_b = camTopology[i].index_b;

        for(size_t j = 0; j < camTopology.size(); j++)
        {
            if(camTopology[j].index_a == index_b && camTopology[j].index_b == index_a)
            {
                camTopology.erase(camTopology.begin()+j);
            }
        }
    }
}

void Room::TurnOnCamera(size_t index)
{
    cameras[index]->TurnOn();
    activeCams++;
    saved = false;
}

void Room::TurnOffCamera(size_t index)
{
    cameras[index]->TurnOff();
    activeCams--;
    if(activeCams == 1)
    {
        for(size_t i = 0; i < cameras.size(); i++)
        {
            if(cameras[i]->getTurnedOn())
            {
                activeCamIndex = i;
            }
        }
    }
    saved = false;
}

void Room::CaptureAnimationStart()
{
    actualAnimation = new Animation(epsilon, checker.getNumOfPoints());

    captureAnimation = true;
}

void Room::setPipe(bool pipe)
{
    if(pipe)
    {
        if(!server->listen(QString("webcamcap6")))
        {
            std::cout << "Server down somehow!" << std::endl;
            Pipe = false;
            return;
        }
        connect(server, SIGNAL(newConnection()), this, SLOT(handleConnection()));

        Pipe = true;
    }
    else
    {
        server->close();
        std::cout << "Server closed" << std::endl;
        Pipe = false;
    }
}

Animation *Room::CaptureAnimationStop()
{
    animations.push_back(actualAnimation);

    Animation * ret = actualAnimation;

    return ret;
}

void Room::RecordingStart()
{
    record = true;

    if(activeCams > 1)
    {
        timer.start();

        emit startWork();
    }
    else if(activeCams == 1)
    {
        timer.start();
        opengl->setTwoDimensions(true);
        emit startWork2D();
    }
}

void Room::RecordingStop()
{
    record = false;
    opengl->setTwoDimensions(false);
    emit stopWork();
}

void Room::Save(std::ofstream &file)
{
    //save dimension and epsilon of room
    file << roomDimensions.x << " " << roomDimensions.y << " " << epsilon << " "
         << checker.getNumOfPoints() << std::endl;
    file << cameras.size() << std::endl;

    for(size_t i = 0; i < cameras.size(); i++)
    {
        cameras[i]->Save(file);
    }

    //set project as saved
    saved = true;
}

void Room::setThreshold(int value)
{
    for(size_t i = 0; i < cameras.size(); i++)
    {
        cameras[i]->setThreshold(value);
    }

    saved = false;
}

void Room::setBrighnessOfCamera(int value)
{
    for(size_t i = 0; i < cameras.size(); i++)
    {
        cameras[i]->setBrightness(value);
    }

    saved = false;
}

void Room::ResultReady(std::vector<Line> lines)
{
    waitKey(1);

    QObject *obj = QObject::sender();

    for(size_t i = 0; i < workers.size(); i++)
    {
        if(obj == workers[i])
        {
            if(haveResults[i])
            {
                std::cout << "bad sync" << std::endl;
            }

            haveResults[i] = true;
            for(size_t j = 0; j < camTopology.size(); j++)
            {
                if(camTopology[j].index_a == i)
                {
                    camTopology[j].a = lines;
                }

                if(camTopology[j].index_b == i)
                {
                    camTopology[j].b = lines;
                }

                results[i] = lines;
            }
        }
    }

    for(size_t i = 0; i < workers.size(); i++)
    {
        if(!haveResults[i])
        {
            return;
        }
    }

    points.clear();

    Intersections();

    if(Pipe)
    {
        sendMessage(points);
    }

    //std::cout << timer.elapsed() << std::endl;

    timer.restart();

    allLines.wakeAll();
    QCoreApplication::processEvents();
}

void Room::sendMessage(std::vector<vec3> Points)
{
    std::stringstream ss;

    ss << " 3D " << Points.size();

    for(size_t i = 0; i < Points.size(); i++)
    {
        ss << " P " << Points[i];
    }

    ss << std::endl;

    std::string msg = ss.str();

    sendMessage(msg);
}

void Room::sendMessage(std::vector<vec2> Points)
{
    std::stringstream ss;

    ss <<  " 2D " << Points.size();

    for(size_t i = 0; i < Points.size(); i++)
    {
        ss << " P " << Points[i];
    }

    ss << std::endl;


    std::string msg = ss.str();

    //std::cout << msg << std::endl;

    sendMessage(msg);
}

void Room::sendMessage(string str)
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out << (quint16)0;
    out.setVersion(QDataStream::Qt_4_0);
    out << QString::fromStdString(str);
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    socket->write(block);
}

void Room::Intersection(Edge &camsEdge)
{
    vec3 tempPoint;

     for(size_t i = 0; i < camsEdge.a.size(); i++)
     {
         for(size_t j = 0; j < camsEdge.b.size(); j++)
         {
             tempPoint = Line::Intersection(camsEdge.a[i], camsEdge.b[j], camsEdge.epsilon);

             if(tempPoint != vec3(0,0,0))
             {
                camsEdge.points.push_back(tempPoint);
             }
         }
     }

     camsEdge.a.clear();
     camsEdge.b.clear();
}

void Room::Intersections()
{
    QtConcurrent::map(camTopology, Room::Intersection);

    for(size_t i = 0; i < camTopology.size(); i++)
    {
        points.insert(points.end(), camTopology[i].points.begin(), camTopology[i].points.end());
        camTopology[i].points.clear();
    }

    //weld points

    labeledPoints = checker.solvePointIDs(points);
    opengl->setFrame(labeledPoints, results);

    QCoreApplication::processEvents();

    if(captureAnimation)
    {
        actualAnimation->AddFrame(Frame(10,labeledPoints, results));
    }

    for(size_t i = 0; i < workers.size(); i++)
    {
        haveResults[i] = false;
    }
}

void Room::record2D()
{
    while(record)
    {
        points2D = cameras[activeCamIndex]->RecordNextFrame2D();


        labeledPoints = checker.solvePointIDs(points2D);
        opengl->setFrame(labeledPoints);

        QCoreApplication::processEvents();

        if(captureAnimation)
        {
            actualAnimation->AddFrame({10,labeledPoints});
        }

        if(Pipe)
        {
            sendMessage(points2D);
        }
    }
}

void Room::handleConnection()
{
    std::cout << "connect" << std::endl;

    socket = server->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}
