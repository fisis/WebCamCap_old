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

#include <QVariant>
#include <QVariantMap>
#include <QApplication>
#include <QtConcurrent/QtConcurrent>

using glm::vec2;
using glm::vec3;
using namespace cv;

Room::Room(OpenGLWindow *opengl, vec3 dimensions, float eps, QString name)
{
    if(dimensions == vec3(0.0f, 0.0f, 0.0f) && eps == 0.5 &&  name == "Default Project")
    {
        m_saved = true;
    }
    else
    {
        m_saved = false;
    }

    this->m_openGLWindow = opengl;
    this->m_name = name;
    m_roomDimensions = dimensions;

    std::cout << m_roomDimensions << std::endl;

    m_usePipe = m_captureAnimation = m_record = false;

    m_maxError  = eps;

    m_activeCamerasCount = 0;
    m_lastActiveCamIndex = 0;

    server = new QLocalServer();
    server->setMaxPendingConnections(1);
}

Room::~Room()
{
    RecordingStop();

    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        delete(workers[i]);
        if(!workerthreads[i]->wait(3000))
        {
            workerthreads[i]->quit();
        }
        delete(workerthreads[i]);

        m_cameras[i]->Hide();
        m_cameras[i]->TurnOff();
        delete(m_cameras[i]);
    }

    for(size_t i = 0; i < animations.size(); i++)
    {
        delete(animations[i]);
    }

    if(m_usePipe)
    {
        setPipe(false);
    }
}

void Room::AddCamera(CaptureCamera *cam)
{
    m_cameras.push_back(cam);

    if(cam->getTurnedOn())
    {
        m_activeCamerasCount++;
        m_lastActiveCamIndex = m_cameras.size()-1;
    }

    m_saved = false;

    haveResults.push_back(false);
    results.push_back(QVector<Line>());

    workers.push_back(new worker(&allLines, cam));
    workerthreads.push_back(new QThread);

    workers[workers.size()-1]->moveToThread(workerthreads[workerthreads.size()-1]);


    connect( this, SIGNAL(startWork2D()), this, SLOT(record2D()));
    connect( this, SIGNAL(startWork()), workers[workers.size()-1], SLOT(StartWork()));
    connect( this, SIGNAL(stopWork()), workers[workers.size()-1], SLOT(StopWork()));
    connect(workers[workers.size()-1],SIGNAL(ResultReady(QVector<Line>)),this,SLOT(ResultReady(QVector<Line>)), Qt::QueuedConnection);

    connect(workers[workers.size()-1], SIGNAL(finished()), workerthreads[workers.size()-1], SLOT(quit()));
    connect(workers[workers.size()-1], SIGNAL(finished()), workers[workers.size()-1], SLOT(deleteLater()));
    connect(workerthreads[workers.size()-1], SIGNAL(finished()), workerthreads[workers.size()-1], SLOT(deleteLater()));

    workerthreads[workerthreads.size()-1]->start();

    MakeTopology();
}

void Room::setDimensions(vec3 dims)
{
    m_roomDimensions = dims;

    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i]->setDimensions(dims);
    }

    m_saved = false;
}

void Room::setEpsilon(float size)
{
    m_maxError = size;
    m_saved = false;
}

void Room::setNumberOfPoints(size_t nOfPts)
{
    checker.setNumOfPoints(nOfPts);
}

void Room::RemoveCamera(size_t index)
{
    if(m_cameras[index]->getTurnedOn())
    {
        m_activeCamerasCount--;
        if(m_activeCamerasCount == 1)
        {
            for(size_t i = 0; i < m_cameras.size(); i++)
            {
                if(m_cameras[i]->getTurnedOn())
                {
                    m_lastActiveCamIndex = i;
                }
            }
        }
    }

    HideCameraVideo(index);
    TurnOffCamera(index);
    m_cameras.erase(m_cameras.begin()+index);
    m_saved = false;

    MakeTopology();
}

void Room::MakeTopology()
{
    m_cameraTopology.clear();

    vec3 pos1, pos2;
    vec3 dir1, dir2;
    cv::Mat intr1, intr2;

    float min = 181.0f, temp_angle;
    int min_index = -1;

    //get
    for(size_t i = 0; i < m_cameras.size(); i++)
    {

        pos1 = m_cameras[i]->getPosition();
        dir1 = m_cameras[i]->getDirVector();
        intr1 = m_cameras[i]->cameraMatrix();

        for(size_t j = i+1; j < m_cameras.size(); j++)
        {

            pos2 = m_cameras[j]->getPosition();
            dir2 = m_cameras[j]->getDirVector();
            intr2 = m_cameras[j]->cameraMatrix();

            temp_angle = Line::LineAngle(vec2(dir1.x, dir1.y),vec2(dir2.x, dir2.y));

            if(glm::abs(temp_angle) < min)
            {
                min = temp_angle;
                min_index = j;
            }
        }

        if(min_index != -1)
        {
            Edge edge(i,min_index, m_maxError);
            //edge.setFundamentalMatrix(pos1, dir1, intr1, pos2, dir2, intr2);

            m_cameraTopology.push_back(edge);

        }
    }

    resolveTopologyDuplicates();

    std::cout << "new camtopology size:" << m_cameraTopology.size() << std::endl;
}

void Room::resolveTopologyDuplicates()
{
    for(size_t i = 0; i < m_cameraTopology.size(); i++)
    {
        size_t index1 = m_cameraTopology[i].m_index1;
        size_t index2 = m_cameraTopology[i].m_index2;

        for(size_t j = 0; j < m_cameraTopology.size(); j++)
        {
            if(m_cameraTopology[j].m_index1 == index2 && m_cameraTopology[j].m_index2 == index1)
            {
                m_cameraTopology.erase(m_cameraTopology.begin()+j);
            }
        }
    }
}

void Room::TurnOnCamera(size_t index)
{
    m_cameras[index]->TurnOn();
    m_activeCamerasCount++;
    m_saved = false;
}

void Room::TurnOffCamera(size_t index)
{
    m_cameras[index]->TurnOff();
    m_activeCamerasCount--;
    if(m_activeCamerasCount == 1)
    {
        for(size_t i = 0; i < m_cameras.size(); i++)
        {
            if(m_cameras[i]->getTurnedOn())
            {
                m_lastActiveCamIndex = i;
            }
        }
    }
    m_saved = false;
}

void Room::CaptureAnimationStart()
{
    actualAnimation = new Animation(m_roomDimensions);

    m_captureAnimation = true;
}

void Room::setPipe(bool pipe)
{
    if(pipe)
    {
        if(!server->listen(QString("webcamcap6")))
        {
            std::cout << "Server down somehow!" << std::endl;
            m_usePipe = false;
            return;
        }
        connect(server, SIGNAL(newConnection()), this, SLOT(handleConnection()));

        m_usePipe = true;
    }
    else
    {
        server->close();
        std::cout << "Server closed" << std::endl;
        m_usePipe = false;
    }
}

Animation *Room::CaptureAnimationStop()
{
    animations.push_back(actualAnimation);

    Animation * ret = actualAnimation;

    m_captureAnimation = false;

    return ret;
}

void Room::RecordingStart()
{
    m_record = true;

    if(m_activeCamerasCount > 1)
    {
        timer.start();

        emit startWork();
    }
    else if(m_activeCamerasCount == 1)
    {
        timer.start();
        m_openGLWindow->setTwoDimensions(true);
        emit startWork2D();
    }
}

void Room::RecordingStop()
{
    m_record = false;
    m_openGLWindow->setTwoDimensions(false);
    emit stopWork();
}
/*
void Room::Save(std::ofstream &file)
{
    //save dimension and epsilon of room
    file << m_roomDimensions.x << " " << m_roomDimensions.y << " " << m_maxError << " "
         << checker.getNumOfPoints() << std::endl;
    file << m_cameras.size() << std::endl;

    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        m_cameras[i]->Save(file);
    }

    //set project as saved
    m_saved = true;
}
*/

void Room::ResultReady(QVector<Line> lines)
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
            for(size_t j = 0; j < m_cameraTopology.size(); j++)
            {
                if(m_cameraTopology[j].m_index1 == i)
                {
                    m_cameraTopology[j].a = lines;
                }

                if(m_cameraTopology[j].m_index2 == i)
                {
                    m_cameraTopology[j].b = lines;
                }

                results[i] = lines;
            }

            break;
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

    if(m_usePipe)
    {
        sendMessage(points);
    }
/*
    for(size_t i = 0; i < points.size(); i++)
    {
        std::cout << points[i] << std::endl;
    }
*/
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
             tempPoint = Line::Intersection(camsEdge.a[i], camsEdge.b[j], camsEdge.m_maxError);

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
    QtConcurrent::map(m_cameraTopology, Room::Intersection);

    for(size_t i = 0; i < m_cameraTopology.size(); i++)
    {
        points.insert(points.end(), m_cameraTopology[i].points.begin(), m_cameraTopology[i].points.end());
        m_cameraTopology[i].points.clear();
    }

    //weld points
    if(m_activeCamerasCount >= 3)
    {
        weldPoints(points);
    }

    labeledPoints = checker.solvePointIDs(points);
    m_openGLWindow->setFrame(labeledPoints, results);

    QCoreApplication::processEvents();

    if(m_captureAnimation)
    {
        actualAnimation->AddFrame(Frame(10,labeledPoints, results));
    }

    for(size_t i = 0; i < workers.size(); i++)
    {
        haveResults[i] = false;
    }
}

void Room::weldPoints(std::vector<glm::vec3> &points)
{

}

void Room::record2D()
{
    while(m_record)
    {
        points2D = m_cameras[m_lastActiveCamIndex]->RecordNextFrame2D();


        labeledPoints = checker.solvePointIDs(points2D);
        m_openGLWindow->setFrame(labeledPoints);

        QCoreApplication::processEvents();

        if(m_captureAnimation)
        {
            actualAnimation->AddFrame({10,labeledPoints});
        }

        if(m_usePipe)
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

const QString projectNameKey("projectName");
const QString dimXKey("dimX");
const QString dimYKey("dimY");
const QString dimZKey("dimZ");
const QString errorKey("maxError");
const QString camerasKey("cameras");


QVariantMap Room::toVariantMap()
{
    QVariantMap retVal;

    retVal[projectNameKey] = m_name;
    retVal[dimXKey] = m_roomDimensions.x;
    retVal[dimYKey] = m_roomDimensions.y;
    retVal[dimZKey] = m_roomDimensions.z;
    retVal[errorKey] = m_maxError;

    QVariantList list;

    for(size_t i = 0; i < m_cameras.size(); i++)
    {
        auto map = m_cameras[i]->toVariantMap();

        list.append(map);
    }

    retVal[camerasKey] = list;

    return retVal;
}


void Room::fromVariantMap(OpenGLWindow *opengl , QVariantMap &varMap)
{
    m_saved = false;
    m_openGLWindow = opengl;

    m_name = varMap[projectNameKey].toString();
    m_roomDimensions = vec3(varMap[dimXKey].toFloat(), varMap[dimYKey].toFloat(), varMap[dimZKey].toFloat());

    std::cout << m_roomDimensions << std::endl;

    m_maxError  = varMap[errorKey].toDouble();
    m_activeCamerasCount = 0;
    m_lastActiveCamIndex = 0;

    server = new QLocalServer();
    server->setMaxPendingConnections(1);

    QVariantList list = varMap[camerasKey].toList();

    for(QVariant &camera: list)
    {
        CaptureCamera* cam = new CaptureCamera();

        cam->fromVariantMap(camera.toMap());

        AddCamera(cam);
    }
}

/*
void Edge::setFundamentalMatrix(glm::vec3 camPos1, glm::vec3 camRot1, cv::Mat intrinsicMatrix1, glm::vec3 camPos2, glm::vec3 camRot2, cv::Mat intrinsicMatrix2)
{
    camRot1 = glm::normalize(camRot1);
    camRot2 = glm::normalize(camRot2);

    vec3 v = glm::cross(camRot1, camRot2);

    float s = glm::length(v);

    float c = glm::dot(camRot1, camRot2);

    cv::Mat diag = cv::Mat::eye(3,3, CV_32F);

    cv::Mat vx = cv::Mat::zeros(3,3, CV_32F);

    vx.at<float>(0,1) = - v.z;
    vx.at<float>(1,0) = v.z;

    vx.at<float>(0,2) = v.y;
    vx.at<float>(2,0) = -v.y;

    vx.at<float>(1,2) = -v.x;
    vx.at<float>(2,1) = v.x;

    std::cout << "vx" << std::endl << vx << std::endl;

    cv::Mat rotMatrix =  diag + vx + vx*vx*((1-c)/s*s);

    std::cout << "rotation matrix" << std::endl << rotMatrix << std::endl;

    vec3 t = camPos2 - camPos1;

    cv::Mat tx = cv::Mat::zeros(3,3, CV_32F);

    tx.at<float>(0,1) = -t.z;
    tx.at<float>(1,0) = t.z;

    tx.at<float>(0,2) = t.y;
    tx.at<float>(2,0) = -t.y;

    tx.at<float>(1,2) = -t.x;
    tx.at<float>(2,1) = t.x;

    cv::Mat essentialMatrix = rotMatrix * tx;
    std::cout << "essential matrix" << std::endl << essentialMatrix << std::endl;

    this->m_fundamentalMatrix = intrinsicMatrix2.t().inv(DECOMP_SVD) * essentialMatrix * intrinsicMatrix1.inv(DECOMP_SVD);

    cv::Mat point = cv::Mat::ones(3,1, CV_32F);
    point.at<float>(0,0) = 331;
    point.at<float>(1,0) = 301;

    std::cout << "point mul" << point.t() * m_fundamentalMatrix * point << std::endl;


    std::cout << "fundamentalMatrix" << std::endl << m_fundamentalMatrix << std::endl;
}
*/
