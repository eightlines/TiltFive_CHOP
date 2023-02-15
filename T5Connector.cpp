#include "T5Connector.h"

extern "C"
{
	DLLEXPORT
		void FillCHOPPluginInfo(CHOP_PluginInfo* info) {
		info->apiVersion = CHOPCPlusPlusAPIVersion;
		info->customOPInfo.opType->setString("T5Connector");
		info->customOPInfo.opLabel->setString("T5Connector");
		info->customOPInfo.authorEmail->setString("brent@eightlines.com");
		info->customOPInfo.authorName->setString("Brent Marshall");
		info->customOPInfo.minInputs = 0;
		info->customOPInfo.maxInputs = 1;
	}

	DLLEXPORT
	CHOP_CPlusPlusBase* CreateCHOPInstance(const OP_NodeInfo* info) {
		return new T5Connector(info);
	}

	DLLEXPORT
	void DestroyCHOPInstance(CHOP_CPlusPlusBase* instance) {
		delete (T5Connector*)instance;
	}
};

constexpr std::chrono::milliseconds operator""_ms(unsigned long long ms) {
	return std::chrono::milliseconds(ms);
}

T5Connector::T5Connector(const OP_NodeInfo* info) : myNodeInfo(info) {
	myExecuteCount = 0;
	prevActive = -1;

	T5_Vec3 p;
	position = &p;
	
	T5_Quat r;
	rotation = &r;

	glassesBound = nullptr;
}

void T5Connector::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void* reserved1) {
	ginfo->cookEveryFrameIfAsked = true;
	ginfo->timeslice = true;
	ginfo->inputMatchIndex = 0;
}

bool T5Connector::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void* reserved1) {
	if (inputs->getNumInputs() > 0) {
		return false;
	} else {
		info->numChannels = 10;
		info->sampleRate = 120;
		return true;
	}
}

void T5Connector::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void* reserved1) {
	const char* label = "";
	switch (index) {
		case 0: label = "bw"; break; // Board Width
		case 1: label = "bl"; break; // Board Length
		case 2: label = "bh"; break; // Board Height
		case 3: label = "px"; break; // Position X
		case 4: label = "py"; break; // Position Y
		case 5: label = "pz"; break; // Position Z
		case 6: label = "qx"; break; // Rotation X
		case 7: label = "qy"; break; // Rotation Y
		case 8: label = "qz"; break; // Rotation Z
		case 9: label = "qw"; break; // Rotation W
	}
	name->setString(label);
}

void T5Connector::execute(CHOP_Output* output, const OP_Inputs* inputs, void* reserved) {
	myExecuteCount++;

	if (inputs->getNumInputs() > 0) {
		//const OP_CHOPInput* cInput = inputs->getInputCHOP(0);
	} else {
		inputs->enablePar("Reset", 1);
		inputs->enablePar("Tiltfive", 1);

		int currentActive = inputs->getParInt("Tiltfive");
		if (currentActive != prevActive) {
			if (prevActive == -1) {
				std::cout << "TiltFive Init (Do nothing)" << std::endl;
			} else if (prevActive == 0) {
				std::cout << "Connect TiltFive" << std::endl;
				connectT5();
			} else {
				std::cout << "Disconnect TiltFive" << std::endl;
			}
			prevActive = currentActive;
		}

		if (glassesBound) {
			auto pose = glassesBound->getLatestGlassesPose(kT5_GlassesPoseUsage_GlassesPresentation);
			if (!pose) {
				if (pose.error() == tiltfive::Error::kTryAgain) {
					std::cout << "Pose unavailable - Is gameboard visible?" << std::endl;
				}
			} else {
				//std::cout << pose << std::endl;

				position = &pose->posGLS_GBD;
				rotation = &pose->rotToGLS_GBD;

				//std::cout << position->x << ", " << position->y << ", " << position->z << std::endl;
				//std::cout << rotation->x << ", " << rotation->y << ", " << rotation->z << ", " << rotation->w << std::endl;
			}
		}

		for (int i = 0; i < output->numChannels; i++) {
			if (i < 3) {
				output->channels[i][1] = boardDimensions[i];
			} else {
				switch (i) {
					case 3: output->channels[i][1] = (glassesBound) ? position->x : 0.0;  break;
					case 4: output->channels[i][1] = (glassesBound) ? position->y : 0.0;  break;
					case 5: output->channels[i][1] = (glassesBound) ? position->z : 0.0;  break;
					case 6: output->channels[i][1] = (glassesBound) ? rotation->x : 0.0;  break;
					case 7: output->channels[i][1] = (glassesBound) ? rotation->y : 0.0;  break;
					case 8: output->channels[i][1] = (glassesBound) ? rotation->z : 0.0;  break;
					case 9: output->channels[i][1] = (glassesBound) ? rotation->w : 0.0;  break;
				}
			}
		}

	}
}

int32_t T5Connector::getNumInfoCHOPChans(void* reserved1) {
	return 4;
}

void T5Connector::getInfoCHOPChan(int32_t index, OP_InfoCHOPChan* chan, void* reserved1) {
	if (index == 0) { // Execute Count
		chan->name->setString("executeCount");
		chan->value = (float)myExecuteCount;
	} else if (index == 1) { // T5 Board Width
		chan->name->setString("board_width");
		chan->value = (float)boardDimensions[0];
	} else if (index == 2) { // T5 Board Length
		chan->name->setString("board_length");
		chan->value = (float)boardDimensions[1];
	} else if (index == 3) { // T5 Board Height
		chan->name->setString("board_height");
		chan->value = (float)boardDimensions[2];
	}
}

bool T5Connector::getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) {
	infoSize->rows = 2;
	infoSize->cols = 2;
	infoSize->byColumn = false;
	return true;
}

void T5Connector::getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1) {
	char tempBuffer[4096];
	if (index == 0) {
		entries->values[0]->setString("executeCount");
#ifdef _WIN32
		sprintf_s(tempBuffer, "%d", myExecuteCount);
#elif // Mac OS
		snprintf(tempBuffer, sizeof(tempBuffer), "%d", myExecuteCount);
#endif
		entries->values[1]->setString(tempBuffer);
	} 
}

void T5Connector::setupParameters(OP_ParameterManager* manager, void* reserved1) {
	{ // Reset
		OP_NumericParameter np;
		np.name = "Reset";
		np.label = "Reset";
		OP_ParAppendResult res = manager->appendPulse(np);
		assert(res == OP_ParAppendResult::Success);
	}

	{ // Enable/Disable Toggle
		OP_NumericParameter	np;
		np.name = "Tiltfive";
		np.label = "TiltFive";
		np.defaultValues[0] = 0;
		OP_ParAppendResult res = manager->appendToggle(np);
		assert(res == OP_ParAppendResult::Success);
	}
}

void T5Connector::pulsePressed(const char* name, void* reserved1) {
	std::cout << "Pulse: " << name << std::endl;
}

void T5Connector::connectT5() {
	auto client = tiltfive::obtainClient("com.eightlines.t5", "0.1.0", nullptr);
	if (!client) {
		std::cerr << "Failed to create client: " << client << std::endl;
		//std::exit(EXIT_FAILURE);
	}
	std::cout << "Obtained Client: " << client << std::endl;

	std::shared_ptr<MyParamChangedListener> paramChangedListener(new MyParamChangedListener());
	auto paramChangeHelper = (*client)->createParamChangedHelper(paramChangedListener);

	auto result = printGameboardDimensions(*client);
	if (!result) {
		std::cerr << "Failed to get service version: " << result << std::endl;
		//std::exit(EXIT_FAILURE);
	}

	{
		std::future<tiltfive::Result<void>> fServiceVersion = std::async(std::launch::async, [&]() {
			return printServiceVersion(*client);
			});
		fServiceVersion.wait();
		std::cout << "Print Service Version: " << fServiceVersion.get() << ", " << serviceVersion << std::endl;
	}

	{
		std::future<tiltfive::Result<void>> fStatusFlags = std::async(std::launch::async, [&]() {
			return printUIStatusFlags(*client);
		});
		fStatusFlags.wait();
		std::cout << "Request UI Status Flags Complete" << std::endl;
	}

	{
		auto fGlasses = std::async(std::launch::async, [&]() {
			return waitForGlasses(*client);
		});
		fGlasses.wait();
		auto glasses = fGlasses.get();
		if (glasses) {
			std::cout << "Obtained Glasses: " << glasses << std::endl;
			paramChangeHelper->registerGlasses(*glasses);
			std::cout << "Registered Glasses: " << glasses << std::endl;
			initGlasses(*glasses);
		}
	}
}

auto T5Connector::printGameboardDimensions(Client& client) -> tiltfive::Result<void> {
	auto result = client->getGameboardSize(kT5_GameboardType_XE);
	if (!result) {
		return result.error();
	}

	float width = result->viewableExtentPositiveX - result->viewableExtentNegativeX;
	float length = result->viewableExtentPositiveY - result->viewableExtentNegativeY;
	float height = result->viewableExtentPositiveZ;

	std::cout << "XE Gameboard size: " << width << "m x " << length << "m x " << height << "m" << std::endl;
	boardDimensions[0] = (float)width;
	boardDimensions[1] = (float)length;
	boardDimensions[2] = (float)height;

	return tiltfive::kSuccess;
}

auto T5Connector::printServiceVersion(Client& client) -> tiltfive::Result<void> {
	auto result = client->getServiceVersion();
	if (!result) {
		return result.error();
	}

	std::cout << "Service Version: " << result << std::endl;
	serviceVersion = result->c_str();
	return tiltfive::kSuccess;
}

auto T5Connector::printUIStatusFlags(Client& client) -> tiltfive::Result<void> {
	auto result = client->isTiltFiveUiRequestingAttention();
	if (!result) {
		return result.error();
	}
	std::cout << "TiltFive UI (Attention Requested): " << ((*result) ? "TRUE" : "FALSE") << std::endl;
	return tiltfive::kSuccess;
}

auto T5Connector::waitForGlasses(Client& client) -> tiltfive::Result<Glasses> {
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

auto T5Connector::initGlasses(Glasses& glasses) -> tiltfive::Result<void> {
	std::cout << "Init Glasses: " << glasses << std::endl;

	auto connectionHelper = glasses->createConnectionHelper("TouchDesigner Connector - Player 1");
	auto connectionResult = connectionHelper->awaitConnection(10000_ms);
	if (connectionResult) {
		std::cout << "Glasses connected for exclusive use" << std::endl;
		glassesBound = glasses;
	} else {
		std::cerr << "Error connecting to glasses for exclusive use" << std::endl;
		return connectionResult.error();
	}

	return tiltfive::kSuccess;
}
