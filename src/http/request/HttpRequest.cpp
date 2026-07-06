#include "HttpRequest.hpp"

Request::Request() : _headRead(false), _bodyRead(false) {}

Request::Request(const Request &other) {
	this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_rawRequest = other._rawRequest;
		this->_methodHeader = other._methodHeader;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return *this;
}

Request::~Request() {}

// Functs
static void tolowerStr(std::string &str) {
	for (size_t i = 0; i < str.size(); ++i) {
		unsigned char c = static_cast<char>(str[i]);
		str[i] = static_cast<char>(std::tolower(c));
	}
}

static void safeGetLine(std::string &ss, std::string &leftover, char delimiter, bool &done) {
	std::string line;
	size_t pos = ss.find(delimiter);
	if (pos != std::string::npos) {
		line = ss.substr(0, pos);
		ss.erase(0, pos + 1);
		leftover += line;
		done = true;
		return;
	}
	leftover += line;
	ss.clear();
	done = false;
}

bool Request::parseMethod() {
	if (!this._methodParsed) {
		safeGetLine(_stream, _method, ' ', this._methodParsed);
		if (!this._methodParsed)
			return false;
		if (methodStr != "GET" && methodStr != "POST" && methodStr != "DELETE") // To be redone by server restrictions.
			// 400 Bad Request.
	}

	if (!this._uriParsed) {
		safeGetLine(_stream, _uriStr, ' ', this._uriParsed);
		if (!this._uriParsed)
			return false;
		this._uri = uriParser(_uriStr);
		this._uriStr.clear();
	}

	if (!this._httpVerParsed) {
		safeGetLine(_stream, _httpVer, '\r', this._httpVerParsed);
		if (!this._httpVerParsed)
			return false;
		if (httpVerStr != "HTTP/1.1")
			// 400 Bad Request.
		this._httpVerParsed = true;
		size_t pos = _stream.find("\n")
		if (pos == std::string::npos || pos != 0)
			// 400 Bad Request.
	}
	return true;
}

static bool Request::findKey(std::string &key) {
	if (!this._parsedKey) {
		safeGetline(_steam, key, ':', this._parsedHeader);
		if (!this._parsedKey)
			return false;
		else if (key[key.find(':') - 1] == ' ')
				// 400 Bad Request;
		tolowerStr(key);
		this._parsedKey = true; 
	}
	return true;
}

static void handleValueSpaces(std::string &value) {
	value.erase(0, value.find_first_not_of(' '));
	value.erase(value.find_last_not_of(' ') + 1);
}

static bool Request::findValue(std::string &value) {
	if (!this._parsedValue) {
		safeGetline(_stream, key, '\r', this._parsedValue);
		if (!this._parsedValue)
			return false;
		tolowerStr(value);
		size_t pos = _stream.find("\n")
		if (pos == std::string::npos || pos != 0)
			// 400 Bad Request.
		this._parsedValue = true;
	}
	handleValueSpaces(value);
	return true;
}

static void Request::addHeader() {
	if (this._parsedKey && this._parsedValue) {
		if (this._header[key]) {
			if (key == "host" && key == "content-length")
				// 400 Bad Request;
			this._headers[key] += ", " + value;
		} else
			this._headers[key] = value;
	}
}

bool Request::parseHeaders() {
	std::string key;
	std::string value;
	
	if (!this.findKey(key))
		return false;

	if (!this.findValue(value))
		return false;

	this.addHeader();

	size_t pos = _stream.find("\r\n") 
	if (pos == std::string::npos || pos != 0)
		this._parsedKey = false;
		this._parsedValue = false;
		return false;
	} else
		// 400 Bad Request;
	}
	return true;


	// El header Host hi ha de ser sempre. -> Func is there a header.
	// El content length ens fa passar a la seguent fase. -> ParseBody
	// Transfer-Encoding: chunked: Indica que el cuerpo no viene de golpe, sino dividido en bloques de tamaño dinámico. Si esta cabecera está presente, se ignora por completo Content-Length.
	// Connection: Puede ser keep-alive (mantener el socket abierto para reutilizarlo en futuras peticiones) o close (cerrar el socket en cuanto envíes la respuesta).
}

static void parseBody();

bool Request::parseRequestHead() {
	if (!parseMethod())
		return false;
	if (!parseHeaders())
		return false;
	_headRead = true;
	// To do
	if (ss.peek() != EOF)
		_leftover = ss.str();
	return true;
}

static bool Request::parseFullBody() {
	
}

bool Request::parseRequestBody(char *stream) {
	std::map<std::string, std::string>::iterator full = request._headers.find("content-length");
	std::map<std::string, std::string>::iterator chunked = request._headers.find("transfer-encoding");
	

	if (full != request._headers.end()) {
		if (chunked != request._headers.end())
			// Error
		if (!this._streamLeft)
			this._streamLeft = std::atoi(full->second);
		this._body.push_back(stream);
		size_t bodyLen = strlen(this._body[0]);
		if (bodyLen != this._streamLeft) {
			return false;
		}
	}

	else if (chunked != request._headers.end()) {
		if (!this._streamLeft) {
			size_t i = 0;
			for (; std::isdigit(stream[i]; ++i)) {
				this._streamLeft = this._streamLeft * 10 + std::atoi(stream[i]);
			}
			if (!this.streamLeft || stream[i] != '\r')
				// Error n the part which indicates the length of he incomin chunk
			if (stream[i] == '\r' && stream[i + 1] != '\n')
				// Error, \r alone.
		}

		
	}
	return true;
}





//Getters
t_method	Request::getMethod {
	return this->_method;
}

std::map<std::string, std::string>  Request::getHeaders {
	return this->_headers;
}

std::vector<char>	Request::getBody {
	return this->body;
}

pal tester:
- Headers en mayuscula
- Espacio no eliminados en headers value despues de los : y al final del value
- Espacio dentro del contenido del value de los headers.

