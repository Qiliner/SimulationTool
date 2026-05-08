#pragma once

#include <string>

#include <vector>

#include <fstream>

#include <iostream>

#include <filesystem>

#ifdef _WIN32

#include <windows.h>

#else

#include <dirent.h>

#include <sys/stat.h>

#endif

class ZipUtils {

public:

    // 使用系统命令打包文件夹

    static bool zipFolder(const std::string& folderPath, const std::string& zipFilePath);

private:

    static int executeCmd(const char* cmd, std::string& result);

};
