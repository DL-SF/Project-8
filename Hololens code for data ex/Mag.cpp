#include "Mag.h"
#include <sstream>
#include "researchmode/ResearchModeApi.h"


using namespace winrt::Windows::Perception;
using namespace winrt::Windows::Perception::Spatial;
using namespace winrt::Windows::Foundation::Numerics;
using namespace winrt::Windows::Storage;


//this is the thread in which we constantly
//give permission to continue accessing
//the sensors
void Mag::MagUpdateThread(Mag* pMagReader, HANDLE imuConsentGiven, ResearchModeSensorConsent* imuAccessConsent)
{
    HRESULT hr = S_OK;

    //open up the stream so that we can extract
    //sensor data from it

    //if succeeded
    if (SUCCEEDED(hr))
    {
        hr = pMagReader->m_pMagSensor->OpenStream();
        if (FAILED(hr))
        {
            pMagReader->m_pMagSensor->Release();
            pMagReader->m_pMagSensor = nullptr;
        }

        while (!pMagReader->m_fExit && pMagReader->m_pMagSensor)
        {
            //assign fundamentals
            HRESULT hr = S_OK;
            IResearchModeSensorFrame* pSensorFrame = nullptr;


            hr = pMagReader->m_pMagSensor->GetNextBuffer(&pSensorFrame);
            if (SUCCEEDED(hr))
            {
                std::lock_guard<std::mutex> guard(pMagReader->m_sensorFrameMutex);

                if (pMagReader->m_pSensorFrame)
                {
                    pMagReader->m_pSensorFrame->Release();
                }
                pMagReader->m_pSensorFrame = pSensorFrame;
            }

        }

        if (pMagReader->m_pMagSensor)
        {
            pMagReader->m_pMagSensor->CloseStream();
        }

    }

}

bool Mag::IsNewTimestamp(IResearchModeSensorFrame* pSensorFrame)
{
    ResearchModeSensorTimestamp timeStamp;
    winrt::check_hresult(pSensorFrame->GetTimeStamp(&timeStamp));

    if (m_prevTimestamp == timeStamp.HostTicks)
    {
        return false;
    }

    m_prevTimestamp = timeStamp.HostTicks;

    return true;
}


void Mag::MagWriteThread(Mag* pReader)
{
    while (!pReader->m_fExit)
    {

        std::unique_lock<std::mutex> storage_lock(pReader->m_storageMutex);
        if (pReader->m_storageFolder == nullptr)
        {
            pReader->m_storageCondVar.wait(storage_lock);
        }

        std::lock_guard<std::mutex> reader_guard(pReader->m_sensorFrameMutex);
        if (pReader->m_pSensorFrame)
        {
            if (pReader->IsNewTimestamp(pReader->m_pSensorFrame))
            {
                pReader->SaveFrame(pReader->m_pSensorFrame);
            }
        }
    }
}

void Mag::SaveFrame(IResearchModeSensorFrame* pSensorFrame)
{

    IResearchModeMagFrame* pSensorMagFrame = nullptr;
    HRESULT   hr = pSensorFrame->QueryInterface(IID_PPV_ARGS(&pSensorMagFrame));

    if (pSensorMagFrame)
    {
        SaveFiles(pSensorFrame, pSensorMagFrame);
        pSensorMagFrame->Release();
    }
}
//this is triggered when the write thread above is triggered
void Mag::SaveFiles(IResearchModeSensorFrame* pSensorFrame, IResearchModeMagFrame* pSensorMagFrame)
{
    DirectX::XMFLOAT3 sample;
    char printString[1000];
    HRESULT hr = S_OK;

    ResearchModeSensorTimestamp timeStamp;
    UINT64 lastSocTickDelta = 0;
    UINT64 glastSocTick = 1;

    pSensorFrame->GetTimeStamp(&timeStamp);

    if (glastSocTick != 0)
    {
        lastSocTickDelta = timeStamp.HostTicks - glastSocTick;
    }
    glastSocTick = timeStamp.HostTicks;

    hr = pSensorMagFrame->GetMagnetometer(&sample);
    if (FAILED(hr))
    {
        return;
    }
    sprintf_s(printString, "####Mag: % 3.4f % 3.4f % 3.4f %f %I64d\n",
        sample.x,
        sample.y,
        sample.z,
        sqrt(sample.x * sample.x + sample.y * sample.y + sample.z * sample.z),
        (lastSocTickDelta * 1000) / timeStamp.HostTicksPerSecond
    );

    size_t outIMUBufferCount = 0;
    wchar_t outputIMUPath[MAX_PATH];
    HundredsOfNanoseconds timestamp = m_converter.RelativeTicksToAbsoluteTicks(HundredsOfNanoseconds((long long)m_prevTimestamp));

    std::string printStringAsStdStr = printString;
    swprintf_s(outputIMUPath, L"%llu_imu.txt", timestamp.count());
    std::vector<BYTE> IMUData;
    IMUData.reserve(printStringAsStdStr.size() + outIMUBufferCount * sizeof(UINT16));
    IMUData.insert(IMUData.end(), printStringAsStdStr.c_str(), printStringAsStdStr.c_str() + printStringAsStdStr.size());

    m_tarball->AddFile(outputIMUPath, &IMUData[0], IMUData.size());

    return;
}

//sets its own tar ball so that info can be recorded inside
void Mag::MagSetStorageFolder(const winrt::Windows::Storage::StorageFolder& storageFolder)
{
    std::lock_guard<std::mutex> storage_guard(m_storageMutex);
    m_storageFolder = storageFolder;
    wchar_t fileName[MAX_PATH] = {};
    swprintf_s(fileName, L"%s\\%s.tar", m_storageFolder.Path().data(), m_pMagSensor->GetFriendlyName());
    m_tarball.reset(new Io::Tarball(fileName));
    m_storageCondVar.notify_all();

}


//dumps IMU extrinsics as the streams close down and the recording is finished
void Mag::MagResetStorageFolder()
{
    std::lock_guard<std::mutex> storage_guard(m_storageMutex);
    DumpFiles();
    m_tarball.reset();
	m_storageFolder = nullptr ;

}

void Mag::DumpFiles()  //dumps extrinsics at the end of the stream
{

    //{
    //    std::lock_guard<std::mutex> guard(m_sensorFrameMutex);
    //    // Assuming we are at the end of the capture
    //    assert(m_pSensorFrame != nullptr);
    //}


    //// Get Mag sensor object
    //IResearchModeMagSensor* pMagSensor = nullptr;
    //HRESULT hr = m_pMagSensor->QueryInterface(IID_PPV_ARGS(&pMagSensor));
    //winrt::check_hresult(hr);

    //wchar_t outputExtrinsicsPath[MAX_PATH] = {};
    //swprintf_s(outputExtrinsicsPath, L"%s\\%s_Magdata_extrinsics.txt", m_storageFolder.Path().data(), m_pMagSensor->GetFriendlyName());

    //std::ofstream fileExtrinsics(outputExtrinsicsPath);
    //DirectX::XMFLOAT4X4 Mag_extrinsics;

    //fileExtrinsics << Mag_extrinsics.m[0][0] << "," << Mag_extrinsics.m[1][0] << "," << Mag_extrinsics.m[2][0] << "," << Mag_extrinsics.m[3][0] << ","
    //    << Mag_extrinsics.m[0][1] << "," << Mag_extrinsics.m[1][1] << "," << Mag_extrinsics.m[2][1] << "," << Mag_extrinsics.m[3][1] << ","
    //    << Mag_extrinsics.m[0][2] << "," << Mag_extrinsics.m[1][2] << "," << Mag_extrinsics.m[2][2] << "," << Mag_extrinsics.m[3][2] << ","
    //    << Mag_extrinsics.m[0][3] << "," << Mag_extrinsics.m[1][3] << "," << Mag_extrinsics.m[2][3] << "," << Mag_extrinsics.m[3][3] << "\n";

    //fileExtrinsics.close();

}