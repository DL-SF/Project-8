#pragma once

#include "researchmode/ResearchModeApi.h"
#include "RMCameraReader.h"
#include "Accel.h"
#include "Gyro.h"
#include "Mag.h"



class SensorScenario
{
public:
	//constr
	SensorScenario(const std::vector<ResearchModeSensorType>& kEnabledSensorTypes);

	//deconstr
	virtual ~SensorScenario();

	//init
	void InitializeSensors();
	void InitializeCameraReaders();
	void InitializeIMU();

	//rec
	void StartRecording(const winrt::Windows::Storage::StorageFolder& folder, const winrt::Windows::Perception::Spatial::SpatialCoordinateSystem& worldCoordSystem);
	void StopRecording();

	//consent
	static void CamAccessOnComplete(ResearchModeSensorConsent consent);
	static void ImuAccessOnComplete(ResearchModeSensorConsent consent);


private:

	//camera ids
	void GetRigNodeId(GUID& outGuid) const;


	std::vector<std::shared_ptr<RMCameraReader>> m_cameraReaders;
	std::shared_ptr<Accel>                              m_Accel;
	std::shared_ptr<Gyro>                               m_Gyro;
	std::shared_ptr<Mag>                                m_Mag;


	const std::vector<ResearchModeSensorType>& m_kEnabledSensorTypes;
	std::vector<ResearchModeSensorDescriptor> m_sensorDescriptors;





	//					INTERFACES
	//
	//

	//device interfaces
	IResearchModeSensorDevice* m_pSensorDevice = nullptr;
	IResearchModeSensorDeviceConsent* m_pSensorDeviceConsent = nullptr;


	//sensor interfaces
	IResearchModeSensor* m_pLFCameraSensor = nullptr;
	IResearchModeSensor* m_pRFCameraSensor = nullptr;
	IResearchModeSensor* m_pLLCameraSensor = nullptr;
	IResearchModeSensor* m_pRRCameraSensor = nullptr;
	IResearchModeSensor* m_pLTSensor = nullptr;
	IResearchModeSensor* m_pAHATSensor = nullptr;
	IResearchModeSensor* m_pAccelSensor = nullptr;
	IResearchModeSensor* m_pGyroSensor = nullptr;
	IResearchModeSensor* m_pMagSensor = nullptr;



};

