#include "Http.hpp"

Http::Http() {
	std::cout << "Http class: I shouldn't exist, yet here I am." << std::endl;
}

Http::Http(const HandleSocket &socket) : _status(READING_HEADERS), _socket(socket) {}

Http::Http(const Http &other) {
	*this = other;
}

Http &Http::operator=(const Http &other) {
	if (this != &other) {
		_socket = other._socket;
		_state = other._state;
		_request = other._request;
		_response = other_response;
	}
	return *this;
}

Http::~Http() {}

//Functs
void Http::HttpRoutine() {
	char buff[1024];

	int bytes_read = recv(this.socket.getFD(), &buff, 1024, 0);
	if (bytes_read < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return;
	}
	if (bytes_read == 0)
		Http._status = FINISHED;

	switch(Http._status) {
		case READING_HEADERS: {
			if (Http._request.parseRequestHead(&buff))
				Http._status = READING_BODY;
			if (http.request._headRead)
				if (Http._request.parseRequestBody(&buff))
					Http._status = PROCESSING;
			break;
		}
		case READING_BODY: {
			if (Http._request.parseRequestBody(&buff))
				Http._status = PROCESSING;
			break;
		}
		case PROCESSING: {
			//Alba
			break;
		}
		case WRITING_RESPONSE: {
			break;
		}
		case FINISHED: {
			// Segurament no fa falta.
			break;
		}
	}
		
}


//Getters
HandleSocket	Http::getSocket {
    return this->_socket;
}