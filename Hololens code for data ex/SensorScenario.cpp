#include "SensorScenario.h"

extern "C"
HMODULE LoadLibraryA(
	LPCSTR lpLibFileName
);

static ResearchModeSensorConsent camAccessCheck;
static HANDLE camConsentGiven;
static ResearchModeSensorConsent imuAccessCheck;
static HANDLE imuConsentGiven;



//constr
SensorScenario::SensorScenario(const std::vector<ResearchModeSensorType>& kEnabledSensorTypes) : m_kEnabledSensorTypes(kEnabledSensorTypes)
{
}



//deconstr
//release all sensors and dont reach them anymore
SensorScenario::~SensorScenario()
{
	if (m_pLFCameraSensor)
	{
		m_pLFCameraSensor->Release();
	}
	if (m_pRFCameraSensor)
	{
		m_pRFCameraSensor->Release();
	}
	if (m_pLLCameraSensor)
	{
		m_pLLCameraSensor->Release();
	}
	if (m_pRRCameraSensor)
	{
		m_pRRCameraSensor->Release();
	}
	if (m_pLTSensor)
	{
		m_pLTSensor->Release();
	}
	if (m_pAHATSensor)
	{
		m_pAHATSensor->Release();
	}

	if (m_pAccelSensor)
	{
		m_pAccelSensor->Release();
	}
	if (m_pGyroSensor)
	{
		m_pGyroSensor->Release();
	}
	if (m_pMagSensor)
	{
		m_pMagSensor->Release();
	}

	if (m_pSensorDevice)
	{
		m_pSensorDevice->EnableEyeSelection();
		m_pSensorDevice->Release();
	}

	if (m_pSensorDeviceConsent)
	{
		m_pSensorDeviceConsent->Release();
	}
}


//for spatial reference purposes
//used for cameras
void SensorScenario::GetRigNodeId(GUID& outGuid) const
{
	IResearchModeSensorDevicePerception* pSensorDevicePerception;
	winrt::check_hresult(m_pSensorDevice->QueryInterface(IID_PPV_ARGS(&pSensorDevicePerception)));
	winrt::check_hresult(pSensorDevicePerception->GetRigNodeId(&outGuid));
	pSensorDevicePerception->Release();
}


//this will initialize all of the sensors
void SensorScenario::InitializeSensors()
{
	size_t sensorCount = 0;
	//given that the user has said okay for conset
	camConsentGiven = CreateEvent(nullptr, true, false, nullptr);
	imuConsentGiven = CreateEvent(nullptr, true, false, nullptr);


	// Load Research Mode library
	HMODULE hrResearchMode = LoadLibraryA("ResearchModeAPI");
	if (hrResearchMode)
	{
		typedef HRESULT(__cdecl* PFN_CREATEPROVIDER) (IResearchModeSensorDevice** ppSensorDevice);
		PFN_CREATEPROVIDER pfnCreate = reinterpret_cast<PFN_CREATEPROVIDER>(GetProcAddress(hrResearchMode, "CreateResearchModeSensorDevice"));
		if (pfnCreate)
		{
			winrt::check_hresult(pfnCreate(&m_pSensorDevice));
		}
	}

	// Manage Sensor Consent
	winrt::check_hresult(m_pSensorDevice->QueryInterface(IID_PPV_ARGS(&m_pSensorDeviceConsent)));
	winrt::check_hresult(m_pSensorDeviceConsent->RequestCamAccessAsync(SensorScenario::CamAccessOnComplete));
	winrt::check_hresult(m_pSensorDeviceConsent->RequestIMUAccessAsync(SensorScenario::ImuAccessOnComplete));


	//some fundamental initialization
	m_pSensorDevice->DisableEyeSelection();
	m_pSensorDevice->GetSensorCount(&sensorCount);
	m_sensorDescriptors.resize(sensorCount);
	m_pSensorDevice->GetSensorDescriptors(m_sensorDescriptors.data(), m_sensorDescriptors.size(), &sensorCount);



	//the Researchmode interface has the ability to
	//query what type of sensor we are using
	//here, a loop checks all of the sensors
	//that are currently active
	for (auto& sensorDescriptor : m_sensorDescriptors)
	{
		if (sensorDescriptor.sensorType == LEFT_FRONT)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), LEFT_FRONT) == m_kEnabledSensorTypes.end())
			{
				continue;
			}

			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pLFCameraSensor));
		}

		if (sensorDescriptor.sensorType == RIGHT_FRONT)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), RIGHT_FRONT) == m_kEnabledSensorTypes.end())
			{
				continue;
			}

			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pRFCameraSensor));
		}

		if (sensorDescriptor.sensorType == LEFT_LEFT)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), LEFT_LEFT) == m_kEnabledSensorTypes.end())
			{
				continue;
			}

			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pLLCameraSensor));
		}

		if (sensorDescriptor.sensorType == RIGHT_RIGHT)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), RIGHT_RIGHT) == m_kEnabledSensorTypes.end())
			{
				continue;
			}

			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pRRCameraSensor));
		}

		if (sensorDescriptor.sensorType == DEPTH_LONG_THROW)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), DEPTH_LONG_THROW) == m_kEnabledSensorTypes.end())
			{
				continue;
			}

			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pLTSensor));

		}

		if (sensorDescriptor.sensorType == DEPTH_AHAT)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), DEPTH_AHAT) == m_kEnabledSensorTypes.end())
			{
				continue;
			}
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pAHATSensor));
		}


		//IMU individual start-ups below
		//for accel
		if (sensorDescriptor.sensorType == IMU_ACCEL)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), IMU_ACCEL) == m_kEnabledSensorTypes.end())
			{
				continue;
			}
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pAccelSensor));
		}

		//for gyr
		if (sensorDescriptor.sensorType == IMU_GYRO)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), IMU_GYRO) == m_kEnabledSensorTypes.end())
			{
				continue;
			}
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pGyroSensor));
		}

		//for mag
		if (sensorDescriptor.sensorType == IMU_MAG)
		{
			if (std::find(m_kEnabledSensorTypes.begin(), m_kEnabledSensorTypes.end(), IMU_MAG) == m_kEnabledSensorTypes.end())
			{
				continue;
			}
			winrt::check_hresult(m_pSensorDevice->GetSensor(sensorDescriptor.sensorType, &m_pMagSensor));
		}
	}
}


//consent 
void SensorScenario::CamAccessOnComplete(ResearchModeSensorConsent consent)
{
	camAccessCheck = consent;
	SetEvent(camConsentGiven);
}

//consent
void SensorScenario::ImuAccessOnComplete(ResearchModeSensorConsent consent)
{
	imuAccessCheck = consent;
	SetEvent(imuConsentGiven);
}


//this step will initialize the cameras
//whatever has been given in main.cpp as enum will
//pop up here
void SensorScenario::InitializeCameraReaders()
{
	// Get RigNode id which will be used to initialize
	// the spatial locators for camera readers objects
	GUID guid;
	GetRigNodeId(guid);

	if (m_pLFCameraSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pLFCameraSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

	if (m_pRFCameraSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pRFCameraSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

	if (m_pLLCameraSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pLLCameraSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

	if (m_pRRCameraSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pRRCameraSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

	if (m_pLTSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pLTSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

	if (m_pAHATSensor)
	{
		auto cameraReader = std::make_shared<RMCameraReader>(m_pAHATSensor, camConsentGiven, &camAccessCheck, guid);
		m_cameraReaders.push_back(cameraReader);
	}

}


void SensorScenario::InitializeIMU()
{
	//fire up the IMUS
	// i dont know if i need to 
	//create the same pushback on vectors
	//as the cameras
	if (m_pAccelSensor)
	{
		m_Accel = std::make_shared<Accel>(m_pAccelSensor, imuConsentGiven, &imuAccessCheck);
		
	}

	if (m_pGyroSensor)
	{
		m_Gyro = std::make_shared<Gyro>(m_pGyroSensor, imuConsentGiven, &imuAccessCheck);
	
	}

	if (m_pMagSensor)
	{
		m_Mag = std::make_shared<Mag>(m_pMagSensor, imuConsentGiven, &imuAccessCheck);
		
	}
	
}



//a storage folder is created in which the streams will be recorded
void SensorScenario::StartRecording(const winrt::Windows::Storage::StorageFolder& folder,
	const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& worldCoordSystem)
{
	for (int i = 0; i < m_cameraReaders.size(); ++i)
	{
		m_cameraReaders[i]->SetWorldCoordSystem(worldCoordSystem);
		m_cameraReaders[i]->SetStorageFolder(folder);
	}

	//sets the IMU storage folder for
	//accel+gyro+mag at the same time
	m_Accel->AccelSetStorageFolder(folder);
	m_Gyro->GyroSetStorageFolder(folder);
	m_Mag->MagSetStorageFolder(folder);



}


//when it stops recording
//it will both cut the recording for
//RM sensors and non-RM sensors 
//but these are done in different files
void SensorScenario::StopRecording()
{
	for (int i = 0; i < m_cameraReaders.size(); ++i)
	{
		m_cameraReaders[i]->ResetStorageFolder();
	}
	//resets the IMU storage folder for
	//accel+gyro+mag at the same time

	m_Accel->AccelResetStorageFolder();
	m_Gyro->GyroResetStorageFolder();
	m_Mag->MagResetStorageFolder();

}
