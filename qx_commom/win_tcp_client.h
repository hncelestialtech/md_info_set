#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstring>
#include <ctime>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

// Global variables
SOCKET g_socket = INVALID_SOCKET;
bool g_connected = false;
bool g_running = true;
vector<string> g_history;
int g_historyPos = -1;
mutex g_outputMutex;
string g_outputBuffer;

// Function declarations
bool ConnectToServer(const string& ip, int port);
void Disconnect();
bool SendCommand(const string& cmd);
void StartReceiveThread();
void StopReceiveThread();
void ReceiveData();
string GetCurrentTimeStr();

// Main function
int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "Winsock initialization failed" << endl;
        return 1;
    }

    // Set console title
    SetConsoleTitle("Linux Shell Client");

    // Clear screen
    system("cls");

    cout << "=== Linux Shell Client ===" << endl;
    cout << "Enter IP:Port to connect (e.g., 192.168.1.100:8888)" << endl;
    cout << "Enter 'quit' to exit" << endl;
    cout << "==========================" << endl << endl;

    string input;
    bool connected = false;

    while (g_running) {
        if (!connected) {
            // Connection phase
            cout << "> ";
            getline(cin, input);

            if (input == "quit" || input == "exit") {
                g_running = false;
                break;
            }

            // Parse IP and port
            size_t colonPos = input.find(':');
            if (colonPos == string::npos) {
                cout << "Format error! Use IP:Port format" << endl;
                continue;
            }

            string ip = input.substr(0, colonPos);
            string portStr = input.substr(colonPos + 1);
            int port;

            try {
                port = stoi(portStr);
            } catch (...) {
                cout << "Invalid port number!" << endl;
                continue;
            }

            if (port <= 0 || port > 65535) {
                cout << "Invalid port! Must be 1-65535" << endl;
                continue;
            }

            cout << "Connecting to " << ip << ":" << port << "..." << endl;

            if (ConnectToServer(ip, port)) {
                connected = true;
                cout << "Connection successful!" << endl;
                cout << "Enter commands, press Enter to send" << endl;
                cout << "Enter 'disconnect' to disconnect" << endl << endl;

                // Start receive thread
                StartReceiveThread();
            } else {
                cout << "Connection failed" << endl;
            }
        } else {
            // Command interaction phase
            cout << GetCurrentTimeStr() << " $ ";
            getline(cin, input);

            if (input == "disconnect" || input == "quit" || input == "exit") {
                if (input == "disconnect") {
                    Disconnect();
                    connected = false;
                    cout << "Disconnected" << endl << endl;
                } else {
                    g_running = false;
                }
                continue;
            }

            if (input.empty()) continue;

            // Save to history
            g_history.push_back(input);
            g_historyPos = (int)g_history.size();

            // Send command
            if (!SendCommand(input)) {
                cout << "Send failed, connection may be lost" << endl;
                connected = false;
                Disconnect();
            }
        }
    }

    // Cleanup
    StopReceiveThread();
    Disconnect();
    WSACleanup();

    cout << "Program exited" << endl;
    return 0;
}

// Connect to server
bool ConnectToServer(const string& ip, int port) {
    if (g_connected) return true;

    // Create socket
    g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (g_socket == INVALID_SOCKET) {
        cout << "Failed to create socket" << endl;
        return false;
    }

    // Set server address
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr) != 1) {
        // Try DNS resolution
        struct addrinfo hints, *addrs;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        char portStr[16];
        sprintf(portStr, "%d", port);

        if (getaddrinfo(ip.c_str(), portStr, &hints, &addrs) != 0) {
            closesocket(g_socket);
            g_socket = INVALID_SOCKET;
            cout << "Failed to resolve hostname" << endl;
            return false;
        }

        serverAddr = *(sockaddr_in*)addrs->ai_addr;
        freeaddrinfo(addrs);
    }

    // Set timeout (optional)
    DWORD timeout = 5000; // 5 seconds
    setsockopt(g_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
    setsockopt(g_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

    // Connect
    if (connect(g_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        int error = WSAGetLastError();
        closesocket(g_socket);
        g_socket = INVALID_SOCKET;

        char errMsg[256];
        sprintf(errMsg, "Connection failed (Error code: %d)", error);
        cout << errMsg << endl;
        return false;
    }

    g_connected = true;
    return true;
}

// Disconnect from server
void Disconnect() {
    g_connected = false;
    if (g_socket != INVALID_SOCKET) {
        closesocket(g_socket);
        g_socket = INVALID_SOCKET;
    }
}

// Send command to server
bool SendCommand(const string& cmd) {
    if (!g_connected || g_socket == INVALID_SOCKET) {
        return false;
    }

    // Send command (add newline)
    string sendCmd = cmd + "\n";
    int bytesSent = send(g_socket, sendCmd.c_str(), (int)sendCmd.length(), 0);
    if (bytesSent == SOCKET_ERROR) {
        return false;
    }

    return true;
}

// Start receive thread
thread g_recvThread;

void StartReceiveThread() {
    g_running = true;
    g_recvThread = thread(ReceiveData);
    g_recvThread.detach();
}

// Stop receive thread
void StopReceiveThread() {
    g_running = false;
    // Wait for thread to finish
    if (g_recvThread.joinable()) {
        g_recvThread.join();
    }
}

// Receive data thread
void ReceiveData() {
    char buffer[4096];

    while (g_running && g_connected && g_socket != INVALID_SOCKET) {
        // Use select to check for data
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(g_socket, &readSet);

        timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // 100ms

        int result = select((int)g_socket + 1, &readSet, NULL, NULL, &timeout);
        if (result == SOCKET_ERROR) {
            if (g_running) {
                cout << endl << "Network error, connection may be lost" << endl;
            }
            break;
        }

        if (result > 0 && FD_ISSET(g_socket, &readSet)) {
            int bytesRead = recv(g_socket, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';

                // Lock output
                lock_guard<mutex> lock(g_outputMutex);
                cout << buffer;
                cout.flush();
            } else if (bytesRead == 0) {
                // Connection closed
                if (g_running) {
                    cout << endl << "Connection closed" << endl;
                }
                g_connected = false;
                break;
            } else {
                // Receive error
                if (g_running) {
                    cout << endl << "Receive error" << endl;
                }
                g_connected = false;
                break;
            }
        }
    }
}

// Get current time string
string GetCurrentTimeStr() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);

    char buffer[20];
    strftime(buffer, sizeof(buffer), "[%H:%M:%S]", timeinfo);
    return string(buffer);
}
