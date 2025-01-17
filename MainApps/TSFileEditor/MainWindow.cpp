﻿#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "XmlRW.h"
#include "ExcelRW.h"
#include "TranslateWorker.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QListView>
#include <QSslSocket>

#include <QProcess>
bool LConvert(const QStringList &ifs, const QString &of) {
    //合并ts文件，不能剔除新的非空翻译。即 lconvert -i f1 f2 -o f3, f3总是保留f2的翻译，即使f2的翻译为空
    QProcess tsMergePrc;
    tsMergePrc.setProgram(("lconvert.exe"));
    QStringList args = ifs;
    args.prepend("-i");
    args.append("-o");
    args.append(of);
    tsMergePrc.setArguments(args);
    qDebug()<< tsMergePrc.program()<< tsMergePrc.arguments();
    tsMergePrc.start();

    bool ret = tsMergePrc.waitForFinished();
    QProcess::ExitStatus es = tsMergePrc.exitStatus();
    int ec = tsMergePrc.exitCode();
    return ret && es == QProcess::NormalExit && ec == 0;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->setStyleSheet("color:blue");  //qt5.12.12默认为红色，这里修改

    ui->tsSaveBtn->hide();  // 自动保存，不用用户点击

    m_toLanguage = "en";
    m_pXmlWorker = new XmlRW(this);
    m_pExcelWorker = new ExcelRW(2, 1, 3, this);
    m_pTranslateWorker = new TranslateWorker(m_transList, this);

    ui->youdaoTipLabel->setVisible(false);
    ui->comboBox->setView(new QListView());
    ui->comboBox->addItem("英文", "en");
    ui->comboBox->addItem("中文", "zh-CHS");
    ui->comboBox->addItem("日文", "ja");
    ui->comboBox->addItem("韩文", "ko");
    ui->comboBox->addItem("法文", "fr");
    ui->comboBox->addItem("俄文", "ru");
    ui->comboBox->addItem("葡萄牙文", "pt");
    ui->comboBox->addItem("西班牙文", "es");
    ui->comboBox->addItem("其他", "other");

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onComboBoxChanged);
    connect(m_pExcelWorker, &ExcelRW::error, this, [this](const QString &msg){
        onReceiveMsg(msg, true);
    });
    connect(m_pTranslateWorker, &TranslateWorker::error, this, [this](const QString &msg){
        onReceiveMsg(msg, true);
    });

    readConfig();
}

MainWindow::~MainWindow()
{
    saveConfig();

    delete ui;
}

QString LineEditBrowsePath(QLineEdit *edit)
{
    QString path = edit->text();
    if (path.isEmpty())
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return path;
}

void MainWindow::on_tsLookBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("select .ts file"), LineEditBrowsePath(ui->tsPathEdit), "Files (*.ts)");

    if(fileName.isEmpty()){
        return;
    }

    ui->tsPathEdit->setText(fileName);
    on_tsImportBtn_clicked();
}

void MainWindow::on_excelLookBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("select excel file"), LineEditBrowsePath(ui->excelPathEdit), "Files (*.xlsx)");

    if(fileName.isEmpty()){
        return;
    }
    else{
        QFileInfo info(fileName);
        if ("xlsx" != info.suffix()){
            onReceiveMsg("File type is not supported", true);
            return;
        }
    }

    ui->excelPathEdit->setText(fileName);
}

void MainWindow::on_generateBtn_clicked()
{
    bool re;

    m_pExcelWorker->SetTransColumn(ui->transSpinBox->value());

    //generate excel file
    QString fileName = ui->excelPathEdit->text();
    if(fileName.isEmpty()) {
        const QString documentLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        QString saveName = documentLocation + "/untitled.xlsx";
        fileName = QFileDialog::getSaveFileName(this, "excel file path", saveName, "Files (*.xlsx)");

        if (fileName.isEmpty())
        {
            return;
        }
        else{
            ui->excelPathEdit->setText(fileName);
        }
    }

    re = m_pExcelWorker->ExportToXlsx(m_transList, fileName);
    if(re) {
        onReceiveMsg("Export excel file success");
        ui->youdaoTipLabel->setVisible(false);
        runExcel(fileName);
    } else {
        onReceiveMsg("Export excel file failed", true);
    }
}

void MainWindow::on_tsUpdateBtn_clicked()
{
    bool re;

    m_pExcelWorker->SetTransColumn(ui->transSpinBox->value());

    //import excel file
    if(ui->excelPathEdit->text().isEmpty()) {
        on_excelLookBtn_clicked();
    }

    re = m_pExcelWorker->ImportFromXlsx(m_transList, ui->excelPathEdit->text());
    if(re) {
        onReceiveMsg("Import excel file success");
        ui->youdaoTipLabel->setVisible(false);
    } else {
        onReceiveMsg("Import excel file failed", true);
    }

    //update ts file
    if(ui->tsPathEdit->text().isEmpty()) {
        on_tsLookBtn_clicked();
    }

    QString curFile = ui->tsPathEdit->text();
    re = m_pXmlWorker->ExportToTS(m_transList, curFile);

    if (re)
        re = LConvert({curFile}, curFile);  // lconvert -i f1 -o f1  整成qt工具的格式

    if(re) {
        onReceiveMsg("Update .ts file success");
    } else {
        onReceiveMsg("Update .ts file failed", true);
    }

}

void MainWindow::on_translateBtn_clicked()
{
    bool re;

    m_pExcelWorker->SetTransColumn(ui->transSpinBox->value());

    //import excel file
    if(ui->excelPathEdit->text().isEmpty()) {
        on_excelLookBtn_clicked();
    }

    re = m_pExcelWorker->ImportFromXlsx(m_transList, ui->excelPathEdit->text());
    if(re) {
        onReceiveMsg("Import excel file success");
    }
    else {
        onReceiveMsg("Import excel file failed", true);
    }

//    m_pTranslateWorker->YoudaoTranslate("你好", "auto", m_toLanguage);

    //translate excel file
    m_pTranslateWorker->SetIdKey(ui->youdaoAppIdlineEdit->text(), ui->youdaoKeylineEdit->text());
    re = m_pTranslateWorker->YoudaoTranslate("auto", m_toLanguage);
    if(re) {
        onReceiveMsg("Translate excel file success");
        ui->youdaoTipLabel->setVisible(true);
    }
    else {
        onReceiveMsg("Translate excel file failed", true);
    }
}

void MainWindow::onComboBoxChanged(int)
{
    QString langCode = ui->comboBox->currentData().toString();

    if ("other" == langCode) {
        m_toLanguage = ui->otherLineEdit->text();
    } else {
        m_toLanguage = langCode;
    }

}



void MainWindow::on_tsImportBtn_clicked()
{
    //import .ts file
    if(ui->tsPathEdit->text().isEmpty()) {
        on_tsLookBtn_clicked();
    }

    QFileInfo info(ui->tsPathEdit->text());

    ImportFromTs(info, false);
}

bool MainWindow::ImportFromTs(const QFileInfo &impFi, bool merge)
{
    if (!impFi.isFile() || "ts" != impFi.suffix()) {
        onReceiveMsg("File type is not supported", true);
        return false;
    }

    m_transList.clear();
    //更新到 m_translateMap，如果是合并，则剔除新的非空翻译
    bool re = m_pXmlWorker->ImportFromTS(m_transList, impFi.filePath(), merge);

    if (merge) {
        QString curFile = ui->tsPathEdit->text();
        re = LConvert({impFi.filePath(), curFile}, curFile);
        if (re)
            on_tsSaveBtn_clicked();  //按照当前ts文件和m_translateMap更新ts文件
    }

    if (re) {
        onReceiveMsg("Import .ts file success");
    } else {
        onReceiveMsg("Import .ts file failed", true);
    }

    return true;
}

void MainWindow::on_tsMergeFromOtherBtn_clicked()
{
    QString dir = QFileInfo(LineEditBrowsePath(ui->tsPathEdit)).path();
    QString path = QFileDialog::getOpenFileName(this, "", dir, "*.ts");
    if (path.isEmpty())
        return;

    ImportFromTs(QFileInfo(path), true);
}

void MainWindow::on_tsSaveBtn_clicked()
{
    QString curFile = ui->tsPathEdit->text();
    bool re = m_pXmlWorker->ExportToTS(m_transList, curFile);

    if (re)
        re = LConvert({curFile}, curFile);  // lconvert -i f1 -o f1  整成qt工具的格式

    if(re) {
        onReceiveMsg(tr("Update .ts file success"));
    } else {
        onReceiveMsg(tr("Update .ts file failed"), true);
    }
}

void MainWindow::onReceiveMsg(const QString &msg, bool err)
{
    static bool errBf = false;
    if (errBf != err) {
        if (err)
            ui->statusBar->setStyleSheet("color:red");
        else
            ui->statusBar->setStyleSheet("color:blue");
        errBf = err;
    }

    ui->statusBar->showMessage(msg, err ? 5000 : 3000);
}

void MainWindow::on_tsDirLookBtn_clicked()
{
    QString dirName = QFileDialog::getExistingDirectory(this, tr("select .ts dir"), LineEditBrowsePath(ui->tsDirEdit));

    if(dirName.isEmpty()){
        return;
    }

    ui->tsDirEdit->setText(dirName);
}

void MainWindow::on_excelDirBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("select excel file"), LineEditBrowsePath(ui->excelDirEdit), "Files (*.xlsx)");

    if(fileName.isEmpty()){
        return;
    }
    else{
        QFileInfo info(fileName);
        if ("xlsx" != info.suffix()){
            onReceiveMsg("File type is not supported", true);
            return;
        }
    }

    ui->excelDirEdit->setText(fileName);
}

void MainWindow::on_generateBtn_2_clicked()
{
    bool re;

    QFileInfo tsDirinfo(ui->tsDirEdit->text());
    if (!tsDirinfo.isDir()){
        onReceiveMsg("Ts dir is empty", true);
        return;
    }

    QFileInfo excelinfo(ui->excelDirEdit->text());
    if (!excelinfo.exists()){
        onReceiveMsg("Excel file is empty", true);
        return;
    }
    qDebug() << excelinfo.filePath() << excelinfo.absoluteDir().path();

    QStringList filters;
    filters << QString("*.ts");
    QDir tsdir(ui->tsDirEdit->text());
    tsdir.setFilter(QDir::Files | QDir::NoSymLinks);
    tsdir.setNameFilters(filters);

    if (tsdir.count() <= 0) {
        onReceiveMsg("Ts dir ts file is 0", true);
        return;
    }

    for (QFileInfo info : tsdir.entryInfoList()) {
        //import ts file
        m_transList.clear();
        re = m_pXmlWorker->ImportFromTS(m_transList, info.absoluteFilePath(), false);

        if(re) {
            onReceiveMsg("Import " + info.fileName() + " success");
        } else {
            onReceiveMsg("Import " + info.fileName() + " failed", true);
        }

        //generate excel file
        m_pExcelWorker->SetTransColumn(ui->transSpinBox->value());
        QString excelFileName = excelinfo.absoluteDir().path() + "/" + info.baseName() + ".xlsx";
        re = m_pExcelWorker->ExportToXlsx(m_transList, excelFileName);
        if(re) {
            onReceiveMsg("Export " + excelFileName + " success");
            ui->youdaoTipLabel->setVisible(false);
        } else {
            onReceiveMsg("Export " + excelFileName + " failed", true);
        }
    }
}

void MainWindow::on_tsUpdateBtn_2_clicked()
{
    bool re;

    QFileInfo tsDirinfo(ui->tsDirEdit->text());
    if (!tsDirinfo.isDir()){
        onReceiveMsg("Ts dir is empty", true);
        return;
    }

    QFileInfo excelDirinfo(ui->excelDirEdit->text());
    if (!excelDirinfo.exists()){
        onReceiveMsg("Excel path is empty", true);
        return;
    }

    QStringList filters;
    filters << QString("*.ts");
    QDir tsdir(ui->tsDirEdit->text());
    tsdir.setFilter(QDir::Files | QDir::NoSymLinks);
    tsdir.setNameFilters(filters);

    if (tsdir.count() <= 0) {
        onReceiveMsg("Ts dir ts file is 0", true);
        return;
    }

    for (QFileInfo info : tsdir.entryInfoList()) {
        if (!m_tsColumnMap.contains(info.fileName())) {
            continue;
        }

        //import ts file
        m_transList.clear();
        re = m_pXmlWorker->ImportFromTS(m_transList, info.absoluteFilePath(), false);

        if(!re) {
            continue;
        }

        m_pExcelWorker->SetTransColumn(m_tsColumnMap[info.fileName()]);
        re = m_pExcelWorker->ImportFromXlsx(m_transList, ui->excelDirEdit->text());
        if(!re) {
            continue;
        }

        re = m_pXmlWorker->ExportToTS(m_transList, info.absoluteFilePath());

        if(!re) {
            continue;
        }
    }

    onReceiveMsg("All ts file update finish");
}

void MainWindow::readConfig()
{
    QString configPath = QApplication::applicationDirPath();
    QSettings settings(configPath + "/config.ini", QSettings::IniFormat);
    settings.beginGroup("path");
    ui->tsPathEdit->setText(settings.value("tsPath").toString());
    ui->tsDirEdit->setText(settings.value("tsDir").toString());

    ui->excelPathEdit->setText(settings.value("excelPath").toString());
    ui->excelDirEdit->setText(settings.value("excelDir").toString());
    settings.endGroup();

    m_tsColumnMap.clear();
    int size = settings.beginReadArray("languages");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        int column = settings.value("column").toInt();
        QString fileName = settings.value("tsFile").toString();
        m_tsColumnMap[fileName] = column;
    }
    settings.endArray();
}

void MainWindow::saveConfig()
{
    QString configPath = QApplication::applicationDirPath();
    QSettings settings(configPath + "/config.ini", QSettings::IniFormat);
    settings.beginGroup("path");
    settings.setValue("tsPath", ui->tsPathEdit->text());
    settings.setValue("tsDir", ui->tsDirEdit->text());
    settings.setValue("excelPath", ui->excelPathEdit->text());
    settings.setValue("excelDir", ui->excelDirEdit->text());
    settings.endGroup();

//    m_tsColumnMap.clear();
//    m_tsColumnMap["i18n_de.ts"] = 3;
//    m_tsColumnMap["i18n_zh_cn.ts"] = 4;
//    settings.beginWriteArray("languages");
//    int index = 0;
//    for (auto i = m_tsColumnMap.constBegin(); i != m_tsColumnMap.constEnd(); i++) {
//        settings.setArrayIndex(index);
//        index++;
//        settings.setValue("column", i.key());
//        settings.setValue("tsFile", i.value());
//    }
//    settings.endArray();
}

void MainWindow::on_pushButton_ExcelFileDirFollowTs_clicked()
{
    QFileInfo tfi(ui->tsPathEdit->text());
    QFileInfo efi(ui->excelPathEdit->text());

    QString newExcelPath = QString("%1/%2.xlsx");
    if (efi.path().isEmpty())
        newExcelPath = newExcelPath.arg(tfi.path(), tfi.baseName());
    else
        newExcelPath = newExcelPath.arg(tfi.path(), efi.baseName());

    ui->excelPathEdit->setText(newExcelPath);
}

void MainWindow::on_pushButton_ExcelFileNameFollowTs_clicked()
{
    QFileInfo tfi(ui->tsPathEdit->text());
    QFileInfo efi(ui->excelPathEdit->text());

    QString newExcelPath = QString("%1/%2.xlsx");
    if (efi.path().isEmpty())
        newExcelPath = newExcelPath.arg(tfi.path(), tfi.baseName());
    else
        newExcelPath = newExcelPath.arg(efi.path(), tfi.baseName());

    ui->excelPathEdit->setText(newExcelPath);
}


#ifdef Q_OS_WIN
#include <Windows.h>
#include <shellapi.h>
bool ShellExecute(const QString &path, int showMode)
{
    uint ret = (uint)ShellExecuteW(0, 0, (wchar_t *)path.utf16(), 0, 0, showMode);  //console app need to add 'win32: LIBS += -lshell32' in pro file
    if (ret > 32) {  // ShellExecute return > 32 if ok
        return true;
    } else
        return false;
}
#endif

void MainWindow::runExcel(const QString &xlsxFile)
{
#ifdef Q_OS_WIN
    ShellExecute(xlsxFile, SW_MAXIMIZE);
#endif
}
