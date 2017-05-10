#include <stdlib.h>
#include <io.h>
#include <string>
#include "md5.h"
#include <iostream>

#define FILENAME_OUT "UpdateFileList.ver"

using namespace std;

std::string g_jsonData = "{";
std::string g_md5 = "";

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
			int len = strlen( fileDir.name );
			if( strcmp( fileDir.name + len - 4, ".ver" ) ==0 )
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
		}
		else
		{
			char szJson[512] = { 0 };
			sprintf_s(szJson, "%s\"%s\":{", (bFirst ? "" : ","), fileDir.name);
			g_jsonData += szJson;

			std::string strDir = strFolder;
			strDir += fileDir.name;
			strDir += "/";
			FilesMD5(strDir.c_str());

			g_jsonData += "}";
		}

		bFirst = false;
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
	std::cout << szMd5 << std::endl;
}

int main( int argc, char **argv )
{
	FilesMD5("");
	g_jsonData += "}";
	SaveToFileList();
	PrintMD5();

	system( "pause" );
	return 0;
}