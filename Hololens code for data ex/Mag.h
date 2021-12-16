#pragma once

#include "researchmode\ResearchModeApi.h"
#include "Tar.h"
#include "TimeConverter.h"


#include <mutex>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Perception.Spatial.Preview.h>

class Mag
{
public:
    Mag(IResearchModeSensor* pMagSensor, HANDLE imuConsentGiven, ResearchModeSensorConsent* imuAccessConsent)
    {
        m_pMagSensor = pMagSensor;
        m_pMagSensor->AddRef();
        m_pSensorFrame = nullptr;

        m_pMagUpdateThread = new std::thread(MagUpdateThread, this, imuConsentGiven, imuAccessConsent);
        m_pMagWriteThread = new std::thread(MagWriteThread, this);


    }
    void MagSetStorageFolder(const winrt::Windows::Storage::StorageFolder& storageFolder);
    void MagResetStorageFolder();

    virtual ~Mag()
    {
        m_fExit = true;
        m_pMagUpdateThread->join();

        if (m_pMagSensor)
        {
            m_pMagSensor->CloseStream();
            m_pMagSensor->Release();
        }
        m_pMagWriteThread->join();

    }




protected:
    //fundamental functions to obtain sensor data, keep looping the data and writing the data to a specific file
    //retrieve frames in this thread
    static void MagUpdateThread(Mag* pMagReader, HANDLE imuConsentGiven, ResearchModeSensorConsent* ImuAccessConsent);

    //write the obtained frames to this in this thread
    static void MagWriteThread(Mag* pReader);

    bool IsNewTimestamp(IResearchModeSensorFrame* pSensorFrame);

    void SaveFrame(IResearchModeSensorFrame* pSensorFrame);
    void SaveFiles(IResearchModeSensorFrame* pSensorFrame, IResearchModeMagFrame* pSensorMagFrame);
    void DumpFiles();

    //fundamental interfaces and vars to access sensor data
    std::mutex m_sensorFrameMutex;
    IResearchModeSensor* m_pMagSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    IResearchModeMagFrame* m_pMagFrame;

    bool m_fExit = { false };
    std::thread* m_pMagUpdateThread;
    std::thread* m_pMagWriteThread;


    //some storage vars
    std::mutex m_storageMutex;
    //conditional storage vars
    std::condition_variable m_storageCondVar;
    winrt::Windows::Storage::StorageFolder m_storageFolder = nullptr;

    //not sure if im gonna use this
    std::unique_ptr<Io::Tarball> m_tarball;


    //timestamp related vars
    TimeConverter m_converter;
    UINT64 m_prevTimestamp = 0;

};