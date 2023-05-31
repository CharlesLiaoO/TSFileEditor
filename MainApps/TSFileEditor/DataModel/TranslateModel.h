#ifndef TRANSLATEMODEL_H
#define TRANSLATEMODEL_H

#include <QString>

class TranslateModel
{
public:
    explicit TranslateModel();
    TranslateModel(const TranslateModel&);
    TranslateModel &operator= (const TranslateModel&);
    ~TranslateModel();

    QString GetKey(bool es=false) const;
    QString GetSource(bool es=false) const;
    QString GetTranslate(bool es=false) const;

    void SetKey(const QString &str, bool es=false);
    void SetSource(const QString &str, bool es=false);
    void SetTranslate(const QString &str, bool es=false);

    static QString ToEscapeStr(QString str) {
        str.replace("\n", "\\n");
        str.replace("\t", "\\t");
        return str;
    }
    static QString FromEscapeStr(QString str) {
        str.replace("\\n", "\n");
        str.replace("\\t", "\t");
        return str;
    }
private:
    QString m_key;
    QString m_source;
    QString m_translate;
};

#endif // TRANSLATEMODEL_H
