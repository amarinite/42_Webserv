
class ServerConfig
{
	public:
		static ServerConfig build(Node* serverNode);
		// fields: port, root, index, error_pages, locations, ...
		std::vector<LocationConfig> locations;
};
