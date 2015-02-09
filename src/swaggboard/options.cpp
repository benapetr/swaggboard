//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU Lesser General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU Lesser General Public License for more details.

#include "ui_options.h"
#include <QSettings>
#include <QMessageBox>
#include <QAudioOutput>
#include "options.hpp"

#ifdef WIN
#include <windows.h>
#include <dsound.h>
#include <functiondiscoverykeys_devpkey.h>
#endif

QList<OutputDevice*> Options::devices;
int Options::PreferredDevice = -1;

static void Error(QString reason)
{
    QMessageBox mb;
    mb.setWindowTitle("Failed");
    mb.setText(reason);
    mb.exec();
}

#ifdef WIN

#define SAFE_RELEASE(punk)  if ((punk) != NULL)  \
            { (punk)->Release(); (punk) = NULL; }

static QString Ms2QString(LPCTSTR lpszDesc)
{
#ifdef UNICODE
    return QString::fromWCharArray(lpszDesc);
#else
    return QString::fromUtf8(lpszDesc);
#endif
}

static QString Ms2QString(LPWSTR lpszDesc)
{
    return QString::fromWCharArray(lpszDesc);
}

BOOL CALLBACK DSEnumProc(LPGUID lpGUID, LPCTSTR lpszDesc, LPCTSTR lpszDrvName, LPVOID lpContext)
{
  LPGUID lpTemp = NULL;

  if (lpGUID != NULL)  //  NULL only for "Primary Sound Driver".
  {
    if ((lpTemp = (LPGUID)malloc(sizeof(GUID))) == NULL)
    {
        return(TRUE);
    }
    memcpy(lpTemp, lpGUID, sizeof(GUID));
  }
  OutputDevice *device = new OutputDevice();
  device->Name = Ms2QString(lpszDesc);
  device->GUID = lpTemp;
  Options::devices.append(device);
  return(TRUE);
}

#endif

#define MESSAGE_ON_ERROR(code, step) if (code != S_OK) { Error("Failed to call EnumAudioEndpoints at step " + QString::number(step) + ": " + QString::number(hr)); return; }

void Options::Initialize()
{
#ifdef WIN
    //DirectSoundEnumerate((LPDSENUMCALLBACK)DSEnumProc, NULL);
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
    IMMDeviceEnumerator *enumerator;
    HRESULT hr = CoCreateInstance(CLSID_MMDeviceEnumerator,NULL,CLSCTX_ALL,IID_IMMDeviceEnumerator, (void**)&enumerator);
    MESSAGE_ON_ERROR(hr, 1);
    IMMDeviceCollection *dl;
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &dl);
    MESSAGE_ON_ERROR(hr, 2);
    unsigned int d = 0;
    UINT device_count;
    IPropertyStore *pProps;
    hr = dl->GetCount(&device_count);
    MESSAGE_ON_ERROR(hr, 3);
    while (d < device_count)
    {
        IMMDevice *pEndpoint;
        hr = dl->Item(d++, &pEndpoint);
        MESSAGE_ON_ERROR(hr, 4);
        OutputDevice *device = new OutputDevice();
        Options::devices.append(device);
        hr = pEndpoint->GetId(&device->ID);
        MESSAGE_ON_ERROR(hr, 5);
        hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
        MESSAGE_ON_ERROR(hr, 6);
        PROPVARIANT varName;
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        MESSAGE_ON_ERROR(hr, 7);
        device->Name = Ms2QString(varName.pwszVal);
        LPWSTR pwszID = NULL;
        hr = pEndpoint->GetId(&pwszID);
        MESSAGE_ON_ERROR(hr, 60);
        device->pDevice = pEndpoint;
        device->ID = pwszID;
        //CoTaskMemFree(pwszID);
        PropVariantClear(&varName);
        SAFE_RELEASE(pProps);
    }
    SAFE_RELEASE(enumerator);
    SAFE_RELEASE(dl);
#endif
}

void Options::OpenDevice(int device)
{
    if (device < 0)
        return;

    if (device >= Options::devices.count())
        return;

    OutputDevice *devp = Options::devices.at(device);

#ifdef WIN
    IAudioClient *pAudioClient = NULL;
    HRESULT hr = devp->pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (hr != S_OK)
    {
        QMessageBox m;
        m.setWindowTitle("Error");
        m.setText("Unable to open output device " + devp->Name + " error code " + QString::number(hr));
        m.exec();
        return;
    }
#endif
}

Options::Options(QWidget *parent) : QDialog(parent), ui(new Ui::Options)
{
    this->ui->setupUi(this);
    foreach (OutputDevice *i, devices)
        this->ui->comboBox->addItem(i->Name);
    QSettings s;
    this->ui->comboBox->setCurrentIndex(s.value("device", 0).toInt());
}

Options::~Options()
{
    delete this->ui;
}

void Options::on_buttonBox_rejected()
{
    this->close();
}

void Options::on_buttonBox_accepted()
{
    QSettings s;
    s.setValue("device", this->ui->comboBox->currentIndex());
    this->PreferredDevice = this->ui->comboBox->currentIndex();
    Options::OpenDevice(this->PreferredDevice);
    this->close();
}


OutputDevice::OutputDevice()
{
    this->GUID = NULL;
    this->pDevice = NULL;
}

OutputDevice::~OutputDevice()
{
    if (this->pDevice)
        SAFE_RELEASE(this->pDevice);
    if (this->GUID)
        free(this->GUID);
}
