#include "file_manager.h"

#include <QDir>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

FileManager::FileManager(QWidget *parent)
    :QWidget(parent)
{
    local_path = "/opt/linuxcnc/nc_files";
    usb_path = "/opt/linuxcnc/nc_files";

    // init ui
    m_model = new QFileSystemModel;
    m_model->setRootPath(QDir::currentPath());
    m_model->setFilter(QDir::AllDirs | QDir::NoDot | QDir::Files);
    m_model->setNameFilterDisables(false);
    m_model->setNameFilters(QStringList("*ngs"));

    m_list = new QListView;
    m_list->setModel(m_model);
    this->updateDirectoryView(this->local_path);
    m_list->setWindowTitle("Dir View");
    connect(m_list, &QListView::clicked, this, &FileManager::clicked);
    connect(m_list, &QListView::activated, this, &FileManager::getPathActivated);
    m_list->setAlternatingRowColors(true);

    QComboBox* cb = new QComboBox;
    connect(cb, &QComboBox::currentTextChanged, this, &FileManager::filterChanged);
    cb->addItem("*.ngc");

    QPushButton* btn = new QPushButton("Local");
    btn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    btn->setToolTip("Jump to Local directory");
    connect(btn, &QPushButton::clicked, this, &FileManager::onLocalClicked);

    QPushButton* btn2 = new QPushButton("Usb");
    btn2->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    btn2->setToolTip("Jump to Usb directory");
    connect(btn2, &QPushButton::clicked, this, &FileManager::onUsbClicked);

    m_editBtn = new QPushButton("Edit");
    m_editBtn->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(btn);
    hbox->addWidget(btn2);
    hbox->addWidget(cb);
    hbox->addWidget(m_editBtn);

    QVBoxLayout* windowLayout = new QVBoxLayout;
    windowLayout->addWidget(m_list);
    windowLayout->addLayout(hbox);
    this->setLayout(windowLayout);
}

void FileManager::updateDirectoryView(QString path)
{
    this->m_list->setRootIndex(this->m_model->setRootPath(path));
}

void FileManager::clicked(QModelIndex index)
{
    QString dir_path = this->m_model->filePath(index);
    if(this->m_model->fileInfo(index).isFile()){
        this->currentPath = dir_path;
        return;
    }
    this->m_list->setRootIndex(this->m_model->setRootPath(dir_path));
}

void FileManager::getPathActivated()
{
    QModelIndex row = this->m_list->selectionModel()->currentIndex();
    this->clicked(row);

    if(!this->currentPath.isEmpty())
        emit this->load(this->currentPath);
}

void FileManager::filterChanged(QString text)
{
    this->m_model->setNameFilters(QStringList(text));
}

void FileManager::onLocalClicked()
{
    this->updateDirectoryView(this->local_path);
}

void FileManager::onUsbClicked()
{
    this->updateDirectoryView(this->usb_path);
}
