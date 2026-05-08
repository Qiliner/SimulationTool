#include "ZipUtils.h"

#include <boost/filesystem.hpp>

using namespace std;

namespace fs = boost::filesystem;

bool ZipUtils::zipFolder(const std::string& folderPath, const std::string& zipFilePath)

{

    {

        if (!fs::exists(folderPath)) {

            std::cerr << "Folder does not exist: " << folderPath << std::endl;

            return false;

        }

        std::string cmd;

#ifdef _WIN32

        // 检查是否有7-Zip

        std::string sevenZipPaths[] = {

            "C:/Program Files/7-Zip/7z.exe",

            "C:/Program Files (x86)/7-Zip/7z.exe",

            "7z.exe"

        };

        bool sevenZipFound = false;

        for (const auto& path : sevenZipPaths) {

            if (fs::exists(path)) {

                cmd = "\"" + path + "\" a -tzip \"" + zipFilePath + "\" \"" + folderPath + "\\*\" -r";

                sevenZipFound = true;

                break;

            }

        }

        if (!sevenZipFound) {

            // 使用PowerShell

            cmd = "powershell -Command \"Compress-Archive -Path '" + folderPath + "\\*' -DestinationPath '" + zipFilePath + "' -Force\"";

        }

#else

        cmd = "zip -r \"" + zipFilePath + "\" \"" + folderPath + "\"";

#endif

        std::string result;

        int ret = executeCmd(cmd.c_str(), result);

        if (ret != 0) {

            std::cerr << "Zip command failed: " << cmd << std::endl;

            std::cerr << "Result: " << result << std::endl;

            return false;

        }

        return fs::exists(zipFilePath);

    }

}

int ZipUtils::executeCmd(const char* cmd, std::string& result)

{

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

	return 0;

}
