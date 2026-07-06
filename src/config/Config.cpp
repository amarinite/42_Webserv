

Config Config::build(Node* root)
{
	Config config;
	for (/* each server Node in root->children */)
		config._servers.push_back(ServerConfig::build(child));
	return config;
}