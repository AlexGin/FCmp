#include "dircomparer.h"

#include "spdlog/spdlog.h"

DirComparer::DirComparer(QObject *parent)
    : m_pDir1(NULL), m_pDir2(NULL), m_dcMode(DIR_COMPARE::DIR_COMPARE_ALL), FileComparer(parent)
{

}

DirComparer::DirComparer(QFileInfo* pDirInfo1, QFileInfo* pDirInfo2, DIR_COMPARE dc, QObject *parent)
    : m_dcMode(dc), FileComparer(parent)
{
    SetDir12(pDirInfo1, pDirInfo2);
}

DirComparer::DirComparer(QDir* pDir1, QDir* pDir2, DIR_COMPARE dc, QObject *parent)
    : m_pDir1(pDir1), m_pDir2(pDir2), m_dcMode(dc), FileComparer(parent)
{

}

void DirComparer::SetDir12(QFileInfo* pDirInfo1, QFileInfo* pDirInfo2)
{
    // m_pDir1 = new QDir(pDirInfo1->absoluteDir());
    // m_pDir2 = new QDir(pDirInfo2->absoluteDir());
	m_pDir1 = new QDir(pDirInfo1->filePath());
	m_pDir2 = new QDir(pDirInfo2->filePath());
}

void DirComparer::SetDirCompareMode(DIR_COMPARE dc)
{
    m_dcMode = dc;
}

char* DirComparer::FillFilterString(DIR_COMPARE dc)
{
	char* szFilterString = new char[64];
    memset(szFilterString, 0, sizeof(szFilterString));

    switch (dc)
	{
        case DIR_COMPARE::DIR_COMPARE_CCPP:
            strcpy(szFilterString, "*.c, *.cc, *.cpp, *.h, *.hpp");
			break;
        case DIR_COMPARE::DIR_COMPARE_WMFC:
			strcpy(szFilterString, "*.cpp, *.h, *.rc, *.idl");
			break;
        case DIR_COMPARE::DIR_COMPARE_CMCD:
            strcpy(szFilterString, "*.c, *.cc, *.cpp, *.h, *.hpp");
            break;
        case DIR_COMPARE::DIR_COMPARE_QT:
			strcpy(szFilterString, "*.cpp, *.h, *.ui, *.qrc, *.pro");
			break;
        case DIR_COMPARE::DIR_COMPARE_CS:
			strcpy(szFilterString, "*.cs, *.resx, *.xaml");
			break;
        case DIR_COMPARE::DIR_COMPARE_JAVA:
            strcpy(szFilterString, "*.java, *.xml");
            break;
	}
	return szFilterString;
}

std::string DirComparer::PrepareStringMode(DIR_COMPARE dc)
{
    std::string sResult;
    switch (dc)
    {
        case DIR_COMPARE::DIR_COMPARE_ALL:  sResult = "'All-files'"; break;
        case DIR_COMPARE::DIR_COMPARE_CCPP: sResult = "'C/C++'"; break;
        case DIR_COMPARE::DIR_COMPARE_WMFC: sResult = "'WinAPI/MFC'"; break;
        case DIR_COMPARE::DIR_COMPARE_CMCD: sResult = "'CMake/VSCode (C++)'"; break;
        case DIR_COMPARE::DIR_COMPARE_QT:   sResult = "'Qt Projects'"; break;
        case DIR_COMPARE::DIR_COMPARE_CS:   sResult = "'C# Projects'"; break;
        case DIR_COMPARE::DIR_COMPARE_JAVA: sResult = "'JAVA (Android)'"; break;
    }
    return sResult;
}

void DirComparer::FillDirsMap()
{	 
    std::string sMode = PrepareStringMode(m_dcMode);
    spdlog::info("DirComparer::FillDirsMap Mode: {0}", sMode);
    if (m_dcMode == DIR_COMPARE::DIR_COMPARE_ALL)
	{		
		PrepareDirsMap(m_pDir1, m_dfm, m_vectFileNames1, true, m_strBase1); // Preparing for first window
        PrepareDirsMap(m_pDir2, m_dfm, m_vectFileNames2, false, m_strBase2); // Preparing for second window
        return;
	}
    else if (m_dcMode == DIR_COMPARE::DIR_COMPARE_CMCD)
    {
        char* szFilterString = FillFilterString(m_dcMode);
        PrepareDirsMap(m_pDir1, m_dfm, m_vectFileNames1, true, m_strBase1, szFilterString, "CMakeLists.txt"); // Preparing for first window
        PrepareDirsMap(m_pDir2, m_dfm, m_vectFileNames2, false, m_strBase2, szFilterString, "CMakeLists.txt"); // Preparing for second window
        delete[] szFilterString;
        return;
    }
    else if ((m_dcMode != DIR_COMPARE::DIR_COMPARE_QT) && (m_dcMode != DIR_COMPARE::DIR_COMPARE_CMCD))
	{		
        char* szFilterString = FillFilterString(m_dcMode);
		PrepareDirsMap(m_pDir1, m_dfm, m_vectFileNames1, true, m_strBase1, szFilterString); // Preparing for first window
		PrepareDirsMap(m_pDir2, m_dfm, m_vectFileNames2, false, m_strBase2, szFilterString); // Preparing for second window
        delete[] szFilterString;
        return;
    }
    else if (m_dcMode == DIR_COMPARE::DIR_COMPARE_QT)
	{	// Exclude files starting with "moc_" from list of files:
        char* szFilterString = FillFilterString(m_dcMode);
		PrepareDirsMap(m_pDir1, m_dfm, m_vectFileNames1, true, m_strBase1, szFilterString, true); // Preparing for first window
		PrepareDirsMap(m_pDir2, m_dfm, m_vectFileNames2, false, m_strBase2, szFilterString, true); // Preparing for second window
		delete[] szFilterString;
        return;
	}
}

void DirComparer::PrepareDfm(DIRS_FILES_MAP& dfm, std::string& sFilePath, std::string& sBase, bool bFirst)
{
    std::string sFileGeneralPath = ConvertAbsPath(sFilePath, sBase);
    DIRS_FILES_MAP::iterator it=dfm.find(sFileGeneralPath);
    if (it==dfm.end()) // if value is not exist
        dfm[sFileGeneralPath] = 0;

    if (bFirst)
        dfm[sFileGeneralPath] += EXIST_IN_FIRST;
    else
        dfm[sFileGeneralPath] += EXIST_IN_SECOND;
}

bool DirComparer::PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst, std::string& strBase, char* szFileExtList)
{
    QString strDirAbsPath = pDir->absolutePath();	
	strBase = strDirAbsPath.toStdString();
    
	if (bFirst)
		dfm.clear();

    vectFileNames.clear();
	QDirIterator dirIt(strDirAbsPath, QDirIterator::Subdirectories);
	while (dirIt.hasNext())
    {
		dirIt.next();
		QFileInfo fileInfo = dirIt.fileInfo();        
        if (fileInfo.isFile())
        {
            QString strFN = fileInfo.completeBaseName(); // .fileName();
            std::string sFN = strFN.toStdString();
			QString strFilePath = dirIt.filePath();
			std::string sFilePath = strFilePath.toStdString();
            if (szFileExtList)
            {
                if (IsFileExtValid(sFilePath /*sFN*/, szFileExtList))
                {
					PrepareDfm(dfm, sFilePath, strBase, bFirst);
					vectFileNames.push_back(sFilePath);
                    spdlog::info("PrepareDirsMap_1 - PATH: {} FileExtValid", sFilePath);
                }
            }
            else
            {
                if (IsFileExtValid(sFilePath))
                {
                    PrepareDfm(dfm, sFilePath, strBase, bFirst);
                    vectFileNames.push_back(sFilePath);
                    spdlog::info("PrepareDirsMap_1 - PATH: {}", sFilePath);
                }
            }
        }        
    }
    if (!vectFileNames.size())
        return false;

	return true;
}
// Param. bQt - for exclude "moc_" files, and other special files (in the Qt projects):
bool DirComparer::PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst, std::string& strBase, char* szFileExtList, bool bQt)
{
	QString strDirAbsPath = pDir->absolutePath();
	strBase = strDirAbsPath.toStdString();

	if (bFirst)
		dfm.clear();

	vectFileNames.clear();
	QDirIterator dirIt(strDirAbsPath, QDirIterator::Subdirectories);
	while (dirIt.hasNext())
	{
		dirIt.next();
		QFileInfo fileInfo = dirIt.fileInfo();
		if (fileInfo.isFile())
		{
			QString strFN = fileInfo.fileName();
			std::string sFN = strFN.toStdString();
			QString strFilePath = dirIt.filePath();
			std::string sFilePath = strFilePath.toStdString();
			if (szFileExtList && bQt)
			{
				if (IsFileExtValid(sFN, szFileExtList, bQt))
				{
					PrepareDfm(dfm, sFilePath, strBase, bFirst);
					vectFileNames.push_back(sFilePath);
                    spdlog::info("PrepareDirsMap_2 - PATH: {} FileExtValid", sFilePath);
				}
			}
			else
			{
				PrepareDfm(dfm, sFilePath, strBase, bFirst);
				vectFileNames.push_back(sFilePath);
                spdlog::info("PrepareDirsMap_2 - PATH: {}", sFilePath);
			}
		}
	}
	if (!vectFileNames.size())
		return false;

	return true;
}
// Param. szCMakeLists - need for comparing appropriate file (during use CMake)
bool DirComparer::PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst, std::string& strBase, char* szFileExtList, const char* szCMakeLists)
{
    QString strDirAbsPath = pDir->absolutePath();
    strBase = strDirAbsPath.toStdString();

    if (bFirst)
        dfm.clear();

    vectFileNames.clear();
    QDirIterator dirIt(strDirAbsPath, QDirIterator::Subdirectories);
    while (dirIt.hasNext())
    {
        dirIt.next();
        QFileInfo fileInfo = dirIt.fileInfo();
        if (fileInfo.isFile())
        {
            QString strFN = fileInfo.completeBaseName(); // .fileName();
            std::string sFN = strFN.toStdString();
            QString strFilePath = dirIt.filePath();
            std::string sFilePath = strFilePath.toStdString();
            if (szFileExtList)
            {
                if (IsFileExtValid(sFilePath /*sFN*/, szFileExtList, szCMakeLists))
                {
                    PrepareDfm(dfm, sFilePath, strBase, bFirst);
                    vectFileNames.push_back(sFilePath);
                    spdlog::info("PrepareDirsMap_3 - PATH: {} FileExtValid", sFilePath);
                }
            }
            else
            {
                if (IsFileExtValid(sFilePath))
                {
                    PrepareDfm(dfm, sFilePath, strBase, bFirst);
                    vectFileNames.push_back(sFilePath);
                    spdlog::info("PrepareDirsMap_3 - PATH: {}", sFilePath);
                }
            }
        }
    }
    if (!vectFileNames.size())
        return false;

    return true;
}

// bool DirComparer::IsFileToCompare(std::string& sFileName, STRING_VECTOR& vectFilters)
// {
//    STRING_VECTOR::iterator it=std::find(vectFilters.begin(),vectFilters.end(),sFileName);
//    if (it==vectFilters.end()) // value is not present
//        return false;
//
//    return true;
// }

bool DirComparer::IsFileQtValid(const std::string& sFileName, char* szExclude)
{
	int iLen = sFileName.length();
	int iExcludeLen = strlen(szExclude);
	bool bIsFileValid = false;
	if (iLen >= iExcludeLen)
	{
		for (int i = 0; i < iExcludeLen; i++)
		{
			char chr = sFileName[i];
			char chrExcl = szExclude[i];
			if (chr != chrExcl)
				bIsFileValid = true;
		}
	}
	return bIsFileValid;
}
// Special method - for CMake projects support:
bool DirComparer::IsFileExtValid(const std::string& sFileName, char* szFileExtList, const char* szCmakeLists)
{
    bool bExistTxt = sFileName.find(szCmakeLists) != std::string::npos;
    if (bExistTxt)
        return true;

    int iLen = sFileName.length() + 1;

    char* szFileExt1 = new char[iLen];
    memset(szFileExt1, 0, iLen);

    char* szFileExt2 = new char[iLen];
    memset(szFileExt2, 0, iLen);
    bool bVSCode = false;
    bool bJsonFlag = false;
    bool bIgnoreFlag = false;
    bool bFileExt1 = false; // String - first "extension"
    bool bFileExt2 = false; // String - second "extension"
    int iExtIndex1 = 0; // Indox of the first "extension"
    int iExtIndex2 = 0; // Indox of the second "extension"
    for (int iIndex = 0; iIndex < iLen; iIndex++)
    {
        char chr = sFileName[iIndex];
        if (bVSCode && (chr == '.'))
        {
            bFileExt2 = true;
            // spdlog::info("DirComparer::IsFileExtValid: CMake/VSCode - SET 'Ext2'");
        }
        if (chr == '.')
            bFileExt1 = true;

        if (bFileExt1)
        {
            szFileExt1[iExtIndex1++] = chr;

            if ( (!strcmp(szFileExt1, ".git")) ||
                (!strcmp(szFileExt1, ".cache")) ||
                (!strcmp(szFileExt1, ".qmake")))
            {
                bIgnoreFlag = true;
                break;
            }

            if (!strcmp(szFileExt1, ".vscode"))
            {
                bVSCode = true;
                // spdlog::info("DirComparer::IsFileExtValid: CMake/VSCode - '.vscode'");
            }
            if (bFileExt2)
            {
                szFileExt2[iExtIndex2++] = chr;
                if (!strcmp(szFileExt2, ".json"))
                {
                    bJsonFlag = true;
                    break;
                }
            }
        }
    }

    if (bIgnoreFlag) // If it's ignored file-name (of folder-name)::
    {
        delete[] szFileExt1;
        delete[] szFileExt2;
        return false;
    }
    if (!bJsonFlag)
    {
        if (!strstr(szFileExtList, szFileExt1))
        {
            delete[] szFileExt1;
            delete[] szFileExt2;
            return false;
        }
        else
        {
            delete[] szFileExt1;
            delete[] szFileExt2;
            return true;
        }
    }
    else
    {
        delete[] szFileExt1;
        delete[] szFileExt2;
        return true;
    }
}
// Special method - for Qt projects support:
bool DirComparer::IsFileExtValid(const std::string& sFileName, char* szFileExtList, bool bQt)
{
	if (!bQt)
		return IsFileExtValid(sFileName, szFileExtList);

	bool bExcludeThisFile = true;
	char szExclude1[] = "moc_";
	char szExclude2[] = "qrc_";
	char szExclude3[] = "ui_";
	if (IsFileQtValid(sFileName, szExclude1) &&
		IsFileQtValid(sFileName, szExclude2) &&
		IsFileQtValid(sFileName, szExclude3))
	{
        if (sFileName != "Makefile" && sFileName != "unsuccessfulbuild")
			bExcludeThisFile = false;
	}
	
	if (!bExcludeThisFile)
		return IsFileExtValid(sFileName, szFileExtList);

	return false;
}

bool DirComparer::IsFileExtValid(const std::string& sFileName, char* szFileExtList)
{
    int iLen = sFileName.length() + 1;

    char* szFileExt = new char[iLen];
    memset(szFileExt, 0, iLen);

    bool bIgnoreFlag = false;
    bool bFileExt = false;
    int iExtIndex = 0;
    for (int iIndex = 0; iIndex < iLen; iIndex++)
    {
        char chr = sFileName[iIndex];
		if (chr == '.')
			bFileExt = true;

        if (bFileExt)
        {
            szFileExt[iExtIndex++] = chr;

            if ( (!strcmp(szFileExt, ".git")) ||
                 (!strcmp(szFileExt, ".cache")) ||
                 (!strcmp(szFileExt, ".qmake")))
            {
                bIgnoreFlag = true;
                break;
            }
        }
    }

    if (bIgnoreFlag) // If it's ignored file-name (of folder-name)::
    {
        delete[] szFileExt;
        return false;
    }

    if (!strstr(szFileExtList, szFileExt))
    {
        delete[] szFileExt;        
        return false;
    }
    else
    {
        delete[] szFileExt;
        return true;
    }
}

bool DirComparer::IsFileExtValid(const std::string& sFilePath)
{
    int iLen = sFilePath.length() + 1;

    char* szFileExt = new char[iLen];
    memset(szFileExt, 0, iLen);

    bool bIgnoreFlag = false;
    bool bFileExt = false;
    int iExtIndex = 0;
    for (int iIndex = 0; iIndex < iLen; iIndex++)
    {
        char chr = sFilePath[iIndex];
        if (chr == '.')
            bFileExt = true;

        if (bFileExt)
        {
            szFileExt[iExtIndex++] = chr;

            if ( (!strcmp(szFileExt, ".git")) ||
                 (!strcmp(szFileExt, ".cache")) ||
                 (!strcmp(szFileExt, ".qmake")) ||
                 (!strcmp(szFileExt, ".vscode")) )
            {
                bIgnoreFlag = true;
                break;
            }
        }
    }

    delete[] szFileExt;
    if (bIgnoreFlag) // If it's ignored file-name (of folder-name):
        return false;

    return true;
}

std::string DirComparer::ConvertAbsPath(const std::string& sDirAbsPath, const std::string& sBase)
{
    int iLen = sDirAbsPath.length() + 1;
    char* szDirAbsPath = new char[iLen];
    memset(szDirAbsPath, 0, iLen);
    strcpy(szDirAbsPath, sDirAbsPath.c_str());

	int iBaseLen = sBase.length();
    char* szOut = new char[iLen];
    memset(szOut, 0, iLen);

	bool bEnableOut = false;
    int iOutIndex = 0;
    for (int iIndex = 0; iIndex < iLen; iIndex++)
    {
        char chr = szDirAbsPath[iIndex];
		char chr1 = 0;
		if (iIndex < iBaseLen)
			chr1 = sBase[iIndex];
		if (chr != chr1)
			bEnableOut = true;

        if (bEnableOut)
            szOut[iOutIndex++] = chr;
    }
    std::string sOut(szOut);
    delete[] szDirAbsPath;
    delete[] szOut;
    return sOut;
}

bool DirComparer::CompareExecute(int* pOutResult)
{
	FCOptions opt;
	opt.bCompareByNames = true;
	opt.bCompareBySizes = true;
    opt.bCompareByContents = true;

	SetOptions(&opt);

	int iOutFlag = 0; // Without errors
	bool bResult = true;
	for (DIRS_FILES_MAP::const_iterator cit = m_dfm.begin(); cit != m_dfm.end(); cit++)
	{
		DIRS_FILES_PAIR pair = *cit;
		if (pair.second == EXIST_IN_BOTH)
		{
			std::string sFilePathInBoth = pair.first;

			std::stringstream ss1;
			ss1 << m_strBase1 << sFilePathInBoth;
			std::string sPathFile1 = ss1.str();

			std::stringstream ss2;
			ss2 << m_strBase2 << sFilePathInBoth;
			std::string sPathFile2 = ss2.str();

			QFileInfo fi1(sPathFile1.c_str());
			QFileInfo fi2(sPathFile2.c_str());
			SetFileInfo(&fi1, &fi2);			

			bool bIsFilesEqual = FileCompareExecute();
			if (!bIsFilesEqual)
			{
				bResult = false;
				iOutFlag |= 1; // Files are NOT IDENTICAL
                spdlog::info("Files are NOT IDENTICAL: 1)'{0}'; 2)'{1}'", sPathFile1, sPathFile2);
			}
		}
		else
		{
			bResult = false;
			iOutFlag |= 2; // Lists of Files are NOT IDENTICAL
            spdlog::info("Contents of Folders are NOT IDENTICAL");
		}
	}
	*pOutResult = iOutFlag;
	return bResult;
}

DIR_COMPARE DirComparer::GetDirCompareMode() const
{
    return m_dcMode;
}
