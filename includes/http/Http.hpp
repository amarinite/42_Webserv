#pragma once

#include <cerrno>
#include <string>
#include "HttpRequest.hpp"
#include "HttpException.hpp"

enum State {
	READING_HEADERS,
	READING_BODY,
	PROCESSING,
	WRITING_RESPONSE,
	FINISHED
};

class Http {
	private:
		std::string		_rawBuff;
		size_t			_rawBuffSize;
		
		State			_status;
		Request			_request;
		Processor		_processor;
		Response		_response;
		
		ServerConfig	_sConfig;

		//Functs
		void addLeftover(std::string &rawBuff, size_t &rawBuffSize);
		void handleBuffer(char *buff, size_t bytesRead);
		bool methodGetCase();

	public:
		Http(ServerConfig	&sc);
		// Http(const HandleSocket &socket);
		// ~Http();

		//Functs
		void 			HttpRoutine(char *buff, size_t bytesRead);
		
		//Getters
		State 			getStatus() const;
		const Request	&getRequest() const;
		Request			&getRequest();
		Response		getResponse();
};

