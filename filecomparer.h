#ifndef FILECOMPARER_H
#define FILECOMPARER_H

// STL:
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <iomanip> // std::setw support

#include <QObject>
#include <QFileInfo>

// using namespace std;

struct FCOptions
{
    bool bCompareByNames;
    bool bCompareBySizes;
    bool bCompareByContents;
};

class FileComparer : public QObject
{
    Q_OBJECT
private:
    QFileInfo* m_pFi1;
    QFileInfo* m_pFi2;
    FCOptions* m_pOpt;
public:
    explicit FileComparer(QObject *parent = 0);
    FileComparer(QFileInfo* pFi1, QFileInfo* pFi2, FCOptions* pOpt, QObject *parent = 0);
    void SetFileInfo(QFileInfo* pFi1, QFileInfo* pFi2);
    void SetOptions(FCOptions* pOpt);

    bool FileCompareExecute();

protected:
	bool LoadFile(QString sAbsFN, QByteArray& byteArr);

signals:

public slots:
};

#endif // FILECOMPARER_H
