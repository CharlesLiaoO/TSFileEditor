﻿#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMap>

#include "DataModel/TranslateModel.h"

namespace Ui {
class MainWindow;
}

class XmlRW;
class ExcelRW;
class TranslateWorker;
class QFileInfo;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_tsLookBtn_clicked();
    void on_excelLookBtn_clicked();
    void on_generateBtn_clicked();
    void on_tsUpdateBtn_clicked();
    void on_translateBtn_clicked();
    void on_tsImportBtn_clicked();
    bool ImportFromTs(const QFileInfo &impFi, bool merge);
    void on_tsMergeFromOtherBtn_clicked();
    void on_tsSaveBtn_clicked();

    void onComboBoxChanged(int);
    void onReceiveMsg(const QString& msg, bool err=false);

    void on_tsDirLookBtn_clicked();
    void on_excelDirBtn_clicked();
    void on_generateBtn_2_clicked();
    void on_tsUpdateBtn_2_clicked();

    void on_pushButton_ExcelFileDirFollowTs_clicked();
    void on_pushButton_ExcelFileNameFollowTs_clicked();

private:
    Ui::MainWindow*         ui;

    QList<TranslateModel>  m_transList;

    QString                 m_toLanguage;

    XmlRW*                  m_pXmlWorker;
    ExcelRW*                m_pExcelWorker;
    TranslateWorker*        m_pTranslateWorker;

    QMap<QString, int>      m_tsColumnMap;

    void runExcel(const QString &xlsxFile);
    void readConfig();
    void saveConfig();
};

#endif // MAINWINDOW_H
