#ifndef TRANSLATEMODEL_H
#define TRANSLATEMODEL_H

#include <QString>

class TranslateModel
{
public:
    QString GetKey(bool es=false) const;
    QString GetComment(bool es=false) const;
    QString GetSource(bool es=false) const;
    QString GetTranslate(bool es=false) const;

    void SetKey(const QString &str, bool es=false);
    void SetComment(const QString &str, bool es=false);
    void SetSource(const QString &str, bool es=false);
    void SetTranslate(const QString &str, bool es=false);

    static QString ToEscapeStr(QString str) {
//        str.prepend("'");  //excel单元格编辑之后，才会消失显示'，不便于复制粘贴
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
    static QString FromEscapeStr(QString str) {
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
private:
    QString m_key;
    QString m_comment;
    QString m_source;
    QString m_translate;
};

#endif // TRANSLATEMODEL_H
