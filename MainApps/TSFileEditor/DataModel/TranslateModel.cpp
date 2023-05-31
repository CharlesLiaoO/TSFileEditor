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
