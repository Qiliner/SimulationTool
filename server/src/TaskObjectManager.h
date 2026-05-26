#pragma once

#include "TaskObject.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <random>
#include <sstream>
#include<set>

using namespace evo;

//TaskObject 回调
class TaskStateCallbackerImpl :public TaskStateCallbacker
{
	// 通过 TaskStateCallbacker 继承
	void systemLogCallback(int severity, const std::string& text) override;
	void modelConnectStateChanged(const UniqueKey& uk, const ModelEntity& data, bool online) override;
	void modelExecuteStateChanged(const UniqueKey& uk, ExceuteState state) override;
	void modelInitDesignResult(const UniqueKey& uk, const ModelInitDesignResult& data) override;
	void modelCommandExecuteReply(const UniqueKey& uk, TaskCommand command, int64 sequenceNo) override;
	void modelCommandExecuteResult(const UniqueKey& uk, const ExecuteResult& data) override;
	void modelEventExecuteResult(const UniqueKey& uk, const ExecuteResult& data) override;
	void taskStateChanged(TaskState current, TaskState previous) override;
	void modelRunRoundCallback(const UniqueKey& uk, const ModelRunRound& data) override;
	void modelIOStatusCallback(const UniqueKey& uk, const ModelIOStatus& data) override;
	void modelCustomStateCallback(const UniqueKey& uk, const ModelCustomState& data) override;
	void modelTransTimeoutWarning(const UniqueKey& uk, const TransTimeoutWaring& data) override;
	void modelCustomLogCallback(const UniqueKey& uk, const LogContext& data) override;
};

using TaskPtr = std::shared_ptr<TaskObject>;
//using CallBackPtr = std::shared_ptr<TaskStateCallbackerImpl>;

/*区分Task和Instance, Task只对应场景设计文件，Instance对应一个运行的实例，即TaskObject*/
class TaskObjectManager {
public:

	TaskObjectManager();
	// 根据 uuid 获取对象（若不存在返回 nullptr）
	TaskPtr getByUuid(const std::string& uuid) const;
	// 根据 name 获取对象（若不存在返回 nullptr）
	TaskPtr getByName(const std::string& name) const;
	// 删除对象，返回是否成功
	bool removeByUuid(const std::string& uuid);

	bool removeByName(const std::string& name);
	// 列出所有 UUID
	std::vector<std::string> listUuids() const;
	//仿真运行操作
	bool initialize(const std::string &taskName, 
		const std::string typeDesign,
		std::string & initParam, 
		std::string& sysDesign,
		int timeoutMs);

	void finalize(const std::string& taskName);

    bool setClockType(const std::string& taskName, ClockType type);

    void setClockRange(const std::string& taskName, const TimeSpec& start, const TimeSpec& end);

    void setClockRate(const std::string& taskName, double rate);

    void getClockStatus(const std::string& taskName, ClockStatus* status);

    int64 control(const std::string& taskName, TaskCommand cmd, const bytearray& data = bytearray());

    int64 postEvent(const std::string& taskName, const std::string& name, const bytearray& data);

    void runModel(const std::string& taskName, const UniqueKey& uk, const std::string& designPara, const std::string& initPara);

    void injectInitPara(const std::string& taskName, const std::string& initPara);

    TaskState state(const std::string& taskName) const;

    TaskState nextState(const std::string& taskName) const;

    bool isFinished(const std::string& taskName);

    bool queryModelEntity(const std::string& taskName, const std::string& modelUuid, const std::string& modelName, evo::ModelEntity& data);

    void setModelStatusReportPeriod(const std::string& taskName, int periodMs, const UniqueKey& uk = INVALID_UNIQUE_KEY);

    void requestReportModelStatusOnce(const std::string& taskName, const UniqueKey& uk);

	// 新增：从数据库加载所有任务
	void loadTasksFromDB();
	// 获取任务内容
	std::string getTaskContent(const std::string& taskName);
	// 创建实例（同时存入数据库）
	int createInstance(const std::string& task_name, const std::string& name, std::string& log);
	// 删除实例
	bool deleteInstance(const std::string& instanceUuid);

	// 添加数据库同步方法
	void loadTasksFromDB(int userId);
	int createTask(const std::string& name, const std::string& description, const std::string& content, int ownerId, bool isPublic, std::string& log);
	bool updateTask(const std::string& name, const std::string& description, const std::string& content,bool isPublic, std::string& log);
	bool deleteTask(const std::string& name, std::string& log);
	void removeInstance(const std::string& name);
	std::vector<std::tuple<std::string, int>> getRunningInstances(const std::string& taskName) const;
private:
	struct ManagedEntry {
		TaskPtr obj;
		std::string name;
		std::string uuid;
	};

	static std::string generate_uuid();

	mutable std::mutex mutex_;
	std::unordered_map<std::string, ManagedEntry> entries_; // uuid -> entry
	std::unordered_map<std::string, std::string> name_index_; // name -> uuid
	std::unordered_map<std::string, TaskStateCallbackerImpl*> callback_;//name->callback
	std::unordered_map < std::string, std::string> instances_;//instances->task
	std::set<std::string> tasks_;
};

