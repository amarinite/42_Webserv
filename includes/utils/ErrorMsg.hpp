#include <string>
#include <map>

class ErrorMsg {
	private:
		std::map<int, std::string> _msg;

	public:
	ErrorMsg() {
		_msg[200] = "OK";
        _msg[201] = "Created";
        _msg[204] = "No Content";
        _msg[301] = "Moved Permanently";
		_msg[302] = "Found";
        _msg[400] = "Bad Request";
        _msg[403] = "Forbidden";
        _msg[404] = "Not Found";
        _msg[405] = "Method Not Allowed";
        _msg[413] = "Payload Too Large";
        _msg[500] = "Internal Server Error";
        _msg[501] = "Not Implemented";
        _msg[505] = "HTTP Version Not Supported";
	}
	
	std::string getErrorMsg(const int &code) const {
		std::map<int, std::string>::const_iterator it = _msg.find(code);
		if (it != _msg.end())
			return it->second;
		return "Error";
	}
};