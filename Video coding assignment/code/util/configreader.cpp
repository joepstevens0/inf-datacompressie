#include "configreader.h"
#include <stdio.h>
#include <string.h>
#include <iostream>

#define MAXLEN 16384

bool ReadInputLine(FILE *fi, std::string &line)
{
	if (fi == 0)
		return false;
	
	char str[MAXLEN];
	int len;
	
	if (fgets(str,MAXLEN-1,fi) == 0)
		return false;

	str[MAXLEN-1] = 0;
	len = strlen(str);
	while (len > 0 && (str[len-1] == '\r' || str[len-1] == '\n'))
	{
		len--;
		str[len] = 0;
	}
	
	line = std::string(str);
	return true;
}

ConfigReader::ConfigReader()
{
}

ConfigReader::~ConfigReader()
{
}

bool ConfigReader::read(const std::string &fileName)
{
	clear();

	FILE *pFile = fopen(fileName.c_str(), "rt");
	if (pFile == NULL)
	{
		m_errStr = std::string("Can't open file");
		return false;
	}
	
	std::string line;

	while (ReadInputLine(pFile, line))
	{
		if (line.length() > 0)
		{
			size_t s = line.find("=");

			if (s == std::string::npos)
			{
				m_errStr = std::string("Can't find '=' in line");
				fclose(pFile);
				return false;
			}

			std::string key = line.substr(0, s);
			std::string value = line.substr(s+1);

			if (key.length() == 0)
			{
				m_errStr = std::string("Detected an empty key");
				fclose(pFile);
				return false;
			}

			if (m_keyValues.find(key) != m_keyValues.end())
			{
				m_errStr = std::string("Key '" + key + "' was found more than once");
				fclose(pFile);
				return false;
			}
			m_keyValues[key] = value;
		}
	}

	fclose(pFile);
	return true;
}

bool ConfigReader::getKeyValue(const std::string &key, std::string &value)
{
	std::map<std::string,std::string>::const_iterator it;

	it = m_keyValues.find(key);
	if (it == m_keyValues.end())
	{
		m_errStr = std::string("Key not found");
		return false;
	}

	value = it->second;

	return true;
}

void ConfigReader::printAll() const
{
	std::map<std::string,std::string>::const_iterator it = m_keyValues.begin();

	while (it != m_keyValues.end())
	{
		std::cout << it->first << "=" << it->second << std::endl;
		it++;
	}
}

void ConfigReader::clear()
{
	m_keyValues.clear();
}

std::string ConfigReader::getErrorDescription() const
{
	return m_errStr;
}
