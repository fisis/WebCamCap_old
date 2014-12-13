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

#ifndef CAPTURECAMERA_H
#define CAPTURECAMERA_H

#include <fstream>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

#include "line.h"
#include "Gui/camwidget.h"

using namespace glm;
using namespace cv;

class CaptureCamera: public QObject
{
    Q_OBJECT

    typedef std::vector<Point> Contour;

    //BASIC PARAMETERS
    int videoUsbId;
    std::string name;

    //width, length
    vec3 roomDimensions;
    vec3 position;
    vec3 directionVectorToMiddle;

    bool turnedOn;
    bool showWindow;

    float angleOfView;
    double anglePerPixel;

    //ADVANCED for camera
    VideoCapture camera;
    bool ROI;
    Mat ROIMask;
    Mat frameBackground ,frame, frameTemp, MOGMask;

    //background substract
    bool useBackgroundSub;
    BackgroundSubtractorMOG* backgroundExtractor;

    //ADVANCED for image process
    size_t thresholdValue;
    Mat dilateKernel;

    Scalar contourColor;
    std::vector <Contour> contours;

    Moments centerMoment;
    vec2 centerTemp;
    vec2 centerRelativeTemp;
    std::vector<vec2> centerOfContour;

    vec3 directionTemp;
    std::vector<Line> lines;

    CamWidget * QtWidgetViewer;
public:
    //public parameters
    vec2 resolution;

    //public functions
    CaptureCamera(vec3 pos, vec3 roomDimensions, std::string name, int ID, float angle, bool backgroudSubstractor = false);

    ~CaptureCamera();

    std::vector<Line> RecordNextFrame();
    std::vector<glm::vec2> RecordNextFrame2D();
    void TurnOn();
    void TurnOff();
    void Show();
    void Hide();
    void Save(std::ofstream &outputFile);
    void CalibNoMarkers();
    int CalibWithMarkers(int numOfMarkers);
    void setROI(cv::Mat roi){ROIMask = roi; ROI = true;}

    void setDimensions(vec3 roomDim){roomDimensions = roomDim; ComputeDirVector();}
    void setWidth(int  width){roomDimensions.x = width; ComputeDirVector();}
    void setLength(int length){roomDimensions.y = length; ComputeDirVector();}
    void setThreshold(size_t Threshold){thresholdValue = Threshold;}
    void setAngle(float Angle){angleOfView = Angle; anglePerPixel = 0;}

    void setContrast(int value);
    void setBrightness(int value);
    void setSaturation(int value);
    void setSharpness(int value);

    std::string getName() const {return name;}
    vec3 getPosition() const {return position;}
    vec3 getDirVector() const {return directionVectorToMiddle;}
    int getID() const {return videoUsbId;}
    float getAngle() const {return angleOfView;}
    bool getTurnedOn() const {return turnedOn;}
    CamWidget *getWidget() const {return QtWidgetViewer;}


    static cv::Mat myColorThreshold(Mat input, Mat dilateKernel, int thresholdValue, int maxValue);

public slots:
    void activeCam(bool active);

    void turnedOnCam(bool turnedOn);

    void thresholdCam(size_t threshold);

private:

    vec2 GetUndisortedPosition(vec2 frameResolution, vec2 position);
    void UseFilter();
    void MiddleOfContours();
    void CreateLines();
    void ComputeDirVector();
    void NormalizeContours();

signals:
    void imageRead(cv::Mat image);

};

#endif // CAPTURECAMERA_H
