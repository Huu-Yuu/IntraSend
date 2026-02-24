#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>

namespace LocalNetworkApp {
namespace Constants {

// 网络相关常量
constexpr quint16 DEFAULT_TCP_PORT = 8888;
constexpr quint16 DEFAULT_UDP_PORT = 8889;
constexpr int HEARTBEAT_INTERVAL_MS = 5000; // 心跳间隔，单位毫秒
constexpr int USER_TIMEOUT_MS = 15000; // 用户超时时间，单位毫秒

// 文件传输相关常量
constexpr int FILE_BLOCK_SIZE = 8192; // 文件块大小，8KB
constexpr int MAX_PENDING_BLOCKS = 100; // 最大待处理块数

// 数据库相关常量
const QString DATABASE_NAME = "local_network_app.db";
const QString USER_TABLE = "users";
const QString CONTACTS_TABLE = "contacts";
const QString MESSAGES_TABLE = "messages";
const QString FILE_TRANSFERS_TABLE = "file_transfers";
const QString PASSWORDS_TABLE = "passwords";

// 设置相关常量
const QString SETTINGS_FILE = "settings.ini";
const QString DEFAULT_NICKNAME = "用户";
const QString DEFAULT_DOWNLOAD_PATH = "./downloads";

// 界面相关常量
const int MIN_WINDOW_WIDTH = 800;
const int MIN_WINDOW_HEIGHT = 600;

// 安全相关常量
const int MAX_PASSWORD_LENGTH = 50;
const int MIN_PASSWORD_LENGTH = 6;

} // namespace Constants
} // namespace LocalNetworkApp

#endif // CONSTANTS_H
