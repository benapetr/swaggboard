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
#include <QSlider>
#include <QLayoutItem>
#include <QTableWidget>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFile>
#if QT_VERSION >= 0x050000
    #include <QMediaPlayer>
    #include <QMediaService>
static QMediaPlayer *player = NULL;
#else
    #include <phonon/mediaobject.h>
static Phonon::MediaObject *mp3 = NULL;
#endif
#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "shortcuthelper.hpp"
#include "items.hpp"
#include "information.hpp"
#include "musicfinder.hpp"
#include "options.hpp"
#ifdef WIN
    #include <dshow.h>
    #include <windows.h>

class Playback
{
    public:
        IGraphBuilder *pGraph = NULL;
        IMediaControl *pControl = NULL;
        IMediaEvent   *pEvent = NULL;
        IBaseFilter   *pFlx = NULL;
        IBasicAudio   *pOutput = NULL;
};

static QList<Playback*> HL;
MainWindow *MainWindow::mw;

#endif

#ifdef PlaySound
    #undef PlaySound
#endif

QSlider *MainWindow::MakeSlider(QObject *t)
{
    QSlider *x = new QSlider();
    x->setMaximum(200);
    QObject::connect(x, SIGNAL(sliderMoved(int)), t, SLOT(OnVolume(int)));
    x->setToolTip("Boost of volume, by default 0 (in middle) can be -100 to 100. This value will be added to default volume.");
    x->setOrientation(Qt::Horizontal);
    return x;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    this->ui->setupUi(this);
    mw = this;
    this->Values = new Items(this);
    this->setCentralWidget(this->Values);
    if (!QDir().exists(this->GetConfig()))
        QDir().mkpath(this->GetConfig());
    this->Load(this->GetConfig() + "/default.xml");
}

MainWindow::~MainWindow()
{
    delete this->Values;
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

    // clear all current shortcuts first
    this->Values->Clear();
    this->File = path;
    this->setWindowTitle(path);
    this->Values->Changed = false;
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
        this->Values->SL.append(helper);
        int id = this->Values->GetWidget()->rowCount();
        this->Values->GetWidget()->insertRow(id);
        QTableWidgetItem *keys = new QTableWidgetItem("No");
        keys->setFlags(keys->flags() ^Qt::ItemIsEditable);
        this->Values->GetWidget()->setItem(id, 0, new QTableWidgetItem(helper->GetKeys()));
        MusicFinder *x = new MusicFinder(this, helper);
        this->Values->Finders.append(x);
        this->Values->GetWidget()->setCellWidget(id, 1, x);
        this->Values->GetWidget()->setItem(id, 2, keys);
        x->SetFile(helper->file);
        QSlider *s = MakeSlider(this);
        this->Values->Sliders.append(s);
        if (!e.attributes().contains("volume"))
        {
            // default
            s->setValue(100);
        }
        else
        {
            helper->offset = e.attribute("volume").toInt();
            s->setValue(e.attribute("volume").toInt() + 100);
        }
        this->Values->GetWidget()->setCellWidget(id, 3, s);
        this->Values->GetWidget()->resizeRowsToContents();
    }
}

#define RELEASEWIN32(x) if (x) { x->Release(); x = NULL; }

void MainWindow::Stop()
{
#ifdef WIN
    foreach(Playback *p, HL)
    {
        RELEASEWIN32(p->pEvent)
        RELEASEWIN32(p->pControl)
        RELEASEWIN32(p->pFlx)
        RELEASEWIN32(p->pOutput)
        RELEASEWIN32(p->pGraph)
    }
    while (HL.count() >= 1)
    {
        delete HL.at(0);
        HL.removeAt(0);
    }
    CoUninitialize();
    return;
#else
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
#endif
}

MusicFinder *MainWindow::GetFinder(int i)
{
    if (this->Values->Finders.count() <= i)
        return NULL;
    return this->Values->Finders.at(i);
}

static void Error(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle("Failed");
    mb.setText(reason);
    mb.exec();
}

#ifdef WIN

static void PlayWin(IBaseFilter *device, wchar_t *path)
{
    Playback *x = new Playback();
    HL.append(x);
    // Create the filter graph manager and query for interfaces.
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, __uuidof(IGraphBuilder), (void **)&x->pGraph);
    if (FAILED(hr))
    {
        Error("ERROR - Could not create the Filter Graph Manager.");
        return;
    }

    hr = x->pGraph->QueryInterface(IID_IBasicAudio, (void**)&x->pOutput);

    if (FAILED(hr))
    {
        Error("ERROR - Could not create the IBasicAudio.");
        return;
    }

    x->pFlx = device;
    if (device)
        x->pGraph->AddFilter(device, L"fd");
    hr = x->pGraph->QueryInterface(__uuidof(IMediaControl), (void **)&x->pControl);
    hr = x->pGraph->QueryInterface(__uuidof(IMediaEvent), (void **)&x->pEvent);

    // Build the graph.
    hr = x->pGraph->RenderFile(path, NULL);
    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = x->pControl->Run();
    }
    else
    {
        Error("Unable to play: " + QString::fromWCharArray(path));
    }
}

static void SetOutputs(wchar_t *path)
{
    if (Options::nodevices)
    {
        // there is no device set
        PlayWin(NULL, path);
    }
    HRESULT hr;
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        Error("Failed SystemDeviceEnum");
        return;
    }

    IEnumMoniker *pEnumCat = NULL;
    QSettings s;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumCat, 0);
    IBaseFilter *pFilter = NULL;
    if (hr == S_OK)
    {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        int i = 0;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))
            {
                // retrieve the filter's friendly name now
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                if (SUCCEEDED(hr))
                {
                    QString name = QString((QChar*)varName.bstrVal, wcslen(varName.bstrVal));
                    if (s.value("d:" + name).toBool())
                    {
                        hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void**)&pFilter);
                        PlayWin(pFilter, path);
                    }
                }
                VariantClear(&varName);
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnumCat->Release();
    }
    pSysDevEnum->Release();
}

#endif

void MainWindow::Volume(int volume)
{
    if (volume == -1)
        volume = this->Values->GetVolumeBar()->value();
    volume += this->VolumeOffset / 2;
#if QT_VERSION >= 0x050000
    if (player)
        player->setVolume(volume);
#endif
#ifdef WIN
    if (volume > 100)
        volume = 100;
    else if (volume < 0)
        volume = 0;
    long vol = (100 - volume) * -100;
    foreach (Playback *x, HL)
    {
        if (x->pOutput) x->pOutput->put_Volume(vol);
    }
#endif
}

void MainWindow::SetOffset(int volume)
{
    this->VolumeOffset = volume;
}

void MainWindow::PlaySound(QString path, int offset)
{
    this->Stop();
    if (path == "stop")
        return;
    this->SetOffset(offset);
#if QT_VERSION >= 0x050000
#ifdef WIN
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

    SetOutputs(file_path);
    mw->Volume(-1);

    delete[] file_path;
#else
    player = new QMediaPlayer();
    QMediaService *svc = player->service();
    QAudioOutputSelectorControl *out = qobject_cast<QAudioOutputSelectorControl *> (svc->requestControl(QAudioOutputSelectorControl_iid));
    out->setActiveOutput(this->ui->comboBox->currentText());
    QStringList items = out->availableOutputs();

    svc->releaseControl(out);
    player->setVolume(this->ui->horizontalSlider->value());
    player->setMedia(QUrl::fromLocalFile(path));
    player->play();
#endif
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

    foreach (ShortcutHelper *xx, this->Values->SL)
    {
        writer->writeStartElement("item");
        writer->writeAttribute("shortcut", xx->GetKeys());
        writer->writeAttribute("fp", xx->file);
        writer->writeAttribute("volume", QString::number(xx->offset));
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
    this->Values->Make(1);
}

void MainWindow::on_actionRemove_shortcut_triggered()
{
    QList<QTableWidgetItem*> items = this->Values->GetWidget()->selectedItems();
    QList<int> selected_rows;
    foreach(QTableWidgetItem *item, items)
    {
        if (!selected_rows.contains(item->row()))
            selected_rows.append(item->row());
    }
    if (selected_rows.count() < 1)
        return;
    // we need to delete them upside down so that we don't cut the branch we are laying on
    qSort(selected_rows);
    this->Values->Changed = true;
    int row = selected_rows.count();
    while(row > 0)
    {
        int id = selected_rows[--row];
        if (id < 0 || id > this->Values->SL.count() - 1)
            return;

        this->Values->GetWidget()->removeRow(id);
        delete this->Values->SL.at(id);
        this->Values->SL.removeAt(id);
        delete this->Values->Finders.at(id);
        this->Values->Finders.removeAt(id);
    }
}

void MainWindow::on_actionPlay_triggered()
{
    int index = this->Values->GetWidget()->currentIndex().row();
    if (index >= this->Values->SL.count() || index < 0)
        return;

    QString path = ((MusicFinder*)this->Values->GetWidget()->cellWidget(index, 1))->RetrievePath();
    if (path.isEmpty())
        return;
    this->PlaySound(path, ((QSlider*)this->Values->GetWidget()->cellWidget(index, 3))->value() - 100);
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

void MainWindow::OnVolume(int position)
{
    QSlider *x = (QSlider*)QObject::sender();
    if (!this->Values->Sliders.contains(x))
        return;
    this->Values->SL.at(this->Values->Sliders.indexOf(x))->offset = position - 100;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    this->Shutdown();
    event->ignore();
}

void MainWindow::on_actionAdd_shortcut_to_stop_triggered()
{
    this->Values->Make(STOP);
}

void MainWindow::on_actionHow_to_use_triggered()
{
    Information *wx = new Information();
    wx->setAttribute(Qt::WA_DeleteOnClose);
    wx->show();
}

void MainWindow::on_actionSave_as_triggered()
{
    QString xx = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Xml (*.xml)"));

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

void MainWindow::on_actionKeys_triggered()
{
    QMessageBox *m = new QMessageBox();
    m->setWindowTitle("Keys");
    m->setText("f1 - f12\n"\
                      "esc space page-up page-down home end left up right down\n"\
                      "numerical keys: num0 - num9\n\n"\
                      "If you are missing any key please request it on https://github.com/benapetr/swaggboard");
    m->exec();
    delete m;
}

