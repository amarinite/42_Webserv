#pragma once

#include <cerrno>

enum class State {
	READING_HEADERS;
	READING_BODY;
	PROCESSING;
	WRITING_RESPONSE;
	FINISHED;
};

class Http {
	private:
		HandleSocket _socket;
		State		_state;
		Request		&_req;
		Response	&_res;

		//Functs
		void HttpRoutine();

	public:
		Http();
		Http(const HandleSocket &socket);
		Http(const Http &other);
		Http &operator=(const Http &other);
		~Http();

		//Getters
		HandleSocket	getSocket;
}