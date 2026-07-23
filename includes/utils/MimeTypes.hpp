#include <string>
#include <map>

class MimeTypes {
    private:
        std:::map<std::string, std::string> _map;

    public:
    MimeTypes() {
        // Text
        _map[".html"] = "text/html";
        _map[".htm"]  = "text/html";
        _map[".css"]  = "text/css";
        _map[".js"]   = "text/javascript";
        _map[".json"] = "application/json";
        _map[".txt"]  = "text/plain";
        _map[".xml"]  = "application/xml";

        // Images
        _map[".png"]  = "image/png";
        _map[".jpg"]  = "image/jpeg";
        _map[".jpeg"] = "image/jpeg";
        _map[".gif"]  = "image/gif";
        _map[".ico"]  = "image/x-icon";
        _map[".svg"]  = "image/svg+xml";

        // Other
        _map[".pdf"]  = "application/pdf";
        _map[".zip"]  = "application/zip";
    }
    std::stirng getType(const std::string &extension) const {
        std::map<std::string, std::string>::iterator it = _map.find(extension);
        if (it != _map.end())
            return it->second;
        return "application/octet-stream";
    }
};
