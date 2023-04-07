#include "MimeMapper.h"

using namespace std;

string extractExtension(const string& path) {
	size_t lastDot = path.find_last_of('.');
	if (lastDot == string::npos) {
		return "";
	}
	size_t lastSeparatorIndex = path.find_last_of('/');
	if (lastDot <= lastSeparatorIndex && lastSeparatorIndex != string::npos) {
		return "";
	}
	if (lastDot + 1 == path.length()) {
		return "";
	}
	size_t nameStart = path.find_first_not_of('.', lastSeparatorIndex + 1);
	if (lastDot != 0 && nameStart == lastDot + 1) {
		return "";
	}
	return path.substr(lastDot + 1);
}

MimeMapper::MimeMapper(
	unordered_map<string, string>&& extensionToMimeMap,
	unordered_map<string, string>&& mimeToExtensionMap
) : extensionToMimeMap(extensionToMimeMap), mimeToExtensionMap(mimeToExtensionMap) {}

MimeMapper::MimeMapper(
	const unordered_map<string, string>& extensionToMimeMap,
	const unordered_map<string, string>& mimeToExtensionMap
) : extensionToMimeMap(extensionToMimeMap), mimeToExtensionMap(mimeToExtensionMap) {}

MimeMapper::MimeMapper() {
}

MimeMapper::MimeMapper(const MimeMapper &src) : 
	extensionToMimeMap(src.extensionToMimeMap), 
	mimeToExtensionMap(src.mimeToExtensionMap) {}

MimeMapper::MimeMapper(MimeMapper &&src) noexcept :
	extensionToMimeMap(move(src.extensionToMimeMap)), 
	mimeToExtensionMap(move(src.mimeToExtensionMap)) {}

string MimeMapper::getMime(const string& path) const {
	string ext = extractExtension(path);
	if (ext.empty()) {
		return "";
	}
	auto it = extensionToMimeMap.find(ext);
	if (it != extensionToMimeMap.end()) {
		return it->second;
	}
	else {
		return "";
	}
}

string MimeMapper::getExtension(const string& mime) const {
	if (mime.empty()) {
		return mime;
	}
	auto it = mimeToExtensionMap.find(mime);
	if (it != mimeToExtensionMap.end()) {
		return it->second;
	}
	else {
		return "";
	}
}

bool MimeMapper::addMapping(const string& path, const string& mime, ForceUpdate forceUpdate) {
	if (mime.empty()) {
		return false;
	}
	string ext = extractExtension(path);
	if (ext.empty()) {
		return false;
	}
	bool addedExtension = addMappingExtensionToMime(ext, mime, forceUpdate);
	bool addedMime = addMappingMimeToExtension(ext, mime, forceUpdate);
	return addedExtension || addedMime;
}

bool MimeMapper::addMappingMimeToExtension(const string& extension, const string& mime, ForceUpdate forceUpdate)
{
	if (forceUpdate != ForceUpdate::MimeToExtension && forceUpdate != ForceUpdate::Both) {
		if (mimeToExtensionMap.find(extension) != mimeToExtensionMap.end()) {
			return false;
		}
	}
	mimeToExtensionMap[mime] = extension;
	return true;
}

bool MimeMapper::addMappingExtensionToMime(const string& extension, const string& mime, ForceUpdate forceUpdate)
{
	if (forceUpdate != ForceUpdate::ExtensionToMime && forceUpdate != ForceUpdate::Both) {
		if (extensionToMimeMap.find(extension) != extensionToMimeMap.end()) {
			return false;
		}
	}
	extensionToMimeMap[extension] = mime;
	return true;
}
