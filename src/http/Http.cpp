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
void Http::HttpRoutine(char *buff) {

	switch(this->_status) {
		case READING_HEADERS: {
			if (this->_request.parseRequestHead(&buff)) {
				this->_status = READING_BODY;
				if (buff && *buff != '\0') {
					if (this->_request.parseRequestBody(&buff))
						this->_status = PROCESSING;
				}
			}
			break;
		}
		case READING_BODY: {
			if (this->_request.parseRequestBody(&buff))
				this->_status = PROCESSING;
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

////////////////////////
// Revisa si inicialitzes correctament la request i la response. 
// - Maybe el constructor per defecte de request hauria de fer inicialitzacio basica. 