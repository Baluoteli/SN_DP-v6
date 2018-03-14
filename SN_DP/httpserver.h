#pragma once
#include "stdafx.h"
#include <time.h>
#include <stdint.h>
#include <string>
#include <winsock.h>
#include <process.h>//_beginthreadex

#pragma comment(lib,"ws2_32.lib")
//#include <WinSock.h>
 extern HWND hRenderWnd;

bool parseact(const std::string& act, std::string& reply, std::string& mime);

bool parsereq(const std::string& req, std::string& reply, std::string& mime);

class http_req_t
{
	static void http_req_handler(http_req_t* req)
	{
		std::string command(req->req), reply, mime("text/plain");
		int32_t code = 200;
		if (!parsereq(command, reply, mime))
		{
			code = 404;
		}
		char s[256];
		sprintf_s(s, "HTTP/1.0 %d X\r\nContent-Length: %d\r\nContent-Type: %s\r\nCache-Control: no-cache\r\nPragma: no-cache\r\nConnection: close\r\n\r\n", code, reply.length(), mime.c_str());
		std::string response(s);
		response += reply;
		req->reply = response;
		req->w_ready = 1;
	}

	static unsigned __stdcall http_req_thread(void* info)
	{
		auto self = (http_req_t*)info;

		timeval tv = { 1, 0 };
		int32_t wantread = 1;
		DWORD start_time = GetTickCount();

		while (1)
		{
			if (GetTickCount() - start_time > 30 * 1000)
			{
				break; // timeout
			}

			FD_SET fds_read, fds_write;
			FD_ZERO(&fds_read);
			FD_ZERO(&fds_write);

			if (wantread) FD_SET(self->fd, &fds_read);
			if (self->w_ready) FD_SET(self->fd, &fds_write);
			if (select(0, &fds_read, &fds_write, NULL, &tv) == SOCKET_ERROR)
			{
				break;
			}
			if (wantread && FD_ISSET(self->fd, &fds_read))
			{
				int32_t ntoread = sizeof(self->req) - self->req_len - 1;
				if (ntoread < 1)
				{
					break;
				}
				int32_t n = recv(self->fd, self->req + self->req_len, ntoread, 0);
				if (n == 0)
				{
					// socket gracefully closed
					break;
				}
				else if (n < 0)
				{
					wantread = 0;
				}
				else
				{
					self->req_len += n;
					self->req[self->req_len] = 0;

					if (strstr(self->req, "\r\n\r\n"))
					{
						wantread = 0;
						http_req_handler(self);
					}
				}
			}
			if (FD_ISSET(self->fd, &fds_write))
			{
				int32_t ntowrite = self->reply.length() - self->w_len;
				if (ntowrite > 1024) ntowrite = 1024;

				int32_t n = send(self->fd, self->reply.c_str() + self->w_len, ntowrite, 0);
				if (n < 1)
				{
					break;
				}
				self->w_len += n;
				if (self->w_len == self->reply.length())
				{
					break;
				}
			}
		}
		//shutdown(self->fd, SD_BOTH);
		closesocket(self->fd);
		delete self;
		return 0;
	}

private:
	friend class SDHttpServer;
	http_req_t(SOCKET s) : fd(s), req_len(0), w_ready(0), w_len(0)
	{
	}

	void begin()
	{
		auto hThread = (HANDLE)_beginthreadex(NULL, 0,  http_req_thread, (void*) this, 0, NULL);
		CloseHandle(hThread);
	}

private:
	SOCKET	fd;
	char	req[4096];
	int32_t req_len;
	std::string reply;
	volatile int32_t w_ready;
	size_t	w_len;
};

class SDHttpServer
{
	static unsigned __stdcall httpd_server_thread(void* p)
	{
		auto self = (SDHttpServer*)p;

		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
		int32_t flags = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flags, sizeof(flags));

		struct sockaddr_in addr = { 0 };
		addr.sin_family = AF_INET;
		addr.sin_port = htons(self->port_);
		addr.sin_addr.s_addr = inet_addr(self->ip_);
		if (::bind(fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
		{
			closesocket(fd);
			::PostMessageW(hRenderWnd, WM_DESTROY, NULL, NULL);
			return 1;
		}

		if (::listen(fd, 511) < 0)
		{
			closesocket(fd);
			return 2;
		}

		timeval tv = { 1, 0 };

		while (!self->should_exit_)
		{
			FD_SET fds_read;
			FD_ZERO(&fds_read);
			FD_SET((SOCKET)fd, &fds_read);
			if (select(0, &fds_read, NULL, NULL, &tv) == SOCKET_ERROR) break;

			if (FD_ISSET(fd, &fds_read))
			{
				sockaddr_in remotesock_addr;
				int32_t socklen = sizeof(remotesock_addr);

				SOCKET client = accept(fd, (sockaddr*)&remotesock_addr, &socklen);
				if (client != INVALID_SOCKET)
				{
					auto req = new http_req_t(client);
					req->begin();
				}
			}
		}

		closesocket(fd);
		WSACleanup();
		return 0;
	}

private:
	volatile int32_t should_exit_;//不使用优化，保证数据正确
	char* ip_;
	USHORT port_;
	HANDLE thread_;

public:
	SDHttpServer(const char* ip, USHORT port) : should_exit_(0), port_(port)
	{
		ip_ = _strdup(ip);
	}

	~SDHttpServer()
	{
		free(ip_);
	}

	void start()
	{
		thread_ = (HANDLE)_beginthreadex(NULL, 0, httpd_server_thread, (void*) this, 0, NULL);
	}

	void stop()
	{
		should_exit_ = 1;
		WaitForSingleObject(thread_, INFINITE);
		CloseHandle(thread_);
	}
};
