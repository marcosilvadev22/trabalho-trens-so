#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {    
    ui->setupUi(this);

    labelTrem[0] = ui->label_trem1;  sliderTrem[0] = ui->slider_a;  labelValor[0] = ui->valor_a;
    labelTrem[1] = ui->label_trem2;  sliderTrem[1] = ui->slider_b;  labelValor[1] = ui->valor_b;
    labelTrem[2] = ui->label_trem3;  sliderTrem[2] = ui->slider_c;  labelValor[2] = ui->valor_c;
    labelTrem[3] = ui->label_trem4;  sliderTrem[3] = ui->slider_d;  labelValor[3] = ui->valor_d;
    labelTrem[4] = ui->label_trem5;  sliderTrem[4] = ui->slider_e;  labelValor[4] = ui->valor_e;
    labelTrem[5] = ui->label_trem6;  sliderTrem[5] = ui->slider_f;  labelValor[5] = ui->valor_f;

    // trem 0 representa o trem a, 1 o trem b e assim por diante ate o trem f.
    trem[0] = new Trem(1, 430,  40);   
    trem[1] = new Trem(2, 170, 250);   
    trem[2] = new Trem(3, 690, 250);   
    trem[3] = new Trem(4,  40, 390);   
    trem[4] = new Trem(5, 430, 460);   
    trem[5] = new Trem(6, 820, 390);   

        for (int i = 0; i < NUM_TRENS; ++i) {
            connect(trem[i], SIGNAL(updateGUI(int,int,int)), SLOT(updateInterface(int,int,int)));
            sliderTrem[i]->setRange(0, Trem::VEL_MAX);
            Trem *t = trem[i];
            QLabel *lv = labelValor[i];
            connect(sliderTrem[i], &QSlider::valueChanged, this, [t, lv](int v) {
                t->setVelocidade(v);
                lv->setNum(v);
            });
        }

    ui->slider_m->setRange(0, Trem::VEL_MAX);
    connect(ui->slider_m, &QSlider::valueChanged, this, &MainWindow::velocidadeMestreAlterada);

    const int velInicial = 100;
    ui->slider_m->setValue(velInicial);
    velocidadeMestreAlterada(velInicial);
        for (int i = 0; i < NUM_TRENS; ++i)
            trem[i]->start();
}

void MainWindow::velocidadeMestreAlterada(int v) {
    ui->valor_m->setNum(v);
        for (int i = 0; i < NUM_TRENS; ++i) {
            sliderTrem[i]->setValue(v);
            trem[i]->setVelocidade(v);
            labelValor[i]->setNum(v);
        }
}

void MainWindow::updateInterface(int id, int x, int y) {
    if (id >= 1 && id <= NUM_TRENS)
        labelTrem[id - 1]->setGeometry(x - Trem::TAMANHO / 2, y - Trem::TAMANHO / 2, Trem::TAMANHO, Trem::TAMANHO);
}

MainWindow::~MainWindow()
{
    for (int i = 0; i < NUM_TRENS; ++i) {
        disconnect(trem[i], 0, this, 0);
        trem[i]->terminate();
        trem[i]->wait();
        delete trem[i];
    }

    delete ui;
}
