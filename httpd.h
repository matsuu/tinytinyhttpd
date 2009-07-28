#ifndef _HTTPD_H_
#define _HTTPD_H_

#define HTTPD_VERSION 0x0100

#pragma warning(disable:4018 4503 4530 4786)
#define _CRT_SECURE_NO_WARNINGS
#include <sys/types.h>
#ifdef _WIN32
#include <winsock2.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/uio.h>
#include <limits.h>
#include <unistd.h>
#endif
#include <sstream>

#include "utils.h"

namespace tthttpd {

class server {
public:
	typedef struct {
		std::string name;
		unsigned long size;
		bool isdir;
		struct tm date;
	} ListInfo;
	typedef struct {
		int msgsock;
		server *httpd;
		tstring address;
	} HttpdInfo;
	typedef std::map<tstring, tstring> BasicAuths;
	typedef struct {
		std::vector<tstring> accept_list;
	} AcceptAuth;
	typedef std::map<tstring, AcceptAuth> AcceptAuths;
	typedef std::vector<tstring> AcceptIPs;

	typedef void (*LoggerFunc)(const HttpdInfo* httpd_info, const tstring& request);
	typedef std::map<tstring, tstring> MimeTypes;
	typedef std::vector<tstring> DefaultPages;
	typedef std::map<tstring, tstring> RequestAliases;
	typedef std::map<tstring, tstring> RequestEnvironments;

private:
#ifdef _WIN32
	HANDLE thread;
#else
	pthread_t thread;
#endif

public:
	int sock;
	std::string hostname;
	std::string hostaddr;
	std::string target;
	std::string root;
	std::string fs_charset;
	unsigned short port;
	BasicAuths basic_auths;
	AcceptAuths accept_auths;
	AcceptIPs accept_ips;
	MimeTypes mime_types;
	DefaultPages default_pages;
	RequestAliases request_aliases;
	RequestEnvironments request_environments;
	LoggerFunc loggerfunc;
	bool debug_mode;

	void initialize() {
		sock = -1;
		port = 80;
		target = "/RPC2";
		fs_charset = "utf-8";
		thread = 0;
		loggerfunc = NULL;
		mime_types[_T("gif")] = _T("image/gif");
		mime_types[_T("jpg")] = _T("image/jpeg");
		mime_types[_T("png")] = _T("image/png");
		mime_types[_T("htm")] = _T("text/html");
		mime_types[_T("html")] = _T("text/html");
		mime_types[_T("txt")] = _T("text/plain");
		mime_types[_T("xml")] = _T("text/xml");
		mime_types[_T("js")] = _T("application/x-javascript");
		mime_types[_T("css")] = _T("text/css");
		default_pages.push_back(_T("index.html"));
		default_pages.push_back(_T("index.php"));
		default_pages.push_back(_T("index.cgi"));
		debug_mode = false;
	};

	server() {
		initialize();
	}
	server(unsigned short _port) {
		initialize();
		port = _port;
	}
	server(unsigned short _port, tstring _target) {
		initialize();
		port = _port;
		target = tstring2string(_target);
	}
	~server() {
		stop();
	}
	bool start();
	bool stop();
	bool wait();
	void set_fs_charset(tstring _fs_charset) {
		fs_charset = tstring2string(_fs_charset);
	}
	tstring get_fs_charset() {
		return string2tstring(fs_charset);
	}
	void setAuthentication(BasicAuths _basic_auths) {
		basic_auths = _basic_auths;
	}
	void bindRoot(tstring _root) {
		root = get_realpath(tstring2string(_root));
	}
	static std::string get_realpath(std::string path) {
#ifdef _WIN32
		char fullpath[_MAX_PATH] = {0};
		char* filepart = NULL;
		if (GetFullPathNameA(path.c_str(), _MAX_PATH, fullpath, &filepart))
			path = fullpath;
#else
		char fullpath[PATH_MAX] = {0};
		if (realpath((char*)path.c_str(), fullpath))
			path = fullpath;
#endif
		std::replace(path.begin(), path.end(), '\\', '/');
		size_t end_pos = path.find_last_of("?#");
		if (end_pos != std::string::npos) path.resize(end_pos);

		std::vector<std::string> path_sep = split_string(path, "/");
		std::vector<std::string>::iterator it;
		while(true) {
			it = std::find(path_sep.begin(), path_sep.end(), "..");
			if (it == path_sep.end()) break;
			if (it == path_sep.begin()) {
				continue;
			}
			path_sep.erase(it-1);
			path_sep.erase(it-1);
		}
		std::string path_real;
		for(it = path_sep.begin(); it != path_sep.end(); it++) {
			path_real += *it;
			if (it+1 != path_sep.end())
				path_real += "/";
		}
		if (path[path.size()-1] == '/')
			path_real += "/";
		return path_real;
	}
};

}

#endif /* _HTTPD_H_ */