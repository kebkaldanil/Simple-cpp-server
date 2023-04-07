#ifndef MIMEMAPPER_H
#define MIMEMAPPER_H

#include <string>
#include <unordered_map>

class ExtensionToMimeMapper {
public:
	virtual ~ExtensionToMimeMapper() {};
	virtual std::string getMime(const std::string& extension) const = 0;
};

class MimeToExtensionMapper {
public:
	virtual ~MimeToExtensionMapper() {};
	virtual std::string getExtension(const std::string& mime) const = 0;
};

class IMimeMapper : public ExtensionToMimeMapper, public MimeToExtensionMapper {
public:
	enum class ForceUpdate
	{
		No,
		MimeToExtension,
		ExtensionToMime,
		Both
	};
	virtual bool addMapping(const std::string& extension, const std::string& mime, ForceUpdate forceUpdate = ForceUpdate::Both) = 0;
	bool addMapping(const std::string& extension, const std::string& mime, bool forceUpdate) {
		return addMapping(extension, mime, forceUpdate ? ForceUpdate::Both : ForceUpdate::No);
	}
};

class MimeMapper : public IMimeMapper {
private:
	std::unordered_map<std::string, std::string> extensionToMimeMap;
	std::unordered_map<std::string, std::string> mimeToExtensionMap;
protected:
	MimeMapper(std::unordered_map<std::string, std::string>&& extensionToMimeMap,
		std::unordered_map<std::string, std::string>&& mimeToExtensionMap);
	MimeMapper(const std::unordered_map<std::string, std::string>& extensionToMimeMap,
		const std::unordered_map<std::string, std::string>& mimeToExtensionMap);
public:
	MimeMapper();
	MimeMapper(const MimeMapper &src);
	MimeMapper(MimeMapper &&src) noexcept;
	static MimeMapper* createDefault();
	std::string getMime(const std::string& path) const override;
	std::string getExtension(const std::string& mime) const override;
	bool addMapping(const std::string& path, const std::string& mime, ForceUpdate forceUpdate = ForceUpdate::Both) override;
private:
	bool addMappingMimeToExtension(const std::string& extension, const std::string& mime, ForceUpdate forceUpdate);
	bool addMappingExtensionToMime(const std::string& extension, const std::string& mime, ForceUpdate forceUpdate);
};

#endif // MIMEMAPPER_H
