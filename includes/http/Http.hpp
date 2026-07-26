#pragma once

#include <cerrno>
#include <string>
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "HttpResponse.hpp"
#include "CgiExecutor.hpp"
#include "ServerConfig.hpp"
#include "Processor.hpp"

enum State {
	READING_HEADERS,
	READING_BODY,
	PROCESSING,
	CGI_WRITING,
	CGI_READING,
	WRITING_RESPONSE,
	FINISHED
};

class Http {
	private:
		std::string		_rawBuff;
		size_t			_rawBuffSize;
		State			_status;
		Request			_request;
		Response 		_response;
		ServerConfig&	_sConfig;
		Processor*		_processor;
		CgiExecutor*	_cgi;
		
		//Functs
		void addLeftover(std::string &rawBuff, size_t &rawBuffSize);
		void handleBuffer(char *buff, size_t bytesRead);
		bool methodGetCase();

		void startProcessing();
		void finishWithError(const HttpException& e);

	public:
		// Http();
		Http(ServerConfig &sc);
		// Http(const Http &other);
		// Http &operator=(const Http &other);
		~Http();

		//Functs
		void HttpRoutine(char *buff, size_t bytesRead);

		void onCgiWritable();
		void onCgiReadable();
		bool isWaitingOnCgi() const;
		int  getCgiWriteFd() const;
		int  getCgiReadFd() const;
		
		//Getters
		State getStatus() const;
		const Request &getRequest() const;
		Request &getRequest();
};

