#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QTextStream>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    string toFile = QFileDialog::getOpenFileName(this, "Выберите файл", "D:/", "(*.cpp)").toStdString();

    QFile file(QString::fromStdString(toFile));
    file.open(QIODevice::ReadOnly);
    QTextStream text(&file);
    string code = text.readAll().toStdString();
    ui->plainTextEdit->setPlainText(QString::fromStdString(code));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_parseButton_clicked()
{
    clear();

    parser.set_code(ui->plainTextEdit->toPlainText().toStdString());
    parser.do_parse();

    for(size_t i = 0; i < parser.blocks_info.size(); ++i)
    {
        block_info &blck = parser.blocks_info[i];
        ui->listWidget->addItem(QString::fromStdString(blck.place.get_title()));
    }
}

inline void MainWindow::clear()
{
    parser.reset();
    ui->listWidget->clear();
    ui->Data->clear();
}


void MainWindow::on_listWidget_currentRowChanged(int currentRow)
{
    if(currentRow < 0) return;

   QString str;
   QTextStream out(&str);
   out << QString::fromStdString(parser.blocks_info[currentRow].place.get_title()) << ": \n";

   out << getInfo(parser.blocks_info[currentRow]) << "\n";

    for (int k = 0; k < parser.blocks_info[currentRow].variables.size(); ++k)
    {
        out << QString::fromStdString(parser.blocks_info[currentRow].variables[k].toString()) << " ";
        vector<position> &chng = parser.blocks_info[currentRow].variables[k].change_pos;
        out << " Changes: ";
        for(size_t t = 0; t < chng.size(); ++t) out << QString::fromStdString(chng[t].toString()) << " ";
        out << "\n";
    }
    out << "\n";
    ui->Data->setPlainText(str);
}

QString MainWindow::getInfo(block_info blck)
{
    string str;
    if(blck.proto != -1)
    {
        prototype &proto = parser.prototypes[blck.proto];

        str += "function type: " + proto.type;
        if(blck.place._class != "\0") str += " class: " + proto._class;
        str += " method name: " + proto.name;
        str += " parametres: ";
        for(size_t i = 0; i < proto.parameters.size();++i)
            str += proto.parameters[i].toString() + " ";
        str += "position: " + proto.pos.toString();
        str += " reloads: " + QString::number(proto.reloads).toStdString();
    }
    else if(blck.place._class != "\0")
        str += "personal type " + blck.place._class;

    return QString::fromStdString(str);
}



void MainWindow::on_if_button_clicked()
{
    QString str;
    QTextStream out(&str);

    out << "ifs\n";
    for (size_t i = 0; i < parser.ifs.size(); ++i)
    {
        out << "if on the " << QString::fromStdString(parser.ifs[i].pos.toString()) << " has " << parser.ifs[i].dif << " dif\n";
    }

    ui->Data->setPlainText(str);
}


void MainWindow::on_errors_button_clicked()
{
    QString str;
    QTextStream out(&str);

    out << "errors\n";
    for (size_t i = 0; i < parser.errors.size(); ++i)
    {
        out << "if on the " << QString::fromStdString(parser.errors[i].toString()) << "\n";
    }

    ui->Data->setPlainText(str);
}

