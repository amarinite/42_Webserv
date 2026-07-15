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
		std::string							_index;

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
		static LocationConfig						buildDefault(const ServerConfig& parent);

		bool										hasAutoindex() const;
		bool										hasUploadEnabled() const;
		bool										hasCgi() const;

		const t_uri&								getPath() const;
		const std::vector<std::string>&				getAllowedMethods() const;
		const t_uri&								getRedirect() const;
		const std::string&							getUploadStore() const;
		const std::map<std::string, std::string>&	getCgiExtension() const;
		const std::string&							getRoot() const;
		const std::string&							getIndex() const;
		
};