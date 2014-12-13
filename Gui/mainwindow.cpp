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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <sstream>

#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("");

    /* SETUP ICONS */
    QIcon icon(QDir::currentPath() + "/Pictures/main_icon.jpg");
    this->setWindowIcon(icon);

    QIcon play(QDir::currentPath() + "/Pictures/PlayIcon.png");
    QIcon edit(QDir::currentPath() + "/Pictures/EditIcon.png");
    QIcon save(QDir::currentPath() + "/Pictures/SaveIcon.png");

    playIcon = play;
    editIcon = edit;
    saveIcon = save;

    project = nullptr;

    logDestinationFolder = "LogFile/log.txt";

    loadLog();

    createRollOutMenu();

    QDesktopWidget window;
    QRect screen = window.screenGeometry( window.screenNumber(this));
    move(screen.width()/2 - this->width()/2, screen.height()/2 - this->height()/2);

    record = false;
    captureAnimation = false;

    //ui->AnimationsTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    ui->CamerasWindows->setWidgetResizable(true);
    scrollWidget = new QWidget;
    scrollWidget->setLayout(new QVBoxLayout);
    ui->CamerasWindows->setWidget(scrollWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    if(e->key() == Qt::Key_W)
    {
        //ui->OpenGLWIndow->ZoomPlus();
    }
    else if(e->key() == Qt::Key_S)
    {
        //ui->OpenGLWIndow->ZoomMinus();
    }

}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{



}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();

    if(project != nullptr)
    {
        delete(project);
    }

    for(int i = 0; i < recentProjectsQActions.size(); i++)
    {
        delete(recentProjectsQActions.at(i));
    }



    saveLog();
}

void MainWindow::on_newProject_triggered()
{
    int msg;

    if(project != nullptr && !project->getSaved())
    {
        msg = NotSavedMessage();

        if(msg == QMessageBox::Cancel)
        {
            return;
        }
        else if(msg == QMessageBox::Save)
        {
            on_saveProject_triggered();

            if(!project->getSaved())
            {
                return;
            }
        }
    }

    AddProject NewProjectDialog(this);
    NewProjectDialog.setModal(true);
    bool ok = NewProjectDialog.exec();

    if(ok)
    {
        delete(project);

        project = NewProjectDialog.getProject();
        handleMainWProject(project);

    }
}

void MainWindow::OpenRecentProjects()
{
    QObject * sender = QObject::sender();

    for(int i = 0; i < recentProjectsQActions.size(); i++)
    {
        if(recentProjectsQActions.at(i) == sender)
        {
            int msg;

            if(project != nullptr && !project->getSaved())
            {
                msg = NotSavedMessage();

                if(msg == QMessageBox::Cancel)
                {
                    return;
                }
                else if(msg == QMessageBox::Save)
                {
                    on_saveProject_triggered();

                    if(!project->getSaved())
                    {
                        return;
                    }
                }
            }

            project = new Room(recentProjects[i]);
            handleMainWProject(project);
        }
    }
}

void MainWindow::on_openProject_triggered()
{
    int msg;

    if(project != nullptr && !project->getSaved())
    {
        msg = NotSavedMessage();

        if(msg == QMessageBox::Cancel)
        {
            return;
        }
        else if(msg == QMessageBox::Save)
        {
            on_saveProject_triggered();

            if(!project->getSaved())
            {
                return;
            }
        }
    }

    QString filename = QFileDialog::getOpenFileName(this,tr("Load Project"), ".", tr(".txt Files (*.txt)"));

    std::string filestring = filename.toStdString();

    if(filestring != "")
    {
        std::cout << "Open project:" << filestring << std::endl;

        if(!searchForRecentProjects(filestring))
        {
            recentProjects.push_back(filestring);
        }

        project = new Room(filestring);
        handleMainWProject(project);
    }

}

void MainWindow::on_editProject_triggered()
{
    AddProject NewProjectDialog(this);
    NewProjectDialog.EditProject(project);
    NewProjectDialog.setModal(true);

    bool ok = NewProjectDialog.exec();

    if(ok)
    {
        delete(project);

        project = NewProjectDialog.getProject();
        handleMainWProject(project);
    }
}

void MainWindow::on_saveProject_triggered()
{
    if(project != nullptr)
    {
        QString filename = QFileDialog::getSaveFileName(this,tr("Save Project"),QString::fromStdString( project->getName()+".txt" ), tr(".txt Files (*.txt)"));

        std::string filestring = filename.toStdString();

        if(filestring != "")
        {
            if(!searchForRecentProjects(filestring))
            {
                recentProjects.push_back(filestring);

                if(!project->getSaved())
                {
                    return;
                }
            }

            std::ofstream outputFile;
            outputFile.open(filestring, std::ios_base::out);

            //save name of project
            outputFile << project->getName() << std::endl;

            project->Save(outputFile);
            outputFile.close();
            std::cout << "Project saved to:" << filestring << std::endl;
        }
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("");
        msgBox.warning(this, "", "No project opened");
        msgBox.setFixedSize(200,100);
    }
}

void MainWindow::on_nahravanie_clicked(bool checked)
{
    if(checked)
    {
        ui->nahravanie->setText("Stop");
        record = true;

        if(project != nullptr)
        {
            project->RecordingStart();
        }
    }
    else
    {
        ui->nahravanie->setText("Record");
        project->RecordingStop();
        record = false;
    }

}

void MainWindow::createRollOutMenu()
{


    for(size_t i = 0; i < recentProjects.size(); i++)
    {
        QAction* temp = new QAction(QString::fromStdString(recentProjects[i]),this);

        recentProjectsQActions.push_back(temp);
        connect(recentProjectsQActions.at(i),  SIGNAL(triggered()), this, SLOT(OpenRecentProjects() ));
    }

    ui->menuRecent_Projects->addActions(recentProjectsQActions);
}

bool MainWindow::searchForRecentProjects(std::string filestring)
{
    for(size_t i = 0; i < recentProjects.size();i++)
    {
        if(recentProjects[i] == filestring)
        {
            return true;
        }
    }

    return false;
}

void MainWindow::saveLog()
{
    std::ofstream log;
    log.open(logDestinationFolder, std::ios_base::ate);

    std::cout << "writing log file" << std::endl;

    for(size_t i = 0; i < recentProjects.size(); i++)
    {
        log << recentProjects[i] << std::endl;
    }

    log.close();
}

void MainWindow::loadLog()
{
    std::ifstream log;
    log.open(logDestinationFolder, std::ios_base::in);

    std::string line;

    std::cout << "reading log filQTime::e" << std::endl;
    while(log.good())
    {
        getline(log,line);

        if(line != "")
        {
            if(QFile::exists(QString::fromStdString(line)))
            {
                recentProjects.push_back(line);
                std::cout << line << std::endl;
            }
        }
    }
    log.close();
}

int MainWindow::NotSavedMessage()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(" ");
    msgBox.setText("The document has been modified.");
    msgBox.setInformativeText("Do you want to save your changes?");
    msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save);
    msgBox.setDefaultButton(QMessageBox::Save);

    return msgBox.exec();
}

void MainWindow::handleMainWProject(Room *p)
{
    p->setOpenglWindow(ui->OpenGLWIndow);
    ui->OpenGLWIndow->setRoomDims(p->getDimensions());

    //add cams to scroll
    std::vector<CaptureCamera*> c = p->getcameras();

    for(size_t i = 0; i < c.size(); i++)
    {
        scrollWidget->layout()->addWidget(c[i]->getWidget());
    }
}

void MainWindow::on_playButton_pressed()
{
    if(!captureAnimation)
    {
        captureAnimation = true;
        project->CaptureAnimationStart();
    }
}

void MainWindow::on_stopButton_pressed()
{
    if(captureAnimation)
    {
        Animation * ActualAnimation = project->CaptureAnimationStop();
        captureAnimation = false;

        int row = ui->AnimationsTable->rowCount();
        ui->AnimationsTable->insertRow(row);

        QTableWidgetItem *x = new QTableWidgetItem(QString::fromStdString(ActualAnimation->getName()));
        ui->AnimationsTable->setItem(row, 0, x);
        x= new QTableWidgetItem(QString::number(ActualAnimation->getFrameRate()));
        ui->AnimationsTable->setItem(row, 1, x);
        x= new QTableWidgetItem(QString::number(ActualAnimation->getLength()));
        ui->AnimationsTable->setItem(row,2,x);
        x= new QTableWidgetItem(playIcon, "");
        ui->AnimationsTable->setItem(row, 3, x);
        x= new QTableWidgetItem(editIcon, "");
        ui->AnimationsTable->setItem(row, 4, x);
        x= new QTableWidgetItem(saveIcon, "");
        ui->AnimationsTable->setItem(row, 5, x);
    }
}

void MainWindow::on_AnimationsTable_cellChanged(int row, int column)
{
    if(column == 0)
    {
         //Animations[row]->setName(ui->AnimationsTable->item(row, column)->text().toStdString());
    }
}

void MainWindow::on_AnimationsTable_cellClicked(int row, int column)
{
    switch (column)
    {
    case 3:
        //play
        ui->AnimationsTable->item(row, column)->setSelected(false);
        break;
    case 4:
        //edit animation
        ui->AnimationsTable->item(row, column)->setSelected(false);
        break;
    case 5:
        //save animation
        ui->AnimationsTable->item(row, column)->setSelected(false);
        break;
    default:
        break;
    }
}

void MainWindow::on_AddStructure_clicked()
{
    scrollWidget->layout()->itemAt(0)->widget()->hide();
}

void MainWindow::on_LinesCheck_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->OpenGLWIndow->setDrawLines(false);
    }
    else
    {
        ui->OpenGLWIndow->setDrawLines(true);
    }
}

void MainWindow::on_JointsCheck_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->OpenGLWIndow->setDrawJoints(false);
    }
    else
    {
        ui->OpenGLWIndow->setDrawJoints(true);
    }
}

void MainWindow::on_BonesCheck_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        ui->OpenGLWIndow->setDrawBones(false);
    }
    else
    {
        ui->OpenGLWIndow->setDrawBones(true);
    }
}

void MainWindow::on_LivePipe_stateChanged(int arg1)
{
    if(arg1 == 0)
    {
        project->setPipe(false);
    }
    else
    {
        project->setPipe(true);
    }
}