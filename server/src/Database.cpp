#include "Database.h"

#include <iostream>

#include <sstream>

#include<boost/filesystem.hpp>

#include <string>

#include <vector>

#include <algorithm>

#ifdef _WIN32

#include <windows.h>

#else

#include <iconv.h>

#include <cerrno>

#include <cstring>

#endif

// UTF-8 ת GBK

std::string utf8_to_gbk(const std::string& utf8_str) {

    if (utf8_str.empty()) return {};

#ifdef _WIN32

    // Windows: MultiByteToWideChar + WideCharToMultiByte

    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), static_cast<int>(utf8_str.size()), nullptr, 0);

    if (wlen == 0) return {};

    std::vector<wchar_t> wbuf(wlen);

    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), static_cast<int>(utf8_str.size()), wbuf.data(), wlen);

    int glen = WideCharToMultiByte(CP_ACP, 0, wbuf.data(), wlen, nullptr, 0, nullptr, nullptr);

    if (glen == 0) return {};

    std::vector<char> gbuf(glen);

    WideCharToMultiByte(CP_ACP, 0, wbuf.data(), wlen, gbuf.data(), glen, nullptr, nullptr);

    return std::string(gbuf.data(), glen);

#else

    // POSIX: iconv

    iconv_t cd = iconv_open("GBK", "UTF-8");   // (to, from)

    if (cd == reinterpret_cast<iconv_t>(-1)) return {};

    const char* in = utf8_str.data();

    size_t in_bytes = utf8_str.size();

    size_t out_size = in_bytes * 2;

    std::vector<char> out_buf(out_size);

    char* out = out_buf.data();

    size_t out_bytes = out_size;

    size_t ret = iconv(cd, const_cast<char**>(&in), &in_bytes, &out, &out_bytes);

    iconv_close(cd);

    if (ret == static_cast<size_t>(-1)) return {};

    size_t converted_len = out_size - out_bytes;

    return std::string(out_buf.data(), converted_len);

#endif

}

// GBK ת UTF-8

std::string gbk_to_utf8(const std::string& gbk_str) {

    if (gbk_str.empty()) return {};

#ifdef _WIN32

    // Windows: MultiByteToWideChar(CP_ACP) + WideCharToMultiByte(CP_UTF8)

    int wlen = MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), static_cast<int>(gbk_str.size()), nullptr, 0);

    if (wlen == 0) return {};

    std::vector<wchar_t> wbuf(wlen);

    MultiByteToWideChar(CP_ACP, 0, gbk_str.c_str(), static_cast<int>(gbk_str.size()), wbuf.data(), wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, nullptr, 0, nullptr, nullptr);

    if (ulen == 0) return {};

    std::vector<char> ubuf(ulen);

    WideCharToMultiByte(CP_UTF8, 0, wbuf.data(), wlen, ubuf.data(), ulen, nullptr, nullptr);

    return std::string(ubuf.data(), ulen);

#else

    // POSIX: iconv

    iconv_t cd = iconv_open("UTF-8", "GBK");   // (to, from)

    if (cd == reinterpret_cast<iconv_t>(-1)) return {};

    const char* in = gbk_str.data();

    size_t in_bytes = gbk_str.size();

    size_t out_size = in_bytes * 2;

    std::vector<char> out_buf(out_size);

    char* out = out_buf.data();

    size_t out_bytes = out_size;

    size_t ret = iconv(cd, const_cast<char**>(&in), &in_bytes, &out, &out_bytes);

    iconv_close(cd);

    if (ret == static_cast<size_t>(-1)) return {};

    size_t converted_len = out_size - out_bytes;

    return std::string(out_buf.data(), converted_len);

#endif

}

std::string getRoleName(UserRole role) {

    std::string name;

    switch (role) {

    case UserRole::SUPER_ADMIN: name = "超级管理员"; break;

    case UserRole::ADMIN: name = "管理员"; break;

    case UserRole::DESIGNER: name = "设计师"; break;

    case UserRole::VIEWER: name = "只读用户"; break;

    default: name = "未知"; break;

    }

    return name;

}

Database& Database::getInstance() {

    static Database instance;

    return instance;

}

Database::~Database() {

    close();

}

bool Database::initialize(const std::string& dbPath) {

    std::lock_guard<std::mutex> lock(m_mutex);

    int rc = sqlite3_open(dbPath.c_str(), &m_db);

    if (rc) {

        std::cerr << "Can't open database: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    return createTables();

}

void Database::close() {

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_db) {

        sqlite3_close(m_db);

        m_db = nullptr;

    }

}

bool Database::executeSql(const std::string& sql) {

    char* errMsg = nullptr;

    int rc = sqlite3_exec(m_db, sql.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {

        std::cerr << "SQL error: " << errMsg << std::endl;

        sqlite3_free(errMsg);

        return false;

    }

    return true;

}

bool Database::createUserTable() {

    const std::string createUsersTable = R"(

        CREATE TABLE IF NOT EXISTS users (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            username TEXT UNIQUE NOT NULL,

            password TEXT NOT NULL,

            email TEXT,

            role INTEGER DEFAULT 3,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            last_login INTEGER DEFAULT 0,

            is_active INTEGER DEFAULT 1

        );

    )";

    return executeSql(createUsersTable);

}

bool Database::insertDefaultSuperAdmin() {

    //std::lock_guard<std::mutex> lock(m_mutex);

    const std::string checkSql = "SELECT COUNT(*) FROM users WHERE role = 0";

    sqlite3_stmt* checkStmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, checkSql.c_str(), -1, &checkStmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare check statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    int count = 0;

    if (sqlite3_step(checkStmt) == SQLITE_ROW) {

        count = sqlite3_column_int(checkStmt, 0);

    }

    sqlite3_finalize(checkStmt);

    if (count > 0) {

        return true; 

    }

    const std::string insertSql = "INSERT INTO users (username, password, email, role, is_active) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, insertSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    std::string defaultPassword = "admin123";  

    sqlite3_bind_text(stmt, 1, "admin", -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, defaultPassword.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, "admin@modeldesigner.com", -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 4, static_cast<int>(UserRole::SUPER_ADMIN));

    sqlite3_bind_int(stmt, 5, 1);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to insert default super admin: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    std::cout << "Default super admin created: username='admin', password='admin123'" << std::endl;

    return true;

}

std::vector<User> Database::getAllUsers() {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<User> result;

    const std::string sql = "SELECT id, username, password, email, role, created_at, last_login, is_active FROM users ORDER BY role, username";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return result;

    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        User user;

        user.id = sqlite3_column_int(stmt, 0);

        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        user.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        if (email) user.email = email;

        user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 4));

        user.createdAt = std::to_string(sqlite3_column_int(stmt, 5));

        user.lastLogin = std::to_string(sqlite3_column_int(stmt, 6));

        user.isActive = sqlite3_column_int(stmt, 7) == 1;

        result.push_back(user);

    }

    sqlite3_finalize(stmt);

    return result;

}

User Database::getUserById(int id) {

    User user;

    user.id = -1;

    const std::string sql = "SELECT id, username, password, email, role, created_at, last_login, is_active FROM users WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return user;

    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        user.id = sqlite3_column_int(stmt, 0);

        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        user.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        if (email) user.email = email;

        user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 4));

        user.createdAt = std::to_string(sqlite3_column_int(stmt, 5));

        user.lastLogin = std::to_string(sqlite3_column_int(stmt, 6));

        user.isActive = sqlite3_column_int(stmt, 7) == 1;

    }

    sqlite3_finalize(stmt);

    return user;

}

User Database::getUserByUsername(const std::string& username) {

    std::lock_guard<std::mutex> lock(m_mutex);

    User user;

    user.id = -1;

    const std::string sql = "SELECT id, username, password, email, role, created_at, last_login, is_active FROM users WHERE username = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return user;

    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        user.id = sqlite3_column_int(stmt, 0);

        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        user.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        const char* email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        if (email) user.email = email;

        user.role = static_cast<UserRole>(sqlite3_column_int(stmt, 4));

        user.createdAt = std::to_string(sqlite3_column_int(stmt, 5));

        user.lastLogin = std::to_string(sqlite3_column_int(stmt, 6));

        user.isActive = sqlite3_column_int(stmt, 7) == 1;

    }

    sqlite3_finalize(stmt);

    return user;

}

bool Database::authenticateUser(const std::string& username, const std::string& password, User& user) {

    user = getUserByUsername(username);

    if (user.id == -1) {

        return false;

    }

    if (user.password != password) {

        return false;

    }

    if (!user.isActive) {

        return false;

    }

    updateLastLogin(user.id);

    return true;

}

bool Database::insertUser(const User& user) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "INSERT INTO users (username, password, email, role, is_active) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 4, static_cast<int>(user.role));

    sqlite3_bind_int(stmt, 5, user.isActive ? 1 : 0);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::updateUser(const User& user) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "UPDATE users SET username = ?, password = ?, email = ?, role = ?, is_active = ? WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, user.email.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 4, static_cast<int>(user.role));

    sqlite3_bind_int(stmt, 5, user.isActive ? 1 : 0);

    sqlite3_bind_int(stmt, 6, user.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::deleteUser(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string checkSql = "SELECT COUNT(*) FROM users WHERE role = 0 AND id != ?";

    sqlite3_stmt* checkStmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, checkSql.c_str(), -1, &checkStmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(checkStmt, 1, id);

    int adminCount = 0;

    if (sqlite3_step(checkStmt) == SQLITE_ROW) {

        adminCount = sqlite3_column_int(checkStmt, 0);

    }

    sqlite3_finalize(checkStmt);

    User user = getUserById(id);

    if (user.role == UserRole::SUPER_ADMIN && adminCount == 0) {

        std::cerr << "Cannot delete the last super admin" << std::endl;

        return false;

    }

    const std::string sql = "DELETE FROM users WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::updateLastLogin(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "UPDATE users SET last_login = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::createTables() {

    if (!createUserTable()) {

        return false;

    }

    insertDefaultSuperAdmin();

    const std::string createDataTypesTable = R"(

        CREATE TABLE IF NOT EXISTS data_types (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            project_id INTEGER NOT NULL,

            name TEXT NOT NULL,

            description TEXT,

            uuid TEXT NOT NULL,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now')),

            FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE,

            UNIQUE(project_id, name)

        );

    )";

    const std::string createDataTypeMembersTable = R"(

        CREATE TABLE IF NOT EXISTS data_type_members (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            data_type_id INTEGER NOT NULL,

            display_name TEXT,

            type TEXT NOT NULL,

            sub_type TEXT,

            range TEXT,

            var_name TEXT,

            default_value TEXT,

            unit TEXT,

            description TEXT,

            sort_order INTEGER DEFAULT 0,

            FOREIGN KEY (data_type_id) REFERENCES data_types(id) ON DELETE CASCADE

        );

    )";

    const std::string createModelsTable = R"(

        CREATE TABLE IF NOT EXISTS models (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            project_id INTEGER NOT NULL,

            name TEXT NOT NULL,

            display_name TEXT,

            uuid TEXT NOT NULL,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now')),

            FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE,

            UNIQUE(project_id, name)

        );

    )";

    const std::string createModelInterfacesTable = R"(

        CREATE TABLE IF NOT EXISTS model_interfaces (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            model_id INTEGER NOT NULL,

            interface_type TEXT NOT NULL,

            name TEXT NOT NULL,

            data_type TEXT NOT NULL,

            topic TEXT,

            display_name TEXT,

            description TEXT,

            sort_order INTEGER DEFAULT 0,

            FOREIGN KEY (model_id) REFERENCES models(id) ON DELETE CASCADE

        );

    )";

    // ����ģ�͹�����

    if (!createModelManageTables()) {

        std::cerr << "Failed to create model manage tables" << std::endl;

        return false;

    }

  return executeSql(createDataTypesTable) &&

           executeSql(createDataTypeMembersTable) &&

           executeSql(createModelsTable) &&

           executeSql(createModelInterfacesTable) &&

           createProjectsTable();  // ������һ��

}

bool Database::insertDataType(const DataType& dataType) {

    //std::lock_guard<std::mutex> lock(m_mutex);

    sqlite3_stmt* stmt = nullptr;

    const std::string sql = "INSERT INTO data_types (name, description, uuid) VALUES (?, ?, ?)";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, dataType.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, /*dataType.description.c_str()*/ "111", -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, dataType.uuid.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to insert data type: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    int dataTypeId = sqlite3_last_insert_rowid(m_db);

    // �����Ա

    for (size_t i = 0; i < dataType.members.size(); ++i) {

        const auto& member = dataType.members[i];

        const std::string memberSql = R"(

            INSERT INTO data_type_members 

            (data_type_id, display_name, type, sub_type, range, var_name, default_value, unit, description, sort_order) 

            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)

        )";

        sqlite3_stmt* memberStmt = nullptr;

        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

        if (rc != SQLITE_OK) {

            std::cerr << "Failed to prepare member statement: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

        sqlite3_bind_int(memberStmt, 1, dataTypeId);

        sqlite3_bind_text(memberStmt, 2, member.displayName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 3, member.type.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 4, member.subType.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 5, member.range.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 6, member.varName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 7, member.defaultValue.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 8, member.unit.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 9, member.description.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_int(memberStmt, 10, static_cast<int>(i));

        rc = sqlite3_step(memberStmt);

        sqlite3_finalize(memberStmt);

        if (rc != SQLITE_DONE) {

            std::cerr << "Failed to insert data type member: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

    }

    return true;

}

bool Database::updateDataType(const DataType& dataType) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��������

    const std::string updateSql = "UPDATE data_types SET name = ?, description = ?, uuid = ?, updated_at = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, updateSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, dataType.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, dataType.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, dataType.uuid.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 4, dataType.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to update data type: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    // ɾ���ɵĳ�Ա

    const std::string deleteMembersSql = "DELETE FROM data_type_members WHERE data_type_id = ?";

    stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, deleteMembersSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare delete members statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_int(stmt, 1, dataType.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    // �����³�Ա

    for (size_t i = 0; i < dataType.members.size(); ++i) {

        const auto& member = dataType.members[i];

        const std::string memberSql = R"(

            INSERT INTO data_type_members 

            (data_type_id, display_name, type, sub_type, range, var_name, default_value, unit, description, sort_order) 

            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)

        )";

        sqlite3_stmt* memberStmt = nullptr;

        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

        if (rc != SQLITE_OK) {

            std::cerr << "Failed to prepare member insert statement: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

        sqlite3_bind_int(memberStmt, 1, dataType.id);

        sqlite3_bind_text(memberStmt, 2, member.displayName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 3, member.type.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 4, member.subType.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 5, member.range.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 6, member.varName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 7, member.defaultValue.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 8, member.unit.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 9, member.description.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_int(memberStmt, 10, static_cast<int>(i));

        rc = sqlite3_step(memberStmt);

        sqlite3_finalize(memberStmt);

        if (rc != SQLITE_DONE) {

            std::cerr << "Failed to insert data type member: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

    }

    return true;

}

bool Database::deleteDataType(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "DELETE FROM data_types WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::deleteDataTypeByUuid(const std::string& uuid) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "DELETE FROM data_types WHERE uuid = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_text(stmt, 1, uuid.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

//

//std::vector<DataType> Database::getAllDataTypes() {

//    std::lock_guard<std::mutex> lock(m_mutex);

//    std::vector<DataType> result;

//

//    const std::string sql = "SELECT id, name, description, uuid FROM data_types ORDER BY name";

//    sqlite3_stmt* stmt = nullptr;

//

//    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

//    if (rc != SQLITE_OK) {

//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

//        return result;

//    }

//

//    while (sqlite3_step(stmt) == SQLITE_ROW) {

//        DataType dt;

//        dt.id = sqlite3_column_int(stmt, 0);

//        dt.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

//        dt.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

//        dt.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

//

//        // ��ȡ��Ա

//        const std::string memberSql = R"(

//            SELECT display_name, type, sub_type, range, var_name, default_value, unit, description 

//            FROM data_type_members 

//            WHERE data_type_id = ? 

//            ORDER BY sort_order

//        )";

//        sqlite3_stmt* memberStmt = nullptr;

//        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

//

//        if (rc == SQLITE_OK) {

//            sqlite3_bind_int(memberStmt, 1, dt.id);

//            while (sqlite3_step(memberStmt) == SQLITE_ROW) {

//                DataTypeMember member;

//                member.displayName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 0));

//                member.type = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 1));

//                const char* subType = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 2));

//                if (subType) member.subType = subType;

//                member.range = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 3));

//                member.varName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 4));

//                member.defaultValue = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 5));

//                member.unit = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 6));

//                member.description = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 7));

//                dt.members.push_back(member);

//            }

//            sqlite3_finalize(memberStmt);

//        }

//

//        result.push_back(dt);

//    }

//

//    sqlite3_finalize(stmt);

//    return result;

//}

// ���Ƶ�ʵ�� getDataTypeById, getDataTypeByUuid, getDataTypeByName

// �Լ�ģ�͵� CRUD ����...

//

//DataType Database::getDataTypeById(int id) {

//    auto all = getAllDataTypes();

//    for (const auto& dt : all) {

//        if (dt.id == id) return dt;

//    }

//    return DataType{};

//}

//

//DataType Database::getDataTypeByUuid(const std::string& uuid) {

//    auto all = getAllDataTypes();

//    for (const auto& dt : all) {

//        if (dt.uuid == uuid) return dt;

//    }

//    return DataType{};

//}

//

//DataType Database::getDataTypeByName(const std::string& name) {

//    auto all = getAllDataTypes();

//    for (const auto& dt : all) {

//        if (dt.name == name) return dt;

//    }

//    return DataType{};

//}

bool Database::insertModel(const Model& model) {

    std::lock_guard<std::mutex> lock(m_mutex);

    sqlite3_stmt* stmt = nullptr;

    const std::string sql = "INSERT INTO models (project_id, name, display_name, uuid) VALUES (?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_int(stmt, 1, model.projectId);

    sqlite3_bind_text(stmt, 2, model.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, model.displayName.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 4, model.uuid.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to insert model: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    int modelId = sqlite3_last_insert_rowid(m_db);

    // ����ӿڵĸ�������

    auto insertInterfaces = [&](const std::vector<ModelInterface>& interfaces, const std::string& interfaceType, int startOrder) -> bool {

        for (size_t i = 0; i < interfaces.size(); ++i) {

            const auto& iface = interfaces[i];

            const std::string ifaceSql = R"(

                INSERT INTO model_interfaces 

                (model_id, interface_type, name, data_type, topic, display_name, description, sort_order) 

                VALUES (?, ?, ?, ?, ?, ?, ?, ?)

            )";

            sqlite3_stmt* ifaceStmt = nullptr;

            rc = sqlite3_prepare_v2(m_db, ifaceSql.c_str(), -1, &ifaceStmt, nullptr);

            if (rc != SQLITE_OK) {

                std::cerr << "Failed to prepare interface statement: " << sqlite3_errmsg(m_db) << std::endl;

                return false;

            }

            sqlite3_bind_int(ifaceStmt, 1, modelId);

            sqlite3_bind_text(ifaceStmt, 2, interfaceType.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 3, iface.name.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 4, iface.dataType.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 5, iface.topic.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 6, iface.displayName.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 7, iface.description.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_int(ifaceStmt, 8, static_cast<int>(startOrder + i));

            rc = sqlite3_step(ifaceStmt);

            sqlite3_finalize(ifaceStmt);

            if (rc != SQLITE_DONE) {

                std::cerr << "Failed to insert interface: " << sqlite3_errmsg(m_db) << std::endl;

                return false;

            }

        }

        return true;

        };

    return insertInterfaces(model.paramInterfaces, "param", 0) &&

        insertInterfaces(model.inputInterfaces, "input", 100) &&

        insertInterfaces(model.outputInterfaces, "output", 200) &&

        insertInterfaces(model.initInterfaces, "init", 300) &&

        insertInterfaces(model.eventInterfaces, "event", 400);

}

bool Database::updateModel(const Model& model) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��������

    const std::string updateSql = "UPDATE models SET name = ?, display_name = ?, uuid = ?, updated_at = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, updateSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, model.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, model.displayName.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, model.uuid.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 4, model.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to update model: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    // ɾ���ɵĽӿ�

    const std::string deleteInterfacesSql = "DELETE FROM model_interfaces WHERE model_id = ?";

    stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, deleteInterfacesSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare delete interfaces statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_int(stmt, 1, model.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    // ���²���ӿ�

    auto insertInterfaces = [&](const std::vector<ModelInterface>& interfaces, const std::string& interfaceType, int startOrder) -> bool {

        for (size_t i = 0; i < interfaces.size(); ++i) {

            const auto& iface = interfaces[i];

            const std::string ifaceSql = R"(

                INSERT INTO model_interfaces 

                (model_id, interface_type, name, data_type, topic, display_name, description, sort_order) 

                VALUES (?, ?, ?, ?, ?, ?, ?, ?)

            )";

            sqlite3_stmt* ifaceStmt = nullptr;

            rc = sqlite3_prepare_v2(m_db, ifaceSql.c_str(), -1, &ifaceStmt, nullptr);

            if (rc != SQLITE_OK) {

                std::cerr << "Failed to prepare interface insert statement: " << sqlite3_errmsg(m_db) << std::endl;

                return false;

            }

            sqlite3_bind_int(ifaceStmt, 1, model.id);

            sqlite3_bind_text(ifaceStmt, 2, interfaceType.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 3, iface.name.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 4, iface.dataType.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 5, iface.topic.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 6, iface.displayName.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_text(ifaceStmt, 7, iface.description.c_str(), -1, SQLITE_STATIC);

            sqlite3_bind_int(ifaceStmt, 8, static_cast<int>(startOrder + i));

            rc = sqlite3_step(ifaceStmt);

            sqlite3_finalize(ifaceStmt);

            if (rc != SQLITE_DONE) {

                std::cerr << "Failed to insert interface: " << sqlite3_errmsg(m_db) << std::endl;

                return false;

            }

        }

        return true;

        };

    return insertInterfaces(model.paramInterfaces, "param", 0) &&

        insertInterfaces(model.inputInterfaces, "input", 100) &&

        insertInterfaces(model.outputInterfaces, "output", 200) &&

        insertInterfaces(model.initInterfaces, "init", 300) &&

        insertInterfaces(model.eventInterfaces, "event", 400);

}

bool Database::deleteModel(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "DELETE FROM models WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

std::vector<ModelFile> Database::getAllModelFilesByLib(int libId) {

    std::vector<ModelFile> files;

    const char* sql = "SELECT * FROM model_files WHERE model_lib_id = ? ORDER BY id DESC";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return files;

    }

    sqlite3_bind_int(stmt, 1, libId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        ModelFile file;

        file.id = sqlite3_column_int(stmt, 0);

        file.directoryId = sqlite3_column_int(stmt, 1);

        file.modelLibId = sqlite3_column_int(stmt, 2);

        file.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        file.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        file.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        file.version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        file.startCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        file.stopCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        file.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        file.fileSize = sqlite3_column_int64(stmt, 10);

        file.fileType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        file.uploaderId = sqlite3_column_int(stmt, 12);

        file.uploaderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));

        file.createdAt = sqlite3_column_int64(stmt, 14);

        file.updatedAt = sqlite3_column_int64(stmt, 15);

        files.push_back(file);

    }

    sqlite3_finalize(stmt);

    return files;

}

//

//std::vector<Model> Database::getAllModels() {

//    std::lock_guard<std::mutex> lock(m_mutex);

//    std::vector<Model> result;

//

//    const std::string sql = "SELECT id, name, display_name, uuid FROM models ORDER BY name";

//    sqlite3_stmt* stmt = nullptr;

//

//    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

//    if (rc != SQLITE_OK) {

//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

//        return result;

//    }

//

//    while (sqlite3_step(stmt) == SQLITE_ROW) {

//        Model model;

//        model.id = sqlite3_column_int(stmt, 0);

//        model.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

//        model.displayName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

//        model.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

//

//        // ��ȡ�ӿ�

//        const std::string ifaceSql = R"(

//            SELECT interface_type, name, data_type, topic, display_name, description 

//            FROM model_interfaces 

//            WHERE model_id = ? 

//            ORDER BY sort_order

//        )";

//        sqlite3_stmt* ifaceStmt = nullptr;

//        rc = sqlite3_prepare_v2(m_db, ifaceSql.c_str(), -1, &ifaceStmt, nullptr);

//

//        if (rc == SQLITE_OK) {

//            sqlite3_bind_int(ifaceStmt, 1, model.id);

//            while (sqlite3_step(ifaceStmt) == SQLITE_ROW) {

//                ModelInterface iface;

//                std::string interfaceType = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 0));

//                iface.name = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 1));

//                iface.dataType = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 2));

//                const char* topic = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 3));

//                if (topic) iface.topic = topic;

//                iface.displayName = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 4));

//                iface.description = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 5));

//

//                if (interfaceType == "param") {

//                    model.paramInterfaces.push_back(iface);

//                }

//                else if (interfaceType == "input") {

//                    model.inputInterfaces.push_back(iface);

//                }

//                else if (interfaceType == "output") {

//                    model.outputInterfaces.push_back(iface);

//                }

//                else if (interfaceType == "init") {

//                    model.initInterfaces.push_back(iface);

//                }

//                else if (interfaceType == "event") {

//                    model.eventInterfaces.push_back(iface);

//                }

//            }

//            sqlite3_finalize(ifaceStmt);

//        }

//

//        result.push_back(model);

//    }

//

//    sqlite3_finalize(stmt);

//    return result;

//}

//

//Model Database::getModelById(int id) {

//    auto all = getAllModels();

//    for (const auto& model : all) {

//        if (model.id == id) return model;

//    }

//    return Model{};

//}

//

//Model Database::getModelByUuid(const std::string& uuid) {

//    auto all = getAllModels();

//    for (const auto& model : all) {

//        if (model.uuid == uuid) return model;

//    }

//    return Model{};

//}

//

//Model Database::getModelByName(const std::string& name) {

//    auto all = getAllModels();

//    for (const auto& model : all) {

//        if (model.name == name) return model;

//    }

//    return Model{};

//}

bool Database::syncDataTypes(const std::vector<DataType>& dataTypes) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��ʼ����

    executeSql("BEGIN TRANSACTION");

    // �����������

    executeSql("DELETE FROM data_type_members");

    executeSql("DELETE FROM data_types");

    // ����������

    for (const auto& dt : dataTypes) {

        if (!insertDataType(dt)) {

            executeSql("ROLLBACK");

            return false;

        }

    }

    executeSql("COMMIT");

    return true;

}

bool Database::syncModels(const std::vector<Model>& models) {

    std::lock_guard<std::mutex> lock(m_mutex);

    executeSql("BEGIN TRANSACTION");

    executeSql("DELETE FROM model_interfaces");

    executeSql("DELETE FROM models");

    for (const auto& model : models) {

        if (!insertModel(model)) {

            executeSql("ROLLBACK");

            return false;

        }

    }

    executeSql("COMMIT");

    return true;

}

// �� createTables() ����������

bool Database::createProjectsTable() {

    const std::string createProjectsTable = R"(

        CREATE TABLE IF NOT EXISTS projects (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            name TEXT NOT NULL,

            description TEXT,

            owner_id INTEGER NOT NULL,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now')),

            is_public INTEGER DEFAULT 0,

            data TEXT,

            FOREIGN KEY (owner_id) REFERENCES users(id) ON DELETE CASCADE

        );

    )";

    const std::string createProjectMembersTable = R"(

        CREATE TABLE IF NOT EXISTS project_members (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            project_id INTEGER NOT NULL,

            user_id INTEGER NOT NULL,

            permission INTEGER DEFAULT 2,

            added_at INTEGER DEFAULT (strftime('%s', 'now')),

            FOREIGN KEY (project_id) REFERENCES projects(id) ON DELETE CASCADE,

            FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,

            UNIQUE(project_id, user_id)

        );

    )";

    return executeSql(createProjectsTable) && executeSql(createProjectMembersTable);

}

// ���̲���ʵ��

std::vector<Project> Database::getAllProjects(int userId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<Project> result;

    std::string sql;

    if (userId == -1) {

        sql = "SELECT p.id, p.name, p.description, p.owner_id, u.username, p.created_at, p.updated_at, p.is_public, p.data "

            "FROM projects p LEFT JOIN users u ON p.owner_id = u.id ORDER BY p.updated_at DESC";

    }

    else {

        sql = "SELECT DISTINCT p.id, p.name, p.description, p.owner_id, u.username, p.created_at, p.updated_at, p.is_public, p.data "

            "FROM projects p "

            "LEFT JOIN users u ON p.owner_id = u.id "

            "LEFT JOIN project_members pm ON p.id = pm.project_id "

            "WHERE p.owner_id = ? OR pm.user_id = ? OR p.is_public = 1 "

            "ORDER BY p.updated_at DESC";

    }

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return result;

    }

    if (userId != -1) {

        sqlite3_bind_int(stmt, 1, userId);

        sqlite3_bind_int(stmt, 2, userId);

    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        Project project;

        project.id = sqlite3_column_int(stmt, 0);

        project.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if (desc) project.description = desc;

        project.ownerId = sqlite3_column_int(stmt, 3);

        project.ownerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        project.createdAt = sqlite3_column_int(stmt, 5);

        project.updatedAt = sqlite3_column_int(stmt, 6);

        project.isPublic = sqlite3_column_int(stmt, 7) == 1;

        const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (data) project.data = data;

        result.push_back(project);

    }

    sqlite3_finalize(stmt);

    return result;

}

Project Database::getProjectById(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    Project project;

    project.id = -1;

    const std::string sql = "SELECT p.id, p.name, p.description, p.owner_id, u.username, p.created_at, p.updated_at, p.is_public, p.data "

        "FROM projects p LEFT JOIN users u ON p.owner_id = u.id WHERE p.id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return project;

    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        project.id = sqlite3_column_int(stmt, 0);

        project.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        if (desc) project.description = desc;

        project.ownerId = sqlite3_column_int(stmt, 3);

        project.ownerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        project.createdAt = sqlite3_column_int(stmt, 5);

        project.updatedAt = sqlite3_column_int(stmt, 6);

        project.isPublic = sqlite3_column_int(stmt, 7) == 1;

        const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (data) project.data = data;

    }

    sqlite3_finalize(stmt);

    return project;

}

bool Database::insertProject(const Project& project) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "INSERT INTO projects (name, description, owner_id, is_public, data) VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_text(stmt, 1, project.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, project.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 3, project.ownerId);

    sqlite3_bind_int(stmt, 4, project.isPublic ? 1 : 0);

    sqlite3_bind_text(stmt, 5, project.data.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::updateProject(const Project& project) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "UPDATE projects SET name = ?, description = ?, is_public = ?, data = ?, updated_at = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_text(stmt, 1, project.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, project.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 3, project.isPublic ? 1 : 0);

    sqlite3_bind_text(stmt, 4, project.data.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 5, project.id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::deleteProject(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "DELETE FROM projects WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::addProjectMember(int projectId, int userId, int permission) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "INSERT OR REPLACE INTO project_members (project_id, user_id, permission) VALUES (?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    sqlite3_bind_int(stmt, 2, userId);

    sqlite3_bind_int(stmt, 3, permission);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

std::vector<ProjectMember> Database::getProjectMembers(int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<ProjectMember> result;

    const std::string sql = "SELECT pm.id, pm.project_id, pm.user_id, u.username, pm.permission, pm.added_at "

        "FROM project_members pm LEFT JOIN users u ON pm.user_id = u.id WHERE pm.project_id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return result;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        ProjectMember member;

        member.id = sqlite3_column_int(stmt, 0);

        member.projectId = sqlite3_column_int(stmt, 1);

        member.userId = sqlite3_column_int(stmt, 2);

        member.userName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        member.permission = sqlite3_column_int(stmt, 4);

        member.addedAt = sqlite3_column_int(stmt, 5);

        result.push_back(member);

    }

    sqlite3_finalize(stmt);

    return result;

}

std::vector<DataType> Database::getAllDataTypes(int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<DataType> result;

    const std::string sql = "SELECT id, name, description, uuid FROM data_types WHERE project_id = ? ORDER BY name";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return result;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        DataType dt;

        dt.id = sqlite3_column_int(stmt, 0);

        dt.projectId = projectId;

        dt.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        dt.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        dt.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        // ��ȡ��Ա

        const std::string memberSql = R"(

            SELECT display_name, type, sub_type, range, var_name, default_value, unit, description 

            FROM data_type_members 

            WHERE data_type_id = ? 

            ORDER BY sort_order

        )";

        sqlite3_stmt* memberStmt = nullptr;

        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

        if (rc == SQLITE_OK) {

            sqlite3_bind_int(memberStmt, 1, dt.id);

            while (sqlite3_step(memberStmt) == SQLITE_ROW) {

                DataTypeMember member;

                member.displayName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 0));

                member.type = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 1));

                const char* subType = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 2));

                if (subType) member.subType = subType;

                member.range = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 3));

                member.varName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 4));

                member.defaultValue = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 5));

                member.unit = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 6));

                member.description = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 7));

                dt.members.push_back(member);

            }

            sqlite3_finalize(memberStmt);

        }

        result.push_back(dt);

    }

    sqlite3_finalize(stmt);

    return result;

}

// �������ݽṹ

bool Database::insertDataType(const DataType& dataType, int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    sqlite3_stmt* stmt = nullptr;

    const std::string sql = "INSERT INTO data_types (project_id, name, description, uuid) VALUES (?, ?, ?, ?)";

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    sqlite3_bind_text(stmt, 2, dataType.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, dataType.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 4, dataType.uuid.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        std::cerr << "Failed to insert data type: " << sqlite3_errmsg(m_db) << std::endl;

        return false;

    }

    int dataTypeId = sqlite3_last_insert_rowid(m_db);

    // �����Ա

    for (size_t i = 0; i < dataType.members.size(); ++i) {

        const auto& member = dataType.members[i];

        const std::string memberSql = R"(

            INSERT INTO data_type_members 

            (data_type_id, display_name, type, sub_type, range, var_name, default_value, unit, description, sort_order) 

            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)

        )";

        sqlite3_stmt* memberStmt = nullptr;

        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

        if (rc != SQLITE_OK) {

            std::cerr << "Failed to prepare member statement: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

        sqlite3_bind_int(memberStmt, 1, dataTypeId);

        sqlite3_bind_text(memberStmt, 2, member.displayName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 3, member.type.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 4, member.subType.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 5, member.range.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 6, member.varName.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 7, member.defaultValue.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 8, member.unit.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_text(memberStmt, 9, member.description.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_int(memberStmt, 10, static_cast<int>(i));

        rc = sqlite3_step(memberStmt);

        sqlite3_finalize(memberStmt);

        if (rc != SQLITE_DONE) {

            std::cerr << "Failed to insert data type member: " << sqlite3_errmsg(m_db) << std::endl;

            return false;

        }

    }

    return true;

}

// ����û��Թ��̵ķ���Ȩ��

bool Database::hasProjectAccess(int userId, int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��ȡ������Ϣ

    const std::string projectSql = "SELECT owner_id, is_public FROM projects WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, projectSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    int ownerId = -1;

    int isPublic = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        ownerId = sqlite3_column_int(stmt, 0);

        isPublic = sqlite3_column_int(stmt, 1);

    }

    sqlite3_finalize(stmt);

    if (ownerId == -1) return false;

    // �����߿��Է���

    if (ownerId == userId) return true;

    // �������̿��Է���

    if (isPublic) return true;

    // ����Ƿ��ǹ��̳�Ա

    const std::string memberSql = "SELECT 1 FROM project_members WHERE project_id = ? AND user_id = ?";

    rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    sqlite3_bind_int(stmt, 2, userId);

    bool hasAccess = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);

    return hasAccess;

}

bool Database::hasProjectEditPermission(int userId, int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��ȡ������Ϣ

    const std::string projectSql = "SELECT owner_id FROM projects WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, projectSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    int ownerId = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        ownerId = sqlite3_column_int(stmt, 0);

    }

    sqlite3_finalize(stmt);

    // �����߿��Ա༭

    if (ownerId == userId) return true;

    // ����Ƿ��Ǳ༭��Ȩ�޵ĳ�Ա

    const std::string memberSql = "SELECT permission FROM project_members WHERE project_id = ? AND user_id = ?";

    rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    sqlite3_bind_int(stmt, 2, userId);

    int permission = -1;

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        permission = sqlite3_column_int(stmt, 0);

    }

    sqlite3_finalize(stmt);

    // permission: 0=owner, 1=editor, 2=viewer

    return permission == 0 || permission == 1;

}

//

//// �޸� getAllDataTypes ���������� projectId ����

//std::vector<DataType> Database::getAllDataTypes(int projectId) {

//    std::lock_guard<std::mutex> lock(m_mutex);

//    std::vector<DataType> result;

//

//    const std::string sql = "SELECT id, name, description, uuid FROM data_types WHERE project_id = ? ORDER BY name";

//    sqlite3_stmt* stmt = nullptr;

//

//    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

//    if (rc != SQLITE_OK) {

//        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

//        return result;

//    }

//

//    sqlite3_bind_int(stmt, 1, projectId);

//

//    while (sqlite3_step(stmt) == SQLITE_ROW) {

//        DataType dt;

//        dt.id = sqlite3_column_int(stmt, 0);

//        dt.projectId = projectId;

//        dt.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

//        dt.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

//        dt.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

//

//        // ��ȡ��Ա

//        const std::string memberSql = R"(

//            SELECT display_name, type, sub_type, range, var_name, default_value, unit, description 

//            FROM data_type_members 

//            WHERE data_type_id = ? 

//            ORDER BY sort_order

//        )";

//        sqlite3_stmt* memberStmt = nullptr;

//        rc = sqlite3_prepare_v2(m_db, memberSql.c_str(), -1, &memberStmt, nullptr);

//

//        if (rc == SQLITE_OK) {

//            sqlite3_bind_int(memberStmt, 1, dt.id);

//            while (sqlite3_step(memberStmt) == SQLITE_ROW) {

//                DataTypeMember member;

//                member.displayName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 0));

//                member.type = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 1));

//                const char* subType = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 2));

//                if (subType) member.subType = subType;

//                member.range = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 3));

//                member.varName = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 4));

//                member.defaultValue = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 5));

//                member.unit = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 6));

//                member.description = reinterpret_cast<const char*>(sqlite3_column_text(memberStmt, 7));

//                dt.members.push_back(member);

//            }

//            sqlite3_finalize(memberStmt);

//        }

//

//        result.push_back(dt);

//    }

//

//    sqlite3_finalize(stmt);

//    return result;

//}

// �޸� getAllModels ���������� projectId ����

std::vector<Model> Database::getAllModels(int projectId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<Model> result;

    const std::string sql = "SELECT id, name, display_name, uuid FROM models WHERE project_id = ? ORDER BY name";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(m_db) << std::endl;

        return result;

    }

    sqlite3_bind_int(stmt, 1, projectId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        Model model;

        model.id = sqlite3_column_int(stmt, 0);

        model.projectId = projectId;

        model.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        model.displayName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        model.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        // ��ȡ�ӿ�

        const std::string ifaceSql = R"(

            SELECT interface_type, name, data_type, topic, display_name, description 

            FROM model_interfaces 

            WHERE model_id = ? 

            ORDER BY sort_order

        )";

        sqlite3_stmt* ifaceStmt = nullptr;

        rc = sqlite3_prepare_v2(m_db, ifaceSql.c_str(), -1, &ifaceStmt, nullptr);

        if (rc == SQLITE_OK) {

            sqlite3_bind_int(ifaceStmt, 1, model.id);

            while (sqlite3_step(ifaceStmt) == SQLITE_ROW) {

                ModelInterface iface;

                std::string interfaceType = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 0));

                iface.name = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 1));

                iface.dataType = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 2));

                const char* topic = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 3));

                if (topic) iface.topic = topic;

                iface.displayName = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 4));

                iface.description = reinterpret_cast<const char*>(sqlite3_column_text(ifaceStmt, 5));

                if (interfaceType == "param") {

                    model.paramInterfaces.push_back(iface);

                }

                else if (interfaceType == "input") {

                    model.inputInterfaces.push_back(iface);

                }

                else if (interfaceType == "output") {

                    model.outputInterfaces.push_back(iface);

                }

                else if (interfaceType == "init") {

                    model.initInterfaces.push_back(iface);

                }

                else if (interfaceType == "event") {

                    model.eventInterfaces.push_back(iface);

                }

            }

            sqlite3_finalize(ifaceStmt);

        }

        result.push_back(model);

    }

    sqlite3_finalize(stmt);

    return result;

}

/********************************* ģ�Ϳ��������*************************/

bool Database::createModelManageTables() {

    // ģ�Ϳ��

    const std::string createModelLibsTable = R"(

        CREATE TABLE IF NOT EXISTS model_libs (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            name TEXT NOT NULL,

            type INTEGER NOT NULL,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            UNIQUE(name, type)

        );

    )";

    // Ŀ¼��

    const std::string createDirectoriesTable = R"(

        CREATE TABLE IF NOT EXISTS directories (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            parent_id INTEGER DEFAULT 0,

            model_lib_id INTEGER NOT NULL,

            name TEXT NOT NULL,

            path TEXT,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now')),

            FOREIGN KEY (model_lib_id) REFERENCES model_libs(id) ON DELETE CASCADE

        );

    )";

    // ģ���ļ���

    const std::string createModelFilesTable = R"(

        CREATE TABLE IF NOT EXISTS model_files (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            directory_id INTEGER NOT NULL,

            model_lib_id INTEGER NOT NULL,

            name TEXT NOT NULL,

            file_name TEXT NOT NULL,

            file_path TEXT NOT NULL,

            version TEXT,

            start_cmd TEXT,

            stop_cmd TEXT,

            description TEXT,

            file_size INTEGER,

            file_type TEXT,

            uploader_id INTEGER,

            uploader_name TEXT,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now')),

            FOREIGN KEY (directory_id) REFERENCES directories(id) ON DELETE CASCADE,

            FOREIGN KEY (model_lib_id) REFERENCES model_libs(id) ON DELETE CASCADE

        );

    )";

    // ������

    const std::string createSoftwareTable = R"(

        CREATE TABLE IF NOT EXISTS software (

            id INTEGER PRIMARY KEY AUTOINCREMENT,

            name TEXT NOT NULL,

            file_name TEXT NOT NULL,

            file_path TEXT NOT NULL,

            version TEXT,

            description TEXT,

            install_cmd TEXT,

            uninstall_cmd TEXT,

            start_cmd TEXT,

            stop_cmd TEXT,

            file_size INTEGER,

            file_type TEXT,

            uploader_id INTEGER,

            uploader_name TEXT,

            created_at INTEGER DEFAULT (strftime('%s', 'now')),

            updated_at INTEGER DEFAULT (strftime('%s', 'now'))

        );

    )";

    bool result = executeSql(createModelLibsTable) &&

                  executeSql(createDirectoriesTable) &&

                  executeSql(createModelFilesTable) &&

                  executeSql(createSoftwareTable);

    if (result) {

        // ��ʼ������ģ�Ϳ�

        createModelLib(gbk_to_utf8("������"), ModelLibType::DEVELOPMENT);

        createModelLib(gbk_to_utf8("�ܿؿ�"), ModelLibType::CONTROLLED);

        createModelLib(gbk_to_utf8("��Ʒ��"), ModelLibType::PRODUCT);

    }

    return result;

}

int Database::createModelLib(const std::string& name, ModelLibType type) {

    //std::lock_guard<std::mutex> lock(m_mutex);

    // ����Ƿ��Ѵ���

    const std::string checkSql = "SELECT id FROM model_libs WHERE name = ? AND type = ?";

    sqlite3_stmt* checkStmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, checkSql.c_str(), -1, &checkStmt, nullptr);

    if (rc == SQLITE_OK) {

        sqlite3_bind_text(checkStmt, 1, name.c_str(), -1, SQLITE_STATIC);

        sqlite3_bind_int(checkStmt, 2, static_cast<int>(type));

        if (sqlite3_step(checkStmt) == SQLITE_ROW) {

            int id = sqlite3_column_int(checkStmt, 0);

            sqlite3_finalize(checkStmt);

            return id;

        }

    }

    sqlite3_finalize(checkStmt);

    const std::string sql = "INSERT INTO model_libs (name, type) VALUES (?, ?)";

    sqlite3_stmt* stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return -1;

    }

    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 2, static_cast<int>(type));

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        return -1;

    }

    return sqlite3_last_insert_rowid(m_db);

}

std::vector<std::pair<int, std::string>> Database::getAllModelLibs() {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<std::pair<int, std::string>> result;

    const std::string sql = "SELECT id, name FROM model_libs ORDER BY type";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return result;

    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        result.push_back({sqlite3_column_int(stmt, 0), 

                          reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))});

    }

    sqlite3_finalize(stmt);

    return result;

}

int Database::createDirectory(int parentId, int modelLibId, const std::string& name) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "INSERT INTO directories (parent_id, model_lib_id, name) VALUES (?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return -1;

    }

    sqlite3_bind_int(stmt, 1, parentId);

    sqlite3_bind_int(stmt, 2, modelLibId);

    sqlite3_bind_text(stmt, 3, name.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        return -1;

    }

    return sqlite3_last_insert_rowid(m_db);

}

bool Database::deleteDirectory(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // ��ɾ��Ŀ¼�µ������ļ�

    const std::string deleteFilesSql = "DELETE FROM model_files WHERE directory_id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, deleteFilesSql.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {

        sqlite3_bind_int(stmt, 1, id);

        sqlite3_step(stmt);

        sqlite3_finalize(stmt);

    }

    // ɾ����Ŀ¼

    const std::string deleteSubdirsSql = "DELETE FROM directories WHERE parent_id = ?";

    stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, deleteSubdirsSql.c_str(), -1, &stmt, nullptr);

    if (rc == SQLITE_OK) {

        sqlite3_bind_int(stmt, 1, id);

        sqlite3_step(stmt);

        sqlite3_finalize(stmt);

    }

    // ɾ��Ŀ¼����

    const std::string sql = "DELETE FROM directories WHERE id = ?";

    stmt = nullptr;

    rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

bool Database::renameDirectory(int id, const std::string& newName) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "UPDATE directories SET name = ?, updated_at = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_text(stmt, 1, newName.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

std::vector<Directory> Database::getDirectories(int modelLibId, int parentId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<Directory> result;

    const std::string sql = "SELECT id, parent_id, model_lib_id, name, path, created_at, updated_at "

                            "FROM directories WHERE model_lib_id = ? AND parent_id = ? ORDER BY name";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return result;

    }

    sqlite3_bind_int(stmt, 1, modelLibId);

    sqlite3_bind_int(stmt, 2, parentId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        Directory dir;

        dir.id = sqlite3_column_int(stmt, 0);

        dir.parentId = sqlite3_column_int(stmt, 1);

        dir.modelLibId = sqlite3_column_int(stmt, 2);

        dir.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (path) dir.path = path;

        dir.createdAt = sqlite3_column_int(stmt, 5);

        dir.updatedAt = sqlite3_column_int(stmt, 6);

        result.push_back(dir);

    }

    sqlite3_finalize(stmt);

    return result;

}

int Database::addModelFile(const ModelFile& modelFile) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = R"(

        INSERT INTO model_files 

        (directory_id, model_lib_id, name, file_name, file_path, version, start_cmd, stop_cmd, 

         description, file_size, file_type, uploader_id, uploader_name) 

        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)

    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return -1;

    }

    sqlite3_bind_int(stmt, 1, modelFile.directoryId);

    sqlite3_bind_int(stmt, 2, modelFile.modelLibId);

    sqlite3_bind_text(stmt, 3, modelFile.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 4, modelFile.fileName.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 5, modelFile.filePath.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 6, modelFile.version.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 7, modelFile.startCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 8, modelFile.stopCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 9, modelFile.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int64(stmt, 10, modelFile.fileSize);

    sqlite3_bind_text(stmt, 11, modelFile.fileType.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 12, modelFile.uploaderId);

    sqlite3_bind_text(stmt, 13, modelFile.uploaderName.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        return -1;

    }

    return sqlite3_last_insert_rowid(m_db);

}

bool Database::deleteModelFile(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // �Ȼ�ȡ�ļ�·���Ա�ɾ�������ļ�

    ModelFile file = getModelFileById(id);

    if (file.id > 0 && !file.filePath.empty()) {

        boost::filesystem::remove(file.filePath);

    }

    const std::string sql = "DELETE FROM model_files WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

ModelFile Database::getModelFileById(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    ModelFile file;

    file.id = -1;

    const std::string sql = "SELECT id, directory_id, model_lib_id, name, file_name, file_path, "

                            "version, start_cmd, stop_cmd, description, file_size, file_type, "

                            "uploader_id, uploader_name, created_at, updated_at "

                            "FROM model_files WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return file;

    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        file.id = sqlite3_column_int(stmt, 0);

        file.directoryId = sqlite3_column_int(stmt, 1);

        file.modelLibId = sqlite3_column_int(stmt, 2);

        file.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        file.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        file.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        const char* version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        if (version) file.version = version;

        const char* startCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        if (startCmd) file.startCmd = startCmd;

        const char* stopCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (stopCmd) file.stopCmd = stopCmd;

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        if (desc) file.description = desc;

        file.fileSize = sqlite3_column_int64(stmt, 10);

        const char* fileType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        if (fileType) file.fileType = fileType;

        file.uploaderId = sqlite3_column_int(stmt, 12);

        file.uploaderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));

        file.createdAt = sqlite3_column_int(stmt, 14);

        file.updatedAt = sqlite3_column_int(stmt, 15);

    }

    sqlite3_finalize(stmt);

    return file;

}

std::vector<ModelFile> Database::getModelFiles(int modelLibId, int directoryId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<ModelFile> result;

    const std::string sql = "SELECT id, directory_id, model_lib_id, name, file_name, file_path, "

                            "version, start_cmd, stop_cmd, description, file_size, file_type, "

                            "uploader_id, uploader_name, created_at, updated_at "

                            "FROM model_files WHERE model_lib_id = ? AND directory_id = ? ORDER BY created_at DESC";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return result;

    }

    sqlite3_bind_int(stmt, 1, modelLibId);

    sqlite3_bind_int(stmt, 2, directoryId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        ModelFile file;

        file.id = sqlite3_column_int(stmt, 0);

        file.directoryId = sqlite3_column_int(stmt, 1);

        file.modelLibId = sqlite3_column_int(stmt, 2);

        file.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        file.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        file.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        const char* version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        if (version) file.version = version;

        const char* startCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        if (startCmd) file.startCmd = startCmd;

        const char* stopCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (stopCmd) file.stopCmd = stopCmd;

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        if (desc) file.description = desc;

        file.fileSize = sqlite3_column_int64(stmt, 10);

        const char* fileType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        if (fileType) file.fileType = fileType;

        file.uploaderId = sqlite3_column_int(stmt, 12);

        file.uploaderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));

        file.createdAt = sqlite3_column_int(stmt, 14);

        file.updatedAt = sqlite3_column_int(stmt, 15);

        result.push_back(file);

    }

    sqlite3_finalize(stmt);

    return result;

}

bool Database::migrateModelFile(int modelFileId, int targetLibId, int targetDirId) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = "UPDATE model_files SET model_lib_id = ?, directory_id = ?, updated_at = strftime('%s', 'now') WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, targetLibId);

    sqlite3_bind_int(stmt, 2, targetDirId);

    sqlite3_bind_int(stmt, 3, modelFileId);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

int Database::addSoftware(const Software& software) {

    std::lock_guard<std::mutex> lock(m_mutex);

    const std::string sql = R"(

        INSERT INTO software 

        (name, file_name, file_path, version, description, install_cmd, uninstall_cmd, start_cmd, stop_cmd,

         file_size, file_type, uploader_id, uploader_name) 

        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)

    )";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return -1;

    }

    sqlite3_bind_text(stmt, 1, software.name.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, software.fileName.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 3, software.filePath.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 4, software.version.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 5, software.description.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 6, software.installCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 7, software.uninstallCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 8, software.startCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 9, software.stopCmd.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int64(stmt, 10, software.fileSize);

    sqlite3_bind_text(stmt, 11, software.fileType.c_str(), -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 12, software.uploaderId);

    sqlite3_bind_text(stmt, 13, software.uploaderName.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {

        return -1;

    }

    return sqlite3_last_insert_rowid(m_db);

}

bool Database::deleteSoftware(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    // �Ȼ�ȡ�ļ�·���Ա�ɾ�������ļ�

    Software sw = getSoftwareById(id);

    if (sw.id > 0 && !sw.filePath.empty()) {

        boost::filesystem::remove(sw.filePath);

    }

    const std::string sql = "DELETE FROM software WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return false;

    }

    sqlite3_bind_int(stmt, 1, id);

    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    return rc == SQLITE_DONE;

}

Software Database::getSoftwareById(int id) {

    std::lock_guard<std::mutex> lock(m_mutex);

    Software sw;

    sw.id = -1;

    const std::string sql = "SELECT id, name, file_name, file_path, version, description, "

                            "install_cmd, uninstall_cmd, start_cmd, stop_cmd, "

                            "file_size, file_type, uploader_id, uploader_name, created_at, updated_at "

                            "FROM software WHERE id = ?";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return sw;

    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {

        sw.id = sqlite3_column_int(stmt, 0);

        sw.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        sw.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        sw.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const char* version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (version) sw.version = version;

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        if (desc) sw.description = desc;

        const char* installCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        if (installCmd) sw.installCmd = installCmd;

        const char* uninstallCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        if (uninstallCmd) sw.uninstallCmd = uninstallCmd;

        const char* startCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (startCmd) sw.startCmd = startCmd;

        const char* stopCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        if (stopCmd) sw.stopCmd = stopCmd;

        sw.fileSize = sqlite3_column_int64(stmt, 10);

        const char* fileType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        if (fileType) sw.fileType = fileType;

        sw.uploaderId = sqlite3_column_int(stmt, 12);

        sw.uploaderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));

        sw.createdAt = sqlite3_column_int(stmt, 14);

        sw.updatedAt = sqlite3_column_int(stmt, 15);

    }

    sqlite3_finalize(stmt);

    return sw;

}

std::vector<Software> Database::getAllSoftware() {

    std::lock_guard<std::mutex> lock(m_mutex);

    std::vector<Software> result;

    const std::string sql = "SELECT id, name, file_name, file_path, version, description, "

                            "install_cmd, uninstall_cmd, start_cmd, stop_cmd, "

                            "file_size, file_type, uploader_id, uploader_name, created_at, updated_at "

                            "FROM software ORDER BY created_at DESC";

    sqlite3_stmt* stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_db, sql.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {

        return result;

    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {

        Software sw;

        sw.id = sqlite3_column_int(stmt, 0);

        sw.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));

        sw.fileName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));

        sw.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

        const char* version = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));

        if (version) sw.version = version;

        const char* desc = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        if (desc) sw.description = desc;

        const char* installCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

        if (installCmd) sw.installCmd = installCmd;

        const char* uninstallCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));

        if (uninstallCmd) sw.uninstallCmd = uninstallCmd;

        const char* startCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));

        if (startCmd) sw.startCmd = startCmd;

        const char* stopCmd = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));

        if (stopCmd) sw.stopCmd = stopCmd;

        sw.fileSize = sqlite3_column_int64(stmt, 10);

        const char* fileType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));

        if (fileType) sw.fileType = fileType;

        sw.uploaderId = sqlite3_column_int(stmt, 12);

        sw.uploaderName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));

        sw.createdAt = sqlite3_column_int(stmt, 14);

        sw.updatedAt = sqlite3_column_int(stmt, 15);

        result.push_back(sw);

    }

    sqlite3_finalize(stmt);

    return result;

}
