#ifndef ENUMS_H
#define ENUMS_H

namespace LocalNetworkApp {

// 用户状态枚举
enum class UserState {
    Online,    // 在线
    DoNotDisturb, // 勿扰
    Invisible  // 隐身
};

// 消息类型枚举
enum class MessageType {
    Text,           // 文本消息
    System,         // 系统消息
    FileTransferNotification, // 文件传输通知
    UserStatusChange // 用户状态变更通知
};

// 网络消息类型枚举
enum class NetworkMessageType {
    UserStatus,          // 用户状态变更
    ChatMessage,         // 聊天消息
    FileTransferRequest, // 文件传输请求
    FileTransferResponse, // 文件传输响应
    FileData,            // 文件数据块
    UserDiscovery,       // 用户发现广播
    Heartbeat            // 心跳包
};

// 文件传输状态枚举
enum class FileTransferStatus {
    Pending,    // 等待中
    Accepted,   // 已接受
    Rejected,   // 已拒绝
    Transferring, // 传输中
    Paused,     // 已暂停
    Completed,  // 已完成
    Failed,     // 失败
    Cancelled   // 已取消
};

} // namespace LocalNetworkApp

#endif // ENUMS_H
