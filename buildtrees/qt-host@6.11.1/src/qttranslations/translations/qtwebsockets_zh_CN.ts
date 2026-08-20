<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>QQmlWebSocket</name>
    <message>
        <source>Messages can only be sent when the socket is open.</source>
        <translation>只有在套接字打开时才能发送消息。</translation>
    </message>
    <message>
        <source>QQmlWebSocket is not ready.</source>
        <translation>QQmlWebSocket未准备好。</translation>
    </message>
</context>
<context>
    <name>QQmlWebSocketServer</name>
    <message>
        <source>QQmlWebSocketServer is not ready.</source>
        <translation>QQmlWebSocketServer未准备好。</translation>
    </message>
</context>
<context>
    <name>QWebSocket</name>
    <message>
        <source>Connection closed</source>
        <translation>连接已关闭</translation>
    </message>
    <message>
        <source>Invalid URL.</source>
        <translation>无效的URL。</translation>
    </message>
    <message>
        <source>Invalid resource name.</source>
        <translation>无效的资源名。</translation>
    </message>
    <message>
        <source>SSL Sockets are not supported on this platform.</source>
        <translation>此平台不支持 SSL 套接字。</translation>
    </message>
    <message>
        <source>Out of memory.</source>
        <translation>内存不足。</translation>
    </message>
    <message>
        <source>Unsupported WebSocket scheme: %1</source>
        <translation>不支持的 WebSocket 方案: %1</translation>
    </message>
    <message>
        <source>Error writing bytes to socket: %1.</source>
        <translation>写入套接字时出错: %1。</translation>
    </message>
    <message>
        <source>Bytes written %1 != %2.</source>
        <translation>写入的字节数 %1 != %2.</translation>
    </message>
    <message>
        <source>Invalid statusline in response: %1.</source>
        <translation>响应的状态行无效: %1.</translation>
    </message>
    <message>
        <source>QWebSocketPrivate::processHandshake: Connection closed while reading header.</source>
        <translation>QWebSocketPrivate::processHandshake: 读取标题时连接关闭。</translation>
    </message>
    <message>
        <source>Accept-Key received from server %1 does not match the client key %2.</source>
        <translation>从服务器 %1 收到的Accept-Key与客户端密钥 %2 不匹配。</translation>
    </message>
    <message>
        <source>QWebSocketPrivate::processHandshake: Invalid statusline in response: %1.</source>
        <translation>QWebSocketPrivate::processHandshake: 响应的状态行无效: %1.</translation>
    </message>
    <message>
        <source>Handshake: Server requests a version that we don&apos;t support: %1.</source>
        <translation>Handshake: 不支持服务器请求的版本: %1.</translation>
    </message>
    <message>
        <source>QWebSocketPrivate::processHandshake: Unknown error condition encountered. Aborting connection.</source>
        <translation>QWebSocketPrivate::processHandshake: 出现未知错误。取消连接。</translation>
    </message>
    <message>
        <source>QWebSocketPrivate::processHandshake: Unhandled http status code: %1 (%2).</source>
        <translation>QWebSocketPrivate::processHandshake: 未处理的 http 状态码: %1 (%2).</translation>
    </message>
    <message>
        <source>The resource name contains newlines. Possible attack detected.</source>
        <translation>资源名称中有空行。检测到可能的攻击。</translation>
    </message>
    <message>
        <source>The hostname contains newlines. Possible attack detected.</source>
        <translation>主机名称中有空行。检测到可能的攻击。</translation>
    </message>
    <message>
        <source>The origin contains newlines. Possible attack detected.</source>
        <translation>原文有空行。检测到可能的攻击。</translation>
    </message>
    <message>
        <source>The extensions attribute contains newlines. Possible attack detected.</source>
        <translation>扩展属性中有空行。检测到可能的攻击。</translation>
    </message>
    <message>
        <source>The protocols attribute contains newlines. Possible attack detected.</source>
        <translation>协议属性中有空行。检测到可能的攻击。</translation>
    </message>
</context>
<context>
    <name>QWebSocketDataProcessor</name>
    <message>
        <source>Received Continuation frame, while there is nothing to continue.</source>
        <translation>收到了一个延续帧，但相应的帧不存在。</translation>
    </message>
    <message>
        <source>All data frames after the initial data frame must have opcode 0 (continuation).</source>
        <translation>初始数据帧之后的所有数据帧的操作码必须为 0（连续帧）。</translation>
    </message>
    <message>
        <source>Received message is too big.</source>
        <translation>收到的消息太大。</translation>
    </message>
    <message>
        <source>Invalid UTF-8 code encountered.</source>
        <translation>检测到无效的 UTF-8 代码。</translation>
    </message>
    <message>
        <source>Payload of close frame is too small.</source>
        <translation>关闭帧的有效载荷太小。</translation>
    </message>
    <message>
        <source>Invalid close code %1 detected.</source>
        <translation>无效的关闭代码 %1。</translation>
    </message>
    <message>
        <source>Invalid opcode detected: %1</source>
        <translation>检测到无效的操作码: %1</translation>
    </message>
</context>
<context>
    <name>QWebSocketFrame</name>
    <message>
        <source>Timeout when reading data from socket.</source>
        <translation>从套接字读取数据时超时。</translation>
    </message>
    <message>
        <source>Error occurred while reading from the network: %1</source>
        <translation>从网络读取时出错: %1</translation>
    </message>
    <message>
        <source>Lengths smaller than 126 must be expressed as one byte.</source>
        <translation>小于 126 的大小必须用 1 个字节表示。</translation>
    </message>
    <message>
        <source>Something went wrong during reading from the network.</source>
        <translation>从网络加载时出现问题。</translation>
    </message>
    <message>
        <source>Highest bit of payload length is not 0.</source>
        <translation>有效载荷长度的最高有效位非0。</translation>
    </message>
    <message>
        <source>Lengths smaller than 65536 (2^16) must be expressed as 2 bytes.</source>
        <translation>小于 65536 (2^16) 的大小必须用 2 个字节表示。</translation>
    </message>
    <message>
        <source>Error while reading from the network: %1.</source>
        <translation>从网络读取时出错: %1。</translation>
    </message>
    <message>
        <source>Maximum framesize exceeded.</source>
        <translation>超出最大帧大小。</translation>
    </message>
    <message>
        <source>Some serious error occurred while reading from the network.</source>
        <translation>从网络加载时发生严重错误。</translation>
    </message>
    <message>
        <source>Rsv field is non-zero</source>
        <translation>RSV 值不为 0</translation>
    </message>
    <message>
        <source>Used reserved opcode</source>
        <translation>使用保留的操作代码</translation>
    </message>
    <message>
        <source>Control frame is larger than 125 bytes</source>
        <translation>控制帧大于 125 字节</translation>
    </message>
    <message>
        <source>Control frames cannot be fragmented</source>
        <translation>控制帧不能拆分</translation>
    </message>
</context>
<context>
    <name>QWebSocketHandshakeResponse</name>
    <message>
        <source>Access forbidden.</source>
        <translation>访问被拒绝。</translation>
    </message>
    <message>
        <source>Unsupported version requested.</source>
        <translation>请求的版本不支持。</translation>
    </message>
    <message>
        <source>One of the headers contains a newline. Possible attack detected.</source>
        <translation>标头中有一个空行。检测到可能的攻击。</translation>
    </message>
    <message>
        <source>Bad handshake request received.</source>
        <translation>收到无效的握手请求。</translation>
    </message>
</context>
<context>
    <name>QWebSocketServer</name>
    <message>
        <source>Server closed.</source>
        <translation>服务器已关闭。</translation>
    </message>
    <message>
        <source>Too many pending connections.</source>
        <translation>等待中的连接太多了。</translation>
    </message>
    <message>
        <source>Upgrade to WebSocket failed.</source>
        <translation>升级到 WebSocket 失败。</translation>
    </message>
    <message>
        <source>Invalid response received.</source>
        <translation>收到无效回复。</translation>
    </message>
</context>
</TS>
