#include "fcmpwidget.h"
#include "ui_fcmpwidget.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <QDebug>
#include <QMenu>

#include "spdlog/spdlog.h"

FCmpWidget::FCmpWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FCmpWidget)
{
    ui->setupUi(this);

    ui->rBtnAll->setChecked(true);
    // see:
    // http://doc.qt.io/qt-5/qfilesystemmodel.html
    QFileSystemModel *model = new QFileSystemModel;
    model->setRootPath(QDir::currentPath());
    m_model = model;

    ui->treeView1->setModel(model);
    ui->treeView1->show();

    // see:
    // http://www.qtcentre.org/threads/2407-Change-column-width-in-a-QTreeWidget

    int iColInddex = 0;
    int iWidth = 180;
    ui->treeView1->header()->resizeSection(iColInddex, iWidth);

    ui->treeView2->setModel(model);
    ui->treeView2->show();

    ui->treeView2->header()->resizeSection(iColInddex, iWidth);

    connect(ui->pushBtnCompare, SIGNAL(clicked()), this, SLOT(on_pushBtnCmp_clicked()));
    connect(ui->pushBtnAbout, SIGNAL(clicked()), this, SLOT(OnAbout()));

    // see:
    // http://stackoverflow.com/questions/24254006/rightclick-event-in-qt-to-open-a-context-menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    // New syntax (Qt-5):
    connect(this, &FCmpWidget::customContextMenuRequested,
            this, &FCmpWidget::ShowContextMenu);
    // Old syntax Qt-4 (also supported in Qt-5):
    // connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
    //            this, SLOT(ShowContextMenu(const QPoint &))); // It's also OK!
}

FCmpWidget::~FCmpWidget()
{
    delete ui;
}

void FCmpWidget::closeEvent(QCloseEvent* e)
{
    Q_UNUSED(e);
    spdlog::info("Exit: 'FCmp' - Application exit");
}

// see (context-menu in Qt):
// 1) http://stackoverflow.com/questions/24254006/rightclick-event-in-qt-to-open-a-context-menu
// 2) http://www.setnode.com/blog/right-click-context-menus-with-qt/
void FCmpWidget::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu("Context menu", this);

    QAction action1("Compare", this);
    QAction action2("Exit", this);
    QAction action3("About...", this);

    connect(&action1, SIGNAL(triggered()), this, SLOT(on_pushBtnCmp_clicked()));
    connect(&action2, SIGNAL(triggered()), this, SLOT(OnExit()));
    connect(&action3, SIGNAL(triggered()), this, SLOT(OnAbout()));

    contextMenu.addAction(&action1);    
    contextMenu.addAction(&action2);
    contextMenu.addSeparator();
    contextMenu.addAction(&action3);

    contextMenu.exec(mapToGlobal(pos)); // пересчитать оконные координаты (координаты виджета) в глобальные
}
// вот что делает mapToGlobal:
// http://doc.qt.io/qt-4.8/qwidget.html#mapToGlobal

void FCmpWidget::OnExit()
{
    this->close();
}

void FCmpWidget::OnAbout()
{
    QString qstrVersion = "Version: " + qApp->applicationVersion();
    QString qstrGenerated = QString::asprintf("generated %s; %s", __DATE__, __TIME__);
    QString qstrAboutText = QString::asprintf("Application 'FCmp' - files comparing %s",
            qstrGenerated.toStdString().c_str());

    QMessageBox msgBoxAbout;
    msgBoxAbout.setText(qstrAboutText);
    msgBoxAbout.setInformativeText(qstrVersion);
    msgBoxAbout.setIcon(QMessageBox::Information);
    msgBoxAbout.setStandardButtons(QMessageBox::Ok);
    msgBoxAbout.setDefaultButton(QMessageBox::Ok);
    msgBoxAbout.exec();
}

bool FCmpWidget::RetrieveFileInfo(QTreeView* treeView, QVector<QFileInfo>& vect)
{
    // see:
    // http://stackoverflow.com/questions/1969484/how-to-get-details-about-the-selected-items-using-qtreeview
    bool bResult = false;
    QModelIndexList list = treeView->selectionModel()->selectedIndexes();
    QFileSystemModel *model = (QFileSystemModel*)treeView->model();
    int row = -1;
    foreach (QModelIndex index, list)
    {
        if (index.row()!=row && index.column()==0)
        {
            QFileInfo fileInfo = model->fileInfo(index);
            qDebug() << fileInfo.fileName() << '\n';
            row = index.row();
            vect.push_back(fileInfo);
            bResult = true;
        }
    }
    return bResult;
}


void FCmpWidget::on_pushBtnCmp_clicked()
{
    spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - start of comparing!");

    QVector<QFileInfo> vect1;
    RetrieveFileInfo(ui->treeView1, vect1);

    QVector<QFileInfo> vect2;
    RetrieveFileInfo(ui->treeView2, vect2);

    if ((vect1.size() < 1) || (vect2.size() < 1))
    {
        QMessageBox msgBoxInvalid;
        QString qstrErrorText = "Invalid files/folders selected! Cancel of Comparing!";
        msgBoxInvalid.setText("File Comparation!");
        msgBoxInvalid.setInformativeText(qstrErrorText);
        msgBoxInvalid.setIcon(QMessageBox::Warning);
        msgBoxInvalid.setStandardButtons(QMessageBox::Ok);
        msgBoxInvalid.setDefaultButton(QMessageBox::Ok);
        msgBoxInvalid.exec();
        spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Invalid files/folders selected! Cancel of Comparing!");
        return;
    }

    bool bValidCompare = true;
    bool bFoldersComparing = false;
    QString qstrCompareText;
    if (vect1.size() == 1 && vect2.size() == 1)
    {
        if (vect1[0].isFile() && vect2[0].isFile())
        {
            qstrCompareText.append("Files are selected to comparing.");
            spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Files are selected to comparing.");
        }
        else if (vect1[0].isDir() && vect2[0].isDir())
        {
            bFoldersComparing = true;
            qstrCompareText.append("Folders are selected to comparing.");
            spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Folders are selected to comparing.");
        }
        else
        {
            qstrCompareText.append("Missmatch of selected types of elements!");
            spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Missmatch of selected types of elements!");
            bValidCompare = false;
        }
    }
    else if (vect1.size() > 1 || vect2.size() > 1)
    {
        bValidCompare = false;
        qstrCompareText.append("Multiply selecting - is not possible!");
        spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Multiply selecting - is not possible!");
    }
    QString qstr1 = "1: " + vect1[0].absoluteFilePath();
    ui->label_2->setText(qstr1);

    QString qstr2 = "2: " + vect2[0].absoluteFilePath();
    ui->label_1->setText(qstr2);

    QMessageBox msgBox;

    msgBox.setText("File Comparation!");
    msgBox.setInformativeText("Mode of Comparing");
    msgBox.setIcon(!bValidCompare ? QMessageBox::Warning : QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);

    LogFilesVector(vect1, 1);
    LogFilesVector(vect2, 2);

    if (bValidCompare)
    {
        spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Folders Comparing");
        if (bFoldersComparing)
        {
            if (ui->rBtnAll->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_ALL, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnCpp->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_CCPP, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnWMfc->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_WMFC, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnCmake->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_CMCD, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnQt->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_QT, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnCs->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_CS, this);
                DirCompare(pDCmpr, msgBox);
            }
            else if (ui->rBtnJavaA->isChecked())
            {
                DirComparer* pDCmpr = new DirComparer(&vect1[0], &vect2[0], DIR_COMPARE::DIR_COMPARE_JAVA, this);
                DirCompare(pDCmpr, msgBox);
            }
        }
        else // if Files comparing:
        {
            spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - Files Comparing");
            FCOptions opt;
            opt.bCompareByNames = true;
            opt.bCompareBySizes = true;

            opt.bCompareByContents = true; // false;

            FileComparer* pFCmpr = new FileComparer(&vect1[0], &vect2[0], &opt, this);
            bool bIsFilesEqual = pFCmpr->FileCompareExecute();

            if (bIsFilesEqual)
            {
                msgBox.setText("File Comparation - files are IDENTICAL.");
                spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - files are IDENTICAL.");
            }
            else
            {
                msgBox.setText("File Comparation - files are NOT identical.");
                spdlog::info("FCmpWidget::on_pushBtnCmp_clicked - files are NOT identical.");
            }
        }
    }
    msgBox.exec();
}

void FCmpWidget::DirCompare(DirComparer* pDCmpr, QMessageBox& mb)
{
    pDCmpr->FillDirsMap();
    int iResult = 0;
    if (!pDCmpr->CompareExecute(&iResult))
    {
        switch (iResult)
        {
        case 1:
            mb.setText("File Comparation - files in folders are NOT identical.");
            break;
        case 2:
            mb.setText("File Comparation - LISTS of files in folders are NOT identical.");
            break;

        case 3:
            mb.setText("File Comparation - FILES and LISTS of files in folders are NOT identical.");
            break;
        }
    }
    else
        mb.setText("File Comparation - files in folders are IDENTICAL.");

    DIR_COMPARE dcMode = pDCmpr->GetDirCompareMode();
    switch (dcMode)
    {
        case DIR_COMPARE::DIR_COMPARE_ALL:
            mb.setInformativeText("Comparing by ALL files.");
            break;

        case DIR_COMPARE::DIR_COMPARE_CCPP:
            mb.setInformativeText("Comparing by files С/С++.");
            break;

        case DIR_COMPARE::DIR_COMPARE_WMFC:
            mb.setInformativeText("Comparing by files WinApi/MFC.");
            break;

        case DIR_COMPARE::DIR_COMPARE_CMCD:
            mb.setInformativeText("Comparing by files CMake/VSCode.");
            break;

        case DIR_COMPARE::DIR_COMPARE_QT:
            mb.setInformativeText("Comparing by files Qt.");
            break;

        case DIR_COMPARE::DIR_COMPARE_CS:
            mb.setInformativeText("Comparing by files C# (.NET).");
            break;

        case DIR_COMPARE::DIR_COMPARE_JAVA:
            mb.setInformativeText("Comparing by files JAVA (Android).");
            break;
    }
}

void FCmpWidget::on_treeView1_clicked(const QModelIndex &index)
{
    // see:
    // http://stackoverflow.com/questions/8396943/how-to-get-the-full-filepath-of-a-selected-file-via-qfilesystemmodel
    QString str = m_model->filePath(index);
    ui->label_2->setText(str);
}

void FCmpWidget::on_treeView2_clicked(const QModelIndex &index)
{
    // QString str = index.data(Qt::DisplayRole).toString();// This call - do not get full path (only "list of tree")!
    QString str = m_model->filePath(index);
    ui->label_1->setText(str);
}

void FCmpWidget::LogFilesVector(const QVector<QFileInfo>& vect, int n_id)
{
    for(const QFileInfo& fi : vect)
    {
        if (fi.isDir())
        {
            QString qstrPath = fi.path();
            QString qstrFileName = fi.fileName();
            spdlog::info("PANEL {0}: DIR: {1}/{2}", n_id, qstrPath.toStdString(), qstrFileName.toStdString());
        }
        if (fi.isFile())
        {
            QString qstrPath = fi.path();
            QString qstrFileName = fi.fileName();
            spdlog::info("PANEL {0}: FILE: {1}/{2}", n_id, qstrPath.toStdString(), qstrFileName.toStdString());
        }
    }
}
