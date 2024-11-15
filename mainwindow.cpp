#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdialog.h"
#include "searchdialog.h"
#include "replacedialog.h"
#include "qfiledialog.h"
#include "QMessageBox"
#include "QTextStream"
#include "QColorDialog"
#include "QFontDialog"
#include "QRegularExpression"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    textChanged = false;
    on_actionNew_triggered();

    statusLabel.setMaximumWidth(180);
    statusLabel.setText("length:" + QString::number(0) + "  lines:" + QString::number(1));
    ui->statusbar->addPermanentWidget(&statusLabel);

    statusCursorLabel.setMaximumWidth(180);
    statusCursorLabel.setText("LN:" + QString::number(0) + "  Col:" + QString::number(1));
    ui->statusbar->addPermanentWidget(&statusCursorLabel);

    QLabel *author = new QLabel(ui->statusbar);
    author->setText(tr("袁炜聪"));
    ui->statusbar->addPermanentWidget(author);

    ui->actionPaste->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);

    QPlainTextEdit::LineWrapMode mode = ui->textEdit->lineWrapMode();

    if (mode == QTextEdit::NoWrap) {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);

        ui->actionLineWrap->setChecked(false);
    } else {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        ui->actionLineWrap->setChecked(true);
    }
    ui->actionStatusBar->setChecked(true);
    ui->actionToolbar->setChecked(true);

    on_actionShowLineNumber_triggered(false);
    // connect(ui->actionShowLineNumber, SIGNAL(triggered(bool)), ui->textEdit,
    //         SLOT(hideLineNumberArea(bool)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDialog dlg;
    dlg.exec();
}


void MainWindow::on_actionFind_triggered()
{
    SearchDialog dlg(this, ui->textEdit);
    dlg.exec();
}


void MainWindow::on_actionReplace_triggered()
{
    ReplaceDialog dlg(this, ui->textEdit);
    dlg.exec();
}


void MainWindow::on_actionNew_triggered()
{
    if (!userEditConfirmed())
        return;

    filePath = "";
    ui->textEdit->clear();
    this->setWindowTitle(tr("新建文本文件-编辑器"));

    textChanged = false;
}


void MainWindow::on_actionOpen_triggered()
{

    if (!userEditConfirmed())
        return;

    QString filename = QFileDialog::getOpenFileName(this, "打开文件", ".", tr("Text files(*.txt);;All(*.*)"));
    QFile file(filename);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "..", "打开文件失败");
        return;
    }

    filePath = filename;

    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->insertPlainText(text);
    file.close();

    this->setWindowTitle(QFileInfo(filename).absoluteFilePath());

    textChanged = false;

}


// void MainWindow::on_actionSave_triggered()
// {
//     QFile file(filePath);

//     if (!file.open(QFile::WriteOnly | QFile::Text)) {

//         QMessageBox::warning(this, "..", "打开文件失败");

//         QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".",
//                            tr("Text files(*.txt)"));

//         QFile file(filename);

//         if (!file.open(QFile::WriteOnly | QFile::Text)) {
//             QMessageBox::warning(this, "..", "保存文件失败");
//             return;
//         }
//         filePath = filename;

//     }

//     QTextStream out(&file);
//     QString text = ui->textEdit->toPlainText();
//     out << text;
//     file.flush();
//     file.close();

//     this->setWindowTitle(QFileInfo(filePath).absoluteFilePath());
// }

void MainWindow::on_actionSave_triggered()
{
    // 如果还没有设置文件路径，则提示用户选择文件名进行保存
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getSaveFileName(this, "保存文件", ".",
                                                tr("Text files (*.txt);;All files (*)"));
        if (filePath.isEmpty()) {
            // 用户取消保存操作
            return;
        }
    }

    QFile file(filePath);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "错误", "无法打开文件：" + file.errorString());
        return;
    }

    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    if (file.error() != QFile::NoError) {
        QMessageBox::warning(this, "错误", "无法写入文件：" + file.errorString());
        file.close();
        return;
    }
    file.close();

    // 更新窗口标题以显示当前文件路径
    setWindowTitle(QFileInfo(filePath).absoluteFilePath() + "[*] - 编辑器");
    // 标记文件已保存，无需再次提示
    textChanged = false;
}


void MainWindow::on_actionSaveAs_triggered()
{
    // if(textChanged);

    QString filename = QFileDialog::getSaveFileName(this, "保存文件", ".",
                       tr("Text files(*.txt)"));

    QFile file(filename);

    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "..", "保存文件失败");
        return;
    }
    filePath = filename;
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();

    this->setWindowTitle(QFileInfo(filePath).absoluteFilePath());

}


void MainWindow::on_textEdit_textChanged()
{
    if (!textChanged) {
        this->setWindowTitle("*" + this->windowTitle());
        textChanged = true;
    }
    statusLabel.setText("length:" + QString::number(ui->textEdit->toPlainText().length()) + "  lines:" +
                        QString::number(ui->textEdit->document()->lineCount()));
}

bool MainWindow::userEditConfirmed()
{
    if (textChanged) {

        QString path = (filePath != "") ? filePath : "无标题.txt";
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Question);
        msg.setWindowTitle("新建文件");
        msg.setWindowFlag(Qt::Drawer);
        msg.setText((QString("是否将更改保存到\n") + "\"" + path + "\"?"));
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        int r = msg.exec();
        switch (r) {
        case QMessageBox::Yes:
            on_actionSave_triggered();
            break;
        case QMessageBox::No:
            textChanged = false;
            break;
        case QMessageBox::Cancel:
            return false;
        }
    }
    return true;

}


void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}


void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
    ui->actionPaste->setEnabled(true);
}


void MainWindow::on_actionCopy_triggered()
{
    ui->textEdit->copy();
    ui->actionPaste->setEnabled(true);
}


void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}


void MainWindow::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}


void MainWindow::on_textEdit_redoAvailable(bool b)
{
    ui->actionRedo->setEnabled(b);
}


void MainWindow::on_textEdit_copyAvailable(bool b)
{
    ui->actionCopy->setEnabled(b);
    ui->actionCut->setEnabled(b);
}


void MainWindow::on_textEdit_undoAvailable(bool b)
{
    ui->actionUndo->setEnabled(b);
}


void MainWindow::on_actionBackgroundColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
    if (color.isValid()) {
        // 获取当前的样式表
        QString currentStyleSheet = ui->textEdit->styleSheet();
        // 使用正则表达式移除现有的背景色设置
        QRegularExpression bgRegExp(R"(background-color\s*:\s*[^;]+;)");
        currentStyleSheet.replace(bgRegExp, "");
        // 添加新的背景色设置
        ui->textEdit->setStyleSheet(currentStyleSheet + QString("background-color: %1;").arg(color.name()));
    }
}

void MainWindow::on_actionFrontColor_triggered()
{
    QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
    if (color.isValid()) {
        // 获取当前的样式表
        QString currentStyleSheet = ui->textEdit->styleSheet();
        // 使用正则表达式移除现有的前景色设置
        QRegularExpression fgRegExp(R"(color\s*:\s*[^;]+;)");
        currentStyleSheet.replace(fgRegExp, "");
        // 添加新的前景色设置
        ui->textEdit->setStyleSheet(currentStyleSheet + QString("color: %1;").arg(color.name()));
    }
}


void MainWindow::on_actionFontBackgroundColor_triggered()
{

}


// void MainWindow::on_actionBackgroundColor_triggered()
// {
//     QColor color = QColorDialog::getColor(Qt::black, this, "选择颜色");
//     if (color.isValid()) {
//         // 获取当前的样式表
//         QString currentStyleSheet = ui->textEdit->styleSheet();
//         // 使用正则表达式移除现有的背景色设置
//         QRegExp bgRegExp("background-color\\s*:\\s*[^;]+;");
//         currentStyleSheet.replace(bgRegExp, "");
//         // 添加新的背景色设置
//         ui->textEdit->setStyleSheet(currentStyleSheet + QString("background-color: %1;").arg(color.name()));
//     }
// }


void MainWindow::on_actionLineWrap_triggered()
{
    QPlainTextEdit::LineWrapMode mode = ui->textEdit->lineWrapMode();

    if (mode == QTextEdit::NoWrap) {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::WidgetWidth);

        ui->actionLineWrap->setChecked(true);
    } else {
        ui->textEdit->setLineWrapMode(QPlainTextEdit::NoWrap);

        ui->actionLineWrap->setChecked(false);
    }
}


void MainWindow::on_actionFont_triggered()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(&ok, this);

    if (ok)
        ui->textEdit->setFont(font);
}


void MainWindow::on_actionStatusBar_triggered()
{
    bool visible = ui->statusbar->isVisible();
    ui->statusbar->setVisible(!visible);
    ui->actionStatusBar->setChecked(!visible);
}


void MainWindow::on_actionToolbar_triggered()
{
    bool visible = ui->toolBar->isVisible();
    ui->toolBar->setVisible(!visible);
    ui->actionToolbar->setChecked(!visible);
}


void MainWindow::on_actionExit_triggered()
{
    if (userEditConfirmed())
        exit(0);
}


void MainWindow::on_textEdit_cursorPositionChanged()
{
    int col = 0;
    int ln = 0;
    int flg = 0;
    int pos = ui->textEdit->textCursor().position();
    QString text = ui->textEdit->toPlainText();

    for (int i = 0; i < pos; i++) {
        if (text[i] == '\n') {
            ln ++;
            flg = 1;
        }
    }
    flg ++;
    col = pos - flg;
    statusCursorLabel.setText("Ln: " + QString::number(ln + 1) + "  Col:" + QString::number(col + 1));


}

void MainWindow::on_actionShowLineNumber_triggered(bool checked)
{
    ui->textEdit->hideLineNumberArea(!checked);
}

