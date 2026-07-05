#include "HttpRequest.hpp"

Request::Request()

Request(const HandleSocket &sourceSocket) : _socket(sourceSocket) {}


Request::Request(const Request &other) {
    this = other;
}

Request::Request &operator=(const Request &other) {
    if (this != &other) {
        this->_rawRequest = other._rawRequest;
        this->_methodHeader = other._methodHeader;
        this->_headers = other._headers;
        this->_body = other._body;
    }
    return *this;
}

Request::~Request() {}

//Functs
//void Request::recieveRawRequest();
// -- He de parlar am el Isaac a veure com em passa la info/com la he danar a buscar. 


void Request::parseRequest() {
    char buff[1024];
    while (1) {
        int  = recv(this.socket.getFD(), &buff, 1024, MSG_DONTWAIT);
    }
}

// Will call this functs staticlly.
// Since they cannot be called independently if've chosen this way.
static void parseMethod();
static void parseHeaders();
static void parseBody();


//Getters
t_method	Request::getSocket {
    return this->_socket;
}

t_method	Request::getMethod {
    return this->_method;
}

std::map<std::string, std::string>  Request::getHeaders {
    return this->_headers;
}

std::vector<char>	Request::getBody {
    return this->body;
}