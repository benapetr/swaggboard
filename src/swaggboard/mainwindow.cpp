//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "shortcuthelper.hpp"
#include "information.hpp"
#include "musicfinder.hpp"
#include "options.hpp"
#include <QDir>
#include <QtXml>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFile>
#if QT_VERSION >= 0x050000
#include <QMediaPlayer>
static QMediaPlayer *player = NULL;
#else
#include <phonon/mediaobject.h>


static Phonon::MediaObject *mp3 = NULL;


#endif
#include <QDesktopServices>
#include "definitions.hpp"

#ifdef WIN
    #include <windows.h>
#endif

#ifdef PlaySound
    #undef PlaySound
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    QStringList header;
    header << "Shortcut" << "Sound" << "Loop";
    this->ui->tableWidget->setColumnCount(3);
    this->ui->tableWidget->setHorizontalHeaderLabels(header);
    this->ui->tableWidget->verticalHeader()->setVisible(false);
    this->ui->tableWidget->horizontalHeader()->setSelectionBehavior(QAbstractItemView::SelectRows);
    //this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->tableWidget->setShowGrid(false);
    this->ui->tableWidget->setColumnWidth(0, 80);
    this->ui->tableWidget->setColumnWidth(1, 200);
    this->ui->tableWidget->setColumnWidth(2, 60);
    this->ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    this->ui->tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    if (!QDir().exists(this->GetConfig()))
        QDir().mkpath(this->GetConfig());
    this->Load(this->GetConfig() + "/default.xml");
    this->on_horizontalSlider_sliderMoved(100);
}

MainWindow::~MainWindow()
{
    delete this->ui;
}

void MainWindow::Shutdown()
{
    this->Save();
    this->Stop();
#ifdef WIN
    ExitProcess(0);
#else
    QApplication::exit(0);
#endif
}

void MainWindow::Load(QString path)
{
    // clear all current shortcuts first
    while (this->SL.count() > 0)
    {
        delete this->SL.at(0);
        this->SL.removeAt(0);
    }
    while (this->ui->tableWidget->rowCount() > 0)
        this->ui->tableWidget->removeRow(0);
    while (this->Finders.count())
    {
        delete this->Finders.at(0);
        // delete it
        this->Finders.removeAt(0);
    }
    this->File = path;
    this->setWindowTitle(path);
    if (!QFile().exists(path))
        return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox *msg = new QMessageBox(this);
        msg->setWindowTitle("Error");
        msg->setText("Unable to open " + path + " for reading");
        msg->exec();
        delete msg;
    }

    this->changed = false;
    QDomDocument xx;
    xx.setContent(file.readAll());
    file.close();
    QDomNodeList list = xx.elementsByTagName("item");
    int item = 0;
    while (item < list.count())
    {
        QDomElement e = list.at(item++).toElement();
        ShortcutHelper *helper = new ShortcutHelper();
        helper->RegisterKeys(e.attribute("shortcut"));
        helper->file = e.attribute("fp");
        this->SL.append(helper);
        int id = this->ui->tableWidget->rowCount();
        this->ui->tableWidget->insertRow(id);
        QTableWidgetItem *keys = new QTableWidgetItem("No");
        keys->setFlags(keys->flags() ^Qt::ItemIsEditable);
        this->ui->tableWidget->setItem(id, 0, new QTableWidgetItem(helper->GetKeys()));
        MusicFinder *x = new MusicFinder(this, helper);
        this->Finders.append(x);
        this->ui->tableWidget->setCellWidget(id, 1, x);
        this->ui->tableWidget->setItem(id, 2, keys);
        x->SetFile(helper->file);
        this->ui->tableWidget->resizeRowsToContents();
    }
}

void MainWindow::Stop()
{
#if QT_VERSION >= 0x050000
    if (player)
    {
        player->stop();
        player->deleteLater();
    }
    player = NULL;
#else
    if (mp3)
    {
        mp3->stop();
        mp3->deleteLater();
    }
    mp3 = NULL;
#endif
}

void MainWindow::Make(int type)
{
    this->changed = true;
    int id = this->ui->tableWidget->rowCount();
    this->ui->tableWidget->insertRow(id);
    QTableWidgetItem *keys = new QTableWidgetItem("No");
    ShortcutHelper *shortcut = new ShortcutHelper();
    keys->setFlags(keys->flags() ^Qt::ItemIsEditable);
    this->ui->tableWidget->setItem(id, 0, new QTableWidgetItem("None"));
    MusicFinder *x = new MusicFinder(this, shortcut);
    this->Finders.append(x);
    this->ui->tableWidget->setCellWidget(id, 1, x);
    this->ui->tableWidget->setItem(id, 2, keys);
    this->ui->tableWidget->resizeRowsToContents();
    this->SL.append(shortcut);
    if (type == 0)
        x->Stop();
}

void MainWindow::PlaySound(QString path)
{
    this->Stop();
    if (path == "stop")
        return;
#if QT_VERSION >= 0x050000
    player = new QMediaPlayer();
    player->setVolume(this->ui->horizontalSlider->value());
    player->setMedia(QUrl::fromLocalFile(path));
    player->play();
#else
    mp3 = Phonon::createPlayer(Phonon::MusicCategory, Phonon::MediaSource(path));
    mp3->play();
#endif
}

void MainWindow::Save()
{
    QFile file(this->File);
    if (!file.open(QIODevice::ReadWrite))
    {
        QMessageBox *msg = new QMessageBox(this);
        msg->setWindowTitle("Error");
        msg->setText("Unable to open " + this->File + " for writing");
        msg->exec();
        delete msg;
        return;
    }

    QXmlStreamWriter *writer = new QXmlStreamWriter();
    writer->setDevice(&file);
    writer->setAutoFormatting(true);
    writer->writeStartDocument();
    writer->writeStartElement("shortcuts");

    foreach (ShortcutHelper *xx, this->SL)
    {
        writer->writeStartElement("item");
        writer->writeAttribute("shortcut", xx->GetKeys());
        writer->writeAttribute("fp", xx->file);
        writer->writeEndElement();
    }

    writer->writeEndDocument();
    delete writer;

    file.close();
}

QString MainWindow::GetConfig()
{
#if QT_VERSION >= 0x050000
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
    return QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#endif
}

void MainWindow::on_actionExit_triggered()
{
    this->Shutdown();
}

void MainWindow::on_actionAdd_shortcut_triggered()
{
    this->Make(1);
}

void MainWindow::on_actionRemove_shortcut_triggered()
{
    this->changed = true;
    int id = this->ui->tableWidget->currentIndex().row();
    if (id < 0 || id > this->SL.count()-1)
        return;

    this->ui->tableWidget->removeRow(id);
    delete this->SL.at(id);
    this->SL.removeAt(id);
    delete this->Finders.at(id);
    this->Finders.removeAt(id);
}

void MainWindow::on_actionPlay_triggered()
{
    int index = this->ui->tableWidget->currentIndex().row();
    if (index >= this->SL.count() || index < 0)
        return;

    QString path = ((MusicFinder*)this->ui->tableWidget->cellWidget(index, 1))->RetrievePath();
    if (path.isEmpty())
        return;
    this->PlaySound(path);
}

void MainWindow::on_actionStop_playing_triggered()
{
    this->Stop();
}

void MainWindow::on_actionSave_triggered()
{
    this->Save();
}

void MainWindow::on_actionLoad_triggered()
{
    QFileDialog x;
    QStringList fltr;
    fltr << "Sound list (.xml) (*.xml)"
         << "Any files (*)";
    x.setNameFilters(fltr);
    x.exec();
    QStringList f = x.selectedFiles();
    if (f.isEmpty())
        return;

    this->Load(f.at(0));
}

void MainWindow::on_tableWidget_cellChanged(int row, int column)
{
    this->changed = true;
    if (column != 0)
        return;
    if (this->SL.count() <= row)
        return;
    this->SL.at(row)->RegisterKeys(this->ui->tableWidget->item(row, column)->text());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->Shutdown();
    event->ignore();
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    if (player)
        player->setVolume(position);
    this->ui->horizontalSlider->setToolTip("VOLUME IS " + QString::number(position) + "% RIGHT NOW");
}

void MainWindow::on_actionAdd_shortcut_to_stop_triggered()
{
    this->Make(STOP);
}

void MainWindow::on_actionHow_to_use_triggered()
{
    Information *wx = new Information();
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void MainWindow::on_actionSave_as_triggered()
{
    QString xx = QFileDialog::getSaveFileName(this, tr("Save File"),
                               "", tr("Xml (*.xml)"));

    if (xx.isEmpty())
        return;

    this->File = xx;
    this->Save();
    this->setWindowTitle(xx);
}

void MainWindow::on_actionPreferences_triggered()
{
    Options *wx = new Options();
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}
