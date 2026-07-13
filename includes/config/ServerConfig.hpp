#pragma once
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
		std::map<int, t_uri>		_error_pages;
		int							_client_max_body_size;
		t_uri						_root;
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
		const std::map<int, t_uri>&			getErrorPages() const;
		int									getClientMaxBodySize() const;
		const t_uri&						getRoot() const;
		const std::string&					getIndex() const;
		const std::vector<LocationConfig>&	getLocations() const;
		const LocationConfig&				getLocationConfig(const t_uri& uri) const;
};
