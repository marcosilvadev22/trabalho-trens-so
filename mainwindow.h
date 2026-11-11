#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSlider>
#include "trem.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateInterface(int, int, int);

private slots:
    void velocidadeMestreAlterada(int v); 

private:
    Ui::MainWindow *ui;

    static const int NUM_TRENS = 6;

    Trem    *trem[NUM_TRENS];
    QLabel  *labelTrem[NUM_TRENS];
    QSlider *sliderTrem[NUM_TRENS];
    QLabel  *labelValor[NUM_TRENS];
};

#endif 
