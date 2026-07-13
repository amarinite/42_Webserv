#pragma once

#include <cerrno>

enum State {
	READING_HEADERS;
	READING_BODY;
	PROCESSING;
	WRITING_RESPONSE;
	FINISHED;
};

class Http {
	private:
		// Config goes here.
		std::string	_rawBuff;
		size_t		_rawBuffSize;
		
		int			_status;
		Request		&_req;
		Response	&_res;
		

		//Functs
		void HttpRoutine();
		void addLeftover();

	public:
		Http();
		Http(const HandleSocket &socket);
		Http(const Http &other);
		Http &operator=(const Http &other);
		~Http();

		//Getters
		HandleSocket	getSocket;
};

class HttpException : public std::exception {
private:
    int _statusCode;
    std::string _message;

public:
    HttpException(int code, const std::string& msg) : _statusCode(code), _message(msg) {}
    virtual ~HttpException() throw() {}

    virtual const char* what() const throw() {
        return _message.c_str();
    }

    int getStatusCode() const {
        return _statusCode;
    }
};

