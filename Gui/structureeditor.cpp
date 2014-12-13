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

#include "structureeditor.h"
#include "ui_structureeditor.h"

#include <QToolBar>
#include <QVBoxLayout>
#include <QTextEdit>

StructureEditor::StructureEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StructureEditor)
{
    currentItem = nullptr;

    ui->setupUi(this);
}

StructureEditor::~StructureEditor()
{
    delete ui;
}

void StructureEditor::BuildStructure()
{
    structure = new ModelStructure(ui->Editor);
}

void StructureEditor::on_Add_clicked()
{
    if(currentItem == nullptr)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList("New Point " + QString::number(ui->Editor->topLevelItemCount())));
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->Editor->addTopLevelItem(item);

        return;
    }

    if(currentItem->parent() != nullptr)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(currentItem->parent() , QStringList("New Point " + QString::number(currentItem->parent()->childCount())));
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        currentItem->parent()->addChild(item);
    }
    else
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(currentItem->parent() , QStringList("New Point " + QString::number(ui->Editor->topLevelItemCount())));
        item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        ui->Editor->addTopLevelItem(item);
    }
}

void StructureEditor::on_AddChild_clicked()
{
    if(currentItem != nullptr)
    {
      QTreeWidgetItem* item = new QTreeWidgetItem(currentItem , QStringList("New Point " + QString::number(currentItemChildCount)));
      item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
      currentItem->addChild(item);
    }
}

void StructureEditor::on_Remove_clicked()
{
    if(currentItem != nullptr)
    {
        currentItem->~QTreeWidgetItem();
    }
}

void StructureEditor::on_Up_clicked()
{

}

void StructureEditor::on_Down_clicked()
{

}

void StructureEditor::on_buttonBox_accepted()
{
    BuildStructure();
}

void StructureEditor::on_Editor_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    currentItem = current;

    if(currentItem != nullptr)
    {
        currentItemColumn = ui->Editor->currentColumn();
        currentItemChildCount = currentItem->childCount();
    }
}
