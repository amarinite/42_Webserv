#pragma once
#include <vector>
#include <map>
#include "UriParser.hpp"

class ServerConfig;

class LocationConfig
{
	private:
		t_uri							_path;
		std::vector<std::string>		_allowed_methods;
		t_uri							_redirect;
		bool							_autoindex;
		t_uri							_upload_store;
		std::map<std::string, t_uri>	_cgi_extension;
		t_uri							_root;
		std::string						_index;

		typedef void (LocationConfig::*DirectiveHandler)(const Node*);
		static std::map<std::string, DirectiveHandler>	initHandlers();

		void setPath(const Node* n);
		void setAllowedMethods(const Node* n);
		void setRedirect(const Node* n);
		void setAutoindex(const Node* n);
		void setUploadStore(const Node* n);
		void setCgiExtension(const Node* n);
		void setRoot(const Node* n);
		void setIndex(const Node* n);

	public:
		LocationConfig();
		static LocationConfig				build(Node* locationNode, const ServerConfig& parent);
		static LocationConfig				buildDefault(const ServerConfig& parent);

		bool								hasAutoindex() const;
		bool								hasUploadEnabled() const;
		bool								hasCgi() const;

		const t_uri&						getPath() const;
		const std::vector<std::string>&		getAllowedMethods() const;
		const t_uri&						getRedirect() const;
		const t_uri&						getUploadStore() const;
		const std::map<std::string, t_uri>&	getCgiExtension() const;
		const t_uri&						getRoot() const;
		const std::string&					getIndex() const;
		
};