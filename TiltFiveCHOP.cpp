/* Shared Use License: This file is owned by Derivative Inc. (Derivative)
* and can only be used, and/or modified for use, in conjunction with
* Derivative's TouchDesigner software, and only if you are a licensee who has
* accepted Derivative's TouchDesigner license or assignment agreement
* (which also govern the use of this file). You may share or redistribute
* a modified version of this file provided the following conditions are met:
*
* 1. The shared file or redistribution must retain the information set out
* above and this list of conditions.
* 2. Derivative's name (Derivative Inc.) or its trademarks may not be used
* to endorse or promote products derived from this file without specific
* prior written permission from Derivative.
*/

#include <stdio.h>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <stdbool.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32

#include "TiltFiveCHOP.h"

constexpr std::chrono::milliseconds operator""_ms(unsigned long long ms) {
	return std::chrono::milliseconds(ms);
}


//// These functions are basic C function, which the DLL loader can find
//// much easier than finding a C++ Class.
//// The DLLEXPORT prefix is needed so the compile exports these functions from the .dll
//// you are creating
//extern "C"
//{
//
//DLLEXPORT
//void
//FillCHOPPluginInfo(CHOP_PluginInfo *info)
//{
//	// Always set this to CHOPCPlusPlusAPIVersion.
//	info->apiVersion = CHOPCPlusPlusAPIVersion;
//
//	// The opType is the unique name for this CHOP. It must start with a 
//	// capital A-Z character, and all the following characters must lower case
//	// or numbers (a-z, 0-9)
//	info->customOPInfo.opType->setString("TiltfiveConnector");
//
//	// The opLabel is the text that will show up in the OP Create Dialog
//	info->customOPInfo.opLabel->setString("TiltFive Connector");
//
//	// Information about the author of this OP
//	info->customOPInfo.authorName->setString("Brent Marshall");
//	info->customOPInfo.authorEmail->setString("brent@eightlines.com");
//
//	// This CHOP can work with 0 inputs
//	info->customOPInfo.minInputs = 0;
//
//	// It can accept up to 1 input though, which changes it's behavior
//	info->customOPInfo.maxInputs = 0;
//}
//
//DLLEXPORT
//CHOP_CPlusPlusBase*
//CreateCHOPInstance(const OP_NodeInfo* info)
//{
//	// Return a new instance of your class every time this is called.
//	// It will be called once per CHOP that is using the .dll
//	return new TiltFiveCHOP(info);
//}
//
//DLLEXPORT
//void
//DestroyCHOPInstance(CHOP_CPlusPlusBase* instance)
//{
//	// Delete the instance here, this will be called when
//	// Touch is shutting down, when the CHOP using that instance is deleted, or
//	// if the CHOP loads a different DLL
//	delete (TiltFiveCHOP*)instance;
//}
//
//};


TiltFiveCHOP::TiltFiveCHOP(const OP_NodeInfo* info) : myNodeInfo(info)
{
	myExecuteCount = 0;
	myOffset = 0.0;
	prevActive = -1; // Set the inverse of the toggle to trigger the first action
}

TiltFiveCHOP::~TiltFiveCHOP()
{

}

void
TiltFiveCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1)
{
	// This will cause the node to cook every frame
	ginfo->cookEveryFrameIfAsked = true;

	// Note: To disable timeslicing you'll need to turn this off, as well as ensure that
	// getOutputInfo() returns true, and likely also set the info->numSamples to how many
	// samples you want to generate for this CHOP. Otherwise it'll take on length of the
	// input CHOP, which may be timesliced.
	ginfo->timeslice = true;

	ginfo->inputMatchIndex = 0;
}

bool
TiltFiveCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1)
{
	// If there is an input connected, we are going to match it's channel names etc
	// otherwise we'll specify our own.
	if (inputs->getNumInputs() > 0)
	{
		return false;
	}
	else
	{
		info->numChannels = 1;

		// Since we are outputting a timeslice, the system will dictate
		// the numSamples and startIndex of the CHOP data
		//info->numSamples = 1;
		//info->startIndex = 0

		// For illustration we are going to output 120hz data
		info->sampleRate = 120;
		return true;
	}
}

void
TiltFiveCHOP::getChannelName(int32_t index, OP_String *name, const OP_Inputs* inputs, void* reserved1)
{
	name->setString("chan1");
}

void
TiltFiveCHOP::execute(CHOP_Output* output,
							  const OP_Inputs* inputs,
							  void* reserved)
{
	myExecuteCount++;
	
	//double	 scale = inputs->getParDouble("Scale");

	// In this case we'll just take the first input and re-output it scaled.

	if (inputs->getNumInputs() > 0)
	{
		// We know the first CHOP has the same number of channels
		// because we returned false from getOutputInfo. 

		//inputs->enablePar("Speed", 0);	// not used
		//inputs->enablePar("Reset", 0);	// not used
		//inputs->enablePar("Shape", 0);	// not used
		//inputs->enablePar("TiltFive", 0);

		int ind = 0;
		const OP_CHOPInput	*cinput = inputs->getInputCHOP(0);

		//for (int i = 0 ; i < output->numChannels; i++)
		//{
		//	for (int j = 0; j < output->numSamples; j++)
		//	{
		//		output->channels[i][j] = float(cinput->getChannelData(i)[ind] * scale);
		//		ind++;

		//		// Make sure we don't read past the end of the CHOP input
		//		ind = ind % cinput->numSamples;
		//	}
		//}

	}
	else // If not input is connected, lets output a sine wave instead
	{
		int currentActive = inputs->getParInt("Tiltfive");
		if (currentActive != prevActive) {
			if (prevActive == -1) {
				std::cout << "TiltFive Init (Do Nothing)" << std::endl;
			} else if (prevActive == 0) {
				std::cout << "Connect TiltFive" << std::endl;
				connectT5();
			} else {
				std::cout << "Disconnect TiltFive" << std::endl;
			}
			prevActive = currentActive;
		}

		//std::cout << inputs->getParInt("Tiltfive") << std::endl;

		//inputs->enablePar("Speed", 1);
		inputs->enablePar("Reset", 1);

		//double speed = inputs->getParDouble("Speed");
		//double step = speed * 0.01f;


		// menu items can be evaluated as either an integer menu position, or a string
		//int shape = inputs->getParInt("Shape");
//		const char *shape_str = inputs->getParString("Shape");

		// keep each channel at a different phase
		double phase = 2.0f * 3.14159f / (float)(output->numChannels);

		// Notice that startIndex and the output->numSamples is used to output a smooth
		// wave by ensuring that we are outputting a value for each sample
		// Since we are outputting at 120, for each frame that has passed we'll be
		// outputing 2 samples (assuming the timeline is running at 60hz).


		//for (int i = 0; i < output->numChannels; i++)
		//{
		//	double offset = myOffset + phase*i;


		//	double v = 0.0f;

			//switch(shape)
			//{
			//	case 0:		// sine
			//		v = sin(offset);
			//		break;

			//	case 1:		// square
			//		v = fabs(fmod(offset, 1.0)) > 0.5;
			//		break;

			//	case 2:		// ramp	
			//		v = fabs(fmod(offset, 1.0));
			//		break;
			//}


		//	v *= scale;

		//	for (int j = 0; j < output->numSamples; j++)
		//	{
		//		output->channels[i][j] = float(v);
		//		offset += step;
		//	}
		//}

		//myOffset += step * output->numSamples; 
	}
}

int32_t
TiltFiveCHOP::getNumInfoCHOPChans(void * reserved1)
{
	// We return the number of channel we want to output to any Info CHOP
	// connected to the CHOP. In this example we are just going to send one channel.
	return 2;
}

void
TiltFiveCHOP::getInfoCHOPChan(int32_t index,
										OP_InfoCHOPChan* chan,
										void* reserved1)
{
	// This function will be called once for each channel we said we'd want to return
	// In this example it'll only be called once.

	if (index == 0)
	{
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	}

	if (index == 1)
	{
		chan->name->setString("offset");
		chan->value = (float)myOffset;
	}
}

bool		
TiltFiveCHOP::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1)
{
	infoSize->rows = 2;
	infoSize->cols = 2;
	// Setting this to false means we'll be assigning values to the table
	// one row at a time. True means we'll do it one column at a time.
	infoSize->byColumn = false;
	return true;
}

void
TiltFiveCHOP::getInfoDATEntries(int32_t index,
										int32_t nEntries,
										OP_InfoDATEntries* entries, 
										void* reserved1)
{
	char tempBuffer[4096];

	if (index == 0)
	{
		// Set the value for the first column
		entries->values[0]->setString("executeCount");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	}

	if (index == 1)
	{
		// Set the value for the first column
		entries->values[0]->setString("offset");

		// Set the value for the second column
#ifdef _WIN32
		sprintf_s(tempBuffer, "%g", myOffset);
#else // macOS
		snprintf(tempBuffer, sizeof(tempBuffer), "%g", myOffset);
#endif
		entries->values[1]->setString( tempBuffer);
	}
}

void
TiltFiveCHOP::setupParameters(OP_ParameterManager* manager, void *reserved1)
{
	// speed
	//{
	//	OP_NumericParameter	np;

	//	np.name = "Speed";
	//	np.label = "Speed";
	//	np.defaultValues[0] = 1.0;
	//	np.minSliders[0] = -10.0;
	//	np.maxSliders[0] =  10.0;
	//	
	//	OP_ParAppendResult res = manager->appendFloat(np);
	//	assert(res == OP_ParAppendResult::Success);
	//}

	// scale
	//{
	//	OP_NumericParameter	np;

	//	np.name = "Scale";
	//	np.label = "Scale";
	//	np.defaultValues[0] = 1.0;
	//	np.minSliders[0] = -10.0;
	//	np.maxSliders[0] =  10.0;
	//	
	//	OP_ParAppendResult res = manager->appendFloat(np);
	//	assert(res == OP_ParAppendResult::Success);
	//}

	// shape
	//{
	//	OP_StringParameter	sp;

	//	sp.name = "Shape";
	//	sp.label = "Shape";

	//	sp.defaultValue = "Sine";

	//	const char *names[] = { "Sine", "Square", "Ramp" };
	//	const char *labels[] = { "Sine", "Square", "Ramp" };

	//	OP_ParAppendResult res = manager->appendMenu(sp, 3, names, labels);
	//	assert(res == OP_ParAppendResult::Success);
	//}

	// pulse
	{
		OP_NumericParameter	np;

		np.name = "Reset";
		np.label = "Reset";
		
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Enable/Disable Toggle
	{
		OP_NumericParameter	np;

		np.name = "Tiltfive";
		np.label = "Tiltfive";

		np.defaultValues[0] = 0.0;

		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}

	// Text Label
	{
		OP_StringParameter sp;
		sp.name = "Glassesid";
		sp.label = "TiltFive ID";
		sp.defaultValue = "Not Connected";
		OP_ParAppendResult res = manager->appendString(sp);
		assert(res == OP_ParAppendResult::Success);
	}

}

void 
TiltFiveCHOP::pulsePressed(const char* name, void* reserved1)
{
	connectT5();

	if (!strcmp(name, "Reset"))
	{
		myOffset = 0.0;
	}
}

auto TiltFiveCHOP::MyParamChangeListener::onSysParamChanged(const std::vector<T5_ParamSys>& changed) -> void {
	for (const auto& param : changed) {
		std::cout << "Systen parameter changed: [" << param << "]" << std::endl;
	}
}

auto TiltFiveCHOP::MyParamChangeListener::onGlassesParamChanged(const Glasses& glasses, const std::vector<T5_ParamGlasses>& changed) -> void {
	for (const auto& param : changed) {
		std::cout << "Glasses Parameter changed: " << glasses << " => [" << param << "]" << std::endl;
	}
}

auto TiltFiveCHOP::printGameboardDimensions(Client& client) -> tiltfive::Result<void> {
	auto result = client->getGameboardSize(kT5_GameboardType_XE);
	if (!result) {
		return result.error();
	}

	float width = result->viewableExtentPositiveX - result->viewableExtentNegativeX;
	float length = result->viewableExtentPositiveY - result->viewableExtentNegativeY;
	float height = result->viewableExtentPositiveZ;

	std::cout << "XE Gameboard size: " << width << "m x " << length << "m x " << height << "m" << std::endl;
	return tiltfive::kSuccess;
}

auto TiltFiveCHOP::printServiceVersion(Client& client) -> tiltfive::Result<void> {
	auto result = client->getServiceVersion();
	if (!result) {
		return result.error();
	}
	std::cout << "Service Version: " << result << std::endl;
	return tiltfive::kSuccess;
}

auto TiltFiveCHOP::printUiStatusFlags(Client& client) -> tiltfive::Result<void> {
	auto result = client->isTiltFiveUiRequestingAttention();
	if (!result) {
		return result.error();
	}
	std::cout << "TiltFive UI (Attention Requested): " << ((*result) ? "TRUE" : "FALSE") << std::endl;
	return tiltfive::kSuccess;
}

//auto TiltFiveCHOP::wait3() {
//
//}
//template <typename T>
//auto TiltFiveCHOP::waitForService(Client& client, const std::function<tiltfive::Result<T>(Client& client)>& func) -> tiltfive::Result<T> {
//	bool waitingForService = false;
//	for (;;) {
//		auto result = func(client);
//		if (result) {
//			return result;
//		}
//		else if (result.error() != tiltfive::Error::kNoService) {
//			return result.error();
//		}
//
//		std::cout << (waitingForService ? "." : "Waiting for service...") << std::flush;
//		waitingForService = true;
//		std::this_thread::sleep_for(100_ms);
//	}
//}

auto TiltFiveCHOP::waitForGlasses(Client& client) -> tiltfive::Result<Glasses> {
	std::cout << "Looking for Glasses..." << std::flush;

	auto glassesList = client->listGlasses();
	if (!glassesList) {
		return glassesList.error();
	}

	while (glassesList->empty()) {
		std::cout << "." << std::flush;
		std::this_thread::sleep_for(100_ms);

		glassesList = client->listGlasses();
		if (!glassesList) {
			return glassesList.error();
		}
	}

	for (auto& glassesInstance : *glassesList) {
		std::cout << "Found: " << glassesInstance << std::endl;
		glassesId = glassesInstance;
	}

	return tiltfive::obtainGlasses(glassesList->front(), client);
}

auto TiltFiveCHOP::readPoses(Glasses& glasses) -> tiltfive::Result<void> {
	auto start = std::chrono::steady_clock::now();
	do {
		auto pose = glasses->getLatestGlassesPose(); //T5_GlassesPoseUsage::kT5_GlassesPoseUsage_GlassesPresentation
		if (!pose) {
			if (pose.error() == tiltfive::Error::kTryAgain) {
				std::cout << "Pose unavailable - Is gameboard visible?" << std::endl;
			}
			else {
				return pose.error();
			}
		}
		else {
			std::cout << pose << std::endl;
		}
	} while ((std::chrono::steady_clock::now() - start) < 10000_ms);
	return tiltfive::kSuccess;
}

auto TiltFiveCHOP::doThingsWithGlasses(Glasses& glasses) -> tiltfive::Result<void> {
	std::cout << "Doing something with: " << glasses << std::endl;

	auto friendlyName = glasses->getFriendlyName();
	if (friendlyName) {
		std::cout << "Obtained friendly name: " << friendlyName << std::endl;
	}
	else if (friendlyName.error() == tiltfive::Error::kSettingUnknown) {
		std::cerr << "Couldn't get friendly name: Service reports it isn't set" << std::endl;
	}
	else {
		std::cerr << "Error obtaining friendly name:" << friendlyName << std::endl;
	}

	//std::promise<int> prom;
	//std::future<int> fut = prom.get_future();
	//std::thread th1(wait3, std::ref(fut));

	//{
	//	auto connectionHelper = glasses->createConnectionHelper("TouchDesigner Connector - Player 1");
	//	auto connectionResult = connectionHelper->awaitConnection(10000_ms);
	//	if (connectionResult) {
	//		std::cout << "Glasses connected for exclusive use" << std::endl;
	//	}
	//	else {
	//		std::cerr << "Error connecting to glasses for exclusive use: " << connectionResult << std::endl;
	//		return connectionResult.error();
	//	}

//		auto result = readPoses(glasses);
//		if (!result) {
//			std::cerr << "Error reading poses: " << result << std::endl;
//			return result.error();
//		}
	//}
//
//	auto releaseResult = glasses->release();
//	if (!releaseResult) {
//		std::cerr << "Failed to release glasses: " << releaseResult << std::endl;
//		return releaseResult.error();
//	}
//	auto readPosesResult = readPoses(glasses);
//	if (readPosesResult) {
//		std::cerr << "Reading poses unexpectedly succeeded after glasses release" << std::endl;
//	}
//	else if (readPosesResult.error() != tiltfive::Error::kNotConnected) {
//		std::cerr << "Unexpected pose read error: " << readPosesResult << std::endl;
//	}

	std::cout << std::endl << "Done with glasses" << std::endl;
	return tiltfive::kSuccess;
}

//template<typename T>
//inline auto TiltFiveCHOP::waitForService(Client& client, const std::function<tiltfive::Result<T>(Client& client)>& func) -> tiltfive::Result<T>
//{
//	return tiltfive::Result<T>();
//}

//template <typename T>
//auto waitForService(std::shared_ptr<tiltfive::Client>& client, const std::function<tiltfive::Result<T>(std::shared_ptr<tiltfive::Client>& client)>& func) -> tiltfive::Result<T> {
//	bool waitingForService = false;
//	for (;;) {
//		auto result = func(client);
//		if (result) {
//			return result;
//		}
//		else if (result.error() != tiltfive::Error::kNoService) {
//			return result.error();
//		}
//
//		std::cout << (waitingForService ? "." : "Waiting for service...") << std::flush;
//		waitingForService = true;
//		std::this_thread::sleep_for(100_ms);
//	}
//};

//template <typename T>
//auto TiltFiveCHOP::GetMax(int a, int b) {
//	return (a > b ? a : b);
//}
//
//template <typename T>
//auto TiltFiveCHOP::waitForService(Client& client, const std::function<tiltfive::Result<T>(Client& client)>& func) -> tiltfive::Result<T> {
//	auto result = func(client);
//	return result;
//};
//
//template <typename T>
//auto wait2(Client& client, const std::function<tiltfive::Result<T>(Client& client)>& func) {
//	auto result = func(client);
//	return result;
//};

void TiltFiveCHOP::connectT5() {
	auto client = tiltfive::obtainClient("com.eightlines.t5", "0.1.0", nullptr);
	if (!client) {
		std::cerr << "Failed to create client: " << client << std::endl;
		std::exit(EXIT_FAILURE);
	}
	std::cout << "Obtained client: " << client << std::endl;

	std::shared_ptr<MyParamChangeListener> paramChangeListener(new MyParamChangeListener());
	auto paramChangeHelper = (*client)->createParamChangedHelper(paramChangeListener);

	auto result = printGameboardDimensions(*client);
	if (!result) {
		std::cerr << "Failed to print gameboard dimensions: " << result << std::endl;
		std::exit(EXIT_FAILURE);
	}

	result = printServiceVersion(*client);
	if (!result) {
		std::cerr << "Failed to get service version: " << result << std::endl;
		std::exit(EXIT_FAILURE);
	}

	//std::future<tiltfive::Result<void>> fut = std::async(printServiceVersion, *client); //(printServiceName, *client);

	//result = waitForService<void>(*client, printServiceVersion);
		
		//waitForService<tiltfive::Result>(client, printServiceVersion);
	//result = wait2<void>(*client, printServiceVersion);

	//result = waitForService<void>(client, printServiceVersion);

	//int res = GetMax<int>(1, 2);

	//TiltFiveCHOP::waitForService<void>(client, printServiceVersion);

	//TiltFiveCHOP::waitForService<void>(*client, printServiceVersion);

	//result = TiltFiveWait::waitForService<void>(*client, printServiceVersion);
	//if (!result) {
	//	std::cerr << "Failed to get service version: " << result << std::endl;
	//	std::exit(EXIT_FAILURE);
	//}

	//result = TiltFiveWait::waitForService<void>(*client, printUiStatusFlags);
	//if (!result) {
	//	std::cerr << "Failed to print UI Status Flags: " << result << std::endl;
	//	std::exit(EXIT_FAILURE);
	//}

	//{
	//	auto glasses = TiltFiveWait::waitForService<Glasses>(*client, waitForGlasses);
	//	if (!glasses) {
	//		std::cerr << "Failed to wait for glasses: " << glasses << std::endl;
	//		std::exit(EXIT_FAILURE);
	//	}

	//	paramChangeHelper->registerGlasses(*glasses);

	//	result = doThingsWithGlasses(*glasses);
	//	if (!result) {
	//		std::cerr << "Failed to do things with glasses: " << result << std::endl;
	//	}
	//}
}

