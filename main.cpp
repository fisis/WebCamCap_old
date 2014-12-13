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

#include <QApplication>
#include "Gui/mainwindow.h"
#include <GL/glut.h>

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    qRegisterMetaType<std::vector<Line> >("std::vector<Line>");
    qRegisterMetaType<std::vector<vec3> >("std::vector<vec3>");
    qRegisterMetaType<std::vector<vec2> >("std::vector<vec2>");
    qRegisterMetaType<cv::Mat >("cv::Mat");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
