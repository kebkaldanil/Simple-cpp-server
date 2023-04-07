type DB = Record<string, {
  source?: string,
  compressible?: boolean,
  extensions?: string[]
}>;
/**Removes the first occurrence of an object from an array.
@template T - The type of the elements in the array.
@param {T[]} arr - The array to remove the object from.
@param {T} obj - The object to remove from the array.
@returns {number} - The index of the removed element or -1 if the element was not found in the array.
*/
const removeFromArray = <T extends {}>(arr: T[], obj: T) => {
  for (let i = 0; i < arr.length; i++) {
    if (Object.is(arr[i], obj)) {
      arr.splice(i, 1);
      return i;
    }
  }
  return -1;
};
const encoder = new TextEncoder();
const decoder = new TextDecoder();
let db: DB | null = JSON.parse(decoder.decode(Deno.readFileSync("db.json")));
let preferMime: Record<string, string> | null = JSON.parse(decoder.decode(Deno.readFileSync("preferMime.json")));
const extensionToMimeMap: Record<string, string> = {};
let mimeToExtensionsMap: Record<string, string[]> | null = {};
for (const mime in db) {
  const { extensions, source } = db[mime];
  if (!extensions) {
    continue;
  }
  mimeToExtensionsMap[mime] = extensions;
  if (extensions.length === 1) {
    const [extension] = extensions;
    const prevMime: string | undefined = extensionToMimeMap[extension];
    if (prevMime) {
      const prevExtensionsFromMime = mimeToExtensionsMap[prevMime];
      if (prevExtensionsFromMime.length === 1) {
        const [prevExtensionFromMime] = prevExtensionsFromMime;
        const { source: prevSource } = db[prevMime];
        if (preferMime![extension]) {
          extensionToMimeMap[extension] = preferMime![extension];
          continue;
        }
        if (source === "iana" && prevSource !== "iana") {
          extensionToMimeMap[extension] = mime;
          continue;
        }
        if ((prevMime !== mime || prevExtensionFromMime !== extension) && source === "iana") {
          console.warn(`Conflict between (prev | cur) "${prevExtensionFromMime}" | "${extension}" for mimes "${prevMime}" | "${mime}", sources: "${prevSource}" | "${source}"`);
        }
        continue;
      }
    }
    extensionToMimeMap[extension] = mime;
    continue;
  }
  for (const extension of extensions) {
    if (!extensionToMimeMap[extension]) {
      extensionToMimeMap[extension] = mime;
    }
  }
}
db = null;
preferMime = null;

const mimeToExtensionMap: Record<string, string> = {};
for (const mime in mimeToExtensionsMap) {
  const extensions = mimeToExtensionsMap[mime];
  if (extensions.length === 1) {
    mimeToExtensionMap[mime] = extensions[0];
    continue;
  }
  for (const extension of extensions) {
    if (extensionToMimeMap[extension] === mime) {
      mimeToExtensionMap[mime] = extension;
      continue;
    }
  }
  mimeToExtensionMap[mime] = extensions[0];
}
mimeToExtensionsMap = null;

const resultFileName = "DefaultMimeMapper.cpp";
const result = `#include "MimeMapper.h"

MimeMapper MimeMapper::createDefault()
{
	std::unordered_map<std::string, std::string> extensionToMimeMap = {
		${Object.entries(extensionToMimeMap).map((ent) => `{${ent.map(v => JSON.stringify(v)).join(", ")}}`).join(",\n		")}
	};
	std::unordered_map<std::string, std::string> mimeToExtensionMap = {
		${Object.entries(mimeToExtensionMap).map((ent) => `{${ent.map(v => JSON.stringify(v)).join(", ")}}`).join(",\n		")}
	};
	return MimeMapper(std::move(extensionToMimeMap), std::move(mimeToExtensionMap));
}
`;
Deno.writeFileSync(resultFileName, encoder.encode(result));
