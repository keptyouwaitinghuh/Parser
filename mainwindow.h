#pragma once
#include "Parser.h"


#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_parseButton_clicked();

    void on_listWidget_currentRowChanged(int currentRow);

    void on_if_button_clicked();

    void on_errors_button_clicked();

private:
    Ui::MainWindow *ui;
    Parser parser;
    void clear();
    QString getInfo(block_info);
};
