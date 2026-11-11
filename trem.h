#ifndef TREM_H
#define TREM_H

#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QVector>
#include <QPoint>
#include <atomic>

extern QMutex regiao[9];
extern QSemaphore semCiclo123;
extern QSemaphore semCiclo356;
extern QSemaphore semCiclo458;
extern QSemaphore semCiclo679;


class Trem : public QThread {
    Q_OBJECT
public:
    Trem(int ID, int x, int y);
    void run() override;               

    void setVelocidade(int v);        
    int  getVelocidade() const;

    static const int VEL_MAX = 200;
    static const int TAMANHO = 20;    

signals:
    void updateGUI(int, int, int);     

private:
    void montarPercurso();             

    int x;                             
    int y;                             
    int ID;                            
    std::atomic<int> velocidade;       

    QVector<QPoint> percurso;          
    QVector<int>    mascaraRegioes;    
    QVector<int>    mascaraSemaforos;  
};

#endif 
