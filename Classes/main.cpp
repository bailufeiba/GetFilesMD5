#include <stdlib.h>
#include <io.h>
#include <string>
#include "md5.h"
#include <iostream>
#include <map>
#include <vector>

#define FILENAME_OUT "UpdateFileList.ver"

using namespace std;

struct SCmd
{
	SCmd()
	{
		cmd = "";
		params = "";
		desc = "";
	}

	SCmd( std::string c, std::string p, std::string d )
	{
		cmd = c;
		params = p;
		desc = d;
	}

	std::string cmd;
	std::string params;
	std::string desc;
};

typedef std::map< std::string , SCmd > mapCmd;
mapCmd g_mapCmd;

std::string g_jsonData = "{";
std::string g_md5 = "";

std::vector<std::string> g_vcAddDirectory;
std::vector<std::string> g_vcNoFile;

int stringLastIndexOf( const char* src, const char* sub )
{
	if (src == nullptr)	return -1;
	if (sub == nullptr) return -1;

	int lSrc = strlen( src );
	int lSub = strlen( sub );
	if (lSub > lSrc) return -1;

	for (int i = lSrc-1; i >= lSub-1; --i)
	{
		bool bSame = true;
		for (int j = lSub-1; j >= 0; --j)
		{
			if (sub[j] != src[i - j])
			{
				bSame = false;
				break;
			}
		}

		if (bSame)
			return i;
	}

	return -1;
}

void stringSplit( char* src, const char* Delim, std::vector<std::string>& vc )
{
	char* buff;
	char* token = strtok_s(src, Delim, &buff);
	while (token != nullptr)
	{
		vc.push_back(token);
		token = strtok_s(NULL, Delim, &buff);
	}
}

std::string getFileType( const char* filename )
{
	int idx = stringLastIndexOf(filename, ".");
	if (idx == -1)
		return "unknown";
	return filename + idx + 1;
}

bool isNeglectFile( const char* filename )
{
	if (g_vcNoFile.empty())
		return false;

	std::string type = getFileType(filename);
	std::vector<std::string>::iterator iter2 = g_vcNoFile.begin();
	for (; iter2 != g_vcNoFile.end(); ++iter2)
	{
		if (iter2->compare(type.c_str()) == 0)
			return true;
	}

	return false;
}

bool isNeglectDirectory( const char* dir )
{
	mapCmd::iterator iter = g_mapCmd.find("-nodir");
	if (iter == g_mapCmd.end())
		return false;
	return iter->second.params.compare(dir) == 0;
}

void FilesMD5( const char* strFolder)
{
	_finddata_t fileDir;
	std::string dir = strFolder;
	dir += "*.*";
	long lfDir = _findfirst(dir.c_str(), &fileDir);

	if (lfDir == -1l)
	{
		_findclose(lfDir);
		return;
	}

	bool bFirst = true;
	do
	{
		if (strcmp(fileDir.name, ".") == 0 || strcmp(fileDir.name, "..") == 0)
			continue;

		if ( (fileDir.attrib & _A_SUBDIR) == 0 )
		{
			if( isNeglectFile(fileDir.name) )
				continue;			

			std::string filename = strFolder;
			filename += fileDir.name;

			FILE* file;
			int nerror = fopen_s(&file,  filename.c_str(), "rb");
			if (nerror != 0)
			{
				//cout << "打开文件失败:" << filename.c_str() << "\n" << endl;
				continue;
			}
			//cout << "读取文件:" << filename.c_str() << endl;

			fseek(file, 0L, SEEK_END);
			int sz = ftell(file);
			char* pBuff = new char[sz + 1];
			pBuff[sz] = 0;
			fseek(file, 0L, SEEK_SET);
			int readsize = fread(pBuff, sizeof(unsigned char), sz, file);
			fclose(file);

			char szMd5[64] = { 0 };
			md5_str(pBuff, sz, szMd5, 33);
			g_md5 += szMd5;
			//cout << "MD5:" << szMd5 << endl;

			char szJson[512] = { 0 };
			sprintf_s( szJson, "%s\"%s\":{\"MD5\":\"%s\",\"size\":%d}", (bFirst?"":","), fileDir.name, szMd5, sz  );
			g_jsonData += szJson;
			bFirst = false;
		}
		else
		{
			std::string strDir = strFolder;
			strDir += fileDir.name;

			if (!isNeglectDirectory(strDir.c_str()))
			{
				char szJson[512] = { 0 };
				sprintf_s(szJson, "%s\"%s\":{", (bFirst ? "" : ","), fileDir.name);
				g_jsonData += szJson;

				strDir += "/";
				FilesMD5(strDir.c_str());

				g_jsonData += "}";
				bFirst = false;
			}
		}

	} while (_findnext(lfDir, &fileDir) == 0);
	_findclose(lfDir);
}

void SaveToFileList()
{
	FILE* file;
	int nError = fopen_s(&file, FILENAME_OUT, "wb");
	if (nError == 0)
	{
		fwrite(g_jsonData.c_str(), g_jsonData.size(), 1, file);
		fflush(file);
		fclose(file);
	}
}

void PrintMD5()
{
	char szMd5[64] = { 0 };
	md5_str(g_md5.c_str(), g_md5.size(), szMd5, 33);
	std::cout << "MD5:" << szMd5 << std::endl;
}

void insertNoFile(const char* type)
{
	std::vector<std::string>::iterator iter = g_vcNoFile.begin();
	for (; iter != g_vcNoFile.end(); ++iter)
	{
		if (iter->compare(type) == 0)
			return;
	}

	g_vcNoFile.push_back(type);
}

void initCmd()
{
	g_mapCmd.clear();
	g_mapCmd.insert(std::pair<std::string, SCmd>("-url", SCmd("-url", "", "-url insert url,such as:-url \"http://test.update.com\\\", it will insert \"url\":\"http://test.update.com\\\" to json ")));
	g_mapCmd.insert(std::pair<std::string, SCmd>("-dir", SCmd("-dir", "", "- dir input direcotry, such as:-dir testdir/a, it will add testdir/a/ to all file path")));
	g_mapCmd.insert(std::pair<std::string, SCmd>("-nodir", SCmd("-nodir", "", "-nodir input direcotry, such as:-nodir tesetdirectory, it will neglect directory")));
	g_mapCmd.insert(std::pair<std::string, SCmd>("-nofile", SCmd("-nofile", "", "-nofile input file type, such as:-nofile exe ver, it will neglect exe files and ver files")));
	g_mapCmd.insert(std::pair<std::string, SCmd>("-help", SCmd("-help", "", "-help it will print all cmd")));
	
	insertNoFile("exe");
	insertNoFile("ver");
}

void PrintParams(int argc, char **argv)
{
	std::cout << "argc = " << argc << endl;
	for (int i = 0; i < argc; ++i)
		std::cout << argv[i] << endl;
}

void PrintCMD()
{
	mapCmd::iterator iter = g_mapCmd.begin();
	for (; iter != g_mapCmd.end(); ++iter)
		std::cout << iter->second.desc.c_str() << endl;
}

bool isCmd( char* arg )
{
	return arg[0] == '-';
}

void procesParams()
{
	mapCmd::iterator iter = g_mapCmd.find("-nofile");
	if (iter != g_mapCmd.end())
	{
		char szParams[512] = { 0 };
		sprintf_s(szParams, "%s", iter->second.params.c_str());

		std::vector<std::string> vcType;
		stringSplit(szParams, " ", vcType);

		std::vector<std::string>::iterator iterType = vcType.begin();
		for (; iterType != vcType.end(); ++iterType)
			insertNoFile(iterType->c_str());
	}

	iter = g_mapCmd.find("-nodir");
	if (iter != g_mapCmd.end())
	{
		char szDir[512] = { 0 };
		sprintf_s(szDir, "%s", iter->second.params.c_str());
		int len = strlen( szDir );
		if (szDir[len - 1] == '/')
		{
			szDir[len - 1] = 0;
			iter->second.params = szDir;
		}
	}
}

void getParams( int argc, char **argv )
{
	std::string strCmd = "";
	std::string strParams = "";
	for (int i = 0; i < argc; ++i)
	{
		if (isCmd(argv[i]))
		{
			mapCmd::iterator iter = g_mapCmd.find(strCmd.c_str());
			if (iter != g_mapCmd.end())
				iter->second.params = strParams;
			strCmd = argv[i];
			strParams = "";
		}
		else
		{
			if( strParams.compare( "" ) != 0 )
				strParams += " ";
			strParams += argv[i];
		}
	}
	
	mapCmd::iterator iter = g_mapCmd.find(strCmd.c_str());
	if (iter != g_mapCmd.end())
		iter->second.params = strParams;

	procesParams();
}

void insertURL()
{
	mapCmd::iterator iter = g_mapCmd.find( "-url" );
	if ( iter != g_mapCmd.end() && iter->second.params.compare("") != 0 )
	{
		char js[512] = { 0 };
		sprintf_s(js, "\"url\":\"%s\",", iter->second.params.c_str());
		g_jsonData += js;
	}
}

void InsertBeginDirectory()
{
	g_vcAddDirectory.clear();
	mapCmd::iterator iter = g_mapCmd.find("-dir");
	if ( iter == g_mapCmd.end() || iter->second.params.compare("") == 0 )
		return;
	
	char szDir[512] = { 0 };
	sprintf_s(szDir, "%s", iter->second.params.c_str());
	int len = strlen( szDir );
	if (szDir[len - 1] == '/')
		szDir[len - 1] = 0;

	stringSplit( szDir, "/", g_vcAddDirectory );
	std::vector<std::string>::iterator iter2 = g_vcAddDirectory.begin();
	for (; iter2 != g_vcAddDirectory.end(); ++iter2)
	{
		char js[128] = { 0 };
		sprintf_s(js, "\"%s\":{", iter2->c_str());
		g_jsonData += js;
	}

}

void InsertEndDirectory()
{
	int sz = g_vcAddDirectory.size();
	for (int i = 0; i < sz; ++i)
		g_jsonData += "}";
}

int main( int argc, char **argv )
{
// 	std::cout << "argc=" << argc << endl;
// 	for (int i = 0; i < argc; ++i)
// 	{
// 		char szArgv[256] = { 0 };
// 		sprintf_s(szArgv, "argv[%d]=%s", i, argv[i]);
// 		std::cout << szArgv << endl;
// 	}
// 	system("pause");

	initCmd();

	if (argc == 2 && strcmp(argv[1], "-help" ) == 0  )
	{
		PrintCMD();
		system("pause");
		return 0;
	}

	if( argc > 1 )
		getParams( argc-1, argv+1 );
	insertURL();
	InsertBeginDirectory();
	FilesMD5("");
	InsertEndDirectory();
	g_jsonData += "}";

	SaveToFileList();
	PrintMD5();

	system( "pause" );
	return 0;
}