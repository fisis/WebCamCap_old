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

#ifndef Room_H
#define Room_H

#include <QTime>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

#include "animation.h"
#include "pointchecker.h"
#include "capturethread.h"

typedef struct Edge
{
    Edge(int a, int b, float Eps): index_a(a), index_b(b), epsilon(Eps) {}

    int index_a, index_b;
    std::vector<Line> a;
    std::vector<Line> b;
    float epsilon;
    std::vector <glm::vec3> points;
} Edge;

class Room : public QObject
{
    Q_OBJECT

    //basic parameters for project
    std::string name;
    glm::vec3 roomDimensions; //centimeters
    double epsilon;
    bool saved;
    OpenGLWindow *opengl;

    std::vector <Edge> camTopology;
    std::vector <CaptureCamera*> cameras;

    size_t activeCams;
    size_t activeCamIndex;

    bool record, captureAnimation;
    Animation* actualAnimation;
    std::vector<Animation*> animations;

    //pipe
    bool Pipe;
    QLocalServer *server;
    QLocalSocket *socket;

    //parallel
    QWaitCondition allLines;

    //cams
    std::vector <bool> haveResults;
    std::vector <std::vector<Line> > results;

    std::vector <worker*> workers;
    std::vector <QThread*> workerthreads;

    //intersections
    QTime timer;

    std::vector<glm::vec3> points;
    std::vector<glm::vec2> points2D; //2Drecording

    PointChecker checker;
    std::vector<Point> labeledPoints;


public:
    Room(OpenGLWindow *opengl = nullptr, glm::vec3 dimensions = glm::vec3(0.0f,0.0f, 0.0f), float eps = 0.5, std::string name = "Default Project");
    Room(std::string file);
    ~Room();

    void AddCamera(glm::vec3 pos, std::string name, int ID,int angle);
    void AddCamera(CaptureCamera *cam);
    void RemoveCamera(size_t index);
    void MakeTopology();
    void Save(std::ofstream &file);

    void TurnOnCamera(size_t index);
    void TurnOffCamera(size_t index);
    void ShowCameraVideo(size_t index){cameras[index]->Show();}
    void HideCameraVideo(size_t index){cameras[index]->Hide();}

    void CaptureAnimationStart();
    void setPipe(bool pipe);
    Animation *CaptureAnimationStop();
    void RecordingStart();
    void RecordingStop();

    void setDimensions(glm::vec3 dims);
    void setName(std::string name){this->name = name;}
    void setEpsilon(float size);
    void setNumberOfPoints(size_t nOfPts);

    std::string getName() const {return name;}
    glm::vec3 getDimensions() const {return roomDimensions;}
    int getWidth()const {return roomDimensions.x;}
    int getLength()const {return roomDimensions.y;}
    float getEpsilon() const {return epsilon;}
    bool getSaved() const {return saved;}
    std::vector<std::vector<Line> > getLines() const {return results;}
    std::vector <CaptureCamera*> getcameras()const {return cameras;}

    void setOpenglWindow(OpenGLWindow * opengl) {this->opengl = opengl;}
    void setThreshold(int value);
    void setBrighnessOfCamera(int value);

signals:
    void startWork();
    void stopWork();
    void startWork2D();

private slots:
    void ResultReady(std::vector<Line> lines);
    void record2D();
    void handleConnection();

private:
    void sendMessage(std::vector<glm::vec3> points);
    void sendMessage(std::vector<glm::vec2> points);
    void sendMessage(std::string str);
    static void Intersection(Edge &camsEdge);
    void Intersections();

};

#endif // Room_H
