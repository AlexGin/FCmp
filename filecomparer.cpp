#include "filecomparer.h"

FileComparer::FileComparer(QObject *parent)
    : m_pFi1(NULL), m_pFi2(NULL), m_pOpt(NULL), QObject(parent)
{
}

FileComparer::FileComparer(QFileInfo* pFi1, QFileInfo* pFi2, FCOptions* pOpt, QObject *parent)
    : m_pFi1(pFi1), m_pFi2(pFi2), m_pOpt(pOpt),  QObject(parent)
{
}

void FileComparer::SetFileInfo(QFileInfo* pFi1, QFileInfo* pFi2)
{
    m_pFi1 = pFi1;
    m_pFi2 = pFi2;
}

void FileComparer::SetOptions(FCOptions* pOpt)
{
    m_pOpt = pOpt;
}

bool FileComparer::LoadFile(QString sAbsFN, QByteArray& byteArr)
{
	// see:
	// https://forum.qt.io/topic/6467/how-to-read-binary-file/2 

	QFile file(sAbsFN);
	if (!file.open(QIODevice::ReadOnly))
		return false; // error occur

	QByteArray blob = file.readAll();
	byteArr = blob;

	return true;
}

bool FileComparer::FileCompareExecute()
{
    bool bFilesAreEqual = false;
    if (!m_pFi1 || !m_pFi2 || !m_pOpt)
        return bFilesAreEqual;

    if (m_pOpt->bCompareByNames && m_pOpt->bCompareBySizes)
    {
		QString sFN1 = m_pFi1->fileName();
		QString sFN2 = m_pFi2->fileName();
		qint64 nSize1 = m_pFi1->size();
		qint64 nSize2 = m_pFi2->size();
		if (!m_pOpt->bCompareByContents)
		{
			if ((sFN1 == sFN2) && (nSize1 == nSize2))
				bFilesAreEqual = true;

			return bFilesAreEqual;			
		}

		QString sAbs1 = m_pFi1->absoluteFilePath();
		QString sAbs2 = m_pFi2->absoluteFilePath();
		
		if ((sFN1 == sFN2) && (nSize1 == nSize2))
		{
			QByteArray ba1, ba2;
			if (LoadFile(sAbs1, ba1) && LoadFile(sAbs2, ba2))
				bFilesAreEqual = ba1 == ba2;				
		}
    }
	return bFilesAreEqual;
}
