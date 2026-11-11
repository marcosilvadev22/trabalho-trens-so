#include "trem.h"
#include <QtCore>
#include <cstdlib>

QMutex regiao[9];               
QSemaphore semCiclo123(2);      
QSemaphore semCiclo356(2);      
QSemaphore semCiclo458(2);      
QSemaphore semCiclo679(2);      

static QSemaphore *const semCiclo[4] = { 
    &semCiclo123, &semCiclo356, &semCiclo458, &semCiclo679 
};
    
enum { 
    C123 = 0, C356 = 1, C458 = 2, C679 = 3 
};

namespace {

const int PASSO = 5;  

struct Retangulo { 
    int x0, y0, x1, y1; 
};

const Retangulo MALHA[6] = {
    {170,  40, 690, 180},   
    {170, 180, 430, 320},   
    {430, 180, 690, 320},   
    { 40, 320, 300, 460},   
    {300, 320, 560, 460},   
    {560, 320, 820, 460},   
};

struct Segmento { 
    int x1, y1, x2, y2; 
};

const Segmento SEG_REGIAO[9] = {
    {170, 180, 430, 180},   
    {430, 180, 690, 180},  
    {430, 180, 430, 320},  
    {170, 320, 300, 320},   
    {300, 320, 430, 320},   
    {430, 320, 560, 320}, 
    {560, 320, 690, 320},   
    {300, 320, 300, 460},  
    {560, 320, 560, 460},  
};

inline int bit(int r) { 
    return 1 << (r - 1); 
}

const int REGIOES_DO_TREM[6] = {
    bit(1) | bit(2),                       
    bit(1) | bit(3) | bit(4) | bit(5),     
    bit(2) | bit(3) | bit(6) | bit(7),     
    bit(4) | bit(8),                       
    bit(5) | bit(6) | bit(8) | bit(9),     
    bit(7) | bit(9),                       
};


struct Regra { 
    int ciclo; int regiaoX; bool naEntradaDaCadeia; 
};
const Regra REGRAS_a[] = {
     {C123, 1, false} 
    };
const Regra REGRAS_b[] = { 
    {C458, 4, true}, {C356, 5, true}, {C123, 3, false} 
};
const Regra REGRAS_c[] = {
     {C123, 2, true}, {C356, 3, true}, {C679, 6, false} 
    };
const Regra REGRAS_d[] = {
     {C458, 8, false} 
    };
const Regra REGRAS_e[] = {
     {C679, 9, true}, {C356, 6, true}, {C458, 5, false} 
    };
const Regra REGRAS_f[] = {
     {C679, 7, false} 
    };

struct ListaRegras {
     const Regra *r; int n; 
    };
const ListaRegras REGRAS[6] = {
    {REGRAS_a, 1}, {REGRAS_b, 3}, {REGRAS_c, 3},
    {REGRAS_d, 1}, {REGRAS_e, 3}, {REGRAS_f, 1},
};

int distancia(const QPoint &p, const Segmento &s) {
    int cx = qBound(qMin(s.x1, s.x2), p.x(), qMax(s.x1, s.x2));
    int cy = qBound(qMin(s.y1, s.y2), p.y(), qMax(s.y1, s.y2));
    return qMax(std::abs(p.x() - cx), std::abs(p.y() - cy));
}

} 

Trem::Trem(int ID, int x, int y) : velocidade(0) {
    this->ID = ID;
    this->x = x;
    this->y = y;
    montarPercurso();
}

void Trem::setVelocidade(int v) {
    velocidade.store(qBound(0, v, VEL_MAX));
}

int Trem::getVelocidade() const {
    return velocidade.load();
}


void Trem::montarPercurso() {
    const Retangulo &m = MALHA[ID - 1];
    int px = x, py = y;
    do {
        percurso.append(QPoint(px, py));
        if (py == m.y0 && px > m.x0)      px -= PASSO;
        else if (px == m.x0 && py < m.y1) py += PASSO;
        else if (py == m.y1 && px < m.x1) px += PASSO;
        else                              py -= PASSO;
    } while (px != x || py != y);

    const int n = percurso.size();
    mascaraRegioes.fill(0, n);
    mascaraSemaforos.fill(0, n);
    for (int i = 0; i < n; ++i)
        for (int r = 1; r <= 9; ++r)
            if ((REGIOES_DO_TREM[ID - 1] & bit(r)) &&
                distancia(percurso[i], SEG_REGIAO[r - 1]) < TAMANHO)
                mascaraRegioes[i] |= bit(r);

    Q_ASSERT(mascaraRegioes[0] == 0);

    int inicioCadeia = 0;
    while (inicioCadeia < n && mascaraRegioes[inicioCadeia] == 0) ++inicioCadeia;

    const ListaRegras &lr = REGRAS[ID - 1];
    for (int k = 0; k < lr.n; ++k) {
        const Regra &rg = lr.r[k];
        int primeiro = -1, ultimo = -1;
        for (int i = 0; i < n; ++i)
            if (mascaraRegioes[i] & bit(rg.regiaoX)) {
                if (primeiro < 0) primeiro = i;
                ultimo = i;
            }
        int inicio = rg.naEntradaDaCadeia ? inicioCadeia : primeiro;
        for (int i = inicio; i <= ultimo; ++i)
            mascaraSemaforos[i] |= (1 << rg.ciclo);
    }
}

void Trem::run() {
    const int n = percurso.size();
    int idx = 0;              
    int regioesTravadas = 0; 
    int semaforosTidos  = 0;  

    emit updateGUI(ID, x, y);

    while (true) {
        const int v = velocidade.load();
        if (v <= 0) {        
            msleep(20);
            continue;
        }

        const int prox = (idx + 1) % n;
        const int novosSem = mascaraSemaforos[prox] & ~semaforosTidos;
        for (int c = 0; c < 4; ++c)
            if (novosSem & (1 << c)) semCiclo[c]->acquire();
        semaforosTidos |= novosSem;

        const int novasReg = mascaraRegioes[prox] & ~regioesTravadas;
        for (int r = 1; r <= 9; ++r)
            if (novasReg & bit(r)) regiao[r - 1].lock();
        regioesTravadas |= novasReg;

        // --- Move ---
        idx = prox;
        x = percurso[idx].x();
        y = percurso[idx].y();
        emit updateGUI(ID, x, y);

        const int sairReg = regioesTravadas & ~mascaraRegioes[idx];
        for (int r = 1; r <= 9; ++r)
            if (sairReg & bit(r)) regiao[r - 1].unlock();
        regioesTravadas &= ~sairReg;

        const int sairSem = semaforosTidos & ~mascaraSemaforos[idx];
        for (int c = 0; c < 4; ++c)
            if (sairSem & (1 << c)) semCiclo[c]->release();
        semaforosTidos &= ~sairSem;
        usleep(200000UL / static_cast<unsigned long>(velocidade.load() > 0 ? velocidade.load() : 1));
    }
}
