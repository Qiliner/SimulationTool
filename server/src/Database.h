#pragma once
#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>
#include <chrono>
// ģ�Ϳ�����ö��
enum class ModelLibType {
    DEVELOPMENT = 0,  // ������
    CONTROLLED = 1,   // �ܿؿ�
    PRODUCT = 2       // ��Ʒ��
};
// Ŀ¼�ṹ��
struct Directory {
    int id;
    int parentId;
    int modelLibId;
    std::string name;
    std::string path;
    int createdAt;
    int updatedAt;
};
// ģ���ļ��ṹ��
struct ModelFile {
    int id;
    int directoryId;
    int modelLibId;
    std::string name;
    std::string fileName;
    std::string filePath;
    std::string version;
    std::string startCmd;
    std::string stopCmd;
    std::string description;
    long long fileSize;
    std::string fileType;
    int uploaderId;
    std::string uploaderName;
    int createdAt;
    int updatedAt;
};
// �����ṹ��
struct Software {
    int id;
    std::string name;
    std::string fileName;
    std::string filePath;
    std::string version;
    std::string description;
    std::string installCmd;
    std::string uninstallCmd;
    std::string startCmd;
    std::string stopCmd;
    long long fileSize;
    std::string fileType;
    int uploaderId;
    std::string uploaderName;
    int createdAt;
    int updatedAt;
};
// �û���ɫö��
enum class UserRole {
    SUPER_ADMIN = 0,  // ��������Ա
    ADMIN = 1,        // ����Ա
    DESIGNER = 2,     // �����
    VIEWER = 3        // ֻ���û�
};
// ��ȡ��ɫ����
 std::string getRoleName(UserRole role);
// �û��ṹ��
struct User {
    int id;
    std::string username;
    std::string password;  // ʵ��Ӧ����Ӧ�ô洢��ϣֵ
    std::string email;
    UserRole role;
    std::string createdAt;
    std::string lastLogin;
    bool isActive;
};
// ���̽ṹ��
struct Project {
    int id;
    std::string name;
    std::string description;
    int ownerId;
    std::string ownerName;
    int createdAt;
    int updatedAt;
    bool isPublic;
    std::string data;  // JSON��ʽ�洢��������
};
// ���̳�Ա�ṹ��
struct ProjectMember {
    int id;
    int projectId;
    int userId;
    std::string userName;
    int permission;  // 0: owner, 1: editor, 2: viewer
    int addedAt;
};
struct DataTypeMember {
    std::string displayName;
    std::string type;
    std::string subType;
    std::string range;
    std::string varName;
    std::string defaultValue;
    std::string unit;
    std::string description;
};
struct DataType {
    int id;
    int projectId;  // ��������������ID
    std::string name;
    std::string description;
    std::string uuid;
    std::vector<DataTypeMember> members;
};
struct ModelInterface {
    std::string name;
    std::string dataType;
    std::string topic;
    std::string displayName;
    std::string description;
};
struct Model {
    int id;
    int projectId;  // ��������������ID
    std::string name;
    std::string displayName;
    std::string uuid;
    std::vector<ModelInterface> paramInterfaces;
    std::vector<ModelInterface> inputInterfaces;
    std::vector<ModelInterface> outputInterfaces;
    std::vector<ModelInterface> initInterfaces;
    std::vector<ModelInterface> eventInterfaces;
};
std::string utf8_to_gbk(const std::string& utf8_str);
// GBK ת UTF-8
std::string gbk_to_utf8(const std::string& gbk_str);
class Database {
public:
    static Database& getInstance();
    bool initialize(const std::string& dbPath = "model_designer.db");
    void close();
    // �û�����
    bool createUserTable();
    bool insertDefaultSuperAdmin();
    std::vector<User> getAllUsers();
    User getUserById(int id);
    User getUserByUsername(const std::string& username);
    bool authenticateUser(const std::string& username, const std::string& password, User& user);
    bool insertUser(const User& user);
    bool updateUser(const User& user);
    bool deleteUser(int id);
    bool updateLastLogin(int id);
    // ���ݽṹ����
  /*  DataType getDataTypeById(int id);
    DataType getDataTypeByUuid(const std::string& uuid);
    DataType getDataTypeByName(const std::string& name);*/
    bool insertDataType(const DataType& dataType);
    bool updateDataType(const DataType& dataType);
    bool deleteDataType(int id);
    bool deleteDataTypeByUuid(const std::string& uuid);
    // ģ�Ͳ���
    //Model getModelById(int id);
    //Model getModelByUuid(const std::string& uuid);
    //Model getModelByName(const std::string& name);
    bool insertModel(const Model& model);
    bool updateModel(const Model& model);
    bool deleteModel(int id);
    bool deleteModelByUuid(const std::string& uuid);
    std::vector<ModelFile> getAllModelFilesByLib(int libId);
    // ��������
    bool syncDataTypes(const std::vector<DataType>& dataTypes);
    bool syncModels(const std::vector<Model>& models);
    bool createProjectsTable();
    std::vector<Project> getAllProjects(int userId = -1);  // userIdΪ-1ʱ��ȡ���У������ȡ�û���Ȩ�޵�
    Project getProjectById(int id);
    Project getProjectByName(const std::string& name);
    bool insertProject(const Project& project);
    bool updateProject(const Project& project);
    bool deleteProject(int id);
    bool deleteProjectByUuid(const std::string& uuid);
    bool addProjectMember(int projectId, int userId, int permission);
    bool removeProjectMember(int projectId, int userId);
    std::vector<ProjectMember> getProjectMembers(int projectId);
    bool hasProjectPermission(int userId, int projectId, int requiredPermission);
    std::vector<DataType> getAllDataTypes(int projectId);
    DataType getDataTypeById(int id, int projectId);
    bool insertDataType(const DataType& dataType, int projectId);
    bool updateDataType(const DataType& dataType, int projectId);
    bool deleteDataType(int id, int projectId);
    // �޸�ģ�Ͳ��������� projectId ����
    std::vector<Model> getAllModels(int projectId);
    Model getModelById(int id, int projectId);
    bool insertModel(const Model& model, int projectId);
    bool updateModel(const Model& model, int projectId);
    bool deleteModel(int id, int projectId);
    // ����û��Թ��̵�Ȩ��
    bool hasProjectAccess(int userId, int projectId);
    bool hasProjectEditPermission(int userId, int projectId);
    /**************************** ģ�Ϳ��������*************************************/
    bool createModelManageTables();
    int createModelLib(const std::string& name, ModelLibType type);
    std::vector<std::pair<int, std::string>> getAllModelLibs();
    // Ŀ¼����
    int createDirectory(int parentId, int modelLibId, const std::string& name);
    bool deleteDirectory(int id);
    bool renameDirectory(int id, const std::string& newName);
    std::vector<Directory> getDirectories(int modelLibId, int parentId = 0);
    Directory getDirectoryById(int id);
    // ģ���ļ�����
    int addModelFile(const ModelFile& modelFile);
    bool updateModelFile(const ModelFile& modelFile);
    bool deleteModelFile(int id);
    ModelFile getModelFileById(int id);
    std::vector<ModelFile> getModelFiles(int modelLibId, int directoryId = 0);
    bool migrateModelFile(int modelFileId, int targetLibId, int targetDirId);
    // ��������
    int addSoftware(const Software& software);
    bool updateSoftware(const Software& software);
    bool deleteSoftware(int id);
    Software getSoftwareById(int id);
    std::vector<Software> getAllSoftware();

    //创建仿真任务表
    bool createTaskTables();
    // Task (工程) 相关
    bool insertTask(const std::string& name, const std::string& description, int ownerId, const std::string& content, bool isPublic = false);
    bool updateTask(int taskId, const std::string& name, const std::string& description, const std::string& content, bool isPublic);
    bool deleteTask(int taskId);
    std::vector<std::tuple<int, std::string, std::string, int, std::string, int, bool>> getTasksForUser(int userId);
    std::vector<std::tuple<int, std::string, std::string, int, std::string, int>> getAllTasks(int userId); // id,name,description,ownerId,content,updatedAt
    std::tuple<bool, std::string> getTaskContent(int taskId); // 成功标志, content
    int getTaskIdByName(const std::string& name);
private:
    Database() = default;
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    bool createTables();
    bool executeSql(const std::string& sql);
    bool executeSql(const std::string& sql, std::function<void(sqlite3_stmt*)> bindFunc);
    sqlite3* m_db = nullptr;
    std::mutex m_mutex;
};
