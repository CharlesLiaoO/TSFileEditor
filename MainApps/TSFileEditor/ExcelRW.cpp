#include "ExcelRW.h"
#include <QRegExpValidator>
#include <QFileInfo>
#include <QDebug>

ExcelRW::ExcelRW(int keyColumn, int sourceColumn, int transColumn, QObject *parent) : QObject(parent)
{
    m_TotalCount = 0;
    m_KeyColumn = keyColumn;
    m_SourceColumn = sourceColumn;
    m_TransColumn = transColumn;
}

bool ExcelRW::ImportFromXlsx(QList<TranslateModel> &list, QString strPath)
{
    bool bSuccess = true;
//    int nErrLine = 1;
    list.clear();

    QString strKey, strSource, strTranslate;
    QXlsx::CellRange cellRange;

    QXlsx::Document* m_pDoc = new QXlsx::Document(strPath);
    if(nullptr == m_pDoc)
    {
        return bSuccess;
    }
    if(m_pDoc->sheetNames().isEmpty())
    {
        return bSuccess;
    }
    m_pDoc->selectSheet(m_pDoc->sheetNames().first());
    cellRange = m_pDoc->currentWorksheet()->dimension();

    do
    {
        if(1 == cellRange.lastRow() && (nullptr == m_pDoc->cellAt(cellRange.lastRow(), cellRange.lastColumn())))
        {
            bSuccess = true;
            break;
        }

        for(int i = 2; i<=cellRange.lastRow(); i++)
        {
            m_TotalCount++;

            if(m_pDoc->cellAt(i, m_KeyColumn) == nullptr)
            {
                strKey = "";
            }else
            {
                strKey = m_pDoc->cellAt(i, m_KeyColumn)->value().toString();
            }

            if(m_pDoc->cellAt(i, m_SourceColumn) == nullptr)
            {
                strSource = "";
            }else
            {
                strSource = m_pDoc->cellAt(i, m_SourceColumn)->value().toString();
//                qDebug()<< "numberFormat"<< m_pDoc->cellAt(i, m_SourceColumn)->format().numberFormat();
//                qDebug()<< "numberFormatIndex"<< m_pDoc->cellAt(i, m_SourceColumn)->format().numberFormatIndex();
            }

            if(m_pDoc->cellAt(i, m_TransColumn) == nullptr)
            {
                strTranslate = "";
            }else
            {
                strTranslate = m_pDoc->cellAt(i, m_TransColumn)->value().toString();
            }

//            nErrLine = i;

//            if(strSite.isEmpty())
//            {
//                addError(EXCEL_EMPTY_ERROR, nErrLine);
//                bSuccess = false;
//                continue;
//            }


            TranslateModel model;
            model.SetKey(strKey, true);
//            model.SetSource(strSource, true);
            model.SetComment(strSource);
            model.SetTranslate(strTranslate, true);
            list.append(model);
        }
    }while(0);

    delete m_pDoc;
    return bSuccess;
}

bool ExcelRW::ExportToXlsx(QList<TranslateModel>& list, QString strPath)
{
    if(strPath.isEmpty()) {
        emit error("Export path cannot be empty");
        return false;
    }

    if (list.count() <= 0)
    {
        emit error("*.ts file is empty");
        return false;
    }

    QString strHeaderkey = tr("源");
    QString strHeaderSource = tr("注释");
    QString strHeaderTranslate = tr("翻译");

    QXlsx::Document xlsx;
    xlsx.addSheet("Sheet1");
    xlsx.write(1, m_KeyColumn, QVariant(strHeaderkey));
    xlsx.write(1, m_SourceColumn, QVariant(strHeaderSource));
    xlsx.write(1, m_TransColumn, QVariant(strHeaderTranslate));
    QXlsx::Format fmH;
    fmH.setFontBold(true);
    xlsx.setRowFormat(1, fmH);

    QXlsx::Format fmC;
    fmC.setTextWarp(true);
//    fmC.setNumberFormat("Text");  //变成了自定义格式
    fmC.setNumberFormatIndex(49);  // text
//    fmC.setFontName("Arial Unicode MS");
    for(int i=0; i < list.count(); i++)
    {
        for(int j=1; j<=3; j++){
            xlsx.write(i+2, m_KeyColumn, QVariant(list[i].GetKey(true)));
//            xlsx.write(i+2, m_SourceColumn, QVariant(list[i].GetSource(true)));
            xlsx.write(i+2, m_SourceColumn, QVariant(list[i].GetComment()));
            xlsx.write(i+2, m_TransColumn, QVariant(list[i].GetTranslate(true)));

            xlsx.setRowFormat(i+2, fmC);
            xlsx.setRowHeight(i+2, 20);
        }
    }

    int commentWidth = 30;
    int langWidth = 60;
    xlsx.setColumnWidth(m_KeyColumn, langWidth);
    xlsx.setColumnWidth(m_SourceColumn, commentWidth);
    xlsx.setColumnWidth(m_TransColumn, langWidth);

    return xlsx.saveAs(strPath);
}

void ExcelRW::SetTransColumn(int column)
{
    m_TransColumn = column;
}

bool ExcelRW::checkAccountName(QString string)
{
    if(string.isEmpty())
    {
        return false;
    }

    QRegExp regp("^[a-zA-Z_0-9]+$");
    QRegExpValidator validator(regp,nullptr);
    int pos = 0;
    if(QValidator::Acceptable != validator.validate(string, pos)){
        return false;
    }

    return true;
}

bool ExcelRW::checkPassword(QString string)
{
    if(string.isEmpty())
    {
        return false;
    }

    QRegExp regp("^[\\x21-\\x7E]+$");
    QRegExpValidator validator(regp, this);
    int pos = 0;
    if(QValidator::Acceptable != validator.validate(string, pos)){
        return false;
    }
    return true;
}

bool ExcelRW::checkIsNumber(QString string)
{
    if(string.isEmpty())
    {
        return false;
    }
    QRegExp regp("^[0-9]+$");
    QRegExpValidator validator(regp,nullptr);
    int pos=0;
    if(QValidator::Acceptable != validator.validate(string,pos))
    {
        return false;
    }
    return true;
}

bool ExcelRW::checkWebSite(QString site)
{
    if(site.isEmpty())
    {
        return false;
    }
    QRegExp regp("^([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6}$");
    QRegExpValidator validator(regp,nullptr);
    int pos=0;
    if(QValidator::Acceptable != validator.validate(site, pos))
    {
        return false;
    }

    return true;
}
