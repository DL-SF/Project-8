#pragma once

#include "Cannon/Common/Timer.h"
#include "Cannon/DrawCall.h"
#include "Cannon/FloatingSlate.h"
#include "Cannon/FloatingText.h"
#include "Cannon/MixedReality.h"
#include "Cannon/TrackedHands.h"

#include "HeTHaTEyeStream.h"
#include "SensorScenario.h"
#include "VideoFrameProcessor.h"

enum StreamTypes
{
	PV,  // RGB
	EYE  // Eye gaze tracking
	// Hands captured by default
};

class AppMain : public IFloatingSlateButtonCallback
{
public:
	//constr
	AppMain();

	//fundamental functions are defined here
	void Update();
	virtual void OnButtonPressed(FloatingSlateButton* pButton);
	void DrawObjects();
	void Render();
	winrt::Windows::Foundation::IAsyncAction StartRecordingAsync();
	void StopRecording();

	//the user is given the choice to select which stream he wants to record
	//this is pre-determined on this app
	//no matter what
	//the app will record long throw and imus
	//this can be changed later
	static std::vector<ResearchModeSensorType> kEnabledRMStreamTypes;
	static std::vector<StreamTypes> kEnabledStreamTypes;

private:
	//for non-RM
	winrt::Windows::Foundation::IAsyncAction InitializeVideoFrameProcessorAsync();
	bool IsVideoFrameProcessorWantedAndReady() const;
	inline bool IsQRCodeDetected() { return m_qrCodeValue.length() > 0; };


	bool SetDateTimePath();


	//fundamental vars for extracting info from cameras
	MixedReality m_mixedReality;
	TrackedHands m_hands;
	FloatingSlate m_menu;
	FloatingText m_debugText;

	// Coord axis showing when surface mapping detects the floor;
	// useful to roughly estimate user height for debugging purposes
	DrawCall m_coordAxes;
	Texture2D m_coordAxesTexture;
	XMMATRIX m_coordAxisTransform;

	DrawCall m_qrCodeCoordAxes;
	std::string m_qrCodeValue;
	XMMATRIX m_qrCodeTransform;

	HeTHaTEyeStream m_hethateyeStream;
	HeTHaTStreamVisualizer m_hethatStreamVis;

	winrt::Windows::Storage::StorageFolder m_archiveFolder = nullptr;
	std::unique_ptr<SensorScenario> m_scenario = nullptr;;

	std::unique_ptr<VideoFrameProcessor> m_videoFrameProcessor = nullptr;
	winrt::Windows::Foundation::IAsyncAction m_videoFrameProcessorOperation = nullptr;

	std::wstring m_datetime;

	Timer m_frameDeltaTimer;
	bool m_recording;
	float m_currentHeight;
};
