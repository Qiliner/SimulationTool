#pragma once

#include "cpp-httplib/httplib.h"

#include <iostream>

#include <map>

#include <vector>

#include <set>

#include <thread>

#include <mutex>

#include <random>

#include <chrono>

#include "Database.h"

#include "nlohmann/json.hpp"

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#define ModelPath "/uploads/models/"

#define SoftwarePath "/uploads/software/"

using json = nlohmann::json;

using websocketpp::connection_hdl;

using websocketpp::server;

using websocketpp::lib::placeholders::_1;

using websocketpp::lib::placeholders::_2;

using websocketpp::lib::bind;

typedef server<websocketpp::config::asio> ws_server;

using namespace std;

// 简单的 Base64 编码/解码函数（用于 JWT）

std::string base64_encode(const std::string& in);

std::string base64_decode(const std::string& in);

// 简单的 JWT 实现（避免依赖问题）

class SimpleJWT {

public:

	static std::string generate(const std::string& secret, const json& payload);

	static bool verify(const std::string& token, const std::string& secret, json& payload);

};

// 为 DataTypeMember 提供 to_json 函数

inline void to_json(json& j, const DataTypeMember& m) {

	j = json{

		{"displayName", m.displayName},

		{"type", m.type},

		{"subType", m.subType},

		{"range", m.range},

		{"varName", m.varName},

		{"defaultValue", m.defaultValue},

		{"unit", m.unit},

		{"description", m.description}

	};

}

// 为 DataType 提供 to_json 函数

inline void to_json(json& j, const DataType& dt) {

	json members = json::array();

	for (const auto& m : dt.members) {

		members.push_back(m);

	}

	j = json{

		{"id", dt.id},

		{"projectId",dt.projectId},

		{"name", dt.name},

		{"description", dt.description},

		{"uuid", dt.uuid},

		{"members", members}

	};

}

// 为 ModelInterface 提供 to_json 函数

inline void to_json(json& j, const ModelInterface& iface) {

	j = json{

		{"name", iface.name},

		{"dataType", iface.dataType},

		{"topic", iface.topic},

		{"displayName", iface.displayName},

		{"description", iface.description}

	};

}

// 为 Model 提供 to_json 函数

inline void to_json(json& j, const Model& model) {

	json paramInterfaces = json::array();

	for (const auto& p : model.paramInterfaces) paramInterfaces.push_back(p);

	json inputInterfaces = json::array();

	for (const auto& i : model.inputInterfaces) inputInterfaces.push_back(i);

	json outputInterfaces = json::array();

	for (const auto& o : model.outputInterfaces) outputInterfaces.push_back(o);

	json initInterfaces = json::array();

	for (const auto& i : model.initInterfaces) initInterfaces.push_back(i);

	json eventInterfaces = json::array();

	for (const auto& e : model.eventInterfaces) eventInterfaces.push_back(e);

	j = json{

		{"id", model.id},

		{"projectId",model.projectId},

		{"name", model.name},

		{"displayName", model.displayName},

		{"uuid", model.uuid},

		{"paramInterfaces", paramInterfaces},

		{"inputInterfaces", inputInterfaces},

		{"outputInterfaces", outputInterfaces},

		{"initInterfaces", initInterfaces},

		{"eventInterfaces", eventInterfaces}

	};

}

// 为 User 提供 to_json 函数

inline void to_json(json& j, const User& user) {

	j = json{

		{"id", user.id},

		{"username", user.username},

		{"email", user.email},

		{"role", static_cast<int>(user.role)},

		{"roleName", getRoleName(user.role)},

		{"isActive", user.isActive},

		{"createdAt", user.createdAt},

		{"lastLogin", user.lastLogin}

	};

}

// 为 Project 提供 to_json 函数

inline void to_json(json& j, const Project& project) {

	j = json{

		{"id", project.id},

		{"name", project.name},

		{"description", project.description},

		{"ownerId", project.ownerId},

		{"ownerName", project.ownerName},

		{"createdAt", project.createdAt},

		{"updatedAt", project.updatedAt},

		{"isPublic", project.isPublic}

	};

}

class ModelDesignerServer {

public:

	ModelDesignerServer();

	~ModelDesignerServer();

	bool start(int httpPort = 8080, int wsPort = 8081);

	void stop();

private:

	httplib::Server m_httpServer;

	ws_server m_wsServer;

	std::set<connection_hdl, std::owner_less<connection_hdl>> m_connections;

	std::mutex m_wsMutex;

	std::thread m_wsThread;

	bool m_running;

	std::string m_jwtSecret;

	void setupHttpRoutes();

	void setupProjectRoutes();

	void setupModelManageRoutes();

	void onOpen(connection_hdl hdl);

	void onClose(connection_hdl hdl);

	void onMessage(connection_hdl hdl, ws_server::message_ptr msg);

	std::string generateUUID();

	// JWT 相关函数

	std::string generateToken(const User& user);

	bool verifyToken(const std::string& token, User& user);

	template<typename T>

	void broadcastUpdate(const std::string& event, const T& data) {

		std::lock_guard<std::mutex> lock(m_wsMutex);

		json message;

		message["event"] = event;

		// 使用 nlohmann::json 的序列化功能，需要 T 类型有 to_json 函数

		message["data"] = data;

		std::string msgStr = message.dump();

		for (auto it = m_connections.begin(); it != m_connections.end();) {

			try {

				m_wsServer.send(*it, msgStr, websocketpp::frame::opcode::text);

				++it;

			}

			catch (const std::exception& e) {

				std::cerr << "Failed to send WebSocket message: " << e.what() << std::endl;

				it = m_connections.erase(it);

			}

		}

	}

	// 特化版本处理 int 类型

	void broadcastUpdateInt(const std::string& event, int data) {

		std::lock_guard<std::mutex> lock(m_wsMutex);

		json message;

		message["event"] = event;

		message["data"] = data;

		std::string msgStr = message.dump();

		for (auto it = m_connections.begin(); it != m_connections.end();) {

			try {

				m_wsServer.send(*it, msgStr, websocketpp::frame::opcode::text);

				++it;

			}

			catch (const std::exception& e) {

				std::cerr << "Failed to send WebSocket message: " << e.what() << std::endl;

				it = m_connections.erase(it);

			}

		}

	}

};
