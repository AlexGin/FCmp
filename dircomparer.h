#ifndef DIRCOMPARER_H
#define DIRCOMPARER_H

#include <QObject>
#include <QDir>
#include <QDirIterator>
#include "filecomparer.h"

// This is constants (using for the Compare-mode):
enum class DIR_COMPARE
{
    DIR_COMPARE_ALL  = 0, // comparing all files, exist in the selected directory
    DIR_COMPARE_CCPP,     // comparing only *.c, *.cc, *.cpp, *.h, *.hpp (pure C/C++ projects)
    DIR_COMPARE_WMFC,     // comparing only *.cpp, *.h, *.rc, *.idl (WinAPI & MFC)
    DIR_COMPARE_CMCD,     // comparing only *.c, *.cc, *.cpp, *.h, *.hpp (C/C++ CMake/VSCode)
    DIR_COMPARE_QT,       // comparing only *.cpp, *.h, *.ui, *.qrc, *.pro (Qt projects)
    DIR_COMPARE_CS,       // comparing only *.cs, *.resx, *.xaml files (C# projects)
    DIR_COMPARE_JAVA      // comparing only *.java and *.xml files (JAVA-Android)
};
// Exist-flags:
constexpr int EXIST_IN_FIRST  = 1; // File exist in the first window
constexpr int EXIST_IN_SECOND = 2; // File exist in the second window
constexpr int EXIST_IN_BOTH   = 3; // File exist in all windows

using STRING_VECTOR = std::vector<std::string>; // Vector with file-names
using DIRS_FILES_MAP = std::map<std::string, int>; // The path-name as a key; and exist-flag as a value
using DIRS_FILES_PAIR = std::pair<std::string, int>;

class DirComparer : public FileComparer
{
    Q_OBJECT
protected:
    DIR_COMPARE m_dcMode;
    QDir* m_pDir1;
    QDir* m_pDir2;
    // Vectors with full path (for output compare-result widget):
    STRING_VECTOR m_vectFileNames1;
	STRING_VECTOR m_vectFileNames2;
    // Map with path names and exist-flags (for comparing files):
    DIRS_FILES_MAP m_dfm;
    // Base pathes for particular window (for example: "C:/"):
    std::string m_strBase1;
    std::string m_strBase2;

public:
    explicit DirComparer(QObject *parent = 0);
    DirComparer(QFileInfo* pDirInfo1, QFileInfo* pDirInfo2, DIR_COMPARE dc, QObject *parent = 0);
    DirComparer(QDir* pDir1, QDir* pDir2, DIR_COMPARE dc, QObject *parent = 0);
    void SetDir12(QFileInfo* pDirInfo1, QFileInfo* pDirInfo2);
    void SetDirCompareMode(DIR_COMPARE dc);
    DIR_COMPARE GetDirCompareMode() const;
	void FillDirsMap();
    std::string PrepareStringMode(DIR_COMPARE dc);
private:
    bool PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst,
                        std::string& strBase, char* szFileExtList = nullptr);
    bool PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst,
                        std::string& strBase, char* szFileExtList, bool bQt);
    bool PrepareDirsMap(QDir* pDir, DIRS_FILES_MAP& dfm, STRING_VECTOR& vectFileNames, bool bFirst,
                        std::string& strBase, char* szFileExtList, const char* szCMakeLists);
	// Param. bQt - for the Qt projects
    bool IsFileQtValid(const std::string& sFileName, char* szExcludeList);
    bool IsFileExtValid(const std::string& sFileName, char* szFileExtList, const char* szCmakeLists); // For the CMake projects
    bool IsFileExtValid(const std::string& sFileName, char* szFileExtList, bool bQt); // For the Qt projects
    bool IsFileExtValid(const std::string& sFileName, char* szFileExtList);
    bool IsFileExtValid(const std::string& sFilePath);
public:
	bool CompareExecute(int* pOutResult = NULL);

protected:
    std::string ConvertAbsPath(const std::string& sDirAbsPath, const std::string& sBase);
	void PrepareDfm(DIRS_FILES_MAP& dfm, std::string& sFilePath, std::string& sBase, bool bFirst);
    char* FillFilterString(DIR_COMPARE dc);

signals:

public slots:
};

#endif // DIRCOMPARER_H
