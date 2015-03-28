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

#include "addproject.h"
#include "ui_addproject.h"

#include <QToolBar>
#include <QLayout>
#include <QMenuBar>

#include <QMessageBox>

using glm::vec2;
using glm::vec3;

AddProject::AddProject(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddProject)
{
    ui->setupUi(this);
    newProject = nullptr;

    this->setWindowTitle("New project");

    calibNoMarkers = calibMarkers = false;
}

Room *AddProject::resolveProject()
{
    newProject = new Room(nullptr, vec3(ui->width->text().toInt(), ui->length->text().toInt(), ui->height->text().toInt() ),
                        ui->epsilon->text().toFloat(), ui->name->text());

    for(size_t i = 0; i < newCameras.size(); i++)
    {
        newProject->AddCamera(newCameras[i]);
    }

    return newProject;
}

AddProject::~AddProject()
{
    delete ui;
}

void AddProject::on_buttonBox_accepted()
{

}

void AddProject::EditProject(Room *Project)
{
    std::vector<CaptureCamera*> cameras = Project->getcameras();

    for(size_t i = 0; i < cameras.size(); i++)
    {
        cameras[i]->TurnOff();
        cameras[i]->Hide();
        addCamToTable(cameras[i]);
    }

    vec3 dims = Project->getDimensions();

    ui->width->setText(QString::number(dims.x));
    ui->length->setText(QString::number(dims.y));
    ui->height->setText(QString::number(dims.z));

    ui->epsilon->setText(QString::number(Project->getEpsilon()));
    ui->name->setText(Project->getName());

    this->setWindowTitle(Project->getName());
}

void AddProject::addCamToTable(CaptureCamera *temp)
{
    CaptureCamera* newCam = new CaptureCamera();
    newCam->fromVariantMap(temp->toVariantMap());

    newCameras.push_back(newCam);

    int row = ui->CameraTable->rowCount();
    ui->CameraTable->insertRow(row);

    QTableWidgetItem *x = new QTableWidgetItem(QString::number(temp->getID()));
    ui->CameraTable->setItem(row, 0, x);
    x = new QTableWidgetItem(QString::number(temp->getPosition().x));
    ui->CameraTable->setItem(row, 1, x);
    x = new QTableWidgetItem(QString::number(temp->getPosition().y));
    ui->CameraTable->setItem(row, 2, x);
    x = new QTableWidgetItem(QString::number(temp->getPosition().z));
    ui->CameraTable->setItem(row, 3, x);
    x = new QTableWidgetItem(QString::number(temp->getAngle()));
    ui->CameraTable->setItem(row, 4, x);
    x = new QTableWidgetItem(temp->getName());
    ui->CameraTable->setItem(row, 5, x);

    x = new QTableWidgetItem(Qt::CheckStateRole);
    x->setCheckState(Qt::Unchecked);
    ui->CameraTable->setItem(row,6,x);

    x = new QTableWidgetItem(Qt::CheckStateRole);
    x->setCheckState(Qt::Unchecked);
    ui->CameraTable->setItem(row,7,x);
}

void AddProject::on_CameraTable_cellChanged(int row, int column)
{
    QTableWidgetItem *item = ui->CameraTable->item(row, column);

    switch (column)
    {
    case 1:
        //break;

    case 2:
        //break;
    case 3:
       /* newCameras[row]->setPosition(vec3(ui->CameraTable->item(row,1)->text().toFloat(),
                                            ui->CameraTable->item(row, 2)->text().toFloat(),
                                            ui->CameraTable->item(row,3)->text().toFloat()));
        break;*/
    case 4:
            newCameras[row]->setAngle(item->text().toFloat());
        break;
    case 5:
            newCameras[row]->setName(item->text());
        break;
    case 6:
        if(item->checkState() == Qt::Checked)
            newCameras[row]->TurnOn();
        else
            newCameras[row]->TurnOff();
        break;
    case 7:
        if(item->checkState() == Qt::Checked)
            newCameras[row]->Show();
        else
            newCameras[row]->Hide();
        break;
    default:
        break;
    }
}

void AddProject::on_deleteCamera_pressed()
{
    QItemSelectionModel *selection = ui->CameraTable->selectionModel();

    if(selection->hasSelection())
    {
        QModelIndexList indexes = selection->selectedRows();

        size_t index;
        for(int i = indexes.size()-1; i >= 0; i--)
        {
            index = indexes.at(i).row();
            ui->CameraTable->removeRow(index);

            newCameras[index]->Hide();
            newCameras[index]->TurnOff();
            newCameras.erase(newCameras.begin()+index);
        }
    }
}

void AddProject::on_AddCamera_clicked()
{
    AddCamera addcamera(this, vec3(ui->width->text().toInt(), ui->length->text().toInt(), ui->height->text().toInt()));
    addcamera.setModal(true);
    bool ok = addcamera.exec();

    if(ok) // indexes are X (0) Y(1) Z(2) USB index(3) Angle(4)
    {
        CaptureCamera *temp = addcamera.getCam();
        //temp->setDimensions(NewProject->getDimensions());
        newCameras.push_back(temp);

        addCamToTable(temp);
    }
}

void AddProject::on_name_textEdited(const QString &arg1)
{
    this->setWindowTitle(arg1);
}


void AddProject::on_CalibNoMarkers_clicked()
{
    for(size_t i = 0; i < newCameras.size(); i++)
    {
        newCameras[i]->CalibNoMarkers();
        ui->progressBar->setValue((100*(i+1))/newCameras.size());
    }

    calibNoMarkers = true;
}

void AddProject::on_CalibWithMarkers_clicked()
{
    ui->TextWithMarkers->setText(QString::fromStdString(""));
    int numOfMarkers = ui->numLEDs->text().toInt();

    for(size_t i = 0; i < newCameras.size(); i++)
    {
        ui->TextWithMarkers->append(newCameras[i]->getName() + ": ");
        ui->TextWithMarkers->append(QString::number(newCameras[i]->CalibWithMarkers(numOfMarkers)));
    }

    calibMarkers = true;
}

void AddProject::on_editCamera_clicked()
{

}
