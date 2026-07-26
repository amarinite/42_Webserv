#pragma once

#include <string>

#include "ServerConfig.hpp"
#include "HttpRequest.hpp"
#include "Processor.hpp"
#include "HttpResponse.hpp"



enum State {
	READING_HEADERS,
	READING_BODY,
	PROCESSING,
	WRITING_RESPONSE,
	FINISHED
};

class Http {
	private:
		std::string			_rawBuff;
		
		const ServerConfig	&_sConfig;
		State		 		_status;
		Request				_request;
		Response			_response;
		Processor			*_processor;
		
		// IP!!!!!!!!!!!!!!!!!!!

		//Functs
		void addLeftover(std::string &rawBuff, size_t &rawBuffSize);
		void handleBuffer(char *buff, size_t bytesRead);
		bool methodGetCase();

	public:
		Http(const ServerConfig	&sc);
		~Http();

		//Functs
		void 			HttpRoutine(char *buff, size_t bytesRead);
		
		//Getters
		State 			getStatus() const;
		const Request	&getRequest() const;
		Request			&getRequest();
		const Response  &getResponse() const;
    	Response		&getResponse();
};

