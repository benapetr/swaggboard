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
#include "options.hpp"

#ifdef WIN
#include <windows.h>
#include <dshow.h>
#endif

QList<OutputDevice> Options::devices;
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

#endif

void Options::Initialize()
{
#ifdef WIN
    HRESULT hr;
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
        return;

    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioRendererCategory, &pEnumCat, 0);
    if (hr == S_OK)
    {
        // Enumerate the monikers.
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        int id;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
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
                    OutputDevice device;
                    device.Name = QString((QChar*)varName.bstrVal, wcslen(varName.bstrVal));
                    Options::devices.append(device);
                }
                VariantClear(&varName);
                pPropBag->Release();
            }
            pMoniker->Release();
        }
        pEnumCat->Release();
    }
    pSysDevEnum->Release();
#endif
}

Options::Options(QWidget *parent) : QDialog(parent), ui(new Ui::Options)
{
    this->ui->setupUi(this);
    foreach (OutputDevice i, devices)
        this->ui->comboBox->addItem(i.Name);
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
    this->close();
}

OutputDevice::OutputDevice()
{

}

OutputDevice::~OutputDevice()
{
    //if (this->pDevice)
    //    SAFE_RELEASE(this->pDevice);
    //if (this->GUID)
    //    free(this->GUID);
}
