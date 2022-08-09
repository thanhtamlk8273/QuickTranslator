#include "MyTextEdit.h"

void writeToDebugDialog(const QString& debug_log);
using TChunk = std::pair<icu::UnicodeString, icu::UnicodeString>;
namespace {
    QString curText;
    struct FmtOfText {
        bool isValid;
        int start;
        int end;
        QTextCharFormat fmt;
        FmtOfText() : isValid(false), start(0), end(0) {};
        FmtOfText(int _start, int _end, QTextCharFormat _fmt) :
            isValid(true), start(_start), end(_end), fmt(_fmt) {};
    };
    std::vector<std::vector<TChunk>> TChunkMap;
    FmtOfText backup;
    std::vector<TChunk> shared_chunks;
}

MyTextEdit::MyTextEdit(Dictionary* _VietPhrases, Dictionary* _Names, Dictionary* _Hanviets, QWidget* parent):
    QTextEdit(parent),
    VietPhrases(_VietPhrases),
    Names(_Names),
    Hanviets(_Hanviets)
{
    /* Basic properties */
    setFont(QFont("SansSerif", 18));
    setReadOnly(true);
    /* Setting up translator */
    translator = Translator(VietPhrases, Names, Hanviets);
}

MyTextEdit::~MyTextEdit()
{
}

void MyTextEdit::contextMenuEvent(QContextMenuEvent* event)
{
    /* Create the standard context menu of QTextEdit */
    QMenu* menu = this->createStandardContextMenu();
    /* Add new custom actions */
    QAction* updateVP_action = new QAction("Update VietPhrases");
    QAction* updateN_action = new QAction("Update Names");
    menu->addAction(updateVP_action);
    menu->addAction(updateN_action);
    /* Lambdas */
    auto createVPDialog = [&]() {createEditDialog(VietPhrases); };
    auto createNDialog = [&]() {createEditDialog(Names); };
    /* Connect actions so slots */
    QObject::connect(updateVP_action, &QAction::triggered, createVPDialog);
    QObject::connect(updateN_action, &QAction::triggered, createNDialog);
    /* Get it running */
    menu->exec(event->globalPos());
    delete menu;
}

void MyTextEdit::mouseReleaseEvent(QMouseEvent* event)
{
    /* Only handle event of left button */
    if (event->button() != Qt::LeftButton) return;
    /* Empty lines? Ignore them */
    QTextCursor cursor(textCursor());
    if (cursor.blockNumber() % 2 != 0) return;
    /* Boundary condition: beginning empty space? Ignore it */
    if (cursor.positionInBlock() < QString("    ").length() + 1) return;
    /* Method starts here */
    int block_index = cursor.blockNumber() / 2;
    /* Text chunks are decided by selection */
    int chunk_index = 0;
    int aggregate_len = 3;
    int startPosInBlock = cursor.selectionStart() - cursor.block().position();
    int endPosInBlock = cursor.selectionEnd() - cursor.block().position();
    int startChunkIndex = -1;
    int endChunkIndex = -1;
    if (TChunkMap[block_index].empty()) return;
    for (TChunk& chunk : TChunkMap[block_index])
    {
        aggregate_len += 1 + chunk.first.length();
        if (aggregate_len > startPosInBlock && startChunkIndex == -1) startChunkIndex = chunk_index;
        if (aggregate_len > endPosInBlock) break;
        ++chunk_index;
    }
    endChunkIndex = chunk_index;
    int max_index = TChunkMap[block_index].size() - 1;
    if (   startChunkIndex == -1 || endChunkIndex == -1
        || (startChunkIndex > max_index)
        || (endChunkIndex > max_index)) {
        return;
    }
    if (endChunkIndex < startChunkIndex) return;
    /* Update shared chunk */
    int total_length = 0;
    shared_chunks.clear();
    for (int i = startChunkIndex; i < endChunkIndex + 1; ++i)
    {
        total_length += 1 + TChunkMap[block_index][i].first.length();
        shared_chunks.push_back(TChunkMap[block_index][i]);
    }
    if (total_length == 0) return;
    /* First move the cursor the beginning of the block to get an reference */
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
    /* The start and the end are determined in reference to the start of the block (in code units) */
    int start = cursor.position() + aggregate_len - total_length + 1;
    int end = cursor.position() + aggregate_len;
    /* Moving the cursor to select the text */
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    /* Backing up to remove the highlight in the next click */
    backup = FmtOfText(start, end, cursor.charFormat());
    /* Highlight the text chunk: change text color to red to highlight it */
    QTextCharFormat fmt;
    fmt.setForeground(Qt::red);
    cursor.setCharFormat(fmt);
    cursor.clearSelection();
    setTextCursor(cursor);
}

void MyTextEdit::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QTextEdit::mousePressEvent(event);
        return;
    }
    /* Clear effect of last call */
    QTextCursor cursor(textCursor());
    if (backup.isValid)
    {
        cursor.setPosition(backup.start, QTextCursor::MoveAnchor);
        cursor.setPosition(backup.end, QTextCursor::KeepAnchor);
        cursor.setCharFormat(backup.fmt);
        backup = FmtOfText();
    }
    QTextEdit::mousePressEvent(event);
}

void MyTextEdit::translate(QString* s)
{
    /* Reset relevant data first */
    backup = FmtOfText();
    TChunkMap.clear();
    shared_chunks.clear();
    /* If there is no input text, get it from clipboard */
    int oldScrollBarValue = verticalScrollBar()->value();
    if (s == nullptr)
    {
        QClipboard* clipboard = QGuiApplication::clipboard();
        curText = clipboard->text();
    }
    else if (s == &curText)
    {

    }
    else
    {
        curText = *s;
    }
    // Don't know why. But if this step did not exist, old format from previous translation would persist and made it all red
    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    setTextCursor(cursor);
    /* Now start the method */
    QTextStream stream(&curText);
    QString translated;
    QString line;
    while (stream.readLineInto(&line))
    {
        if (line.trimmed().isEmpty()) continue;
        icu::UnicodeString tmp = line.trimmed().utf16();
        translated += "   ";
        std::vector<TChunk> translation_result = translator.TranslateALine(tmp);
        for (auto it = translation_result.begin(); it != translation_result.end();)
        {
            QString piece = QString::fromUtf16(it->first.getTerminatedBuffer());
            if (piece.isEmpty())
            {
                it = translation_result.erase(it);
            }
            else
            {
                translated += " " + piece;
                ++it;
            }
        }
        translated += "\n\n";
        /* Add translation_result to global map TChunkMap */
        TChunkMap.emplace_back(translation_result);
    }
    setPlainText(translated);
    /* Restore old scroll bar position */
    verticalScrollBar()->setValue(oldScrollBarValue);
}

void MyTextEdit::createEditDialog(Dictionary* dic)
{
    /* Texts to show */
    QString first_line;
    QString second_line;
    for (TChunk& chunk : shared_chunks)
    {
        first_line += QString::fromUtf16(chunk.second.getTerminatedBuffer());
        second_line += QString::fromUtf16(chunk.first.getTerminatedBuffer()) + " ";
    }
    second_line.chop(1);
    if(shared_chunks.size() == 1)
    {
        second_line = QString::fromUtf16(dic->getTranslated(first_line.utf16()).getTerminatedBuffer());
    }
    /* First inner horizontal layout */
    QHBoxLayout* first_layout = new QHBoxLayout();
    first_layout->addWidget(new QLabel("Original: "));
    QLineEdit* first_linedit = new QLineEdit(first_line);
    first_linedit->setReadOnly(true);
    first_layout->addWidget(first_linedit);
    /* Second inner horizontal layout */
    QHBoxLayout* second_layout = new QHBoxLayout();
    second_layout->addWidget(new QLabel("New:      "));
    QLineEdit* second_linedit = new QLineEdit(second_line);
    second_layout->addWidget(second_linedit);
    /* Third layout and buttons */
    QHBoxLayout* third_layout = new QHBoxLayout();
    QPushButton* add_button = new QPushButton("Add");
    QPushButton* delete_button = new QPushButton("Delete");
    QPushButton* cancel_button = new QPushButton("Cancel");
    third_layout->addWidget(add_button);
    third_layout->addWidget(delete_button);
    third_layout->addWidget(cancel_button);
    third_layout->addStretch(1);
    /* Main layout */
    QVBoxLayout* main_layout = new QVBoxLayout();
    main_layout->addLayout(first_layout);
    main_layout->addLayout(second_layout);
    main_layout->addLayout(third_layout);
    /* Finally, the dialog */
    QDialog* dialog = new QDialog();
    dialog->setFont(QFont("SansSerif", 18));
    dialog->setLayout(main_layout);
    /* Lambdas */
    auto handleAddButton = [&]() {
        dic->addNewRecord(first_line.utf16(), second_linedit->text().utf16());
        translate(&curText);
        dialog->close();
    };
    auto handleDelButton = [&]() {
        dic->delRecord(first_line.utf16());
        translate(&curText);
        dialog->close();
    };
    auto handleCancelButton = [&]() {
        dialog->close();
    };
    /* Signals and slots */
    QObject::connect(add_button, &QPushButton::released, handleAddButton);
    QObject::connect(delete_button, &QPushButton::released, handleDelButton);
    QObject::connect(cancel_button, &QPushButton::released, handleCancelButton);
    /* Execute*/
    dialog->exec();
    /* Clean up */
    delete dialog;
}

void MyTextEdit::translateButtonReleased()
{
    translate();
}

void MyTextEdit::handleAboutToQuitSignal()
{
    VietPhrases->update();
    Names->update();
    Hanviets->update();
}

void MyTextEdit::handleOpenFileSignal()
{
    QString file_name;
    QFile in_file_desc;
    /* Create dialog to select file */
    file_name = QFileDialog::getOpenFileName(nullptr, "Open Image", "~", "Text files (*.txt *)");
    in_file_desc.setFileName(file_name);
    if(!in_file_desc.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }
    /* Setting up a stream for it */
    QTextStream in_stream(&in_file_desc);
    /* Start reading file */
    QString translated;
    QString line;
    while(in_stream.readLineInto(&line))
    {
        icu::UnicodeString u_line = line.trimmed().utf16();
        if (u_line.isEmpty())
        {
            continue;
        }
        translated += "   ";
        translated += QString::fromUtf16(translator.translateALine(u_line).getTerminatedBuffer());
        translated += "\n\n";
    }
    /* Write "translated" result file */
    QFileInfo file_info(file_name); // I know, I know. I am a lazy fuck
    QString out_file_name(file_info.canonicalPath() + "/" + file_info.baseName() + "_translated");
    out_file_name += (file_info.suffix().isEmpty()) ? "" : "." + file_info.suffix();
    writeToDebugDialog(out_file_name);
    QFile out_file_desc(out_file_name);
    /* Create dialog to select file */
    if(!out_file_desc.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }
    out_file_desc.write(translated.toUtf8());
    writeToDebugDialog("Wrote result to file");
}
