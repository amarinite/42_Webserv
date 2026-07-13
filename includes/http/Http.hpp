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
		// Config goes here.
		std::string	_rawBuff;
		size_t		_rawBuffSize;
		
		State		_status;
		Request		_request;
		// Response	_response;
		
		//Functs
		void addLeftover(std::string &rawBuff, size_t &rawBuffSize);
		void handleBuffer(char *buff, size_t bytesRead);
		bool methodGetCase();

	public:
		Http();
		// Http(const HandleSocket &socket);
		Http(const Http &other);
		Http &operator=(const Http &other);
		~Http();

		//Functs
		void HttpRoutine(char *buff, size_t bytesRead);
		
		//Getters
		State getStatus() const;
		const Request &getRequest() const;
		Request &getRequest();
};

