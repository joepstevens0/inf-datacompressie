#ifndef CONFIGREADER_H

#define CONFIGREADER_H

#include <string>
#include <map>


// Helper class to read config files containing 'key=value' lines.
// If a function returns 'false', the getErrorDescription() member
// provides an error string

class ConfigReader
{
public:
	ConfigReader();
	~ConfigReader();

	bool read(const std::string &fileName);
	bool getKeyValue(const std::string &key, std::string &value);
	void printAll() const;
	void clear();

	std::string getErrorDescription() const;
private:
	std::map<std::string, std::string> m_keyValues;
	std::string m_errStr;
};

#endif // CONFIGREADER_H

