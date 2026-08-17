#pragma once
#include <string>
#include <vector>
#include <map>
#include "ParseConfig.hpp"
#include "UriParser.hpp"

class ServerConfig;

class LocationConfig
{
	private:
		t_uri								_path;
		std::vector<std::string>			_allowed_methods;
		t_uri								_redirect;
		bool								_autoindex;
		std::string							_upload_store;
		std::map<std::string, std::string>	_cgi_extension;
		std::string							_root;
		std::vector<std::string>			_index;
		std::vector<LocationConfig>			_locations;

		typedef void (LocationConfig::*DirectiveHandler)(const Node*);
		static std::map<std::string, DirectiveHandler>	initHandlers();

		void setAllowedMethods(const Node* n);
		void setRedirect(const Node* n);
		void setAutoindex(const Node* n);
		void setUploadStore(const Node* n);
		void setCgiExtension(const Node* n);
		void setRoot(const Node* n);
		void setIndex(const Node* n);

	public:
		LocationConfig();
		static LocationConfig						build(Node* locationNode, const ServerConfig& parent);
		static LocationConfig						build(Node* locationNode, const LocationConfig& parent);
		static LocationConfig						buildDefault(const ServerConfig& parent);
		static LocationConfig						buildFrom(Node* locationNode, const std::string& root, const std::vector<std::string>& index);

		bool										hasAutoIndex() const;
		bool										hasUploadEnabled() const;
		bool										hasCgi() const;

		const t_uri&								getPath() const;
		const std::vector<std::string>&				getAllowedMethods() const;
		const t_uri&								getRedirect() const;
		const std::string&							getUploadStore() const;
		const std::map<std::string, std::string>&	getCgiExtension() const;
		const std::string&							getRoot() const;
		const std::vector<std::string>&				getIndex() const;
		const std::vector<LocationConfig>&			getLocations() const;
		const LocationConfig*						getLocationConfig(const t_uri& uri) const;
};

bool	isValidMatch(const std::string& reqPath, const std::string& configPath);
