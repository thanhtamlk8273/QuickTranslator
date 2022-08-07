#ifndef HEADER_H_MYTEXTEDIT
#define HEADER_H_MYTEXTEDIT
#include <unicode/uconfig.h>
#include <QtWidgets>
#include "Translator.h"
#include <memory>

class MyTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    MyTextEdit(Dictionary* _VietPhrases, Dictionary* _Names, Dictionary* _Hanviets, QWidget* parent = nullptr);
    ~MyTextEdit();

public slots:
    void translateButtonReleased();
    void handleAboutToQuitSignal();

protected:
    void contextMenuEvent(QContextMenuEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);

private:
    /* Dictionary */
    Dictionary * VietPhrases;
    Dictionary * Names;
    Dictionary * Hanviets;
    /* Translator */
    Translator translator;
    /* Methods */
    void translate(QString* text = nullptr);
    void createEditDialog(Dictionary * dic);
};

#endif
