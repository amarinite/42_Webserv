
class LocationConfig
{
	public:
		static LocationConfig build(Node* locationNode, const ServerConfig& parent);
		// fields: path, root, index, allowMethods, cgiHandlers, ...
};