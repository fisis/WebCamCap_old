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


#include "addcamera.h"
#include "ui_addcamera.h"

#include <QDir>
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

using glm::vec2;
using glm::vec3;
using namespace cv;

AddCamera::AddCamera(QWidget *parent, vec3 roomDims) :
    QDialog(parent),
    ui(new Ui::AddCamera)
{
    cam = nullptr;
    m_process = nullptr;
    ui->setupUi(this);
    record = false;
    this->roomDims = roomDims;
    this->setWindowTitle("Add new Camera");
    warning = false;

}

AddCamera::~AddCamera()
{
    if(m_process)
    {
        delete m_process;
    }

    delete ui;
}

void AddCamera::readYaml(int status)
{
    readConfigFile(ui->Name->text());
}

void AddCamera::on_buttonBox_accepted()
{
    if(cam)
    {
        delete cam;
    }

    cam = new CaptureCamera(vec3(ui->X->text().toFloat(),
                                 ui->Y->text().toFloat(),
                                 ui->Z->text().toFloat()),
                            roomDims,
                            ui->Name->text().toStdString(),
                            ui->deviceUSB_ID->text().toInt(),
                            ui->zorny_uhol->text().toFloat(), ui->useBackgroundSub->isChecked());

    cam->resolution = vec2(ui->FrameCols->text().toInt(), ui->FrameRows->text().toInt());


    //cam->setDistortionCoeffs(m_coefficient);
    //cam->setCameraMatrix(m_cameraMatrix);

    if(warning)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(" ");
        msgBox.setText("Scene contains bright areas!");
        msgBox.setInformativeText("Do you want to ignore them in capture?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);

        int status = msgBox.exec();

        if(status == QMessageBox::Yes)
        {
            Mat ex = getStructuringElement(MORPH_ELLIPSE, Size(20,20));
            morphologyEx(mask, mask, MORPH_DILATE, ex);
            cam->setROI(255 - mask);
        }
        else if(status == QMessageBox::Cancel)
        {
            return;
        }
    }

    endRecording();

    accept();
}

void AddCamera::on_buttonBox_rejected()
{
    endRecording();
}

void AddCamera::on_Play_ID_clicked(bool checked)
{
    if(checked)
    {
        record = true;

        if(!temp.open(ui->deviceUSB_ID->text().toInt()))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("");
            msgBox.warning(this, "", "Wrong device ID");
            msgBox.setFixedSize(200,100);

            ui->Play_ID->setChecked(false);
            ui->deviceUSB_ID->setEnabled(true);
            return;
        }
        ui->FrameCols->setText(QString::number(temp.get(CV_CAP_PROP_FRAME_WIDTH)));
        ui->FrameRows->setText(QString::number(temp.get(CV_CAP_PROP_FRAME_HEIGHT)));

        recording();
    }
    else
    {
        endRecording();
    }
}

void AddCamera::recording()
{
    warning = false;

    while(record)
    {
        QCoreApplication::processEvents();

        temp >> frame;
        mask = CaptureCamera::myColorThreshold(frame, getStructuringElement(MORPH_ELLIPSE, Size(2,2)), 220, 255);

        for(int i = 0; i < frame.rows; i++)
        {
            for(int j = 0; j < frame.cols; j++)
            {
                if(mask.at<uchar>(i,j) != 0)
                {
                    frame.at<Vec3b>(i,j) = Vec3b(0,0,255);
                    warning = true;
                }
            }
        }

        if(warning)
        {
            ui->warning->setText("Some areas are too bright");
        }
        else
        {
            ui->warning->setText("");
        }

        ui->glImage->showImage(frame);

        cv::waitKey(10);

        warning = false;
    }
}

void AddCamera::endRecording()
{
    if(record)
    {
        record = false;
        ui->warning->setText("");
        temp.release();
    }
}

void AddCamera::readConfigFile(QString path)
{
    cv::FileStorage file(path.toStdString(), FileStorage::READ);

    file["camera_matrix"] >> m_cameraMatrix;
    file["distortion_coefficients"] >> m_coefficient;
}


void AddCamera::on_FrameCols_editingFinished()
{
    temp.set(CV_CAP_PROP_FRAME_WIDTH, ui->FrameCols->text().toInt());
}

void AddCamera::on_FrameRows_editingFinished()
{
    temp.set(CV_CAP_PROP_FRAME_HEIGHT, ui->FrameRows->text().toInt());
}

void AddCamera::on_pushButton_clicked()
{
    QStringList arguments;

    QString usbid = ui->deviceUSB_ID->text();
    if(usbid == "")
    {
        usbid = "0";
    }

    arguments << usbid;
    arguments << "-w" << "7" << "-h" << "5" << "-pt" << "chessboard" << "-n" << "50";
    arguments << "-o" << ui->Name->text() + ".yaml";

    m_process = new QProcess(this);
    m_process->start(QDir::currentPath() + "/Calib",  arguments);

    connect(m_process, SIGNAL(finished(int)), this, SLOT(readYaml(int)));
}

void AddCamera::on_readYAML_clicked()
{

    QString filename = QFileDialog::getOpenFileName(this,tr("Load Distortion Coefficients"), ".", tr(".yaml Files (*.yaml)"));

    if(filename != "")
    {
        std::cout << filename.toStdString() << std::endl;

        readConfigFile(filename);
    }
}
