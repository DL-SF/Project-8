#pragma once

#include "researchmode\ResearchModeApi.h"
#include "Tar.h"
#include "TimeConverter.h"


#include <mutex>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Perception.Spatial.Preview.h>

class Accel
{
public:
    Accel(IResearchModeSensor* pAccelSensor, HANDLE imuConsentGiven, ResearchModeSensorConsent* imuAccessConsent)
    {
        m_pAccelSensor = pAccelSensor;
        m_pAccelSensor->AddRef();
        m_pSensorFrame = nullptr;

        m_pAccelUpdateThread = new std::thread(AccelUpdateThread, this, imuConsentGiven, imuAccessConsent);
        m_pAccelWriteThread = new std::thread(AccelWriteThread, this);


    }
    void AccelSetStorageFolder(const winrt::Windows::Storage::StorageFolder& storageFolder);
    void AccelResetStorageFolder();

    virtual ~Accel()
    {
        m_fExit = true;
        m_pAccelUpdateThread->join();

        if (m_pAccelSensor)
        {
            m_pAccelSensor->CloseStream();
            m_pAccelSensor->Release();
        }
        m_pAccelWriteThread->join();

    }




protected:
    //fundamental functions to obtain sensor data, keep looping the data and writing the data to a specific file
    //retrieve frames in this thread
    static void AccelUpdateThread(Accel* pAccelReader, HANDLE imuConsentGiven, ResearchModeSensorConsent* ImuAccessConsent);

    //write the obtained frames to this in this thread
    static void AccelWriteThread(Accel* pReader);

    bool IsNewTimestamp(IResearchModeSensorFrame* pSensorFrame);

    void SaveFrame(IResearchModeSensorFrame* pSensorFrame);
    void SaveFiles(IResearchModeSensorFrame* pSensorFrame, IResearchModeAccelFrame* pSensorAccelFrame);
    void DumpFiles();

    //fundamental interfaces and vars to access sensor data
    std::mutex m_sensorFrameMutex;
    IResearchModeSensor* m_pAccelSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    IResearchModeAccelFrame* m_pAccelFrame;

    bool m_fExit = { false };
    std::thread* m_pAccelUpdateThread;
    std::thread* m_pAccelWriteThread;


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