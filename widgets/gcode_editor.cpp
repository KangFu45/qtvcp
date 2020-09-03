#include "gcode_editor.h"

#include <QVBoxLayout>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include <QAction>
#include <QToolBar>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>

//-------------------GcodeLexer-------------------

GcodeLexer::GcodeLexer()
{
    QFont font = QFont();
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(12);
    font.setBold(true);
    this->setFont(font, 2);
}

void GcodeLexer::setPaperBackground(QColor color, STYLE style)
{
    //if(style == None){
    //    for(int i = 0; i<=5; ++i)
    //        this->setPaper(color,i);
    //}
    //else
        this->setPaper(color, style);
}

QColor GcodeLexer::defaultColor(int style) const
{
    if(style == Default)
        return QColor("#000000"); // black
    else if(style == Comment)
        return QColor("#000000"); // black
    else if(style == Key)
        return QColor("#0000CC"); // blue
    else if(style == Assignment)
        return QColor("#CC0000"); // red
    else if(style == Value)
        return QColor("#00CC00"); // green
    return QsciLexerCustom::defaultColor(style);
}

void GcodeLexer::styleText(int start, int end)
{
    QsciScintilla* editor = this->editor();
    if(!editor) return;

// scintilla works with encoded bytes, not decoded characters.
// this matters if the source contains non-ascii characters and
// a multi-byte encoding is used (e.g. utf-8)
    QString source;
    if(end > editor->length())
        end = editor->length();
    if(end > start){
        source = editor->text(start, end);
    }
    if(source.isEmpty()) return;

    STYLE state;
    //long index = editor->SendScintilla(editor->SCI_LINEFROMINDEXPOSITION, start);
    //if(index > 0){
    //    long pos = editor->SendScintilla(editor->SCI_GETLINEENDPOSITION, index - 1);
    //    state = editor->SendScintilla(editor->SCI_GETSTYLEAT, pos);
    //}
    //else
        state = Default;

    this->startStyling(start);

    int first = 0;
    int second = source.indexOf("\n", first);
    QString line;
    while(second != -1){
        line = source.mid(first, second - first);
        char cc, t;
        bool graymode = false;
        bool msg = line.toLower().indexOf("msg") == -1 or line.toLower().indexOf("debug") == -1;
        foreach (const QString c, line) {
            cc = c.toStdString().c_str()[0];
            t = c.toLower().toStdString().c_str()[0];
            if(cc == '('){
                graymode = true;
                this->setStyling(1,Comment);
                continue;
            }
            else if(cc == ')'){
                graymode = false;
                this->setStyling(1,Comment);
                continue;
            }
            else if(graymode){
                if(msg && (t == 'g' || t== 'm' || t=='s' || t==',' || t=='d' || t=='e' || t=='b' || t=='u')){
                    this->setStyling(1,Assignment);
                    if(cc == ',') msg = false;
                }
                else
                    this->setStyling(1, Comment);
                continue;
            }
            else if(cc == '%' || cc=='<' || cc=='>' || cc=='#' || cc== '=')
                state = Assignment;
            else if(cc == '[' || cc== ']')
                state = Value;
            else if(t>='a' && t<='z')
                state = Key;
            else if(t>='0' && t<='9')
                state = Default;
            else
                state = Default;
            this->setStyling(1,state);
        }

        // skip "/n"
        this->setStyling(1,state);
        first = second + 1;
        second = source.indexOf("\n", first);
    }
}

//---------------------Editorbase-------------------------

Editorbase::Editorbase(QWidget * parent)
    : QsciScintilla(parent)
{
    this->setReadOnly(true);
    this->font.setFamily("Courier");
    this->font.setFixedPitch(true);
    this->font.setPointSize(12);
    this->setFont(this->font);
    this->setMarginsFont(this->font);

    this->set_margin_width(7);
    this->setMarginLineNumbers(0, true);
    //this->setMarginsBackgroundColor(QColor("#cccccc"));
    //this->setMarginsForegroundColor(QColor("#aa0000"));

    this->setMarginSensitivity(1, true);
    this->setMarginWidth(1, 0);
    this->markerDefine(QsciScintilla::Background,
                       this->ARROW_MARKER_NUM);

    this->setBraceMatching(QsciScintilla::SloppyBraceMatch);

    this->setCaretLineVisible(true);
    this->setCaretLineBackgroundColor(QColor("#ffe4e4"));

    this->set_gcode_lexer();

    this->SendScintilla(QsciScintilla::SCI_SETSCROLLWIDTH, 700);
    this->SendScintilla(QsciScintilla::SCI_SETSCROLLWIDTHTRACKING);

    //this->set_background_color("#C0C0C0");

    this->setMinimumSize(200, 100);
}

void Editorbase::set_margin_width(int width)
{
    QFontMetrics fontmetrics(this->font);
    this->setMarginsFont(this->font);
    //fontmetrics.width("0"*width)
    this->setMarginWidth(0, fontmetrics.width(width) + 15);
}

void Editorbase::set_background_color(QColor color)
{
    this->SendScintilla(QsciScintilla::SCI_STYLEGETBACK, QsciScintilla::STYLE_DEFAULT, color);
    this->lexer->setPaperBackground(color);
}
void Editorbase::on_margin_clicked(int nline)
{
    if(this->markersAtLine(nline) != 0)
        this->markerDelete(nline, this->ARROW_MARKER_NUM);
    else
        this->markerAdd(nline, this->ARROW_MARKER_NUM);
}

//void Editorbase::set_python_lexer()
//{
//    if(this->lexer != nullptr)
//        delete this->lexer;
//     this->lexer = new QsciLexerPython();
//    this->setLexer(this->lexer);
//    this->SendScintilla(QsciScintilla::SCI_STYLESETFONT, 1, "Courier");
//}

void Editorbase::set_gcode_lexer()
{
    if(this->lexer != nullptr)
        delete this->lexer;
    this->lexer = new GcodeLexer;
    this->lexer->setDefaultFont(this->font);
    this->setLexer(this->lexer);
    this->set_background_color("#C0C0C0");
    this->setMarginsBackgroundColor(QColor("#dddddd"));
}

void Editorbase::new_text()
{
    this->setText("");
}

void Editorbase::load_text(QString filepath)
{
    this->filepath = filepath;
    QFile file(this->filepath);
    if(file.open(QIODevice::ReadOnly)){
        this->setText(file.readAll());
    }
    else
        return;
    file.close();

    this->ensureCursorVisible();
    this->SendScintilla(QsciScintilla::SCI_VERTICALCENTRECARET);
    this->setModified(false);
}

void Editorbase::save_text()
{
    QFile file(this->filepath);
    if(file.open(QIODevice::WriteOnly))
        file.write(this->text().toUtf8());
    else
        return;
    file.close();
}

void Editorbase::replace_text(QString text)
{
    this->replace(text);
}

void Editorbase::search(QString text,bool re, bool case_, bool word, bool wrap, bool fwd)
{
    this->findFirst(text, re, case_, word, wrap, fwd);
}

void Editorbase::search_Next()
{
    this->SendScintilla(QsciScintilla::SCI_SEARCHANCHOR);
}

//---------------------GcodeDisplay----------------------------

void GcodeDisplay::hal_init()
{
    connect(this,&GcodeDisplay::cursorPositionChanged, this, &GcodeDisplay::line_changed);
    if(this->auto_show_mdi){
        connect(STATUS, &_GStat::mode_mdi, this, &GcodeDisplay::load_mdi);
        connect(STATUS, &_GStat::mdi_history_changed, this, &GcodeDisplay::load_mdi);
        //connect(STATUS, &_GStat::mode_auto, this, &GcodeDisplay::reload_last);
        connect(STATUS, &_GStat::move_text_lineup, this, &GcodeDisplay::select_lineup);
        connect(STATUS, &_GStat::move_text_linedown, this, &GcodeDisplay::select_linedown);
    }
    if(this->auto_show_manual){
        connect(STATUS, &_GStat::mode_manual, this, &GcodeDisplay::load_manual);
        connect(STATUS, &_GStat::machine_log_changed, this, &GcodeDisplay::load_manual);
    }
    if(this->auto_show_preference)
        connect(STATUS, &_GStat::show_preference, this, &GcodeDisplay::load_preference);
    connect(STATUS, &_GStat::file_loaded, this, &GcodeDisplay::load_program);
    connect(STATUS, &_GStat::line_changed, this, &GcodeDisplay::highlight_line);
    connect(STATUS, &_GStat::graphics_line_selected, this, &GcodeDisplay::highlight_line);

    if(this->idle_line_reset)
        connect(STATUS, &_GStat::interp_idle, this, &GcodeDisplay::set_line_0);
}

//void GcodeDisplay::load_text(QString filename)
//{
//    if(!filename.isEmpty()){
//        QFile file(filename);
//        if(file.open(QIODevice::ReadOnly)){
//            QTextStream in(&file);
//            QApplication::setOverrideCursor(Qt::WaitCursor);
//            qDebug() << in.readAll();
//            this->setText(in.readAll());
//            QApplication::restoreOverrideCursor();
//        }
//        else
//            return;
//        file.close();
//
//        this->last_line = -1;
//        this->ensureCursorVisible();
//        this->SendScintilla(QsciScintilla::SCI_VERTICALCENTRECARET);
//        return;
//    }
//    this->setText("");
//}

void GcodeDisplay::set_line_number(int line)
{
    emit STATUS->gcode_line_selected(line);
}

void GcodeDisplay::jump_line(int jump)
{
    int line, index;
    this->getCursorPosition(&line, &index);
    line += jump;
    if(line<0) line = 0;
    if(line>this->lines()) line = this->lines();
    this->setCursorPosition(line, 0);
    this->highlight_line(line);
}

void GcodeDisplay::reset_auto_show_mdi()
{
    this->auto_show_mdi = true;
    //?
}

void GcodeDisplay::reset_auto_show_manual()
{
    this->auto_show_manual = true;
    //?
}

void GcodeDisplay::load_program(QString _filename)
{
    if(_filename.isEmpty())
        _filename = this->last_filename;
    else
        this->last_filename = _filename;
    this->load_text(_filename);
    this->setCursorPosition(0, 0);
    this->setModified(false);
}

void GcodeDisplay::reload_last()
{
    this->load_text(STATUS->fresh.file.c_str());
}

void GcodeDisplay::load_mdi()
{
    this->load_text(INFO->MDI_HISTORY_PATH.c_str());
    this->last_filename = INFO->MDI_HISTORY_PATH.c_str();
    this->setCursorPosition(this->lines(), 0);
}

void GcodeDisplay::load_manual()
{
    if(STATUS->is_man_mode()){
        this->load_text(INFO->MACHINE_LOG_HISTORY_PATH.c_str());
        this->setCursorPosition(this->lines(), 0);
    }
}

void GcodeDisplay::load_preference()
{
    //this->load_text(this-);
}

void GcodeDisplay::line_changed(int line)
{
    if(!STATUS->is_auto_running()){
        this->markerDeleteAll(-1);
        if(STATUS->is_mdi_mode()){
            QString line_text = this->text(line);
            emit STATUS->mdi_line_selected(line_text.toStdString(), this->last_filename.toStdString());
        }
        else
            this->set_line_number(line);
    }
}

void GcodeDisplay::highlight_line(int line)
{
    if(STATUS->is_auto_running()){
        if(STATUS->fresh.file.c_str() != this->last_filename){
            this->load_text(STATUS->fresh.file.c_str());
            this->last_filename = STATUS->fresh.file.c_str();
        }
        //self.emit_percent(line*100/self.lines())
    }
    this->markerAdd(line, this->ARROW_MARKER_NUM);
    if(this->last_line)
        this->markerDelete(this->last_line, this->ARROW_MARKER_NUM);
    this->setCursorPosition(line, 0);
    this->ensureCursorVisible();
    this->SendScintilla(QsciScintilla::SCI_VERTICALCENTRECARET);
    this->last_line = line;
}

void GcodeDisplay::select_lineup()
{
    int line, index;
    this->getCursorPosition(&line, &index);
    this->setCursorPosition(line-1, 0);
    this->highlight_line(line-1);
}

void GcodeDisplay::select_linedown()
{
    int line, index;
    this->getCursorPosition(&line, &index);
    this->setCursorPosition(line+1, 0);
    this->highlight_line(line+1);
}

//--------------------GcodeEditor-------------------

GcodeEditor::GcodeEditor(QWidget* parent)
    : QWidget(parent)
{
    //connect(STATUS, &_GStat::general, this, &GcodeEditor::returnFromDialog);

    QVBoxLayout* lay = new QVBoxLayout;
    lay->setContentsMargins(0,0,0,0);
    this->setLayout(lay);

    m_editor = new GcodeDisplay(this);

    this->m_editor->setReadOnly(true);
    this->m_editor->setModified(false);

    // add menubar actions

    //QAction* newAction = new QAction(QIcon::fromTheme("document-new"), "New");
    QAction* newAction = new QAction(QIcon(":/images/icons/filenew.png"), "New", this);
    newAction->setStatusTip("New document");
    connect(newAction, &QAction::triggered, this, &GcodeEditor::newCall);

    QAction* openAction = new QAction(QIcon(":/images/icons/fileopen.png"), "Open", this);
    openAction->setStatusTip("Open document");
    connect(openAction, &QAction::triggered, this, &GcodeEditor::openCall);

    QAction* saveAction = new QAction(QIcon(":/images/icons/filesave.png"), "Save", this);
    saveAction->setStatusTip("save document");
    connect(saveAction, &QAction::triggered, this, &GcodeEditor::saveCall);

    m_exitAction = new QAction(QIcon(":/images/icons/exit.png"), "Exit", this);
    m_exitAction->setStatusTip("Exit application");
    connect(m_exitAction, &QAction::triggered, this, &GcodeEditor::exitCall);

    QToolBar* toolBar = new QToolBar("File");
    toolBar->addAction(newAction);
    toolBar->addAction(openAction);
    toolBar->addAction(saveAction);
    toolBar->addAction(m_exitAction);

    toolBar->addSeparator();

    //toolBar->addWidget(QLabel('<html><head/><body><p><span style=" font-size:20pt; font-weight:600;">Edit Mode</span></p></body></html>'));
    QLabel* aaa = new QLabel("EDIT MODE");
    QFont font("Microsoft YaHei", 20, 600);
    aaa->setFont(font);
    toolBar->addWidget(aaa);

    QHBoxLayout* box = new QHBoxLayout;
    box->addWidget(toolBar);

    this->m_topMenu = new QFrame;
    this->m_topMenu->setLayout(box);

    lay->addWidget(this->m_topMenu);
    lay->addWidget(this->m_editor);
    lay->addWidget(this->createGroup());

    this->readOnlyMode();
}

QFrame* GcodeEditor::createGroup()
{
    this->m_bottomMenu = new QFrame;

    this->m_searchText = new LineEdit(this);
    this->m_replaceText = new LineEdit(this);

    QToolBar* toolBar = new QToolBar;

    m_keyBoardAct = new QAction(QIcon(":/images/icons/rectangle.png"), "Undo", this);
    m_keyBoardAct->setStatusTip("Key Board");
    toolBar->addAction(m_keyBoardAct);

    toolBar->addSeparator();

    QAction* undoAction = new QAction(QIcon(":/images/icons/undo.png"), "Undo", this);
    undoAction->setStatusTip("Undo");
    connect(undoAction, &QAction::triggered, this, &GcodeEditor::undoCall);
    toolBar->addAction(undoAction);

    QAction* redoAction = new QAction(QIcon(":/images/icons/redo.png"), "Redo", this);
    redoAction->setStatusTip("Redo");
    connect(redoAction, &QAction::triggered, this, &GcodeEditor::redoCall);
    toolBar->addAction(redoAction);

    toolBar->addSeparator();

    QAction* replaceAction = new QAction(QIcon(":/images/icons/replace_black.png"), "Replace", this);
    replaceAction->setStatusTip("Replace");
    connect(replaceAction, &QAction::triggered, this, &GcodeEditor::replaceCall);
    toolBar->addAction(replaceAction);

    QAction* findAction = new QAction(QIcon(":/images/icons/find_black.png"), "Find", this);
    findAction->setStatusTip("Find");
    connect(findAction, &QAction::triggered, this, &GcodeEditor::findCall);
    toolBar->addAction(findAction);

    QAction* nextAction = new QAction(QIcon(":/images/icons/previous_black.png"), "Find Previous", this);
    nextAction->setStatusTip("Find Previous");
    connect(nextAction, &QAction::triggered, this, &GcodeEditor::nextCall);
    toolBar->addAction(nextAction);

    toolBar->addSeparator();

    //QAction* caseAction = new QAction(QIcon::fromTheme("edit-case"), "Aa");
    //caseAction->setCheckable(1);
    //connect(caseAction, &QAction::triggered, this, &GcodeEditor::caseCall);
    //toolBar->addAction(caseAction);

    QHBoxLayout* box = new QHBoxLayout;
    box->addWidget(toolBar);
    box->addWidget(this->m_searchText);
    box->addWidget(this->m_replaceText);
    box->addStretch(1);
    this->m_bottomMenu->setLayout(box);

    return this->m_bottomMenu;
}

void GcodeEditor::caseCall()
{
    this->isCaseSensitive -= 1;
    this->isCaseSensitive *= -1;
    // print self.isCaseSensitive
}

void GcodeEditor::exitCall()
{
    if(this->m_editor->isModified()){
        if(this->killCheck()) {
            this->readOnlyMode();
        }
        else
            return;
    }
    emit STATUS->file_edit_exit();
}

void GcodeEditor::findCall()
{
    this->m_editor->search(this->m_searchText->text(),
                           false, this->isCaseSensitive, false, false, true);
}

void GcodeEditor::nextCall()
{
    this->m_editor->search(this->m_searchText->text(), false);
    this->m_editor->search_Next();
}

void GcodeEditor::newCall()
{
    if(this->m_editor->isModified()){
        if(this->killCheck())
            this->m_editor->new_text();
    }
}

void GcodeEditor::openCall()
{
    this->getFileName();
}

void GcodeEditor::openReturn(QString f)
{
    ACTION->OPEN_PROGRAM(f.toStdString());
    this->m_editor->setModified(false);
}

void GcodeEditor::redoCall()
{
    this->m_editor->redo();
}

void GcodeEditor::replaceCall()
{
    this->m_editor->replace_text(this->m_replaceText->text());
}

void GcodeEditor::saveCall()
{
    this->getSaveFileName();
}

void GcodeEditor::saveReturn(QString fname)
{
    ACTION->SAVE_PROGRAM(this->m_editor->text().toStdString(), fname.toStdString());
    this->m_editor->setModified(false);
    ACTION->OPEN_PROGRAM(fname.toStdString());
}

void GcodeEditor::undoCall()
{
    this->m_editor->undo();
}

void GcodeEditor::GcodeEditor::editMode()
{
    this->m_topMenu->show();
    this->m_bottomMenu->show();
    this->m_editor->setReadOnly(false);
}

void GcodeEditor::GcodeEditor::readOnlyMode()
{
    this->m_topMenu->hide();
    this->m_bottomMenu->hide();
    this->m_editor->setReadOnly(true);
}

void GcodeEditor::GcodeEditor::getFileName()
{
    QString fname = QFileDialog::getOpenFileName(this, "open file", "/opt", "gcode (*.ngc)");
    this->openReturn(fname);
}

void GcodeEditor::GcodeEditor::getSaveFileName()
{
    QString fname = QFileDialog::getSaveFileName(this, "save file", "/opt", "gcode (*.ngc)");
    this->saveReturn(fname);
}

void GcodeEditor::GcodeEditor::returnFromDialog(QString message)
{

}

bool GcodeEditor::GcodeEditor::killCheck()
{
    if(QMessageBox::question(this, "Warning!!",
                             "This file has changed since loading...Still want to proceed?",
                             QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        return true;
    else
        return false;

}

void GcodeEditor::hal_init()
{
    this->m_editor->hal_init();
}
