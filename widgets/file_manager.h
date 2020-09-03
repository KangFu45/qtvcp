#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <QWidget>
#include <QFileSystemModel>
#include <QListView>
#include <QPushButton>

class FileManager : public QWidget
{
    Q_OBJECT

public:
    FileManager(QWidget *parent = Q_NULLPTR);

    void updateDirectoryView(QString path);
    QPushButton* m_editBtn;

signals:
    // make load program for other widget
    void load(QString fname);

public slots:
    void clicked(QModelIndex index);
    void getPathActivated();
    void filterChanged(QString text);
    void onLocalClicked();
    void onUsbClicked();

private:
    QFileSystemModel* m_model;
    QListView* m_list;

    QString local_path;
    QString usb_path;
    QString currentPath;
};

#endif // FILE_MANAGER_H
