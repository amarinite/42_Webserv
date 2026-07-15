#pragma once
#include <string>
#include <vector>
#include <map>
#include "ParseConfig.hpp"
#include "ParseUtils.hpp"
#include "UriParser.hpp"
#include "LocationConfig.hpp"

class ServerConfig
{
	private:
		std::vector<ListenAddr>		_listen;
		std::map<int, std::string>	_error_pages;
		int							_client_max_body_size;
		std::string					_root;
		std::string					_index;
		std::vector<LocationConfig>	_locations;

		typedef void (ServerConfig::*DirectiveHandler)(const Node*);
		static std::map<std::string, DirectiveHandler>	initHandlers();

		void setListen(const Node* n);
		void setErrorPage(const Node* n);
		void setClientMaxBodySize(const Node* n);
		void setRoot(const Node* n);
		void setIndex(const Node* n);

	public:
		ServerConfig();
		static ServerConfig					build(Node* serverNode);

		const std::vector<ListenAddr>&		getListen() const;
		const std::map<int, std::string>&	getErrorPages() const;
		int									getClientMaxBodySize() const;
		const std::string&					getRoot() const;
		const std::string&					getIndex() const;
		const std::vector<LocationConfig>&	getLocations() const;
		const LocationConfig&				getLocationConfig(const t_uri& uri) const;
};
