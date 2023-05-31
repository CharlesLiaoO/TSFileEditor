#include "TranslateModel.h"

TranslateModel::TranslateModel()
{

}

TranslateModel::TranslateModel(const TranslateModel &other)
{
    this->m_key = other.GetKey();
    this->m_source = other.GetSource();
    this->m_translate = other.GetTranslate();
}

TranslateModel &TranslateModel::operator=(const TranslateModel &other)
{
    if(&other != this){
        this->m_key = other.GetKey();
        this->m_source = other.GetSource();
        this->m_translate = other.GetTranslate();
    }

    return *this;
}

TranslateModel::~TranslateModel()
{

}

//****************************Getter && Setter*************************************
QString TranslateModel::GetKey(bool es) const
{
    return es ? ToEscapeStr(m_key) : m_key;
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

void TranslateModel::SetSource(const QString &str, bool es)
{
    m_source = es ? FromEscapeStr(str) : str;
}

void TranslateModel::SetTranslate(const QString &str, bool es)
{
    m_translate = es ? FromEscapeStr(str) : str;
}
