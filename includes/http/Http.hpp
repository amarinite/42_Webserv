#pragma once

#include <string>
#include <map>

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "HttpResponse.hpp"
#include "cgi/CgiExecutor.hpp"
#include "Processor.hpp"
#include "cgi/CgiHandler.hpp"

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
		std::string			_rawBuff;
		size_t				_rawBuffSize;
		State				_status;
		Request				_request;
		Response 			_response;
		const ServerConfig	&_sConfig;
		Processor			*_processor;
		CgiExecutor			*_cgi;
		std::string			_clientIp;

		//Functs
		void addLeftover(std::string &rawBuff, size_t &rawBuffSize);
		void handleBuffer(char *buff, size_t bytesRead);
		bool methodGetCase();

		void startProcessing();
		void startCgi();
		void finishWithError(const HttpException& e);
		void buildResponse(const HttpException &e);

	public:
		Http(const ServerConfig &sc);
		~Http();

		//Functs
		void HttpRoutine(char *buff, size_t bytesRead);
		void setClientIp(const std::string &ip);
		bool checkCgiTimeout(double timeoutSeconds);

		void onCgiWritable();
		void onCgiReadable();
		bool isWaitingOnCgi() const;
		int  getCgiWriteFd() const;
		int  getCgiReadFd() const;

		//Getters
		State getStatus() const;
		const Request &getRequest() const;
		Request &getRequest();
		const Response &getResponse() const;
		Response &getResponse();
};

