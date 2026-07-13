#pragma once
#include <vector>
#include "ParseUtils.hpp"
#include "ParseConfig.hpp"
#include "ServerConfig.hpp"

class Config
{
	private:
		std::vector<ServerConfig> _servers;

	public:
		static Config						build(Node* root);
		
		const std::vector<ServerConfig>&	getServers() const;
		const ServerConfig&					getServer(const ListenAddr& ipPort) const;
};