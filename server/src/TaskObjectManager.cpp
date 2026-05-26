#include "TaskObjectManager.h"
#include "Database.h"

TaskObjectManager::TaskObjectManager()
{
	//todo 读取数据库

}

TaskPtr TaskObjectManager::getByUuid(const std::string& uuid) const
{
	std::lock_guard<std::mutex> lk(mutex_);
	auto it = entries_.find(uuid);
	if (it == entries_.end()) return nullptr;
	return it->second.obj;
}

TaskPtr TaskObjectManager::getByName(const std::string& name) const
{
	std::lock_guard<std::mutex> lk(mutex_);
	auto it = name_index_.find(name);
	if (it == name_index_.end()) return nullptr;
	return getByUuid(it->second);
}

bool TaskObjectManager::removeByUuid(const std::string& uuid)
{
	std::lock_guard<std::mutex> lk(mutex_);
	auto it = entries_.find(uuid);
	if (it == entries_.end()) return false;
	name_index_.erase(it->second.name);
	entries_.erase(it);
	return true;
}

bool TaskObjectManager::removeByName(const std::string& name)
{
	std::lock_guard<std::mutex> lk(mutex_);
	auto itn = name_index_.find(name);
	if (itn == name_index_.end()) return false;
	std::string uuid = itn->second;
	name_index_.erase(itn);
	entries_.erase(uuid);
	return true;
}

std::vector<std::string> TaskObjectManager::listUuids() const
{
	std::lock_guard<std::mutex> lk(mutex_);
	std::vector<std::string> out;
	out.reserve(entries_.size());
	for (const auto& kv : entries_) out.push_back(kv.first);
	return out;
}

bool TaskObjectManager::initialize(const std::string &taskName, const std::string typeDesign, std::string& initParam, std::string& sysDesign, int timeoutMs)
{
	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return false;
	}
	StartupParam para;
	para.domainId = 101;
	para.typeDesign = typeDesign;
	para.initParam = initParam;
	para.systemDesign = sysDesign;
	return p->initialize(para, timeoutMs);
}

void TaskObjectManager::finalize(const std::string& taskName)
{
	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return ;
	}
	p->finalize();
}

bool TaskObjectManager::setClockType(const std::string& taskName, ClockType type)
{
	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return false;
	}
	return p->setClockType(type);
}

void TaskObjectManager::setClockRange(const std::string& taskName, const TimeSpec& start, const TimeSpec& end)
{

}

void TaskObjectManager::setClockRate(const std::string& taskName, double rate)
{
}

void TaskObjectManager::getClockStatus(const std::string& taskName, ClockStatus* status)
{
}

int64 TaskObjectManager::control(const std::string& taskName, TaskCommand cmd, const bytearray& data)
{
	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return false;
	}
	p->control(cmd, data);
}

int64 TaskObjectManager::postEvent(const std::string& taskName, const std::string& name, const bytearray& data)
{
	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return -1;
	}
	p->postEvent(name, data);
}

void TaskObjectManager::runModel(const std::string& taskName, const UniqueKey& uk, const std::string& designPara, const std::string& initPara)
{

	TaskPtr p = getByName(taskName);
	if (!p)
	{
		return ;
	}
	p->runModel(uk, designPara, initPara);
}

void TaskObjectManager::injectInitPara(const std::string& taskName, const std::string& initPara)
{
}

TaskState TaskObjectManager::state(const std::string& taskName) const
{
	return TaskState();
}

TaskState TaskObjectManager::nextState(const std::string& taskName) const
{
	return TaskState();
}

bool TaskObjectManager::isFinished(const std::string& taskName)
{
	return false;
}

bool TaskObjectManager::queryModelEntity(const std::string& taskName, const std::string& modelUuid, const std::string& modelName, evo::ModelEntity& data)
{
	return false;
}

void TaskObjectManager::setModelStatusReportPeriod(const std::string& taskName, int periodMs, const UniqueKey& uk)
{
}

void TaskObjectManager::requestReportModelStatusOnce(const std::string& taskName, const UniqueKey& uk)
{
}

std::string TaskObjectManager::generate_uuid()
{
	// simple UUID v4-like generator (not RFC-compliant, but unique enough for typical use)
	static thread_local std::mt19937_64 rng(std::random_device{}());
	std::uniform_int_distribution<uint64_t> dist(0, (uint64_t)-1);
	uint64_t a = dist(rng);
	uint64_t b = dist(rng);
	std::ostringstream oss;
	oss << std::hex;
	oss << (a >> 32);
	oss << '-';
	oss << (a & 0xffffffff);
	oss << '-';
	oss << (b >> 32);
	oss << '-';
	oss << (b & 0xffffffff);
	return oss.str();
}

void TaskObjectManager::loadTasksFromDB() {
	auto tasks = Database::getInstance().getAllTasks(-1); // 获取所有任务（管理员可看全部，后续根据用户过滤）
	std::lock_guard<std::mutex> lock(mutex_);
	for (const auto& task : tasks) {
		int id = std::get<0>(task);
		std::string name = std::get<1>(task);
		// 如果内存中不存在，则创建 TaskObject（但不启动实例）
		if (name_index_.find(name) == name_index_.end()) {
			// 仅为工程创建管理对象，不创建实例
			// 可以创建一个轻量级的 TaskPlaceholder
		}
	}
}

std::string TaskObjectManager::getTaskContent(const std::string& taskName) {
	auto tasks = Database::getInstance().getAllTasks(-1);
	for (const auto& t : tasks) {
		if (std::get<1>(t) == taskName) {
			return std::get<4>(t); // content
		}
	}
	return "";
}

void TaskObjectManager::loadTasksFromDB(int userId) {
	auto tasks = Database::getInstance().getAllTasks(userId);
	std::lock_guard<std::mutex> lock(mutex_);
	for (const auto& task : tasks) {
		std::string name = std::get<1>(task);
		if (tasks_.find(name) == tasks_.end()) {
			// 只记录工程名称，不创建运行对象（运行对象是实例）
			tasks_.insert(name);
		}
	}
}

int TaskObjectManager::createTask(
	const std::string& name, const std::string& description,
	const std::string& content, int ownerId,bool isPublic, std::string& log) {
	if (Database::getInstance().getTaskIdByName(name) != -1) {
		log = "Task name already exists: " + name;
		return -1;
	}
	if (!Database::getInstance().insertTask(name, description, ownerId, content,isPublic)) {
		log = "Database insert failed";
		return -1;
	}
	{
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.insert(name);
	}
	return 0;
}

bool TaskObjectManager::updateTask(
	const std::string& name, const std::string& description,
	const std::string& content,bool isPublic, std::string& log) {
	int taskId = Database::getInstance().getTaskIdByName(name);
	if (taskId == -1) {
		log = "Task not found: " + name;
		return false;
	}
	return Database::getInstance().updateTask(taskId, name, description, content, isPublic);
}

bool TaskObjectManager::deleteTask(const std::string& name, std::string& log) {
	int taskId = Database::getInstance().getTaskIdByName(name);
	if (taskId == -1) {
		log = "Task not found: " + name;
		return false;
	}
	if (!Database::getInstance().deleteTask(taskId)) {
		log = "Database delete failed";
		return false;
	}
	{
		std::lock_guard<std::mutex> lock(mutex_);
		tasks_.erase(name);
		// 同时清理该工程下的所有实例（内存中）
		for (auto it = name_index_.begin(); it != name_index_.end();) {
			// 需要知道实例属于哪个工程，简单实现：实例名前缀或额外存储映射，这里略
			++it;
		}
	}
	return true;
}

void TaskObjectManager::removeInstance(const std::string& name)
{
	std::lock_guard<std::mutex> lk(mutex_);
	auto it = name_index_.find(name);
	if (it != name_index_.end()) {
		std::string uuid = it->second;
		entries_.erase(uuid);
		name_index_.erase(it);
		instances_.erase(name);
	}
}

std::vector<std::tuple<std::string, int>> TaskObjectManager::getRunningInstances(const std::string& taskName) const {
	std::lock_guard<std::mutex> lk(mutex_);
	std::vector<std::tuple<std::string, int>> result;
	for (const auto& it : instances_) {
		if (it.second == taskName) {
			// 获取实例状态 (需要 TaskObject 提供 getStatus)
			auto entryIt = name_index_.find(it.first);
			int status = 0; // 默认 running? 可根据实际状态获取
			if (entryIt != name_index_.end()) {
				auto objIt = entries_.find(entryIt->second);
				if (objIt != entries_.end() && objIt->second.obj) {
					// 假设 TaskObject 有 getState 方法
					status = static_cast<int>(objIt->second.obj->state());
				}
			}
			result.emplace_back(it.first, status);
		}
	}
	return result;
}

int TaskObjectManager::createInstance(const std::string& task_name, const std::string& name, std::string& log) {
	std::lock_guard<std::mutex> lk(mutex_);
	// 检查是否已有同名实例
	if (name_index_.count(name)) {
		log = "Instance name already exists: " + name;
		return -1;
	}
	// 获取任务ID
	int taskId = -1;
	auto tasks = Database::getInstance().getAllTasks(-1);
	for (const auto& t : tasks) {
		if (std::get<1>(t) == task_name) {
			taskId = std::get<0>(t);
			break;
		}
	}
	if (taskId == -1) {
		log = "Task not found: " + task_name;
		return -1;
	}
	std::string instanceUuid = generate_uuid();
	// 插入数据库
	//if (!Database::getInstance().insertInstance(taskId, instanceUuid, name, 0)) { // createdBy 需传入当前用户ID，暂用0
	//	log = "Failed to insert instance into database";
	//	return -1;
	//}
	// 创建 TaskObject 实例（实际运行对象）
	TaskStateCallbackerImpl* cb = new TaskStateCallbackerImpl;
	TaskPtr taskObject = std::make_shared<TaskObject>(name, instanceUuid, cb);
	entries_.emplace(instanceUuid, ManagedEntry{ taskObject, name, instanceUuid });
	name_index_[name] = instanceUuid;
	callback_[name] = cb;
	instances_[name] = task_name;
	return 0;
}

void TaskStateCallbackerImpl::systemLogCallback(int severity, const std::string& text)
{
}

void TaskStateCallbackerImpl::modelConnectStateChanged(const UniqueKey& uk, const ModelEntity& data, bool online)
{
}

void TaskStateCallbackerImpl::modelExecuteStateChanged(const UniqueKey& uk, ExceuteState state)
{
}

void TaskStateCallbackerImpl::modelInitDesignResult(const UniqueKey& uk, const ModelInitDesignResult& data)
{
}

void TaskStateCallbackerImpl::modelCommandExecuteReply(const UniqueKey& uk, TaskCommand command, int64 sequenceNo)
{
}

void TaskStateCallbackerImpl::modelCommandExecuteResult(const UniqueKey& uk, const ExecuteResult& data)
{
}

void TaskStateCallbackerImpl::modelEventExecuteResult(const UniqueKey& uk, const ExecuteResult& data)
{
}

void TaskStateCallbackerImpl::taskStateChanged(TaskState current, TaskState previous)
{
}

void TaskStateCallbackerImpl::modelRunRoundCallback(const UniqueKey& uk, const ModelRunRound& data)
{
}

void TaskStateCallbackerImpl::modelIOStatusCallback(const UniqueKey& uk, const ModelIOStatus& data)
{
}

void TaskStateCallbackerImpl::modelCustomStateCallback(const UniqueKey& uk, const ModelCustomState& data)
{
}

void TaskStateCallbackerImpl::modelTransTimeoutWarning(const UniqueKey& uk, const TransTimeoutWaring& data)
{
}

void TaskStateCallbackerImpl::modelCustomLogCallback(const UniqueKey& uk, const LogContext& data)
{
}
