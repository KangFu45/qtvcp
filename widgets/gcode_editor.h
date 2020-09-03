#ifndef GCODE_EDIT_H
#define GCODE_EDIT_H

#include <QFont>
#include <QLineEdit>

#include "Qsci/qscilexercustom.h"
#include "Qsci/qsciscintilla.h"
#include "Qsci/qscilexerpython.h"

#include "qt_action.h"
#include "mdi_line.h"

class GcodeLexer : public QsciLexerCustom
{
public:
    GcodeLexer();

    enum STYLE{
        None = -1,
        Default = 0,
        Comment = 1,
        Key = 2,
        Assignment = 3,
        Value = 4
    };

    QColor defaultColor(int style) const;
    const char * language() const {return "G code";}
    QString description(int style) const {return "Custom lexer for the G code programming languages";}
    void styleText(int start, int end);
    void setPaperBackground(QColor color, STYLE style = None);
};

class Editorbase : public QsciScintilla
{
    Q_OBJECT

public:
    Editorbase(QWidget *parent = 0);

    void set_margin_width(int width);
    void set_background_color(QColor color);
    void on_margin_clicked(int nline);
    //void set_python_lexer();
    void set_gcode_lexer();
    void new_text();
    void load_text(QString filepath);
    void save_text();
    void replace_text(QString text);
    void search(QString text,bool re=false, bool case_=false,bool word=false, bool wrap=false, bool fwd=true);
    void search_Next();

    QString filepath;
    QFont font;
    int ARROW_MARKER_NUM = 8;
    GcodeLexer* lexer = nullptr;
};

class GcodeDisplay : public Editorbase
{
    Q_OBJECT

public:
    GcodeDisplay(QWidget *parent = 0) : Editorbase(parent){}

    shared_ptr<_IStat> INFO;
    _GStat* STATUS;

    bool idle_line_reset = false;
    bool auto_show_mdi = true;
    bool auto_show_manual = true;
    bool auto_show_preference = true;
    int last_line;
    QString filename;

    void hal_init();
    //void load_text(QString filename);
    void set_line_number(int line);
    void jump_line(int jump);

    void set_auto_show_mdi(bool data) {this->auto_show_mdi = data;}
    bool get_auto_show_mdi() {return this->auto_show_mdi;}
    void reset_auto_show_mdi();

    void set_auto_show_manual(bool data) {this->auto_show_manual;}
    bool get_auto_show_manual() {return this->auto_show_manual;}
    void reset_auto_show_manual();

public slots:
    void load_program(QString _filename = "");
    void reload_last();
    void load_mdi();
    void load_manual();
    void load_preference();
    void line_changed(int line);
    void set_line_0() {this->set_line_number(0);}
    void highlight_line(int line);
    void select_lineup();
    void select_linedown();

private:
    QString last_filename;
};

class GcodeEditor : public QWidget
{
    Q_OBJECT

public:
    GcodeEditor(QWidget* parent = Q_NULLPTR);

    shared_ptr<_Lcnc_Action> ACTION;
    _GStat* STATUS;

    QFrame* createGroup();
    void editMode();
    void readOnlyMode();
    void getFileName();
    void getSaveFileName();
    void returnFromDialog(QString message);
    bool killCheck();
    void openReturn(QString f);
    void saveReturn(QString fname);
    void hal_init();

    GcodeDisplay* m_editor;
    QString load_dialog_code = "LOAD";
    QString save_dialog_code = "SAVE";
    int isCaseSensitive = 0;

    QFrame* m_topMenu;
    QFrame* m_bottomMenu;
    LineEdit* m_searchText;
    LineEdit* m_replaceText;
    QAction* m_exitAction;
    QAction* m_keyBoardAct;

private slots:
    void caseCall();
    void exitCall();
    void findCall();
    void nextCall();
    void newCall();
    void openCall();
    void redoCall();
    void replaceCall();
    void saveCall();
    void undoCall();

};

#endif // GCODE_EDIT_H
