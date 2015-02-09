//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

// Copyright (c) Petr Bena 2015

#include <QDesktopServices>
#include "definitions.hpp"
#include <QDir>
#include <QtXml>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFile>
#if QT_VERSION >= 0x050000
#include <QMediaPlayer>
#include <QMediaService>
#include <QAudioOutputSelectorControl>
static QMediaPlayer *player = NULL;
#else
#include <phonon/mediaobject.h>
static Phonon::MediaObject *mp3 = NULL;
#endif
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "shortcuthelper.hpp"
#include "information.hpp"
#include "musicfinder.hpp"
#include "options.hpp"
#ifdef WIN
    #include <dshow.h>
    #include <windows.h>

static IGraphBuilder *pGraph = NULL;
static IMediaControl *pControl = NULL;
static IMediaEvent   *pEvent = NULL;
static IBasicAudio   *pOutput = NULL;
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
#ifdef WIN
    if (pEvent)
    {
        pEvent->Release();
        pEvent = NULL;
    }
    if (pControl)
    {
        pControl->Release();
        pControl = NULL;
    }
    if (pOutput)
    {
        pOutput->Release();
        pOutput = NULL;
    }
    if (pGraph)
    {
        pGraph->Release();
        pGraph = NULL;
        CoUninitialize();
    }
    return;
#endif
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

static void Error(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle("Failed");
    mb.setText(reason);
    mb.exec();
}

#ifdef WIN

static IBaseFilter *SetOutput()
{
    if (Options::PreferredDevice == -1)
        return NULL;
    HRESULT hr;
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        return NULL;
    }

    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumCat, 0);
    IBaseFilter *pFilter = NULL;
    if (hr == S_OK)
    {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        int id;
        int i = 0;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            if (i++ != Options::PreferredDevice)
                continue;
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))
            {
                // To retrieve the filter's friendly name, do the following:
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    // Display the name in your UI somehow.
                }
                VariantClear(&varName);
                hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
                // Now add the filter to the graph. 
                //Remember to release pFilter later.
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnumCat->Release();
    }
    pSysDevEnum->Release();
    return pFilter;
}

#endif

void MainWindow::Volume(int volume)
{
#ifdef WIN
    if (pOutput)
    {
        if (volume > 100)
            volume = 100;
        else if (volume < 0)
            volume = 0;
        long vol = (100 - volume) * -100;
        pOutput->put_Volume(vol);
    }
#endif
}

void MainWindow::PlaySound(QString path)
{
    this->Stop();
    if (path == "stop")
        return;
#if QT_VERSION >= 0x050000
    //player = new QMediaPlayer();
    //QMediaService *svc = player->service();
    //QAudioOutputSelectorControl *out = qobject_cast<QAudioOutputSelectorControl *> (svc->requestControl(QAudioOutputSelectorControl_iid));
    //out->setActiveOutput(this->ui->comboBox->currentText());
    //QStringList items = out->availableOutputs();
   
    //svc->releaseControl(out);
    //player->setVolume(this->ui->horizontalSlider->value());
    //player->setMedia(QUrl::fromLocalFile(path));
    //player->play();
    wchar_t *file_path = new wchar_t[path.length() + 1];
    path.toWCharArray(file_path);
    file_path[path.length()] = '\0';

    // Initialize the COM library.
    HRESULT hr = ::CoInitialize(NULL);
    if (FAILED(hr))
    {
        Error("ERROR - Could not initialize COM library");
        return;
    }

    IBaseFilter *f = SetOutput();

    // Create the filter graph manager and query for interfaces.
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), (void **)&pGraph);
    if (FAILED(hr))
    {
        Error("ERROR - Could not create the Filter Graph Manager.");
        return;
    }

    hr = pGraph->QueryInterface(IID_IBasicAudio, (void**)&pOutput);

    if (FAILED(hr))
    {
        Error("ERROR - Could not create the IBasicAudio.");
        return;
    }

    this->Volume(this->ui->horizontalSlider->value());

    pGraph->AddFilter(f, L"device");
    hr = pGraph->QueryInterface(__uuidof(IMediaControl), (void **)&pControl);
    hr = pGraph->QueryInterface(__uuidof(IMediaEvent), (void **)&pEvent);

    // Build the graph.
    hr = pGraph->RenderFile(file_path, NULL);
    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = pControl->Run();
    }
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
    this->Volume(position);
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
