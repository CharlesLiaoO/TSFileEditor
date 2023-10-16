#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "XmlRW.h"
#include "ExcelRW.h"
#include "TranslateWorker.h"

#include <QStandardPaths>
#include <QFileDialog>
#include <QListView>
#include <QSslSocket>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusBar->setStyleSheet("color:blue");  //qt5.12.12默认为红色，这里修改

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

    re = m_pXmlWorker->ExportToTS(m_transList, ui->tsPathEdit->text());

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
    bool re;

    //import .ts file
    if(ui->tsPathEdit->text().isEmpty()) {
        on_tsLookBtn_clicked();
    }

    QFileInfo info(ui->tsPathEdit->text());
    if (!info.isFile() || "ts" != info.suffix()){
        onReceiveMsg("File type is not supported", true);
        return;
    }

    m_transList.clear();
    bool mergeTs = ui->checkBox_MergeTs->isChecked();
    re = m_pXmlWorker->ImportFromTS(m_transList, ui->tsPathEdit->text(), mergeTs);
    if(re) {
        onReceiveMsg("Import .ts file success");
    } else {
        onReceiveMsg("Import .ts file failed", true);
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
    ui->lineEdit_ExcelExePath->setText(settings.value("ExcelExePath").toString());

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

// 不支持excel的快捷方式去调用，所以不做浏览定位excel位置
void MainWindow::BrowseExcelExePath()
{
    QString configPath = QApplication::applicationDirPath();
    QSettings settings(configPath + "/config.ini", QSettings::IniFormat);

    QString excelPath = settings.value("path/ExcelExePath").toString();
    if (excelPath.isEmpty())
        excelPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
    excelPath = QFileDialog::getOpenFileName(this, tr("Select Excel Exe Path"), excelPath, "", nullptr,  QFileDialog::DontResolveSymlinks);
    if (excelPath.isEmpty())
        return;

    settings.setValue("path/ExcelExePath", excelPath);
}

void MainWindow::runExcel(const QString &xlsxFile)
{
    QString excelPath = ui->lineEdit_ExcelExePath->text();
    //if (excelPath.isEmpty() || !QFileInfo::exists(excelPath))
        //return;

    if (!QProcess::startDetached(excelPath, {xlsxFile}))  //不支持excel的快捷方式去调用，所以不做浏览定位excel位置
        return;

    QString configPath = QApplication::applicationDirPath();
    QSettings settings(configPath + "/config.ini", QSettings::IniFormat);
    settings.setValue("path/ExcelExePath", excelPath);
}
