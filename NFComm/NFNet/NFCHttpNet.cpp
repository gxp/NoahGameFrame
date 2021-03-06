#include "NFComm/NFPluginModule/NFPlatform.h"

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#include "NFCHttpNet.h"
#include <string.h>
#include "event2/bufferevent_struct.h"
#include "event2/event.h"
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <atomic>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#pragma comment( lib, "libevent.lib")


bool NFCHttpNet::Execute()
{
	if (base)
	{
		event_base_loop(base, EVLOOP_ONCE | EVLOOP_NONBLOCK);
	}

	return true;
}


int NFCHttpNet::InitServer(const unsigned short port)
{
	mPort = port;
	//struct event_base *base;
	struct evhttp *http;
	struct evhttp_bound_socket *handle;

#if NF_PLATFORM == NF_PLATFORM_WIN
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return (1);
#endif

	base = event_base_new();
	if (!base)
	{
		std::cout << "create event_base fail" << std::endl;;
		return 1;
	}

	http = evhttp_new(base);
	if (!http) {
		std::cout << "create evhttp fail" << std::endl;;
		return 1;
	}

	evhttp_set_gencb(http, listener_cb, (void*)this);
	handle = evhttp_bind_socket_with_handle(http, "0.0.0.0", mPort);
	if (!handle) {
		std::cout << "bind port :" << mPort << " fail" << std::endl;;
		return 1;
	}
	return 0;
}


void NFCHttpNet::listener_cb(struct evhttp_request *req, void *arg)
{
	NFCHttpNet* pNet = (NFCHttpNet*)arg;

	//uri
	const char *uri = evhttp_request_get_uri(req);
	//std::cout << "Got a GET request:" << uri << std::endl;

	//get decodeUri
	struct evhttp_uri *decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}
	const char *decode1 = evhttp_uri_get_path(decoded);
	if (!decode1) decode1 = "/";
	const char* decodeUri = evhttp_uridecode(decode1, 0, NULL);
	if (decodeUri == NULL)
	{
		evhttp_send_error(req, 404, "error");
	}
	std::string strUri;
	if (decodeUri[0] == '/')
	{
		strUri = decodeUri;
		strUri.erase(0, 1);
		decodeUri = strUri.c_str();
	}
	//get argMap
	int msgId = 0;
	std::map<std::string, std::string> argMap;
	std::vector<std::string> argList = Split(decodeUri, ",");
	std::vector<std::string>::iterator it = argList.begin();
	for (; it != argList.end(); it++)
	{
		std::vector<std::string> param = Split(*it, "=");
		if (param.size() == 2)
		{
			if (param.at(0) == "id")
			{
				msgId = lexical_cast<int>(param.at(1));
			}
			argMap.insert(make_pair(param.at(0), param.at(1)));
		}
	}
	// call cb
	if (pNet->mRecvCB)
	{
		pNet->mRecvCB(req, msgId, argMap);
	}
	else
	{
		SendMsg(req, "mRecvCB empty");
	}



	//close
	/*{
	if (decoded)
	evhttp_uri_free(decoded);
	if (decodeUri)
	free(decodeUri);
	if (eventBuffer)
	evbuffer_free(eventBuffer);
	}*/
}
bool NFCHttpNet::SendMsg(struct evhttp_request *req, const std::string& strMsg)
{
	//create buffer
	struct evbuffer *eventBuffer = evbuffer_new();
	//send data
	evbuffer_add_printf(eventBuffer, strMsg.c_str());
	evhttp_send_reply(req, 200, "OK", eventBuffer);

	//free
	evbuffer_free(eventBuffer);
	return true;
}


bool NFCHttpNet::Final()
{
	if (base)
	{
		event_base_free(base);
		base = NULL;
	}
	return true;
}

std::vector<std::string> NFCHttpNet::Split(const std::string& str, std::string delim)
{
	std::vector<std::string> result;
	if (str.empty() || delim.empty())
	{
		return result;
	}

	std::string tmp;
	size_t pos_begin = str.find_first_not_of(delim);
	size_t pos = 0;
	while (pos_begin != std::string::npos)
	{
		pos = str.find(delim, pos_begin);
		if (pos != std::string::npos)
		{
			tmp = str.substr(pos_begin, pos - pos_begin);
			pos_begin = pos + delim.length();
		}
		else
		{
			tmp = str.substr(pos_begin);
			pos_begin = pos;
		}

		if (!tmp.empty())
		{
			result.push_back(tmp);
			tmp.clear();
		}
	}
	return result;
}
