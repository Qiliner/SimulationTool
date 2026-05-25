#pragma once
#include <string>
#include <iostream>
#include <map>
#include <iomanip>>
#include <mutex>
#include <vector>
#include <atomic>
#include "jmsg/data_version_msg.h"

#ifdef _WIN32
#ifdef  NETWORKQUERY_LIB
#define NETWORKQUERY_EXPORT __declspec(dllexport)
#else
#define NETWORKQUERY_EXPORT __declspec(dllimport)
#endif
#else
#define NETWORKQUERY_EXPORT
#endif 
class DataVersionClient;


class NETWORKQUERY_EXPORT NetWorkQueryAPI
{
public:
	static NetWorkQueryAPI* getNetWorkQueryAPI(const std::string& host, const int port);
	~NetWorkQueryAPI();
	//====================================目录的基本操作方法==============================================//
	/**
	 * @brief 连接到服务器
	 *
	 * @return 返回true表示成功连接服务器，返回false表示连接失败
	 */
	bool ConnectServer();

	/**
	 * @brief 添加目录
	 *
	 * @param uuid 要添加目录的uuid
	 * @param server_path 服务器上的路径
	 * @param type 要添加的类型，默认是目录
	 * @return 返回true表示成功添加目录，返回false表示添加失败
	 */
	bool AddDirectory(std::string& uuid, std::string serverPath, int type = ItemType::DIR_ITEM);

	/**
	* @brief 上传模型时需要添加的模型的方法
	*
	* @param uuid 要添加目录的uuid
	* @param serverPath 服务器的路径
	* @param guid 添加成功后返回的添加模型的guid
	* @param guid 添加成功后返回的添加模型的guid
	* @param model_source 要添加模型时需要携带的一些其他信息
	* @param type 默认是模型
	* @return 返回true表示成功添加目录，返回false表示添加失败
	*/
	bool AddUploadDirectory(const std::string& uuid, std::string serverPath, std::string& guid, std::string& model_source, int type = ItemType::MODEL_ITEM);

	/**
	 * @brief 重命名目录
	 * @param guid 需要重命名id的guid
	 * @param newName 新的名称
	 * @return 返回true表示重命名成功，返回false表示重命名失败
	 */
	bool RenameDirOrFile(const std::string& guid, std::string newName);

	/**
	 * @brief 删除目录
	 *
	 * @param guid 删除目录的guid
	 * @return 返回true表示删除成功，返回false表示删除失败
	 */
	bool DeleteDirOrFile(const std::string& guid);

	//====================================上传下载的方法==============================================//
	/**
	 * @brief 单个文件下载
	 *
	 * @param localPath 本地路径
	 * @param serverPath 服务器路径
	 * @param guid 下在的目标的uuid
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	bool DownloadSingleFile(const std::string& localPath, const std::string& serverPath, std::string& guid);

	/**
	 * @brief 下载目录或文件
	 *
	 * @param localPath 本地路径
	 * @param serverPath 服务器路径
	 * @param guid 下在的目标的uuid
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	bool DownloadDirOrFile(const std::string& localPath, const std::string& serverPath, std::string guid);

	/**
	 * @brief 仿真测试下载服务器上模型信息，只下载xml包含的文件
	 *
	 * @param localPath 需要存放模型信息和模型配置
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	bool DownloadModelInfo(const std::string& localPath);

	/**
	* @brief 根据模型全路径返回uuid
	*
	* @param localPath 模型的全路径
	* @return 成功返回 uuid,失败返回 ""
	*/
	std::string GetUUIDByFullPath(const std::string& localPath);


	/**
	* @brief 模型管理下载服务器上模型信息，下载空目录和XML
	*
	* @param localPath 需要存放模型信息和模型配置
	* @return 返回true表示下载成功，返回false表示下载失败
	*/
	bool DownloadModelManageInfo(const std::string& localPath);
	/**
	 * @brief 上传目录或文件
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @param guid 服务器路径
	 * @param modelName 模型名称获取软件名称
	 * @param version 模型生成时要用的版本
	 * @param type    类型从1开始  DB_ITEM-数据库文件； DIR_ITEM-目录； SOFTWRAE_ITEM-软件；  NORMAL_FILE-普通文件； MODEL_ITEM-模型； 
	 * @return 返回true表示上传成功，返回false表示上传失败 
	 */
	bool UploadDirOrFile(const std::string& localPath, const std::string& remoteDir, std::string& guid, const std::string& modelName,const std::string& version, ItemType type);

	/**
	* @brief  单个文件的上传
	*
	* @param localhost_path 本地路径
	* @param server_path 服务器路径
	* @param guid uuid值
	* @return 返回true表示下载成功，返回false表示下载失败
	*/
	bool UploadloadSingleFile(const std::string& localPath, const std::string& server_path, std::string& guid);

	//================== 获取服务器上的目录及模型的树形结构 ======================================//
	/**
	 * @brief  获取服务器上的根节点下的全部目录
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	std::vector<Item> GetRootList();
	/**
	 * @brief  获取服务器上的root目录下的三个库的uuid及路径名称
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	std::vector<Item> GetRootSanKuItemList();

	/**
	 * @brief  获取服务上指定的路径
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	std::vector<Item> GetAssignItemList(const std::string& guid, const std::string& file_path, int type = ItemType::GET_ASSIGN_DIR);

	//================== 模型相关的操作,获取和更新模型信息 ======================================//
	/**
	 * @brief  获取模型信息
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	std::string GetModelInfo(std::string& guid, int type = ItemType::GET_MODEL_INFO);
	/**
	* @brief  获取模型信息的sh指令
	*
	* @param localhost_path 本地路径
	* @param server_path 服务器路径
	* @return 返回true表示下载成功，返回false表示下载失败
	*/
	std::string GetModelSHInfo(std::string& guid);

	/**
	 * @brief  更新模型信息
	 *
	 * @param guid 模型的guid
	 * @param server_path 本地路径
	 * @param data 模型数据
	 * @return 返回true表示成功，返回false表示下载失败
	 */
	bool UpdataInstructionInfo(std::string& guid, const std::string server_path, const std::string data);

	/**
	* @brief  用户查询（单个）
	*
	* @param username 用户名
	* @return 返回true表示成功，返回false表示查找失败
	*/
	bool GetByNameToUser(const std::string& username, User &user);

	/**
	* @brief  用户查询（多个）
	*
	* @param username 用户名
	* @param role 用户角色  GetAllUsers()
	* @return 返回true表示成功，返回false表示查找失败
	*/
	bool GetAllUsers(const std::string& username, int role, std::vector<User>& users);

	/**
	* @brief  新增用户
	*
	* @param username 用户名
	* @param password 密码
	* @param role 用户角色
	* @return 返回true表示成功，返回false表示新增失败
	*/
	bool AddUser(const std::string& username, const std::string password, int role);

	/**
	* @brief  删除单个用户
	*
	* @param username 用户名
	* @return 返回true表示成功，返回false表示删除失败
	*/
	bool DeleteUser(const std::string& username);

	/**
	* @brief  修改单个用户
	*
	* @param username 用户名
	* @param password 密码
	* @param role 用户角色
	* @param update_time 更改时间
	* @return 返回true表示成功，返回false表示删除失败
	*/
	bool UpdateUser(const std::string& username, const std::string& password, int role, const std::string& update_time);

	//================== 获取上传和下载的数量的方法 ======================================//
	/**
	 * @brief  获取上传或下载的总数量
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	int GetFileTotal();

	/**
	 * @brief  获取正在上传或下载的完成的数量
	 *
	 * @param localhost_path 本地路径
	 * @param server_path 服务器路径
	 * @return 返回true表示下载成功，返回false表示下载失败
	 */
	int GetUploadOrDownNum();


	/**
	* @brief  释放内存
	*
	* @param localhost_path 本地路径
	* @param server_path 服务器路径
	* @return 返回true表示下载成功，返回false表示下载失败
	*/
	void FreeClient(std::string host,int port);

public:
	/**
	* @brief 创建基于主机名和端口的 DataVersionClient 实例。
	* @param host 服务器主机名或 IP 地址。
	* @param port 服务器端口号。
	* @return 返回 DataVersionClient 实例，失败返回 nullptr。
	*/
	static NetWorkQueryAPI* CreateClient(const std::string& host, int port);

	/**
	* @brief 创建基于完整根 URL 的 DataVersionClient 实例。
	* @param rootUrl 完整的根 URL。
	* @return 返回 DataVersionClient 实例，失败返回 nullptr。
	*/
	static NetWorkQueryAPI* CreateClient(const std::string& rootUrl);

	/**
	* @brief 释放基于主机名和端口的 DataVersionClient 实例。
	* @param host 服务器主机名或 IP 地址。
	* @param port 服务器端口号。
	*/
	static void ReleaseClient(const std::string& host, int port);

	/**
	* @brief 释放基于完整根 URL 的 DataVersionClient 实例。
	* @param rootUrl 完整的根 URL。
	*/
	static void ReleaseClient(const std::string& rootUrl);
private:
	/**
	* @brief 构造函数，初始化 DataVersionClient 实例。
	* @param rootUrl 完整的根 URL。
	*/
	NetWorkQueryAPI(const std::string& rootUrl);
	// 可以在此添加私有成员变量和方法
	DataVersionClient* m_client;
	std::mutex m_mutex;
	size_t  m_fileTotal = 0; // 上传或下载的总数量
	std::atomic<int> m_number{ 0 };  // 已下载文件数
	static std::map<std::string, NetWorkQueryAPI*>* m_instance;


};