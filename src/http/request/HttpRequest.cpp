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

static void safeGetLine(std::string &stream, std::string &var, char delimiter, bool &done) {
	size_t pos = stream.find(delimiter);
	if (pos != std::string::npos) {
		var = stream.substr(0, pos);
		stream.erase(0, pos + 1);
		done = true;
		return;
	}
	var += stream;
	stream.clear();
	done = false;
}

static bool Request::safeEnd() {
	if (_stream.size() == 0) {
		_incompleteEndLine = true;
		return false;
	}
	if (_stream[0] != '\n')
		// 400 Bad Request.
	_incompleteEndLine = false;
	_stream.erase(0, 1);
	return true;
}

bool Request::parseMethod() {
	if (!this->_methodParsed) {
		safeGetLine(_stream, _method, ' ', this->_methodParsed);
		if (!this->_methodParsed)
			return false;
		if (_method != "GET" && methodStr != "POST" && methodStr != "DELETE") // To be redone by server restrictions.
			// 400 Bad Request.
	}

	if (!this->_uriParsed) {
		safeGetLine(_stream, _uriStr, ' ', this->_uriParsed);
		if (!this._uriParsed)
			return false;
		parseUri(this->_uri, _uriStr);
		this->_uriStr.clear();
	}

	if (!this->_httpVerParsed) {
		safeGetLine(_stream, _httpVer, '\r', this->_httpVerParsed);
		if (!this->_httpVerParsed)
			return false;
		if (_httpVer != "HTTP/1.1")
			// 400 Bad Request.
		if (!safeEnd())
			return false;
	}
	if (_incompleteEndLine == true) {
		if (!safeEnd())
			return false;
	}
	return true;
}

bool Request::findKey() {
	if (!this->_parsedKey) {
		safeGetLine(_stream, _tmpKey, ':', this->_parsedKey);
		if (!this->_parsedKey)
			return false;
		else if (_tmpKey.find(' ') != std::string::npos)
				// 400 Bad Request;
		tolowerStr(_tmpKey);
	}
	return true;
}

static void handleValueSpaces(std::string &value) {
	size_t first = value.find_first_not_of(' ');
	if (first == std::string::npos) { // todo espacios o vacío
		value.clear();
		return;
	}
	size_t last = value.find_last_not_of(' ');
	value.erase(last + 1);
	value.erase(0, first);
}

bool Request::findValue(std::string &_tmpValue) {
	if (!this->_parsedValue) {
		safeGetLine(_stream, _tmpValue, '\r', this->_parsedValue);
		if (!this->_parsedValue)
			return false;
		if (!safeEnd())
			return false;
	}
	handleValueSpaces(_tmpValue);
	return true;
}

void Request::addHeader() {
	if (this->_parsedKey && this->_parsedValue) {
		if (this->_headers.count(_tmpKey) {
			if (_tmpKey == "host" || _tmpKey == "content-length") {
				_tmpKey.clear();
				_tmpVal.clear();
				// 400 Bad Request;
			}
			this->_headers[_tmpKey] += ", " + _tmpValue;
		} else
			this->_headers[_tmpKey] = _tmpValue;
	}
	_tmpKey.clear();
	_tmpVal.clear();
}

bool Request::parseHeaders() {
	if (!this->findKey())
		return false;

	if (!this->findValue())
		return false;

	this->addHeader();

	size_t pos = _stream.find("\r\n"); 
	if (pos == std::string::npos || pos != 0) {
		this->_parsedKey = false;
		this->_parsedValue = false;
		_leftover = _stream;
		_stream.clear();
		return false;
	}
	_stream.erase(0, 2);
	return true;


	// El header Host hi ha de ser sempre. -> Func is there a header.
	// El content length ens fa passar a la seguent fase. -> ParseBody
	// Transfer-Encoding: chunked: Indica que el cuerpo no viene de golpe, sino dividido en bloques de tamaño dinámico. Si esta cabecera está presente, se ignora por completo Content-Length.
	// Connection: Puede ser keep-alive (mantener el socket abierto para reutilizarlo en futuras peticiones) o close (cerrar el socket en cuanto envíes la respuesta).
}

static void addLeftover() {
	// Restriccio missatge massa llarg.
	if (_leftover) {
		_stream = _leftover + _stream;
		_leftover.clear();
	}
}

bool Request::parseRequestHead() {
	addLeftover();
	if (!parseMethod())
		return false;
	while (true) {
		if (_stream.size() >= 2 && _stream[0] == '\r' && _stream[1] == '\n') {
			_stream.erase(0, 2);
			return true;
		}
		if (!parseHeaders())
			return false;
	}
	return true;
}

// Body Functs

static bool Request::parseFullBody() {
	
}

void Request::setBodyType() {
	if (_bodyType == EMPTY) {
		std::map<std::string, std::string>::iterator full = _headers.find("content-length");
		std::map<std::string, std::string>::iterator chunked = _headers.find("transfer-encoding");
		if (full != _headers.end()) {
			if (chunked != _headers.end())
				//  Error
			_bodyType = FULL;
		} else if (chunked != _headers.end()) {
			if (_headers["transfer-encoding"] ==  "chunked")
				_bodyType = CHUNKED;
			else
				// Error -> transfer encoding present pero diferent de chunked.
		}
	}
}

void Request::setBodySize(str::string &key) {
	if (_maxBodySize != 0)
		return;

	for (size_t i = 0; i < _headers[key].size(); ++i) {
		if (!std::isdigit(static_cast<unsigned char>(s[i])))
			// Error
	}
	
	std::stringstream ss(s);
	unsigned long long tmp = 0;
	ss >> tmp;
	if (ss.fail() || !ss.eof())
		// 413/400 según política

	if (tmp > static_cast<unsigned long long>(std::numeric_limits<size_t>::max()))
		// 413/400 según política

	_maxBodySize = static_cast<size_t>(tmp);
}

bool Request::fullBody() {
	setBodySize("content-length");
	if (_streamBody.size() < _maxBodySize) {
		_body.append(_streamBody);
		_maxBodySize -= _streamBody.size();
		_streamBody.clear();
		return false;
	} else if (_streamBody.size() == _maxBodySize) {
		_body.append(_streamBody);
		_maxBodySize = 0;
		_streamBody.clear();
		return true;
	} else {
		_body.append(_streamBody, 0, _maxBodySize);
		_streamBody.erase(0, _maxBodySize);
		_newRequest = _streamBody; // Isaaaaaaaaaaaaaaac
		_streamBody.clear();
		_maxBodySize = 0;
		return true;
	}
}

bool Request::chunkedBody() {
	while (true) {
		if (!_leftoverBody.empty()) {
			_leftoverBody.insert(_leftoverBody.end(), _streamBody.begin(), _streamBody.end());
			_streamBody = _leftoverBody;
			_leftoverBody.clear();
		}
		std::string delimiter = "\r\n"
		std::vector<char>::iterator it = std::search(_streamBody.begin(), _streamBody.end(), delimiter.begin(), delimiter.end());
		if (it == _streamBody.end()) {
			_leftoverBody = _streamBody;
			_streamBody.clear();
			return false;
		}
		else {
			if (_streamBody[it.size() + 2]) {
				_leftoverBody.insert(_leftoverBody.end(), _streamBody[it.size() + 2], _streamBody.end());
				_leftoverBody.erase(_streamBody[it.size() + 2], _streamBody.end());
			}
			if (!_chunkSize) {
				else if (it != _streamBody.end() && it.size() == 3 && it[0] == 0)
					break;
				for (std::vector<char>::iterator char_it = _streamBody.begin(); char_it != it; ++char_it) {
					if (!std::isdigit(static_cast<unsigned char>(*char_it))) { 
						// // Error, non digit chars in the nuber section of the chunk.
					}
					_maxBodySize = _maxBodySize * 10 + static_cast<size_t>(*char_it - '0');
				}
				_streamBody.clear();
				_chunkSize = true;
			}
			else if (_chunkSize) {
				if (_streamBody[it.size()] == '\r' && _streamBody[it.size() + 1] == '\n')
					_streamBody.erase(_streamBody[it.size()], _streamBody[it.size() + 1]);
				else
					// Error
				_body.insert(_streamBody.begin(), _streamBody.end());
			}
		}
	}
	if (_streamBody[it.size()] == '\r' && _streamBody[it.size() + 1] == '\n')
		// Error -> no \r\n after 0\r\n.
	return true;
}

bool Request::parseRequestBody() {
	setBodyType();
	if (_bodyType == FULL) {
		if(!fullBody())
			return false;
	}
	else if (_bodyType == CHUNKED) {
		if (!chunkedBody())
			return false;
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

////////////////
// Corregeix Request -> Revisa si cal refactorizar Body, en principi no