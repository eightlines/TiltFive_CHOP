#pragma once

#include "CHOP_CPlusPlusBase.h"
#include "include/TiltFiveNative.hpp"

#include <future>
#include <thread>
#include <chrono>

using namespace TD;

class T5Connector : public CHOP_CPlusPlusBase
{
public:
	T5Connector(const OP_NodeInfo* info);
	virtual ~T5Connector() {}
	virtual void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void*) override;
	virtual bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
	virtual void getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void* reserved) override;
	virtual void execute(CHOP_Output*, const OP_Inputs*, void* reserved) override;
	virtual int32_t getNumInfoCHOPChans(void* reserved1) override;
	virtual void getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void* reserved1) override;
	virtual bool getInfoDATSize(OP_InfoDATSize* infoSize, void* reserved1) override;
	virtual void getInfoDATEntries(int32_t index, int32_t nEntries, OP_InfoDATEntries* entries, void* reserved1) override;
	virtual void setupParameters(OP_ParameterManager* manager, void* reserved1) override;
	virtual void pulsePressed(const char* name, void* reserved1) override;
private:
	const OP_NodeInfo* myNodeInfo;
	int32_t myExecuteCount;

	using Client = std::shared_ptr<tiltfive::Client>;
	using Glasses = std::shared_ptr<tiltfive::Glasses>;
	using Wand = std::shared_ptr<tiltfive::Wand>;

	int prevActive = -1;
	float boardDimensions[3] = { 0.0, 0.0, 0.0 };
	std::string serviceVersion = "0.0.0"; // 1.3.0
	std::string glassesId;
	float position[3];
	float rotation[4];

	void connectT5();
	auto printGameboardDimensions(Client& client) -> tiltfive::Result<void>;
	auto printServiceVersion(Client& client)->tiltfive::Result<void>;
	auto printUIStatusFlags(Client& client)->tiltfive::Result<void>;
	auto waitForGlasses(Client& client) -> tiltfive::Result<Glasses>;
	auto initGlasses(Glasses& glasses) -> tiltfive::Result<void>;
	auto readPoses(Glasses& glasses) -> tiltfive::Result<void>;
	void updatePosition(T5_Vec3 pos);
	void updateRotation(T5_Quat rot);

	class MyParamChangedListener : public tiltfive::ParamChangeListener {
		auto onSysParamChanged(const std::vector<T5_ParamSys>& changed) -> void override {
			for (const auto& param : changed) {
				std::cout << "Systen parameter changed: [" << param << "]" << std::endl;
			}
		}
		auto onGlassesParamChanged(const Glasses& glasses, const std::vector<T5_ParamGlasses>& changed) -> void override {
			for (const auto& param : changed) {
				std::cout << "Glasses Parameter changed: " << glasses << " => [" << param << "]" << std::endl;
			}
		}
	};
};