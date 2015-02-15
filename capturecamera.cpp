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

#include "capturecamera.h"

//#define GLM_FORCE_RADIANS
#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>

#include <QMessageBox>
#include <QVBoxLayout>

using namespace cv;
using glm::vec2;
using glm::vec3;

#ifdef _MSC_VER
    using glm::tvec3;
#else
    using glm::detail::tvec3;
#endif


CaptureCamera::CaptureCamera(vec3 pos, vec3 roomDimensions, std::string name, int ID, float angle, bool backgroudSubstractor)
{
    ROI = turnedOn = showWindow = useBackgroundSub = false;
    videoUsbId = ID;
    this->name = name;

    position = pos;
    angleOfView = angle;
    anglePerPixel = 0;
    thresholdValue = 255;
    this->roomDimensions = roomDimensions;
    ComputeDirVector();

    std::cout << "Vector to middle: " << directionVectorToMiddle << std::endl;

    contourColor = Scalar(0, 0, 255);

    dilateKernel = getStructuringElement(MORPH_ELLIPSE, Size(3,3));

    backgroundExtractor = new BackgroundSubtractorMOG(50, 10, 0.3, 0.4);
    useBackgroundSub = backgroudSubstractor;

    QtWidgetViewer = new CamWidget;
    CQtOpenCVViewerGl *t = QtWidgetViewer->getImageViewer();
    connect(this, SIGNAL(imageRead(cv::Mat)), t, SLOT(showImage(cv::Mat)));
    connect(QtWidgetViewer, SIGNAL(activeCam(bool)), this, SLOT(activeCam(bool)));
    connect(QtWidgetViewer, SIGNAL(turnedOnCam(bool)), this, SLOT(turnedOnCam(bool)));
    connect(QtWidgetViewer, SIGNAL(thresholdCam(size_t)), this, SLOT(thresholdCam(size_t)));
}

CaptureCamera::~CaptureCamera()
{
    Hide();
    TurnOff();
    delete QtWidgetViewer;
    delete backgroundExtractor;
}

std::vector<Line> CaptureCamera::RecordNextFrame()
{
    lines.clear();

    if(!turnedOn)
    {
        std::vector<Line> blank;
        return blank;
    }

    camera >> frame;
    UseFilter();
    MiddleOfContours();
    CreateLines();

    circle(frame, cv::Point(frame.cols/2, frame.rows/2), 1, CV_RGB(0,255,0), 2);

    if(showWindow)
    {
        emit imageRead(frame);
    }

    return lines;
}

std::vector<vec2> CaptureCamera::RecordNextFrame2D()
{
    if(!turnedOn)
    {
        std::vector<vec2> blank;
        return blank;
    }

    camera >> frame;

    UseFilter();
    MiddleOfContours();

    circle(frame, cv::Point(frame.cols/2, frame.rows/2), 1, CV_RGB(0,255,0), 2);

    if(showWindow)
    {
        emit imageRead(frame);
    }

    NormalizeContours();

    return centerOfContour;
}

void CaptureCamera::UseFilter()
{
    GetUndisortedPosition();

    if(ROI)
    {
        frame.copyTo(ROIMask, ROIMask);
        ROIMask.copyTo(frame);
    }

    if(useBackgroundSub)
    {
        backgroundExtractor->operator ()(frame, MOGMask);
        frame.copyTo(frameTemp,MOGMask);
    }
    else
    {
        frame.copyTo(frameTemp);
        absdiff(frameTemp,frameBackground, frameTemp);

        frameTemp = myColorThreshold(frameTemp,dilateKernel, 20, 255);

        frame.copyTo(frameTemp, frameTemp);
    }

    cvtColor(frameTemp, frameTemp, COLOR_BGR2GRAY);
    medianBlur(frameTemp, frameTemp, 3);

    threshold(frameTemp,frameTemp, thresholdValue, 255, THRESH_BINARY);

    morphologyEx(frameTemp, frameTemp, MORPH_OPEN , dilateKernel);

    findContours(frameTemp, contours , RETR_EXTERNAL, CHAIN_APPROX_NONE);

    for(size_t i = 0; i < contours.size(); i++)
    {
        double contArea = contourArea(contours[i]);

        if(contArea > 500 || contArea <= 20)
        {
            contours.erase(contours.begin()+i);
        }
    }

    drawContours(frame, contours, -1, contourColor , CV_FILLED);
}

void CaptureCamera::GetUndisortedPosition()
{
    //std::cout << "old_position " << position.x << " " << position.y << std::endl;

    Mat framein;

    frame.copyTo(framein);

    float koefs[5];

    float k1 = koefs[0] = -6.3798734804597357e-02;
    float k2 = koefs[1] = -5.7040895008453737e-03;
    float p1 = koefs[2] = -3.1326748002862385e-02;
    float p2 = koefs[3] = 1.1024120014532667e-02;
    float k3 = koefs[4] = 3.2344786027968481e-01;

    float camMatrix[9];

    camMatrix[0] = 6.8234628143642976e+02;
    camMatrix[1] = 0;
    camMatrix[2] = 3.3462073472534672e+02;
    camMatrix[3] = 0;
    camMatrix[4] = 6.9022187945217445e+02;
    camMatrix[5] = 1.6499630532779648e+02;
    camMatrix[6] = 0;
    camMatrix[7] = 0;
    camMatrix[8] = 1;

    Mat intrinsic = Mat(3, 3, CV_32FC1, camMatrix);
    Mat distCoeffs = Mat(1, 5, CV_32FC1, koefs);

    cv::undistort(framein, frame, intrinsic, distCoeffs);

}

void CaptureCamera::MiddleOfContours()
{
    centerOfContour.clear();

    for(size_t i = 0; i < contours.size(); i++)
    {
        centerMoment = moments(contours[i]);
        centerTemp = vec2(centerMoment.m10/centerMoment.m00, centerMoment.m01/centerMoment.m00);

        if(!isnan(centerTemp.x) && !isnan(centerTemp.y))
        {
            centerOfContour.push_back(centerTemp);

            circle(frame, cv::Point(centerTemp.x, centerTemp.y), 1, CV_RGB(0,0,255), 2);
        }
    }
}

void CaptureCamera::CreateLines()
{
    lines.clear();

    if(anglePerPixel == 0)
    {
        anglePerPixel = ( (double)  angleOfView ) / glm::sqrt( (frame.cols * frame.cols + frame.rows * frame.rows));
    }

    for(size_t i = 0; i < centerOfContour.size(); i++)
    {
        //vypocitam stred contury vzhľadom ku stredu obrazovky
        centerRelativeTemp = vec2(centerOfContour[i].x - frame.cols/2,centerOfContour[i].y - frame.rows/2);
        //rotacie
        directionTemp = glm::rotateZ((tvec3<double, (glm::precision)0u>) directionVectorToMiddle, (-centerRelativeTemp.x * anglePerPixel));//*0.0174532925);

        directionTemp = glm::rotateX((tvec3<double, (glm::precision)0u>) directionTemp , (-centerRelativeTemp.y * anglePerPixel));//*0.0174532925);
        lines.push_back(Line(position , directionTemp));
    }
}

void CaptureCamera::ComputeDirVector()
{
    directionVectorToMiddle = vec3(roomDimensions.x/2 - position.x , roomDimensions.y/2 - position.y , roomDimensions.z/2 - position.z);
}

void CaptureCamera::NormalizeContours()
{
    for(size_t i = 0; i < centerOfContour.size(); i++)
    {
         centerOfContour[i] *= vec2(1.0/(float) frame.cols, 1.0f / (float) frame.rows);
    }
}

cv::Mat CaptureCamera::myColorThreshold(cv::Mat input, Mat dilateKernel , int thresholdValue, int maxValue)
{
/*
    std::vector<Mat> ChannelsFrameTemp;

    split(input, ChannelsFrameTemp);

    for(size_t i = 0; i < 3; i++)
    {
        threshold(ChannelsFrameTemp[i], ChannelsFrameTemp[i], thresholdValue, maxValue, THRESH_BINARY);
    }

    bitwise_and(ChannelsFrameTemp[0], ChannelsFrameTemp[1], input);
    bitwise_and(input, ChannelsFrameTemp[2], input);

    morphologyEx(input, input, MORPH_OPEN , dilateKernel);// , Point(-1,-1),  5);*/

    cvtColor(input, input, COLOR_BGR2GRAY);

    threshold(input, input, thresholdValue, maxValue, THRESH_BINARY);

    return input;
}

void CaptureCamera::activeCam(bool active)
{
    if(active)
    {
        Show();
    }
    else
    {
        Hide();
    }
}

void CaptureCamera::turnedOnCam(bool turnedOn)
{
    if(turnedOn)
    {
        TurnOn();
    }
    else
    {
        TurnOff();
    }
}

void CaptureCamera::thresholdCam(size_t threshold)
{
    setThreshold(threshold);
}

void CaptureCamera::TurnOn()
{
    if(turnedOn)
        return;

    if(camera.open(videoUsbId))
    {
        //QtWidgetViewer->setCheckTurnedOn(true);
        turnedOn = true;
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("");
        msgBox.warning(nullptr , "", "No project opened");
        msgBox.setFixedSize(200,100);

        //QtWidgetViewer->setCheckTurnedOn(false);
        turnedOn = false;
    }
/*
        if(resolution.x != 0 && resolution.y !=0)
        {
            Camera.set(CAP_PROP_FRAME_WIDTH, resolution.x);
            Camera.set(CAP_PROP_FRAME_HEIGHT, resolution.y);
        }

#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
    // Windows (x64 and x86)
#elif __linux__
    // linux
#elif __APPLE__
    // Mac OS, not sure if this is covered by __posix__ and/or __unix__ though...
#endif

*/
}

void CaptureCamera::TurnOff()
{
    if(turnedOn)
    {
        turnedOn = false;
        QtWidgetViewer->setCheckTurnedOn(false);
        camera.release();
    }

}

void CaptureCamera::Show()
{
    if(!showWindow)
    {
        QtWidgetViewer->setCheckActive(true);
        showWindow = true;

    }
}

void CaptureCamera::Hide()
{
    if(showWindow)
    {
        QtWidgetViewer->setCheckActive(false);
        showWindow = false;
    }
}

void CaptureCamera::Save(std::ofstream &outputFile)
{
    outputFile << name << " " << position.x << " " << position.y << " "
               << position.z << " " << videoUsbId << " " << angleOfView << " "
               << resolution.x << " " << resolution.y << " " << thresholdValue
               << std::endl;
}

void CaptureCamera::CalibNoMarkers()
{
    if(turnedOn)
    {
        int i = 0, maxIters = 10;
        Scalar meanValue, lastMeanValue;

        camera >> frameBackground;
        waitKey(33);

        lastMeanValue = mean(frameBackground);

        camera >> frameBackground;
        waitKey(33);

        meanValue = mean(frameBackground);

        while(i < maxIters && ( abs( lastMeanValue.val[0] - meanValue.val[0] ) > 1 || abs( lastMeanValue.val[1] - meanValue.val[1] ) > 1 || abs( lastMeanValue.val[2] - meanValue.val[2] ) > 1 ) )
        {
            camera >> frameBackground;
            lastMeanValue = meanValue;
            meanValue  = mean(frameBackground);
            ++i;
            waitKey(66);
        }

        std::cout << name << " calibrated in " << i << " iterations" << std::endl;

        Mat temp;

        for(size_t i = 0; i < 50; i++)
        {
            camera >> temp;
            backgroundExtractor->operator ()(frame, MOGMask);

            if(i < 15)
            {
                for(int i = 0; i < temp.rows; i++)
                {
                    for(int j = 0; j < temp.cols; j++)
                    {
                        if(temp.at<Vec3b>(i,j)[0] > frameBackground.at<Vec3b>(i,j)[0] || temp.at<Vec3b>(i,j)[1] > frameBackground.at<Vec3b>(i,j)[1] || temp.at<Vec3b>(i,j)[2] > frameBackground.at<Vec3b>(i,j)[2])
                        {
                            frameBackground.at<Vec3b>(i,j) = temp.at<Vec3b>(i,j);
                        }
                    }
                }
            }

            waitKey(20);
        }
    }
}

int CaptureCamera::CalibWithMarkers(int numOfMarkers)
{
    thresholdValue = 255;

    if(turnedOn)
    {

        size_t thresholdUp, thresholdLow;

        for(size_t i = 0; i < 15; i++)
        {
            camera >> frame;
            waitKey(10);
        }

        size_t nLines;

        //step 1, find first value which gives some Lines
        while(thresholdValue > 20)
        {
            UseFilter();
            MiddleOfContours();
            CreateLines();

            if(lines.size() == 0)
            {
                --thresholdValue;
                continue;
            }
            else
            {
                if(numOfMarkers == 0)
                {
                    break;
                }
                else
                {
                    if(lines.size() == numOfMarkers)
                    {
                        break;
                    }
                }
            }
        }

        //some difference in light intensity (rotation of LED)
        thresholdValue -= 10;

        UseFilter();
        MiddleOfContours();
        CreateLines();

        nLines = lines.size();

        thresholdUp = thresholdValue;
        thresholdLow = 0;
        std::cout << "calibrated upper value" << thresholdUp << std::endl;

        //step 2 , find threshold where num of lines is starting to grow
        while(thresholdValue > 0)
        {
            --thresholdValue;

            UseFilter();
            MiddleOfContours();
            CreateLines();

            if(nLines < lines.size())
            {
                thresholdLow = thresholdValue;
                std::cout << "calibrated lower value" << thresholdLow << std::endl;
                break;
            }
        }

        thresholdValue = thresholdLow + (thresholdUp + thresholdLow)/8;

        QtWidgetViewer->setThreshold(thresholdValue);
    }

    return thresholdValue;
}

void CaptureCamera::setContrast(int value)
{
    camera.set(CV_CAP_PROP_CONTRAST, value/100.0f);
}

void CaptureCamera::setBrightness(int value)
{
    camera.set(CV_CAP_PROP_BRIGHTNESS, value/100.0f);
}

void CaptureCamera::setSaturation(int value)
{
    camera.set(CV_CAP_PROP_SATURATION, value/100.0f);
}

void CaptureCamera::setSharpness(int value)
{
    camera.set(CV_CAP_PROP_SHARPNESS, value/100.0f);
}
