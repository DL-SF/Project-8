
#pragma once

#include "researchmode\ResearchModeApi.h"
#include "Tar.h"
#include "TimeConverter.h"


#include <mutex>
#include <winrt/Windows.Perception.Spatial.h>
#include <winrt/Windows.Perception.Spatial.Preview.h>

class Gyro
{
public:
    Gyro(IResearchModeSensor* pGyroSensor, HANDLE imuConsentGiven, ResearchModeSensorConsent* imuAccessConsent)
    {
        m_pGyroSensor = pGyroSensor;
        m_pGyroSensor->AddRef();
        m_pSensorFrame = nullptr;

        m_pGyroUpdateThread = new std::thread(GyroUpdateThread, this, imuConsentGiven, imuAccessConsent);
        m_pGyroWriteThread = new std::thread(GyroWriteThread, this);


    }
    void GyroSetStorageFolder(const winrt::Windows::Storage::StorageFolder& storageFolder);
    void GyroResetStorageFolder();

    virtual ~Gyro()
    {
        m_fExit = true;
        m_pGyroUpdateThread->join();

        if (m_pGyroSensor)
        {
            m_pGyroSensor->CloseStream();
            m_pGyroSensor->Release();
        }
        m_pGyroWriteThread->join();

    }




protected:
    //fundamental functions to obtain sensor data, keep looping the data and writing the data to a specific file
    //retrieve frames in this thread
    static void GyroUpdateThread(Gyro* pGyroReader, HANDLE imuConsentGiven, ResearchModeSensorConsent* ImuAccessConsent);

    //write the obtained frames to this in this thread
    static void GyroWriteThread(Gyro* pReader);

    bool IsNewTimestamp(IResearchModeSensorFrame* pSensorFrame);

    void SaveFrame(IResearchModeSensorFrame* pSensorFrame);
    void SaveFiles(IResearchModeSensorFrame* pSensorFrame, IResearchModeGyroFrame* pSensorGyroFrame);
    void DumpFiles();

    //fundamental interfaces and vars to access sensor data
    std::mutex m_sensorFrameMutex;
    IResearchModeSensor* m_pGyroSensor = nullptr;
    IResearchModeSensorFrame* m_pSensorFrame;
    IResearchModeGyroFrame* m_pGyroFrame;

    bool m_fExit = { false };
    std::thread* m_pGyroUpdateThread;
    std::thread* m_pGyroWriteThread;


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