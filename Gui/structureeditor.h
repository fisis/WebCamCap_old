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

#ifndef STRUCTUREEDITOR_H
#define STRUCTUREEDITOR_H

#include <QDialog>

#include "modelstructure.h"

namespace Ui {
class StructureEditor;
}

class StructureEditor : public QDialog
{
    Q_OBJECT

    QTreeWidgetItem* currentItem;
    int currentItemColumn;
    size_t currentItemChildCount;

    ModelStructure* structure;
public:
    ModelStructure* getStructure() const {return structure;}
    explicit StructureEditor(QWidget *parent = 0);
    ~StructureEditor();

private slots:
    void on_Add_clicked();

    void on_AddChild_clicked();

    void on_Remove_clicked();

    void on_Up_clicked();

    void on_Down_clicked();

    void on_buttonBox_accepted();

    void on_Editor_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

private:
    void BuildStructure();
    Ui::StructureEditor *ui;
};

#endif // STRUCTUREEDITOR_H
