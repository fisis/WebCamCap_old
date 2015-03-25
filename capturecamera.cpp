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
//#include <glm/gtx/rotate_vector.hpp>
#include <glm/ext.hpp>

#include <QMessageBox>
#include <QVBoxLayout>
#include <QMatrix4x4>

using namespace cv;
using glm::vec2;
using glm::vec3;

/*
#ifdef _MSC_VER
    using glm::tvec3;
#else
    using glm::detail::tvec3;
#endif
*/


cv::Mat CaptureCamera::distortionCoeffs() const
{
    return m_distortionCoeffs;
}

void CaptureCamera::setDistortionCoeffs(const cv::Mat &distortionCoeffs)
{
    m_distortionCoeffs = distortionCoeffs;
}

cv::Mat CaptureCamera::cameraMatrix() const
{
    return m_IntrinsicMatrix;
}

void CaptureCamera::setCameraMatrix(const cv::Mat &cameraMatrix)
{
    cameraMatrix.convertTo(m_IntrinsicMatrix, CV_32F);

    m_projectionMatrix = m_IntrinsicMatrix * m_CameraMatrix;
}

cv::Mat CaptureCamera::cameraProjectionMatrix() const
{
    return m_projectionMatrix;
}

void CaptureCamera::setCameraProjectionMatrix(const cv::Mat &cameraProjectionMatrix)
{
    m_projectionMatrix = cameraProjectionMatrix;
}

cv::Mat CaptureCamera::IntrinsicMatrix() const
{
    return m_IntrinsicMatrix;
}

void CaptureCamera::setIntrinsicMatrix(const cv::Mat &IntrinsicMatrix)
{
    m_IntrinsicMatrix = IntrinsicMatrix;
}
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

    createExtrinsicMatrix();

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

/*
    if(!m_distortionCoeffs.empty() && !m_IntrinsicMatrix.empty())
    {
        GetUndisortedPosition();
    }
*/

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

        if(contArea > 500 || contArea <= 10)
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

    cv::undistort(framein, frame, m_IntrinsicMatrix, m_distortionCoeffs);
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
        //centerRelativeTemp = vec2(centerOfContour[i].x - frame.cols/2,centerOfContour[i].y - frame.rows/2);

        centerRelativeTemp = cv::Point2f(centerOfContour[i].x - frame.cols/2,centerOfContour[i].y - frame.rows/2);
        /*
        //rotacie
        directionTemp = glm::rotateZ((tvec3<double, (glm::precision)0u>) directionVectorToMiddle, (-centerRelativeTemp.x * anglePerPixel));//*0.0174532925);

        directionTemp = glm::rotateX((tvec3<double, (glm::precision)0u>) directionTemp , (-centerRelativeTemp.y * anglePerPixel));//*0.0174532925);
        lines.push_back(Line(position , directionTemp));
        */

        cv::Mat m_invertedRotationMatrix =  m_rotationMatrix.inv();


        QMatrix4x4 rotCamMatrix;
        QMatrix4x4 rotCamInvertedMatrix;

        for(size_t i = 0; i < 4; i++)
            for(size_t j = 0; j < 4; j++)
            {
                rotCamMatrix(i,j) = m_rotationMatrix.at<float>(i,j);
                rotCamInvertedMatrix(i,j) = m_invertedRotationMatrix.at<float>(i,j);
            }

        QMatrix4x4  rotMatrix;
        rotMatrix.rotate((-centerRelativeTemp.y * anglePerPixel), 1,0,0);

        QMatrix4x4 rotMatrix2;
        rotMatrix2.rotate((-centerRelativeTemp.x * anglePerPixel), 0, 1, 0);

        QVector4D vector(directionVectorToMiddle.x, directionVectorToMiddle.y, directionVectorToMiddle.z, 0);

        QVector4D result = rotCamInvertedMatrix* rotMatrix2 * rotMatrix * rotCamMatrix * vector;

        vec3 final = vec3(result.x(), result.y(), result.z());

        lines.push_back({position,final});
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

void CaptureCamera::createExtrinsicMatrix()
{
    m_rotationMatrix = cv::Mat::eye(4, 4, CV_32F);

    glm::vec3 normDirVector = glm::normalize(directionVectorToMiddle); //L

    m_rotationMatrix.at<float>(2,0) = -normDirVector.x;
    m_rotationMatrix.at<float>(2,1) = -normDirVector.y;
    m_rotationMatrix.at<float>(2,2) = -normDirVector.z;

    glm::vec3 sVector = glm::cross(normDirVector, glm::vec3(0,0,1));
    glm::vec3 normSVector = glm::normalize(sVector);

    m_rotationMatrix.at<float>(0,0) = normSVector.x;
    m_rotationMatrix.at<float>(0,1) = normSVector.y;
    m_rotationMatrix.at<float>(0,2) = normSVector.z;

    glm::vec3 uVector = glm::cross(normSVector, normDirVector);

    m_rotationMatrix.at<float>(1,0) = uVector.x;
    m_rotationMatrix.at<float>(1,1) = uVector.y;
    m_rotationMatrix.at<float>(1,2) = uVector.z;


    cv::Mat tMatrix;
    tMatrix = cv::Mat::eye(4,4, CV_32F);

    tMatrix.at<float>(0,3) = -position.x;
    tMatrix.at<float>(1,3) = -position.y;
    tMatrix.at<float>(2,3) = -position.z;

    //float myAngle = Line::LineAngle(Line(glm::vec3(0,0,0), glm::vec3(1, 0, 0)), Line(glm::vec3(0,0,0), directionVectorToMiddle));

    cv::Mat temp = m_rotationMatrix * tMatrix;
    m_CameraMatrix = temp(Rect(0,0, 4, 3));

    std::cout << m_CameraMatrix << std::endl;
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

        if(resolution.x != 0 && resolution.y !=0)
        {
            camera.set(CV_CAP_PROP_FRAME_WIDTH, resolution.x);
            camera.set(CV_CAP_PROP_FRAME_HEIGHT, resolution.y);
        }

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
                std::cout << "distance: " << "calibrated lower value" << thresholdLow << std::endl;
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
