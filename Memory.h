#ifndef MEMORY_H
#define MEMORY_H

#include <QDialog>
#include <QMainWindow>
#include <QPushButton>
#include <QTime>
#include <QWidget>

class Card : public QWidget {

    Q_OBJECT

public:
    enum EMode {
        HIDDEN,
        SHOWN,
        ACTIVE
    };

private:
    int x, y;
    int N;
    EMode m_eMode;;
    QPushButton *m_Button;

public:
    Card(int i, int j, int n);
    inline int getN() { return N; }
    inline EMode getMode() { return m_eMode; }
    void setMode(EMode e);

public slots:
    void clicked();

signals:
    void selected(int i, int j, int n);

};

class PlayingGrid : public QWidget {

    Q_OBJECT

private:
    int m_iSize;
    int m_iSelX, m_iSelY;
    QTime m_Time;
    QTimer *m_Wait;

public:
    PlayingGrid(int size);

public slots:
    void cardSelected(int x, int y, int n);
    void hideCards();

signals:
    void gameWon(int);

};

class Memory : public QMainWindow {

    Q_OBJECT

private:
    int m_iGridSize;
    bool m_bPlaying;
    QDialog *m_Rules;

public:
    Memory();

public slots:
    void newGame();

    void setEasy();
    void setNormal();
    void setHard();

    void gameWon(int);

    void showRules();
    void about();

};

#endif
