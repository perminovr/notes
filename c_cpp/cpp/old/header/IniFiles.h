#ifndef INI_H
#define INI_H


#include <string>
#include <map>
#include <fstream>


namespace perri {

/*! ----------------------------------------------------------------------------------------
 * @brief: Reads ini file and devides pairs (key, value) by sections
 * -----------------------------------------------------------------------------------------
 * */
class IniFile {
public:
	/*! ------------------------------------------------------------------------------------
	 * @brief: Opens file and checks for errors
	 * -------------------------------------------------------------------------------------
	 * */
	IniFile(std::string fileName);

	/*! ------------------------------------------------------------------------------------
	 * @brief: Closes file and clears config info
	 * -------------------------------------------------------------------------------------
	 * */
	~IniFile();

	/*! ------------------------------------------------------------------------------------
	 * @brief: Reads ini file into pairs
	 * -------------------------------------------------------------------------------------
	 * */
	void Parse();

	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	bool isOpened() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Returns all sections with its pairs
	 * -------------------------------------------------------------------------------------
	 * */
	const std::map <std::string, std::map<std::string, std::string> >& GetConfig() const;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Returns pairs of one of sections with section_name
	 * -------------------------------------------------------------------------------------
	 * */
	const std::map <std::string, std::string>& GetSection(std::string section_name) const;

private:
	/*! ------------------------------------------------------------------------------------
	 * @brief:
	 * -------------------------------------------------------------------------------------
	 * */
	std::ifstream file;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Pairs (key, values) devided by sections
	 * -------------------------------------------------------------------------------------
	 * */
	std::map <std::string, std::map<std::string, std::string> > configInfo;

	/*! ------------------------------------------------------------------------------------
	 * @brief: Resets errno
	 * -------------------------------------------------------------------------------------
	 * */
	int GetLastSystemError() const;
};

}

#endif