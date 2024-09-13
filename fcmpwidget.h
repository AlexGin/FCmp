#ifndef FCMPWIDGET_H
#define FCMPWIDGET_H

#include <QWidget>
#include <QFileInfo>
#include "filecomparer.h"
#include "dircomparer.h"

class QTreeView;
class QMessageBox;
class QFileSystemModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class FCmpWidget;
}
QT_END_NAMESPACE

class FCmpWidget : public QWidget
{
    Q_OBJECT

public:
    FCmpWidget(QWidget *parent = nullptr);
    ~FCmpWidget();

    QFileSystemModel *m_model;
    void LogFilesVector(const QVector<QFileInfo>& vect, int n_id);
    bool RetrieveFileInfo(QTreeView* treeView, QVector<QFileInfo>& vect);
    void DirCompare(DirComparer* pDCmpr, QMessageBox& mb);
protected:
    void closeEvent(QCloseEvent* e) override;
private:
    Ui::FCmpWidget *ui;

private slots:
    void on_pushBtnCmp_clicked();
    void on_treeView1_clicked(const QModelIndex &index);
    void on_treeView2_clicked(const QModelIndex &index);
private slots:
    void ShowContextMenu(const QPoint &pos);
    void OnExit();
    void OnAbout();
};
#endif // FCMPWIDGET_H
