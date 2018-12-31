/*
 *  IXWebSocket.h
 *  Author: Benjamin Sergeant
 *  Copyright (c) 2017-2018 Machine Zone, Inc. All rights reserved.
 *
 *  WebSocket RFC
 *  https://tools.ietf.org/html/rfc6455
 */

#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <atomic>

#include "IXWebSocketTransport.h"
#include "IXWebSocketErrorInfo.h"
#include "IXWebSocketSendInfo.h"
#include "IXWebSocketPerMessageDeflateOptions.h"
#include "IXWebSocketHttpHeaders.h"

namespace ix
{
    // https://developer.mozilla.org/en-US/docs/Web/API/WebSocket#Ready_state_constants
    enum ReadyState 
    {
        WebSocket_ReadyState_Connecting = 0,
        WebSocket_ReadyState_Open = 1,
        WebSocket_ReadyState_Closing = 2,
        WebSocket_ReadyState_Closed = 3
    };

    enum WebSocketMessageType
    {
        WebSocket_MessageType_Message = 0,
        WebSocket_MessageType_Open = 1,
        WebSocket_MessageType_Close = 2,
        WebSocket_MessageType_Error = 3,
        WebSocket_MessageType_Ping = 4,
        WebSocket_MessageType_Pong = 5
    };

    struct WebSocketCloseInfo
    {
        uint16_t code;
        std::string reason;

        WebSocketCloseInfo(uint64_t c = 0,
                           const std::string& r = std::string())
            : code(c)
            , reason(r)
        {
            ;
        }
    };

    using OnMessageCallback = std::function<void(WebSocketMessageType,
                                                 const std::string&,
                                                 size_t wireSize,
                                                 const WebSocketErrorInfo&,
                                                 const WebSocketCloseInfo&,
                                                 const WebSocketHttpHeaders&)>;
    using OnTrafficTrackerCallback = std::function<void(size_t size, bool incoming)>;

    class WebSocket 
    {
    public:
        WebSocket();
        ~WebSocket();

        void setUrl(const std::string& url);
        void setPerMessageDeflateOptions(const WebSocketPerMessageDeflateOptions& perMessageDeflateOptions);

        void start();
        void stop();
        WebSocketSendInfo send(const std::string& text);
        WebSocketSendInfo ping(const std::string& text);
        void close();

        void setOnMessageCallback(const OnMessageCallback& callback);
        static void setTrafficTrackerCallback(const OnTrafficTrackerCallback& callback);
        static void resetTrafficTrackerCallback();

        ReadyState getReadyState() const;
        const std::string& getUrl() const;
        const WebSocketPerMessageDeflateOptions& getPerMessageDeflateOptions() const;

    private:
        void run();

        WebSocketSendInfo sendMessage(const std::string& text, bool ping);

        WebSocketInitResult connect();
        bool isConnected() const;
        bool isClosing() const;
        void reconnectPerpetuallyIfDisconnected();
        std::string readyStateToString(ReadyState readyState);
        static void invokeTrafficTrackerCallback(size_t size, bool incoming);

        // Server
        void setSocketFileDescriptor(int fd);

        WebSocketTransport _ws;

        std::string _url;
        WebSocketPerMessageDeflateOptions _perMessageDeflateOptions;
        mutable std::mutex _configMutex; // protect all config variables access

        OnMessageCallback _onMessageCallback;
        static OnTrafficTrackerCallback _onTrafficTrackerCallback;

        std::atomic<bool> _stop;
        std::atomic<bool> _automaticReconnection;
        std::thread _thread;
        std::mutex _writeMutex;

        friend class WebSocketServer;
    };
}
