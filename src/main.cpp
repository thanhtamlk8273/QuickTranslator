#include <QtWidgets/QApplication>
#include <QtWidgets>
#include <QObject>

#include "Translator.h"
#include "MyTextEdit.h"

QDialog* ddialog;

void writeToDebugDialog(const QString& debug_log) {
    auto ddialog_textEdit = ddialog->findChild<QTextEdit*>();
    QTextCursor cursor(ddialog_textEdit->textCursor());
    cursor.insertText(debug_log);
    cursor.insertBlock();
};

int main(int argc, char* argv[])
{
    /* Setting up translator */
    auto VietPhrases = std::make_unique<Dictionary>();
    auto Names = std::make_unique<Dictionary>();
    auto HanViets = std::make_unique<Dictionary>();
    if (!VietPhrases->loadFromFile("VietPhrase.txt"))
    {
        exit(1);
    }
    if (!Names->loadFromFile("Names.txt"))
    {
        exit(1);
    }
    if (!HanViets->loadFromFile("ChinesePhienAmWords.txt"))
    {
        exit(1);
    }
    /* Code starts here */
    QApplication a(argc, argv);
    QWidget window;
    window.resize(800, 600);

    /* Debug QDialog */
    ddialog = new QDialog();
    QHBoxLayout* ddialog_layout = new QHBoxLayout();
    ddialog_layout->addWidget(new QTextEdit());
    ddialog->setLayout(ddialog_layout);
    ddialog->show();

    QVBoxLayout* layout = new QVBoxLayout();

    /* Add buttons */
    QPushButton* pushButton = new QPushButton("Translate");
    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addWidget(pushButton);
    button_layout->addStretch(1);
    layout->addLayout(button_layout);

    /* Add QTextEdit*/
    MyTextEdit* textEdit = new MyTextEdit(VietPhrases.get(), Names.get(), HanViets.get());
    layout->addWidget(textEdit);

    /* Signals and slots */
    QObject::connect(pushButton, &QPushButton::released, textEdit, &MyTextEdit::translateButtonReleased);
    QObject::connect(&a, &QApplication::aboutToQuit, textEdit, &MyTextEdit::handleAboutToQuitSignal);

    window.setLayout(layout);
    window.setWindowTitle(QApplication::translate("windowlayout", "Window layout"));
    window.show();
    return a.exec();
}
