#include "ModelDesignerServer.h"
#include"rapidxml/rapidxml.hpp"
#include<fstream>
#include <boost/filesystem.hpp>
#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <sstream>
#endif
#include"ZipUtils.h"
#include <sstream>
#include "httplib.h"
#include "Database.h"

using namespace httplib;
using namespace rapidxml;

int getCurrentProjectId(const httplib::Request& req) {
	std::string projectIdStr = req.get_header_value("X-Project-Id");
	if (projectIdStr.empty()) return -1;
	return std::stoi(projectIdStr);
}
int executeCmd(const char* cmd, std::string& result)
{
#ifdef _WIN32
	std::string strCmd = "cmd.exe /c ";
	strCmd += cmd;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	HANDLE g_hChildStd_ERR_Rd = NULL;
	HANDLE g_hChildStd_ERR_Wr = NULL;
	SECURITY_ATTRIBUTES sa;
	// Set the bInheritHandle flag so pipe handles are inherited.
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if (!CreatePipe(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr, &sa, 0)) { return 1; } // Create a pipe for the child process's STDERR.
	if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; } // Ensure the read handle to the pipe for STDERR is not inherited.
	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0)) { return 1; } // Create a pipe for the child process's STDOUT.
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) { return 1; } // Ensure the read handle to the pipe for STDOUT is not inherited
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	bool bSuccess = FALSE;
	// Set up members of the PROCESS_INFORMATION structure.
	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
	// Set up members of the STARTUPINFO structure.
	// This structure specifies the STDERR and STDOUT handles for redirection.
	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_ERR_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
	int wcharSize = MultiByteToWideChar(CP_ACP, 0,
		strCmd.c_str(), -1,
		NULL, 0);
	WCHAR* serverCommandLine = new WCHAR[wcharSize];
	MultiByteToWideChar(CP_ACP, 0,
		strCmd.c_str(), -1,
		serverCommandLine, wcharSize);
	// Create the child process.
	bSuccess = CreateProcess(
		NULL,             // program name
		serverCommandLine,       // command line
		NULL,             // process security attributes
		NULL,             // primary thread security attributes
		TRUE,             // handles are inherited
		CREATE_NO_WINDOW, // creation flags (this is what hides the window)
		NULL,             // use parent's environment
		NULL,             // use parent's current directory
		&siStartInfo,     // STARTUPINFO pointer
		&piProcInfo       // receives PROCESS_INFORMATION
	);
	if (!bSuccess)
	{
		printf("CreateProcess failed %d", GetLastError());
		return 1;
	}
	CloseHandle(g_hChildStd_ERR_Wr);
	CloseHandle(g_hChildStd_OUT_Wr);
	// read output
#define BUFSIZE 4096
	DWORD dwRead;
	CHAR chBuf[BUFSIZE];
	bool bSuccess2 = FALSE;
	for (;;) { // read stdout
		bSuccess2 = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess2 || dwRead == 0) break;
		std::string s(chBuf, dwRead);
		result += s;
	}
	dwRead = 0;
	for (;;) { // read stderr
		bSuccess2 = ReadFile(g_hChildStd_ERR_Rd, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess2 || dwRead == 0) break;
		std::string s(chBuf, dwRead);
		result += s;
	}
	// The remaining open handles are cleaned up when this process terminates.
	// To avoid resource leaks in a larger application,
	// close handles explicitly.
	CloseHandle(g_hChildStd_OUT_Rd);
	CloseHandle(g_hChildStd_ERR_Rd);
	CloseHandle(piProcInfo.hProcess);
	CloseHandle(piProcInfo.hThread);
	return 0;
#else
	// Unix implementation
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		return 1;
	}
	pid_t pid = fork();
	if (pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		return 1;
	}
	if (pid == 0) {
		// Child process
		close(pipefd[0]); // Close read end
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		std::string command(cmd);
		std::vector<std::string> args = { "sh", "-c", command };
		std::vector<char*> argv;
		for (auto& arg : args) {
			argv.push_back(const_cast<char*>(arg.c_str()));
		}
		argv.push_back(nullptr);
		execvp(argv[0], argv.data());
		_exit(1); // If execvp fails
	}
	else {
		// Parent process
		close(pipefd[1]); // Close write end
		char buf[4096];
		ssize_t n;
		while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
			result.append(buf, n);
		}
		close(pipefd[0]);
		int status;
		waitpid(pid, &status, 0);
		return 0;
	}
#endif
}
//保存为文件
int export_to_file(std::string file_name, std::string content)
{
	ofstream f(file_name);
	f.write(content.c_str(), content.length());
	return 0;
}
void get_language(const std::string xml, std::string& language, std::string& model_name)
{
	xml_document<> doc;
	doc.parse<0>((char*)xml.c_str());
	xml_node<>* root = doc.first_node("root");
	//获取语言
	for (xml_node<>* child = root->first_node(); child; child = child->next_sibling())
	{
		std::string name = child->name();
		if (name == "models")
		{
			for (xml_node<>* grandChild = child->first_node(); grandChild;
				grandChild = grandChild->next_sibling())
			{
				name = grandChild->name();
				if (name == "model")
				{
					for (xml_node<>* greatGrandChild = grandChild->first_node();
						greatGrandChild; greatGrandChild = greatGrandChild->next_sibling())
					{
						name = greatGrandChild->name();
						if (name == "info")
						{
							xml_attribute<>* lAttr = greatGrandChild->first_attribute("language");
							{
								language = lAttr->value();
							}
							xml_attribute<>* nameAttr = greatGrandChild->first_attribute("name");
							{
								model_name = nameAttr->value();
							}
						}
					}
				}
			}
		}
	}
	return;
}
ModelDesignerServer::ModelDesignerServer() : m_running(false) {
	m_wsServer.init_asio();
	m_wsServer.set_open_handler(bind(&ModelDesignerServer::onOpen, this, ::_1));
	m_wsServer.set_close_handler(bind(&ModelDesignerServer::onClose, this, ::_1));
	m_wsServer.set_message_handler(bind(&ModelDesignerServer::onMessage, this, ::_1, ::_2));
}
ModelDesignerServer::~ModelDesignerServer() {
	stop();
}
bool ModelDesignerServer::start(int httpPort, int wsPort) {
	if (!Database::getInstance().initialize("model_designer.db")) {
		std::cerr << "Failed to initialize database" << std::endl;
		return false;
	}
	// 创建上传目录
	try {
		boost::filesystem::create_directories("uploads/models");
		boost::filesystem::create_directories("uploads/software");
		std::cout << "Upload directories created" << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Failed to create upload directories: " << e.what() << std::endl;
	}
	setupHttpRoutes();
	m_running = true;
	// 启动 HTTP 服务器（在单独的线程中运行）
	std::thread httpThread([this, httpPort]() {
		std::cout << "HTTP server started on port " << httpPort << std::endl;
		m_httpServer.listen("0.0.0.0", httpPort);
		});
	// 启动 WebSocket 服务器
	m_wsServer.listen(wsPort);
	m_wsServer.start_accept();
	std::cout << "WebSocket server started on port " << wsPort << std::endl;
	m_wsThread = std::thread([this]() {
		m_wsServer.run();
		});
	httpThread.detach();
	return true;
}
void ModelDesignerServer::stop() {
	m_running = false;
	m_httpServer.stop();
	m_wsServer.stop();
	if (m_wsThread.joinable()) {
		m_wsThread.join();
	}
	Database::getInstance().close();
}
void ModelDesignerServer::setupHttpRoutes() {
	// CORS 设置
	//m_httpServer.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
	//	res.set_header("Access-Control-Allow-Origin", "*");
	//	res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
	//	res.set_header("Access-Control-Allow-Headers", "Content-Type");
	//	if (req.method == "OPTIONS") {
	//		res.status = 200;
	//		return httplib::Server::HandlerResponse::Handled;
	//	}
	//	return httplib::Server::HandlerResponse::Unhandled;
	//	});
	m_httpServer.set_pre_routing_handler([this](const httplib::Request& req, httplib::Response& res) {
		// 清除可能已存在的头
		res.headers.erase("Access-Control-Allow-Origin");
		res.headers.erase("Access-Control-Allow-Methods");
		res.headers.erase("Access-Control-Allow-Headers");
		// 设置CORS头
		res.set_header("Access-Control-Allow-Origin", "*");
		res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
		res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Project-Id");
		// 处理OPTIONS预检请求
		if (req.method == "OPTIONS") {
			res.status = 200;
			return httplib::Server::HandlerResponse::Handled;
		}
		return httplib::Server::HandlerResponse::Unhandled;
		});
	//g
	setupProjectRoutes();
	setupModelManageRoutes();
	setupTaskManagerRoutes();
	// ==================== 用户认证和管理路由 ====================
// 用户登录
	m_httpServer.Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
		std::string body = utf8_to_gbk(req.body);
		const char* str = body.c_str();
		try {
			json data = json::parse(req.body);
			std::string username = data["username"].get<std::string>();
			std::string password = data["password"].get<std::string>();
			User user;
			if (Database::getInstance().authenticateUser(username, password, user)) {
				std::string token = generateToken(user);
				json response;
				response["success"] = true;
				response["token"] = token;
				response["user"] = {
					{"id", user.id},
					{"username", user.username},
					{"email", user.email},
					{"role", static_cast<int>(user.role)},
					{"roleName", getRoleName(user.role)}
				};
				string content = response.dump();
				res.set_content(response.dump(), "application/json");
			}
			else {
				json response;
				response["success"] = false;
				response["error"] = "用户名或密码错误";
				res.status = 401;
				res.set_content(response.dump(), "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			printf("exception %s", e.what());
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 获取所有用户 (需要管理员权限)
	m_httpServer.Get("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 检查权限（只有超级管理员和管理员可以查看用户列表）
		if (currentUser.role != UserRole::SUPER_ADMIN && currentUser.role != UserRole::ADMIN) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		auto users = Database::getInstance().getAllUsers();
		json result = json::array();
		for (const auto& u : users) {
			json userJson;
			userJson["id"] = u.id;
			userJson["username"] = u.username;
			userJson["email"] = u.email;
			userJson["role"] = static_cast<int>(u.role);
			userJson["roleName"] = getRoleName(u.role);
			userJson["isActive"] = u.isActive;
			userJson["createdAt"] = u.createdAt;
			userJson["lastLogin"] = u.lastLogin;
			result.push_back(userJson);
		}
		res.set_content(result.dump(), "application/json");
		});

	// 创建用户 (需要管理员权限)
	m_httpServer.Post("/api/users", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		if (currentUser.role != UserRole::SUPER_ADMIN && currentUser.role != UserRole::ADMIN) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			User newUser;
			newUser.username = data["username"].get<std::string>();
			newUser.password = data["password"].get<std::string>();
			newUser.email = data.value("email", "");
			newUser.role = static_cast<UserRole>(data["role"].get<int>());
			newUser.isActive = data.value("isActive", true);
			if (Database::getInstance().insertUser(newUser)) {
				res.status = 201;
				res.set_content(R"({"message": "User created successfully"})", "application/json");
				broadcastUpdate("user_created", newUser);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to create user"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content("{\"error\":" + std::string(e.what()) + "\}", "application / json");
		}
		});
	// 更新用户
	m_httpServer.Put(R"(/api/users/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int userId = std::stoi(req.matches[1]);
		User targetUser = Database::getInstance().getUserById(userId);
		if (targetUser.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "User not found"})", "application/json");
			return;
		}
		// 检查权限
		if (currentUser.role != UserRole::SUPER_ADMIN &&
			(currentUser.role != UserRole::ADMIN || currentUser.id != userId)) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			targetUser.username = data.value("username", targetUser.username);
			if (data.contains("password") && !data["password"].get<std::string>().empty()) {
				targetUser.password = data["password"].get<std::string>();
			}
			targetUser.email = data.value("email", targetUser.email);
			targetUser.role = static_cast<UserRole>(data.value("role", static_cast<int>(targetUser.role)));
			targetUser.isActive = data.value("isActive", targetUser.isActive);
			if (Database::getInstance().updateUser(targetUser)) {
				res.set_content(R"({"message": "User updated successfully"})", "application/json");
				broadcastUpdate("user_updated", targetUser);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to update user"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 删除用户
	m_httpServer.Delete(R"(/api/users/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		if (currentUser.role != UserRole::SUPER_ADMIN) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		int userId = std::stoi(req.matches[1]);
		if (currentUser.id == userId) {
			res.status = 400;
			res.set_content(R"({"error": "Cannot delete yourself"})", "application/json");
			return;
		}
		if (Database::getInstance().deleteUser(userId)) {
			res.set_content(R"({"message": "User deleted successfully"})", "application/json");
			broadcastUpdate("user_deleted", userId);
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete user"})", "application/json");
		}
		});
	// 获取当前用户信息
	m_httpServer.Get("/api/me", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User user;
		if (!verifyToken(token.substr(7), user)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		json response;
		response["id"] = user.id;
		response["username"] = user.username;
		response["email"] = user.email;
		response["role"] = static_cast<int>(user.role);
		response["roleName"] = getRoleName(user.role);
		res.set_content(response.dump(), "application/json");
		});
	// 登出
	m_httpServer.Post("/api/logout", [this](const httplib::Request& req, httplib::Response& res) {
		// 实际应用中可以将token加入黑名单
		res.set_content(R"({"message": "Logged out successfully"})", "application/json");
		});
	// 获取所有数据结构
	m_httpServer.Get("/api/datatypes", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 获取工程ID
		int projectId = getCurrentProjectId(req);
		if (projectId == -1) {
			res.status = 400;
			res.set_content(R"({"error": "Project ID required"})", "application/json");
			return;
		}
		// 检查权限
		if (!Database::getInstance().hasProjectAccess(currentUser.id, projectId)) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		// 使用带 projectId 的版本
		auto dataTypes = Database::getInstance().getAllDataTypes(projectId);
		json result = json::array();
		for (const auto& dt : dataTypes) {
			json members = json::array();
			for (const auto& m : dt.members) {
				json member;
				member["displayName"] = m.displayName;
				member["type"] = m.type;
				if (!m.subType.empty()) member["subType"] = m.subType;
				member["range"] = m.range;
				member["varName"] = m.varName;
				member["defaultValue"] = m.defaultValue;
				member["unit"] = m.unit;
				member["description"] = m.description;
				members.push_back(member);
			}
			json dtJson;
			dtJson["id"] = dt.id;
			dtJson["name"] = dt.name;
			dtJson["description"] = dt.description;
			dtJson["uuid"] = dt.uuid;
			dtJson["members"] = members;
			result.push_back(dtJson);
		}
		res.set_content(result.dump(), "application/json");
		});
	// 获取单个数据结构
	/*m_httpServer.Get(R"(/api/datatypes/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		int id = std::stoi(req.matches[1]);
		auto dt = Database::getInstance().getDataTypeById(id);
		if (dt.id == 0) {
			res.status = 404;
			res.set_content(R"({"error": "Data type not found"})", "application/json");
			return;
		}
		json result;
		result["id"] = dt.id;
		result["name"] = dt.name;
		result["description"] = dt.description;
		result["uuid"] = dt.uuid;
		json members = json::array();
		for (const auto& m : dt.members) {
			json member;
			member["displayName"] = m.displayName;
			member["type"] = m.type;
			if (!m.subType.empty()) member["subType"] = m.subType;
			member["range"] = m.range;
			member["varName"] = m.varName;
			member["defaultValue"] = m.defaultValue;
			member["unit"] = m.unit;
			member["description"] = m.description;
			members.push_back(member);
		}
		result["members"] = members;
		res.set_content(result.dump(), "application/json");
		});*/
		// 创建数据结构
	m_httpServer.Post("/api/datatypes", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 获取工程ID
		int projectId = getCurrentProjectId(req);
		if (projectId == -1) {
			res.status = 400;
			res.set_content(R"({"error": "Project ID required"})", "application/json");
			return;
		}
		// 检查编辑权限
		if (!Database::getInstance().hasProjectEditPermission(currentUser.id, projectId)) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			DataType dt;
			dt.name = data["name"].get<std::string>();
			dt.description = data.value("description", "");
			dt.uuid = data.value("uuid", "");
			if (dt.uuid.empty()) {
				dt.uuid = generateUUID();
			}
			if (data.contains("members")) {
				for (const auto& m : data["members"]) {
					DataTypeMember member;
					member.displayName = m.value("displayName", "");
					member.type = m.value("type", "int32");
					member.subType = m.value("subType", "");
					member.range = m.value("range", "");
					member.varName = m.value("varName", "");
					member.defaultValue = m.value("defaultValue", "");
					member.unit = m.value("unit", "");
					member.description = m.value("description", "");
					dt.members.push_back(member);
				}
			}
			if (Database::getInstance().insertDataType(dt, projectId)) {
				res.status = 201;
				json response = { {"message", "Data type created successfully"}, {"data", dt} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("data_type_created", dt);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to create data type"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 更新数据结构
	m_httpServer.Put(R"(/api/datatypes/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		try {
			int id = std::stoi(req.matches[1]);
			json data = json::parse(req.body);
			DataType dt;
			dt.id = id;
			dt.name = data["name"].get<std::string>();
			dt.description = data.value("description", "");
			dt.uuid = data.value("uuid", "");
			if (data.contains("members")) {
				for (const auto& m : data["members"]) {
					DataTypeMember member;
					member.displayName = m.value("displayName", "");
					member.type = m.value("type", "int32");
					member.subType = m.value("subType", "");
					member.range = m.value("range", "");
					member.varName = m.value("varName", "");
					member.defaultValue = m.value("defaultValue", "");
					member.unit = m.value("unit", "");
					member.description = m.value("description", "");
					dt.members.push_back(member);
				}
			}
			if (Database::getInstance().updateDataType(dt)) {
				json response = { {"message", "Data type updated successfully"}, {"data", dt} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("data_type_updated", dt);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to update data type"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 删除数据结构
	m_httpServer.Delete(R"(/api/datatypes/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		int id = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteDataType(id)) {
			res.set_content(R"({"message": "Data type deleted successfully"})", "application/json");
			broadcastUpdate("data_type_deleted", id);
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete data type"})", "application/json");
		}
		});
	// 获取所有模型
	m_httpServer.Get("/api/models", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 获取工程ID
		int projectId = getCurrentProjectId(req);
		if (projectId == -1) {
			res.status = 400;
			res.set_content(R"({"error": "Project ID required"})", "application/json");
			return;
		}
		// 检查权限
		if (!Database::getInstance().hasProjectAccess(currentUser.id, projectId)) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		// 使用带 projectId 的版本
		auto models = Database::getInstance().getAllModels(projectId);
		json result = json::array();
		for (const auto& model : models) {
			json modelJson;
			modelJson["id"] = model.id;
			modelJson["name"] = model.name;
			modelJson["displayName"] = model.displayName;
			modelJson["uuid"] = model.uuid;
			auto serializeInterfaces = [](const std::vector<ModelInterface>& interfaces) {
				json arr = json::array();
				for (const auto& iface : interfaces) {
					json ifaceJson;
					ifaceJson["name"] = iface.name;
					ifaceJson["dataType"] = iface.dataType;
					ifaceJson["topic"] = iface.topic;
					ifaceJson["displayName"] = iface.displayName;
					ifaceJson["description"] = iface.description;
					arr.push_back(ifaceJson);
				}
				return arr;
				};
			modelJson["paramInterfaces"] = serializeInterfaces(model.paramInterfaces);
			modelJson["inputInterfaces"] = serializeInterfaces(model.inputInterfaces);
			modelJson["outputInterfaces"] = serializeInterfaces(model.outputInterfaces);
			modelJson["initInterfaces"] = serializeInterfaces(model.initInterfaces);
			modelJson["eventInterfaces"] = serializeInterfaces(model.eventInterfaces);
			result.push_back(modelJson);
		}
		res.set_content(result.dump(), "application/json");
		});
	// 创建模型
	m_httpServer.Post("/api/models", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 获取工程ID
		int projectId = getCurrentProjectId(req);
		if (projectId == -1) {
			res.status = 400;
			res.set_content(R"({"error": "Project ID required"})", "application/json");
			return;
		}
		// 检查编辑权限
		if (!Database::getInstance().hasProjectEditPermission(currentUser.id, projectId)) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			Model model;
			model.name = data["name"].get<std::string>();
			model.displayName = data.value("displayName", "");
			model.uuid = data.value("uuid", "");
			model.projectId = projectId;
			if (model.uuid.empty()) {
				model.uuid = generateUUID();
			}
			auto deserializeInterfaces = [](const json& arr) {
				std::vector<ModelInterface> interfaces;
				for (const auto& item : arr) {
					ModelInterface iface;
					iface.name = item.value("name", "");
					iface.dataType = item.value("dataType", "int32");
					iface.topic = item.value("topic", "");
					iface.displayName = item.value("displayName", "");
					iface.description = item.value("description", "");
					interfaces.push_back(iface);
				}
				return interfaces;
				};
			model.paramInterfaces = deserializeInterfaces(data["paramInterfaces"]);
			model.inputInterfaces = deserializeInterfaces(data["inputInterfaces"]);
			model.outputInterfaces = deserializeInterfaces(data["outputInterfaces"]);
			model.initInterfaces = deserializeInterfaces(data["initInterfaces"]);
			model.eventInterfaces = deserializeInterfaces(data["eventInterfaces"]);
			if (Database::getInstance().insertModel(model)) {
				res.status = 201;
				json response = { {"message", "Model created successfully"}, {"data", model} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("model_created", model);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to create model"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 更新模型
	m_httpServer.Put(R"(/api/models/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		try {
			int id = std::stoi(req.matches[1]);
			json data = json::parse(req.body);
			Model model;
			model.id = id;
			model.name = data["name"].get<std::string>();
			model.displayName = data.value("displayName", "");
			model.uuid = data.value("uuid", "");
			auto deserializeInterfaces = [](const json& arr) {
				std::vector<ModelInterface> interfaces;
				for (const auto& item : arr) {
					ModelInterface iface;
					iface.name = item.value("name", "");
					iface.dataType = item.value("dataType", "int32");
					iface.topic = item.value("topic", "");
					iface.displayName = item.value("displayName", "");
					iface.description = item.value("description", "");
					interfaces.push_back(iface);
				}
				return interfaces;
				};
			model.paramInterfaces = deserializeInterfaces(data["paramInterfaces"]);
			model.inputInterfaces = deserializeInterfaces(data["inputInterfaces"]);
			model.outputInterfaces = deserializeInterfaces(data["outputInterfaces"]);
			model.initInterfaces = deserializeInterfaces(data["initInterfaces"]);
			model.eventInterfaces = deserializeInterfaces(data["eventInterfaces"]);
			if (Database::getInstance().updateModel(model)) {
				json response = { {"message", "Model updated successfully"}, {"data", model} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("model_updated", model);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to update model"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 删除模型
	m_httpServer.Delete(R"(/api/models/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		int id = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteModel(id)) {
			res.set_content(R"({"message": "Model deleted successfully"})", "application/json");
			broadcastUpdate("model_deleted", id);
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete model"})", "application/json");
		}
		});
	// 同步所有数据
	m_httpServer.Post("/api/sync", [this](const httplib::Request& req, httplib::Response& res) {
		try {
			json data = json::parse(req.body);
			// 同步数据结构
			std::vector<DataType> dataTypes;
			for (const auto& dtJson : data["dataTypes"]) {
				DataType dt;
				dt.name = dtJson["name"].get<std::string>();
				dt.description = dtJson.value("description", "");
				dt.uuid = dtJson.value("uuid", generateUUID());
				for (const auto& m : dtJson["members"]) {
					DataTypeMember member;
					member.displayName = m.value("displayName", "");
					member.type = m.value("type", "int32");
					member.subType = m.value("subType", "");
					member.range = m.value("range", "");
					member.varName = m.value("varName", "");
					member.defaultValue = m.value("defaultValue", "");
					member.unit = m.value("unit", "");
					member.description = m.value("description", "");
					dt.members.push_back(member);
				}
				dataTypes.push_back(dt);
			}
			// 同步模型
			std::vector<Model> models;
			for (const auto& modelJson : data["models"]) {
				Model model;
				model.name = modelJson["name"].get<std::string>();
				model.displayName = modelJson.value("displayName", "");
				model.uuid = modelJson.value("uuid", generateUUID());
				auto deserializeInterfaces = [](const json& arr) {
					std::vector<ModelInterface> interfaces;
					for (const auto& item : arr) {
						ModelInterface iface;
						iface.name = item.value("name", "");
						iface.dataType = item.value("dataType", "int32");
						iface.topic = item.value("topic", "");
						iface.displayName = item.value("displayName", "");
						iface.description = item.value("description", "");
						interfaces.push_back(iface);
					}
					return interfaces;
					};
				model.paramInterfaces = deserializeInterfaces(modelJson["paramInterfaces"]);
				model.inputInterfaces = deserializeInterfaces(modelJson["inputInterfaces"]);
				model.outputInterfaces = deserializeInterfaces(modelJson["outputInterfaces"]);
				model.initInterfaces = deserializeInterfaces(modelJson["initInterfaces"]);
				model.eventInterfaces = deserializeInterfaces(modelJson["eventInterfaces"]);
				models.push_back(model);
			}
			Database::getInstance().syncDataTypes(dataTypes);
			Database::getInstance().syncModels(models);
			res.set_content(R"({"message": "Sync completed successfully"})", "application/json");
			broadcastUpdate("sync_completed", "all");
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	//代码生成
	m_httpServer.Post("/generate", [this](const httplib::Request& req, httplib::Response& res) {
		string content = utf8_to_gbk(req.body);
		//获取生成的语言信息
		std::string curPath = boost::filesystem::current_path().string();
		std::string language, model_name;
		get_language(req.body, language, model_name);
		std::string style = language;
		std::string output_dir = curPath + "/output/" + model_name;
		std::string model_file = output_dir + "/Model.xml";
		std::string projectCmd = curPath + "/ProjectGenerator.exe" +
			string(" --input=") + model_file +
			string(" --output=") + output_dir +
			/*string(" --language=") + language +*/
			string(" --object=") + model_name +
			string(" --array=1") +
			string(" --solution=1") +
			string(" --active=1");
		std::string result;
		export_to_file(model_file, req.body);
		if (language == "python" || language == "matlab")
		{
			style = "cstyle";
			std::string interfaceCmd = curPath + "/InterfaceGenerator.exe" +
				" -i " + model_file + " -d" + output_dir + "-l" +
				(language == "python" ? "python" : "matlab");
			executeCmd(interfaceCmd.c_str(), result);
			std::cout << "生成命令:" << interfaceCmd << endl << "结果：" << result << endl;
		}
		projectCmd += string(" --language=") + style;
		executeCmd(projectCmd.c_str(), result);
		std::cout << "生成命令:" << projectCmd << endl << "结果：" << result << endl;
		//todo 拷贝sdK
		// 打包为ZIP文件
		std::string zipFileName = model_name + "_code_" + std::to_string(time(nullptr)) + ".zip";
		std::string zipFilePath = curPath + "/" + zipFileName;
		if (!ZipUtils::zipFolder(output_dir, zipFilePath))
		{
			std::cout << "压缩文件失败:";
			res.status = 400;
			res.set_content(R"({"error": "zip file failed"})", "application/json");
		}
		else
		{
			std::ifstream zipFile(zipFilePath, std::ios::binary);
			if (!zipFile.is_open()) {
				res.status = 500;
				res.set_content(R"({"error": "Failed to read zip file"})", "application/json");
				// 清理临时目录
				//boost::filesystem::remove_all(tempDir);
				boost::filesystem::remove(zipFilePath);
				return;
			}
			std::stringstream buffer;
			buffer << zipFile.rdbuf();
			std::string zipContent = buffer.str();
			zipFile.close();
			// 设置响应头为ZIP文件
			res.set_header("Content-Type", "application/zip");
			res.set_header("Content-Disposition", ("attachment; filename=\"" + zipFileName + "\"").c_str());
			res.set_content(zipContent, "application/zip");
			// 清理临时文件
			/*boost::filesystem::remove_all(tempDir);
			boost::filesystem::remove(zipFilePath);*/
			std::cout << "Code generation completed for model: " << model_name << std::endl;
		}
		});
}
void ModelDesignerServer::setupProjectRoutes()
{
	// ==================== 工程管理路由 ====================
// 获取工程列表
	m_httpServer.Get("/api/projects", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		auto projects = Database::getInstance().getAllProjects(currentUser.id);
		json result = json::array();
		for (const auto& p : projects) {
			json projectJson;
			projectJson["id"] = p.id;
			projectJson["name"] = p.name;
			projectJson["description"] = p.description;
			projectJson["ownerId"] = p.ownerId;
			projectJson["ownerName"] = p.ownerName;
			projectJson["createdAt"] = p.createdAt;
			projectJson["updatedAt"] = p.updatedAt;
			projectJson["isPublic"] = p.isPublic;
			// 解析工程数据
			if (!p.data.empty()) {
				try {
					projectJson["data"] = json::parse(p.data);
				}
				catch (...) {
					projectJson["data"] = json::object();
				}
			}
			else {
				projectJson["data"] = json::object();
			}
			result.push_back(projectJson);
		}
		res.set_content(result.dump(), "application/json");
		});
	// 获取单个工程
	m_httpServer.Get(R"(/api/projects/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int projectId = std::stoi(req.matches[1]);
		auto project = Database::getInstance().getProjectById(projectId);
		if (project.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "Project not found"})", "application/json");
			return;
		}
		// 检查权限
		if (project.ownerId != currentUser.id && !project.isPublic) {
			auto members = Database::getInstance().getProjectMembers(projectId);
			bool hasAccess = false;
			for (const auto& m : members) {
				if (m.userId == currentUser.id) {
					hasAccess = true;
					break;
				}
			}
			if (!hasAccess) {
				res.status = 403;
				res.set_content(R"({"error": "Forbidden"})", "application/json");
				return;
			}
		}
		json result;
		result["id"] = project.id;
		result["name"] = project.name;
		result["description"] = project.description;
		result["ownerId"] = project.ownerId;
		result["ownerName"] = project.ownerName;
		result["createdAt"] = project.createdAt;
		result["updatedAt"] = project.updatedAt;
		result["isPublic"] = project.isPublic;
		if (!project.data.empty()) {
			try {
				result["data"] = json::parse(project.data);
			}
			catch (...) {
				result["data"] = json::object();
			}
		}
		else {
			result["data"] = json::object();
		}
		res.set_content(result.dump(), "application/json");
		});
	// 创建工程
	m_httpServer.Post("/api/projects", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			Project project;
			project.name = data["name"].get<std::string>();
			project.description = data.value("description", "");
			project.ownerId = currentUser.id;
			project.isPublic = data.value("isPublic", false);
			if (data.contains("data")) {
				project.data = data["data"].dump();
			}
			if (Database::getInstance().insertProject(project)) {
				res.status = 201;
				json response = { {"message", "Project created successfully"} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("project_created", project);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to create project"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 更新工程
	m_httpServer.Put(R"(/api/projects/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int projectId = std::stoi(req.matches[1]);
		auto project = Database::getInstance().getProjectById(projectId);
		if (project.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "Project not found"})", "application/json");
			return;
		}
		// 只有拥有者可以编辑
		if (project.ownerId != currentUser.id) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			project.name = data.value("name", project.name);
			project.description = data.value("description", project.description);
			project.isPublic = data.value("isPublic", project.isPublic);
			if (data.contains("data")) {
				project.data = data["data"].dump();
			}
			if (Database::getInstance().updateProject(project)) {
				json response = { {"message", "Project updated successfully"} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("project_updated", project);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to update project"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 删除工程
	m_httpServer.Delete(R"(/api/projects/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int projectId = std::stoi(req.matches[1]);
		auto project = Database::getInstance().getProjectById(projectId);
		if (project.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "Project not found"})", "application/json");
			return;
		}
		// 只有拥有者可以删除
		if (project.ownerId != currentUser.id) {
			res.status = 403;
			res.set_content(R"({"error": "Forbidden"})", "application/json");
			return;
		}
		if (Database::getInstance().deleteProject(projectId)) {
			res.set_content(R"({"message": "Project deleted successfully"})", "application/json");
			broadcastUpdateInt("project_deleted", projectId);
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete project"})", "application/json");
		}
		});
	// 保存工程数据
	m_httpServer.Post(R"(/api/projects/(\d+)/save)", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int projectId = std::stoi(req.matches[1]);
		auto project = Database::getInstance().getProjectById(projectId);
		if (project.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "Project not found"})", "application/json");
			return;
		}
		// 检查权限（拥有者或编辑者）
		if (project.ownerId != currentUser.id) {
			auto members = Database::getInstance().getProjectMembers(projectId);
			bool canEdit = false;
			for (const auto& m : members) {
				if (m.userId == currentUser.id && m.permission <= 1) {
					canEdit = true;
					break;
				}
			}
			if (!canEdit) {
				res.status = 403;
				res.set_content(R"({"error": "Forbidden"})", "application/json");
				return;
			}
		}
		try {
			json data = json::parse(req.body);
			project.data = data.dump();
			if (Database::getInstance().updateProject(project)) {
				json response = { {"message", "Project saved successfully"} };
				res.set_content(response.dump(), "application/json");
				broadcastUpdate("project_saved", project);
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to save project"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
}
void ModelDesignerServer::setupModelManageRoutes()
{
	// ==================== 模型库管理路由 ====================
	// 获取模型库列表
	m_httpServer.Get("/api/model/libs", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		auto libs = Database::getInstance().getAllModelLibs();
		json result = json::array();
		for (const auto& lib : libs) {
			result.push_back({ {"id", lib.first}, {"name", lib.second} });
		}
		res.set_content(result.dump(), "application/json");
		});
	// 获取目录树
	m_httpServer.Get("/api/model/directories", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int libId = 0;
		int parentId = 0;
		auto libIt = req.params.find("libId");
		if (libIt != req.params.end()) {
			libId = std::stoi(libIt->second);
		}
		auto dirIt = req.params.find("parentId");
		if (dirIt != req.params.end()) {
			parentId = std::stoi(dirIt->second);
		}
		if (libId == 0) {
			res.status = 400;
			res.set_content(R"({"error": "libId is required"})", "application/json");
			return;
		}
		auto dirs = Database::getInstance().getDirectories(libId, parentId);
		json result = json::array();
		for (const auto& dir : dirs) {
			result.push_back({
				{"id", dir.id},
				{"parentId", dir.parentId},
				{"modelLibId", dir.modelLibId},
				{"name", dir.name},
				{"path", dir.path},
				{"createdAt", dir.createdAt},
				{"updatedAt", dir.updatedAt}
				});
		}
		res.set_content(result.dump(), "application/json");
		});
	// 创建目录
	m_httpServer.Post("/api/model/directories", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			int parentId = data.value("parentId", 0);
			int libId = data["libId"].get<int>();
			std::string name = data["name"].get<std::string>();
			int dirId = Database::getInstance().createDirectory(parentId, libId, name);
			if (dirId > 0) {
				json response = { {"message", "Directory created successfully"}, {"id", dirId} };
				res.set_content(response.dump(), "application/json");
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to create directory"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 重命名目录
	m_httpServer.Put(R"(/api/model/directories/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int dirId = std::stoi(req.matches[1]);
		try {
			json data = json::parse(req.body);
			std::string newName = data["name"].get<std::string>();
			if (Database::getInstance().renameDirectory(dirId, newName)) {
				res.set_content(R"({"message": "Directory renamed successfully"})", "application/json");
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to rename directory"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// 删除目录
	m_httpServer.Delete(R"(/api/model/directories/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int dirId = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteDirectory(dirId)) {
			res.set_content(R"({"message": "Directory deleted successfully"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete directory"})", "application/json");
		}
		});
	// 获取模型文件列表
	m_httpServer.Get("/api/model/files", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int libId = 0;
		int dirId = -1;  // 改为 -1 表示未指定
		auto libIt = req.params.find("libId");
		if (libIt != req.params.end()) {
			libId = std::stoi(libIt->second);
		}
		auto dirIt = req.params.find("dirId");
		if (dirIt != req.params.end()) {
			dirId = std::stoi(dirIt->second);
		}
		if (libId == 0) {
			res.status = 400;
			res.set_content(R"({"error": "libId is required"})", "application/json");
			return;
		}
		std::vector<ModelFile> files;
		if (dirId == -1) {
			// 如果没有传 dirId，返回整个库的所有文件
			files = Database::getInstance().getAllModelFilesByLib(libId);
		}
		else {
			// 如果传了 dirId，只返回指定目录的文件
			files = Database::getInstance().getModelFiles(libId, dirId);
		}
		json result = json::array();
		for (const auto& file : files) {
			result.push_back({
				{"id", file.id},
				{"directoryId", file.directoryId},
				{"modelLibId", file.modelLibId},
				{"name", file.name},
				{"fileName", file.fileName},
				{"version", file.version},
				{"startCmd", file.startCmd},
				{"stopCmd", file.stopCmd},
				{"description", file.description},
				{"fileSize", file.fileSize},
				{"fileType", file.fileType},
				{"uploaderId", file.uploaderId},
				{"uploaderName", file.uploaderName},
				{"createdAt", file.createdAt},
				{"updatedAt", file.updatedAt}
				});
		}
		std::string str = result.dump();
		res.set_content(result.dump(), "application/json");
		});
	// 上传模型文件 - 使用 httplib 内置的 form 解析
	m_httpServer.Post("/api/model/files", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 检查是否是 multipart/form-data
		if (!req.is_multipart_form_data()) {
			res.status = 400;
			res.set_content(R"({"error": "Expected multipart/form-data"})", "application/json");
			return;
		}
		// 获取表单字段（普通文本字段）
		std::string name = req.form.get_field("name");
		std::string version = req.form.get_field("version");
		std::string startCmd = req.form.get_field("startCmd");
		std::string stopCmd = req.form.get_field("stopCmd");
		std::string description = req.form.get_field("description");
		// 获取 libId 和 directoryId
		int libId = 1;
		int dirId = 0;
		std::string libIdStr = req.form.get_field("libId");
		if (!libIdStr.empty()) {
			try {
				libId = std::stoi(libIdStr);
			}
			catch (...) {
				libId = 1;
			}
		}
		std::string dirIdStr = req.form.get_field("directoryId");
		if (!dirIdStr.empty()) {
			try {
				dirId = std::stoi(dirIdStr);
			}
			catch (...) {
				dirId = 0;
			}
		}
		// 获取上传的文件
		if (!req.form.has_file("file")) {
			res.status = 400;
			res.set_content(R"({"error": "No file uploaded"})", "application/json");
			return;
		}
		// 使用 httplib::FormData
		httplib::FormData fileData = req.form.get_file("file");
		std::string filename = fileData.filename;
		std::string fileContent = fileData.content;
		std::cout << "File received: " << filename << ", size: " << fileContent.size() << std::endl;
		std::cout << "Form fields - name: " << name << ", libId: " << libId << ", dirId: " << dirId << std::endl;
		// 保存文件
		std::string uploadDir = boost::filesystem::current_path().string() + "/uploads/models/";
		boost::filesystem::create_directories(uploadDir);
		// 生成唯一文件名
		std::string uniqueFileName = std::to_string(std::time(nullptr)) + "_" + filename;
		size_t lastSlash = uniqueFileName.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			uniqueFileName = uniqueFileName.substr(lastSlash + 1);
		}
		std::string fullPath = uploadDir + uniqueFileName;
		std::cout << "Saving file to: " << fullPath << std::endl;
		// 写入文件
		std::ofstream ofs(fullPath, std::ios::binary);
		if (!ofs.is_open()) {
			std::cerr << "Failed to open file for writing: " << fullPath << std::endl;
			res.status = 500;
			res.set_content(R"({"error": "Failed to create file"})", "application/json");
			return;
		}
		ofs.write(fileContent.c_str(), fileContent.size());
		ofs.close();
		std::cout << "File saved successfully, size: " << fileContent.size() << std::endl;
		// 保存到数据库
		ModelFile modelFile;
		modelFile.directoryId = dirId;
		modelFile.modelLibId = libId;
		modelFile.name = name.empty() ? filename : name;
		modelFile.fileName = uniqueFileName;
		modelFile.filePath = fullPath;
		modelFile.version = version;
		modelFile.startCmd = startCmd;
		modelFile.stopCmd = stopCmd;
		modelFile.description = description;
		modelFile.fileSize = fileContent.size();
		size_t dotPos = filename.find_last_of('.');
		modelFile.fileType = (dotPos != std::string::npos) ? filename.substr(dotPos) : "";
		modelFile.uploaderId = currentUser.id;
		modelFile.uploaderName = currentUser.username;
		int fileId = Database::getInstance().addModelFile(modelFile);
		if (fileId > 0) {
			json response = {
				{"message", "File uploaded successfully"},
				{"id", fileId},
				{"fileName", uniqueFileName}
			};
			res.set_content(response.dump(), "application/json");
			std::cout << "File uploaded successfully: " << uniqueFileName << " to lib " << libId << std::endl;
		}
		else {
			boost::filesystem::remove(fullPath);
			std::cerr << "Failed to save to database" << std::endl;
			res.status = 500;
			res.set_content(R"({"error": "Failed to save file info to database"})", "application/json");
		}
		});
	// 下载模型文件
	m_httpServer.Get(R"(/api/model/files/(\d+)/download)", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int fileId = std::stoi(req.matches[1]);
		auto modelFile = Database::getInstance().getModelFileById(fileId);
		if (modelFile.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "File not found"})", "application/json");
			return;
		}
		if (!boost::filesystem::exists(modelFile.filePath)) {
			res.status = 404;
			res.set_content(R"({"error": "File not found on disk"})", "application/json");
			return;
		}
		std::ifstream file(modelFile.filePath, std::ios::binary);
		if (!file.is_open()) {
			res.status = 500;
			res.set_content(R"({"error": "Failed to open file"})", "application/json");
			return;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();
		res.set_header("Content-Type", "application/octet-stream");
		res.set_header("Content-Disposition", ("attachment; filename=\"" + modelFile.fileName + "\"").c_str());
		res.set_content(content, "application/octet-stream");
		});
	// 删除模型文件
	m_httpServer.Delete(R"(/api/model/files/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int fileId = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteModelFile(fileId)) {
			res.set_content(R"({"message": "File deleted successfully"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete file"})", "application/json");
		}
		});
	// 迁移模型文件
	m_httpServer.Post(R"(/api/model/files/(\d+)/migrate)", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int fileId = std::stoi(req.matches[1]);
		try {
			json data = json::parse(req.body);
			int targetLibId = data["targetLibId"].get<int>();
			int targetDirId = data.value("targetDirId", 0);
			if (Database::getInstance().migrateModelFile(fileId, targetLibId, targetDirId)) {
				res.set_content(R"({"message": "File migrated successfully"})", "application/json");
			}
			else {
				res.status = 500;
				res.set_content(R"({"error": "Failed to migrate file"})", "application/json");
			}
		}
		catch (const std::exception& e) {
			res.status = 400;
			res.set_content(R"({"error": "Invalid request body"})", "application/json");
		}
		});
	// ==================== 软件管理路由 ====================
	// 获取软件列表
	m_httpServer.Get("/api/software", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		auto software = Database::getInstance().getAllSoftware();
		json result = json::array();
		for (const auto& sw : software) {
			result.push_back({
				{"id", sw.id},
				{"name", sw.name},
				{"fileName", sw.fileName},
				{"version", sw.version},
				{"installCmd", sw.installCmd},
				{"uninstallCmd", sw.uninstallCmd},
				{"startCmd", sw.startCmd},
				{"stopCmd", sw.stopCmd},
				{"description", sw.description},
				{"fileSize", sw.fileSize},
				{"fileType", sw.fileType},
				{"uploaderId", sw.uploaderId},
				{"uploaderName", sw.uploaderName},
				{"createdAt", sw.createdAt},
				{"updatedAt", sw.updatedAt}
				});
		}
		res.set_content(result.dump(), "application/json");
		});
	// 上传模型文件（修复 multipart 解析）
	m_httpServer.Post("/api/model/files", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		// 检查 Content-Type
		std::string contentType = req.get_header_value("Content-Type");
		if (contentType.find("multipart/form-data") == std::string::npos) {
			res.status = 400;
			res.set_content(R"({"error": "Expected multipart/form-data"})", "application/json");
			return;
		}
		// 解析 boundary
		std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
		if (boundary.front() == '"') boundary.erase(0, 1);
		if (boundary.back() == '"') boundary.pop_back();
		std::string body = req.body;
		// 解析表单字段
		std::map<std::string, std::string> formFields;
		std::string fileContent;
		std::string fileName;
		std::string fileFieldName;
		std::string boundaryDelimiter = "--" + boundary;
		size_t pos = 0;
		std::cout << "=== Starting multipart parsing ===" << std::endl;
		std::cout << "Boundary: " << boundary << std::endl;
		std::cout << "Body size: " << body.size() << std::endl;
		while (true) {
			size_t start = body.find(boundaryDelimiter, pos);
			if (start == std::string::npos) break;
			start += boundaryDelimiter.length();
			// 检查是否是结束边界
			if (body.substr(start, 2) == "--") break;
			// 跳过可能的 \r\n
			if (body.substr(start, 2) == "\r\n") {
				start += 2;
			}
			size_t end = body.find(boundaryDelimiter, start);
			if (end == std::string::npos) break;
			// 移除末尾的 \r\n
			size_t partEnd = end;
			while (partEnd > start && (body[partEnd - 1] == '\r' || body[partEnd - 1] == '\n')) {
				partEnd--;
			}
			std::string part = body.substr(start, partEnd - start);
			// 查找头部结束位置
			size_t headerEnd = part.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				pos = end;
				continue;
			}
			std::string headers = part.substr(0, headerEnd);
			std::string content = part.substr(headerEnd + 4);
			// 解析字段名
			std::string name;
			size_t namePos = headers.find("name=\"");
			if (namePos != std::string::npos) {
				namePos += 6;
				size_t nameEnd = headers.find("\"", namePos);
				name = headers.substr(namePos, nameEnd - namePos);
			}
			// 检查是否是文件
			size_t filenamePos = headers.find("filename=\"");
			if (filenamePos != std::string::npos) {
				// 这是文件部分
				filenamePos += 10;
				size_t filenameEnd = headers.find("\"", filenamePos);
				fileName = headers.substr(filenamePos, filenameEnd - filenamePos);
				fileContent = content;
				fileFieldName = name;
				std::cout << "Found file field: " << name << ", filename: " << fileName << ", size: " << fileContent.size() << std::endl;
			}
			else {
				// 这是普通表单字段，去除末尾的换行符
				while (!content.empty() && (content.back() == '\r' || content.back() == '\n')) {
					content.pop_back();
				}
				formFields[name] = content;
				std::cout << "Found form field: " << name << " = [" << content << "]" << std::endl;
			}
			pos = end;
		}
		// 调试输出所有表单字段
		std::cout << "=== Form fields ===" << std::endl;
		for (const auto& field : formFields) {
			std::cout << field.first << " = [" << field.second << "]" << std::endl;
		}
		// 获取表单数据
		std::string name = formFields["name"];
		std::string version = formFields["version"];
		std::string startCmd = formFields["startCmd"];
		std::string stopCmd = formFields["stopCmd"];
		std::string description = formFields["description"];
		// 安全转换 libId 和 directoryId
		int libId = 1;
		int dirId = 0;
		auto libIt = formFields.find("libId");
		if (libIt != formFields.end() && !libIt->second.empty()) {
			try {
				libId = std::stoi(libIt->second);
				std::cout << "libId from form: " << libId << std::endl;
			}
			catch (...) {
				libId = 1;
			}
		}
		auto dirIt = formFields.find("directoryId");
		if (dirIt != formFields.end() && !dirIt->second.empty()) {
			try {
				dirId = std::stoi(dirIt->second);
				std::cout << "directoryId from form: " << dirId << std::endl;
			}
			catch (...) {
				dirId = 0;
			}
		}
		std::cout << "Final values - name: [" << name << "], libId: " << libId << ", dirId: " << dirId << std::endl;
		// 检查是否有文件
		if (fileContent.empty() || fileName.empty()) {
			std::cerr << "No file uploaded - fileContent empty: " << fileContent.empty() << ", fileName empty: " << fileName.empty() << std::endl;
			res.status = 400;
			res.set_content(R"({"error": "No file uploaded"})", "application/json");
			return;
		}
		// 保存文件
		std::string uploadDir = boost::filesystem::current_path().string() + "/uploads/models/";
		boost::filesystem::create_directories(uploadDir);
		// 生成唯一文件名
		std::string uniqueFileName = std::to_string(std::time(nullptr)) + "_" + fileName;
		// 清理文件名中的路径字符
		size_t lastSlash = uniqueFileName.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			uniqueFileName = uniqueFileName.substr(lastSlash + 1);
		}
		std::string fullPath = uploadDir + uniqueFileName;
		std::cout << "Saving file to: " << fullPath << std::endl;
		// 写入文件
		std::ofstream ofs(fullPath, std::ios::binary);
		if (!ofs.is_open()) {
			std::cerr << "Failed to open file for writing: " << fullPath << std::endl;
			res.status = 500;
			res.set_content(R"({"error": "Failed to create file"})", "application/json");
			return;
		}
		ofs.write(fileContent.c_str(), fileContent.size());
		ofs.close();
		std::cout << "File saved successfully, size: " << fileContent.size() << std::endl;
		// 保存到数据库
		ModelFile modelFile;
		modelFile.directoryId = dirId;
		modelFile.modelLibId = libId;
		modelFile.name = name.empty() ? fileName : name;
		modelFile.fileName = uniqueFileName;
		modelFile.filePath = fullPath;
		modelFile.version = version;
		modelFile.startCmd = startCmd;
		modelFile.stopCmd = stopCmd;
		modelFile.description = description;
		modelFile.fileSize = fileContent.size();
		// 获取文件扩展名
		size_t dotPos = fileName.find_last_of('.');
		modelFile.fileType = (dotPos != std::string::npos) ? fileName.substr(dotPos) : "";
		modelFile.uploaderId = currentUser.id;
		modelFile.uploaderName = currentUser.username;
		int fileId = Database::getInstance().addModelFile(modelFile);
		if (fileId > 0) {
			json response = {
				{"message", "File uploaded successfully"},
				{"id", fileId},
				{"fileName", uniqueFileName}
			};
			res.set_content(response.dump(), "application/json");
			std::cout << "File uploaded successfully: " << uniqueFileName << " to lib " << libId << std::endl;
		}
		else {
			// 删除已保存的文件
			boost::filesystem::remove(fullPath);
			std::cerr << "Failed to save to database" << std::endl;
			res.status = 500;
			res.set_content(R"({"error": "Failed to save file info to database"})", "application/json");
		}
		});
	// 下载软件
	m_httpServer.Get(R"(/api/software/(\d+)/download)", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int swId = std::stoi(req.matches[1]);
		auto software = Database::getInstance().getSoftwareById(swId);
		if (software.id == -1) {
			res.status = 404;
			res.set_content(R"({"error": "Software not found"})", "application/json");
			return;
		}
		if (!boost::filesystem::exists(software.filePath)) {
			res.status = 404;
			res.set_content(R"({"error": "File not found on disk"})", "application/json");
			return;
		}
		std::ifstream file(software.filePath, std::ios::binary);
		if (!file.is_open()) {
			res.status = 500;
			res.set_content(R"({"error": "Failed to open file"})", "application/json");
			return;
		}
		std::stringstream buffer;
		buffer << file.rdbuf();
		std::string content = buffer.str();
		file.close();
		res.set_header("Content-Type", "application/octet-stream");
		res.set_header("Content-Disposition", ("attachment; filename=\"" + software.fileName + "\"").c_str());
		res.set_content(content, "application/octet-stream");
		});
	// 删除软件
	m_httpServer.Delete(R"(/api/software/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		int swId = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteSoftware(swId)) {
			res.set_content(R"({"message": "Software deleted successfully"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Failed to delete software"})", "application/json");
		}
		});
}
void ModelDesignerServer::setupTaskManagerRoutes()
{
	//获取工程列表
	m_httpServer.Get("/api/tasks", [this](const httplib::Request& req, httplib::Response& res) {
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		auto tasks = Database::getInstance().getTasksForUser(currentUser.id);
		json result = json::array();
		for (const auto& t : tasks) {
			int id = std::get<0>(t);
			std::string name = std::get<1>(t);
			std::string description = std::get<2>(t);
			int ownerId = std::get<3>(t);
			int updatedAt = std::get<5>(t);
			int isPublic = std::get<6>(t);

			// 获取该工程下的运行实例
			auto instances = m_taskManager.getRunningInstances(name);
			json instancesArr = json::array();
			for (const auto& inst : instances) {
				std::string instanceName = std::get<0>(inst);
				int status = std::get<1>(inst);
				instancesArr.push_back({
					{"name", instanceName},
					{"status", status}
					});
			}

			result.push_back({
				{"id", id},
				{"name", name},
				{"description", description},
				{"ownerId", ownerId},
				{"updatedAt", updatedAt},
				{ "isPublic", isPublic? "true":"false"},
				{"runningInstances", instancesArr}
				});
		}
		res.set_content(result.dump(), "application/json");
		});

	// 创建工程
	m_httpServer.Post("/api/tasks", [this](const httplib::Request& req, httplib::Response& res) {
		// 验证token
		std::string token = req.get_header_value("Authorization");
		if (token.empty() || token.substr(0, 7) != "Bearer ") {
			res.status = 401;
			res.set_content(R"({"error": "Unauthorized"})", "application/json");
			return;
		}
		User currentUser;
		if (!verifyToken(token.substr(7), currentUser)) {
			res.status = 401;
			res.set_content(R"({"error": "Invalid token"})", "application/json");
			return;
		}
		try {
			json data = json::parse(req.body);
			std::string name = data["name"];
			std::string description = data.value("description", "");
			std::string content = data.value("content", "");
			bool isPublic = data.value("isPublic", false);
			std::string log;
			if (m_taskManager.createTask(name, description, content, currentUser.id, isPublic, log) == 0) {
				res.status = 201;
				res.set_content(R"({"message": "Task created"})", "application/json");
			}
			else {
				res.status = 400;
				res.set_content(R"({"error": ")" + log + "\"}", "application/json");
			}
		}
		catch (...) { res.status = 400; }
		});

	// 获取单个工程内容
	m_httpServer.Get(R"(/api/tasks/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		// 认证并检查权限
		int taskId = std::stoi(req.matches[1]);
		auto [ok, content] = Database::getInstance().getTaskContent(taskId);
		if (!ok) {
			res.status = 404;
			res.set_content(R"({"error": "Task not found"})", "application/json");
			return;
		}
		json result = { {"content", content} };
		res.set_content(result.dump(), "application/json");
		});
	// 更新工程
	m_httpServer.Put(R"(/api/tasks/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		// 认证 + 权限检查（owner）
		int taskId = std::stoi(req.matches[1]);
		json data = json::parse(req.body);
		std::string name = data.value("name", "");
		std::string description = data.value("description", "");
		std::string content = data.value("content", "");
		bool isPublic = data.value("isPublic", false);
		if (Database::getInstance().updateTask(taskId, name, description, content,isPublic)) {
			res.set_content(R"({"message": "Task updated"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Update failed"})", "application/json");
		}
		});

	// 删除工程
	m_httpServer.Delete(R"(/api/tasks/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
		// 认证 + 权限检查
		int taskId = std::stoi(req.matches[1]);
		if (Database::getInstance().deleteTask(taskId)) {
			res.set_content(R"({"message": "Task deleted"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Delete failed"})", "application/json");
		}
		});
	//创建实例
	m_httpServer.Post("/api/instances", [this](const httplib::Request& req, httplib::Response& res) {
		json data = json::parse(req.body);
		std::string taskName = data["taskName"];
		std::string instanceName = data["instanceName"];
		std::string log;
		int ret = m_taskManager.createInstance(taskName, instanceName, log);
		if (ret == 0) {
			res.set_content(R"({"message": "Instance created"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": log})", "application/json");
		}
		});

	m_httpServer.Post("/api/instances/initialize", [this](const httplib::Request& req, httplib::Response& res) {
		json data = json::parse(req.body);
		std::string instanceName = data["instanceName"];
		std::string typeDesign = data["typeDesign"];   // 数据类型设计
		std::string initParam = data.value("initParam", "");
		std::string sysDesign = data.value("sysDesign", "");
		int timeoutMs = data.value("timeoutMs", 5000);
		if (m_taskManager.initialize(instanceName, typeDesign, initParam, sysDesign, timeoutMs)) {
			res.set_content(R"({"message": "Initialized"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Initialization failed"})", "application/json");
		}
		});

	m_httpServer.Post("/api/instances/control", [this](const httplib::Request& req, httplib::Response& res) {
		json data = json::parse(req.body);
		std::string instanceName = data["instanceName"];
		std::string cmdStr = data["command"]; // "start", "pause", "resume", "stop"
		TaskCommand cmd;
		if (cmdStr == "start") cmd = TaskCommand::TASK_COMMAND_START;
		else if (cmdStr == "pause") cmd = TaskCommand::TASK_COMMAND_PAUSE;
		else if (cmdStr == "resume") cmd = TaskCommand::TASK_COMMAND_RESUME;
		else if (cmdStr == "stop") cmd = TaskCommand::TASK_COMMAND_STOP;
		else { res.status = 400; return; }
		int64 seq = m_taskManager.control(instanceName, cmd);
		if (seq >= 0) {
			res.set_content(R"({"message": "Command sent"})", "application/json");
		}
		else {
			res.status = 500;
			res.set_content(R"({"error": "Control failed"})", "application/json");
		}
		});
}

void ModelDesignerServer::onOpen(connection_hdl hdl) {
	std::lock_guard<std::mutex> lock(m_wsMutex);
	m_connections.insert(hdl);
	std::cout << "WebSocket connection opened, total connections: " << m_connections.size() << std::endl;
}

void ModelDesignerServer::onClose(connection_hdl hdl) {
	std::lock_guard<std::mutex> lock(m_wsMutex);
	m_connections.erase(hdl);
	std::cout << "WebSocket connection closed, remaining connections: " << m_connections.size() << std::endl;
}

void ModelDesignerServer::onMessage(connection_hdl hdl, ws_server::message_ptr msg) {
	std::cout << "WebSocket message received: " << msg->get_payload() << std::endl;
}

std::string ModelDesignerServer::generateUUID() {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 15);
	const char* hex = "0123456789abcdef";
	std::string uuid;
	for (int i = 0; i < 36; i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			uuid += '-';
		}
		else if (i == 14) {
			uuid += '4';
		}
		else if (i == 19) {
			uuid += hex[dis(gen) & 0x3 | 0x8];
		}
		else {
			uuid += hex[dis(gen)];
		}
	}
	return uuid;
}
// Base64 编码表
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";
static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}
std::string base64_encode(const std::string& in) {
	std::string out;
	int val = 0, valb = -6;
	for (unsigned char c : in) {
		val = (val << 8) + c;
		valb += 8;
		while (valb >= 0) {
			out.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}
	if (valb > -6) {
		out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
	}
	while (out.size() % 4) {
		out.push_back('=');
	}
	return out;
}
std::string base64_decode(const std::string& in) {
	std::string out;
	std::vector<int> T(256, -1);
	for (int i = 0; i < 64; i++) {
		T[base64_chars[i]] = i;
	}
	int val = 0, valb = -8;
	for (unsigned char c : in) {
		if (T[c] == -1) break;
		val = (val << 6) + T[c];
		valb += 6;
		if (valb >= 0) {
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}
	return out;
}
// JWT 生成
std::string SimpleJWT::generate(const std::string& secret, const json& payload) {
	// Header
	json header;
	header["alg"] = "HS256";
	header["typ"] = "JWT";
	std::string header_str = header.dump();
	std::string header_encoded = base64_encode(header_str);
	// Payload
	std::string payload_str = payload.dump();
	std::string payload_encoded = base64_encode(payload_str);
	// Signature
	std::string signature_input = header_encoded + "." + payload_encoded;
	// HMAC-SHA256 (简化的实现，实际使用中应该用 OpenSSL)
	// 这里使用简单的方式，生产环境建议使用 OpenSSL 的 HMAC
	std::string signature = base64_encode(signature_input + secret);
	return header_encoded + "." + payload_encoded + "." + signature;
}
bool SimpleJWT::verify(const std::string& token, const std::string& secret, json& payload) {
	size_t first_dot = token.find('.');
	size_t second_dot = token.find('.', first_dot + 1);
	if (first_dot == std::string::npos || second_dot == std::string::npos) {
		return false;
	}
	std::string header_encoded = token.substr(0, first_dot);
	std::string payload_encoded = token.substr(first_dot + 1, second_dot - first_dot - 1);
	std::string signature = token.substr(second_dot + 1);
	// 解码 payload
	std::string payload_str = base64_decode(payload_encoded);
	try {
		payload = json::parse(payload_str);
	}
	catch (...) {
		return false;
	}
	// 检查过期时间
	if (payload.contains("exp")) {
		long long exp = payload["exp"].get<long long>();
		auto now = std::chrono::system_clock::now();
		auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
		if (now_sec > exp) {
			return false;
		}
	}
	// 验证签名
	std::string expected_signature = base64_encode(header_encoded + "." + payload_encoded + secret);
	return signature == expected_signature;
}
// 在 ModelDesignerServer 类中添加实现
std::string ModelDesignerServer::generateToken(const User& user) {
	auto now = std::chrono::system_clock::now();
	auto now_sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
	auto exp_sec = now_sec + 24 * 3600;  // 24小时后过期
	json payload;
	payload["sub"] = user.id;
	payload["username"] = user.username;
	payload["role"] = static_cast<int>(user.role);
	payload["iat"] = now_sec;
	payload["exp"] = exp_sec;
	return SimpleJWT::generate(m_jwtSecret, payload);
}
bool ModelDesignerServer::verifyToken(const std::string& token, User& user) {
	json payload;
	if (!SimpleJWT::verify(token, m_jwtSecret, payload)) {
		return false;
	}
	int userId = payload["sub"].get<int>();
	user = Database::getInstance().getUserById(userId);
	return user.id != -1;
}
