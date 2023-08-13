#include "TranslateModel.h"

//****************************Getter && Setter*************************************
QString TranslateModel::GetKey(bool es) const
{
    return es ? ToEscapeStr(m_key) : m_key;
}

QString TranslateModel::GetComment(bool es) const
{
    return es ? ToEscapeStr(m_comment) : m_comment;
}

QString TranslateModel::GetSource(bool es) const
{
    return es ? ToEscapeStr(m_source) : m_source;
}

QString TranslateModel::GetTranslate(bool es) const
{
    return es ? ToEscapeStr(m_translate) : m_translate;
}

void TranslateModel::SetKey(const QString &str, bool es)
{
    m_key = es ? FromEscapeStr(str) : str;
}

void TranslateModel::SetComment(const QString &str, bool es)
{
    m_comment = es ? FromEscapeStr(str) : str;
}

void TranslateModel::SetSource(const QString &str, bool es)
{
    m_source = es ? FromEscapeStr(str) : str;
}

void TranslateModel::SetTranslate(const QString &str, bool es)
{
    m_translate = es ? FromEscapeStr(str) : str;
}

QString TranslateModel::ToEscapeStr(QString str)
{
//    str.prepend("'");  //excel单元格编辑之后，才会消失显示'，不便于复制粘贴
    if (str.startsWith("\'"))  //只替换开头的
        str.replace(0, 1, R"(\')");
    if (str.startsWith("\""))
        str.replace(0, 1, R"(\")");
//        str.replace("\'", R"(\')");
//        str.replace("\"", R"(\")");

    str.replace("\n", R"(\n)");
    str.replace("\t", R"(\t)");
    return str;
}

QString TranslateModel::FromEscapeStr(QString str)
{
    if (str.startsWith("'"))
        str.remove(0, 1);  //防止翻译人员自己加'
    if (str.startsWith(R"(\')"))  //只替换开头的
        str.replace(0, 1, "\'");
    if (str.startsWith(R"(\")"))
        str.replace(0, 1, "\"");
//        str.replace(R"(\')", "\'");
//        str.replace(R"(\")", "\"");
    str.replace(R"(\n)", "\n");
    str.replace(R"(\t)", "\t");
    return str;
}
