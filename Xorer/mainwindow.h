#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void normalizeWindow();

private slots:
    void on_chooseFileButton_released();

    void on_defaultXorMaskRadioButton_clicked();

    void on_customXorMaskEdit_editingFinished();

    void on_customXorMaskRadioButton_clicked();

    void on_customXorMaskEdit_inputRejected();

    void on_customXorMaskEdit_returnPressed();

    void on_chooseFileButton_clicked();

    void on_doXorButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
