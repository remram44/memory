#include "Memory.h"

#include <QApplication>

#ifndef __WIN32__
#include <unistd.h>
#include <sys/time.h>
#else
#include <windows.h>
#endif

void ssleep(unsigned int secs)
{
#ifdef __WIN32__
    Sleep(secs);
#else
    struct timespec req;
    req.tv_sec = secs;
    req.tv_nsec = 0;
    nanosleep(&req, NULL);
#endif
}

Memory::Memory()
  : m_iGridSize(3), m_bPlaying(false)
{
    setWindowTitle(tr("Memory"));
    {
        QMenu *file = menuBar()->addMenu(tr("&Game"));
        QAction *new_game = file->addAction(tr("&New game..."));
        connect(new_game, SIGNAL(triggered()), this, SLOT(newGame()));
        QAction *quit = file->addAction(tr("&Quit"));
        connect(quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    }
    {
        QMenu *diff = menuBar()->addMenu(tr("&Difficulty"));
        QActionGroup *dg = new QActionGroup(diff);
        QAction *easy = dg->addAction(tr("&Easy"));
        easy->setCheckable(true); diff->addAction(easy); easy->setChecked(true);
        connect(easy, SIGNAL(triggered()), this, SLOT(setEasy()));
        QAction *normal = dg->addAction(tr("&Normal"));
        normal->setCheckable(true); diff->addAction(normal);
        connect(normal, SIGNAL(triggered()), this, SLOT(setNormal()));
        QAction *hard = dg->addAction(tr("&Hard"));
        hard->setCheckable(true); diff->addAction(hard);
        connect(hard, SIGNAL(triggered()), this, SLOT(setHard()));
    }
    {
        QMenu *help = menuBar()->addMenu(tr("&Help"));
        QAction *rules = help->addAction(tr("Game &rules..."));
        connect(rules, SIGNAL(triggered()), this, SLOT(showRules()));
        QAction *aboutQt = help->addAction(tr("About &Qt"));
        connect(aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
        QAction *about = help->addAction(tr("&About Memory"));
        connect(about, SIGNAL(triggered()), this, SLOT(about()));
    }
    show();

    m_Rules = new QDialog(this);
    m_Rules->setWindowTitle(tr("About Memory"));
    QVBoxLayout *l = new QVBoxLayout;
    QTextEdit *text = new QTextEdit;
    text->setReadOnly(true);
    text->setHtml(tr("<h2>Rules</h2>"
        "<p>This game is a <em>remake</em> of the classic game published by "
        "Ravensburger in 1959. The game shows pairs of identical cards placed "
        "randomly and showing their hidden side. You can reveal a card by "
        "clicking on it. When you reveal two cards, if they are not identical, "
        "they are hidden again.</p>"
        
        "<h2>Goal</h2>"
        "<p>The goal is to reveal all the cards (as quickly as possible). "
        "In order to win, you have to remember the position of every card to "
        "be able to associate the pair when you discover the other card.</p>"));
    l->addWidget(text);
    l->setSizeConstraint(QLayout::SetFixedSize);
    m_Rules->setModal(true);
    m_Rules->setLayout(l);
}

void Memory::newGame()
{
    m_bPlaying = true;
    PlayingGrid *game = new PlayingGrid(m_iGridSize);
    connect(game, SIGNAL(gameWon(int)), this, SLOT(gameWon(int)));
    setCentralWidget(game);
    resize(20 + 100 * m_iGridSize, 40 + 120 * (m_iGridSize-1));
}

void Memory::setEasy()
{
    if(m_bPlaying && m_iGridSize != 3)
    {
        m_iGridSize = 3;
        QMessageBox::StandardButton b = QMessageBox::question(this, tr("Abort "
            "current game?"), tr("Do you wish to restart in easy mode?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(b == QMessageBox::Yes)
            newGame();
    }
    else
        m_iGridSize = 3;
}

void Memory::setNormal()
{
    if(m_bPlaying && m_iGridSize != 4)
    {
        m_iGridSize = 4;
        QMessageBox::StandardButton b = QMessageBox::question(this, tr("Abort "
            "current game?"), tr("Do you wish to restart in normal mode?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(b == QMessageBox::Yes)
            newGame();
    }
    else
        m_iGridSize = 4;
}

void Memory::setHard()
{
    if(m_bPlaying && m_iGridSize != 5)
    {
        m_iGridSize = 5;
        QMessageBox::StandardButton b = QMessageBox::question(this, tr("Abort "
            "current game?"), tr("Do you wish to restart in hard mode?"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(b == QMessageBox::Yes)
            newGame();
    }
    else
        m_iGridSize = 5;
}

void Memory::gameWon(int time)
{
    QMessageBox::information(this, tr("Victory"),
        tr("You won the game in %1 seconds.").arg(time));
}

void Memory::showRules()
{
    m_Rules->show();
}

void Memory::about()
{
    QMessageBox::information(this, tr("About Memory..."),
        tr("Memory is a game created by Rémi RAMPIN, aka remram44 "
            "<remirampin@gmail.com>\n"
            "It is not believed to be useful, it was written for educationnal "
            "purpose."));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load(QString("memory_") + QLocale::system().name());
    app.installTranslator(&translator);

    new Memory;

    return app.exec();
}

PlayingGrid::PlayingGrid(int size)
  : m_iSize(size), m_iSelX(-1), m_iSelY(0)
{
    QGridLayout *layout = new QGridLayout;
    setLayout(layout);
    int n;
    for(n = 0; n < size*(size-1); n++)
    {
        // Choose the position of the card
        int r = (int)( ((double)rand()) / RAND_MAX * (size*(size-1)-n) );

        // Looks for the r-th empty cell
        int p = 0;
        while(r >= 0)
        {
            if(layout->itemAtPosition(p/size, p%size) == NULL)
            {
                r--;
                if(r < 0)
                {
                    Card *c = new Card(p/size, p%size, n/2);
                    connect(c, SIGNAL(selected(int, int, int)),
                        this, SLOT(cardSelected(int, int, int)));
                    layout->addWidget(c, p/size, p%size);
                }
            }
            p++;
        }
    }

    m_Wait = new QTimer;
    m_Wait->setSingleShot(true);
    connect(m_Wait, SIGNAL(timeout()), this, SLOT(hideCards()));

    m_Time.start();
}

#define GET_CARD(i, j) static_cast<Card*>(grid->itemAtPosition((i), (j))  \
                        ->widget())

void PlayingGrid::cardSelected(int x, int y, int n)
{
    // Waiting period during which two different cards stay visible (they are
    // then hidden again)
    if(m_Wait->isActive())
        return ;

    QGridLayout *grid = static_cast<QGridLayout*>(layout());
    GET_CARD(x, y)->setMode(Card::ACTIVE);

    if(m_iSelX == -1)
    {
        m_iSelX = x;
        m_iSelY = y;
    }
    else
    {
        if(GET_CARD(m_iSelX, m_iSelY)->getN() == n)
        {
            // TODO : play a sound or something
            GET_CARD(m_iSelX, m_iSelY)->setMode(Card::SHOWN);
            GET_CARD(x, y)->setMode(Card::SHOWN);
            m_iSelX = -1;
            int i, j;
            for(i = 0; i < m_iSize-1; i++)
            {
                for(j = 0; j < m_iSize; j++)
                {
                    if(GET_CARD(i, j)->getMode() == Card::HIDDEN)
                        return ;
                }
            }
            emit gameWon(m_Time.elapsed()/1000);
        }
        else
        {
            m_iSelX = -1;
            m_Wait->start(2000);
        }
    }
}

void PlayingGrid::hideCards()
{
    QGridLayout *grid = static_cast<QGridLayout*>(layout());
    int i, j;
    for(i = 0; i < m_iSize-1; i++)
    {
        for(j = 0; j < m_iSize; j++)
        {
            if(GET_CARD(i, j)->getMode() == Card::ACTIVE)
                GET_CARD(i, j)->setMode(Card::HIDDEN);
        }
    }
}

Card::Card(int i, int j, int n)
  : x(i), y(j), N(n), m_eMode(Card::HIDDEN)
{
    // TODO : graphics
    m_Button = new QPushButton("-", this);
    connect(m_Button, SIGNAL(clicked()),
        this, SLOT(clicked()));
}

void Card::clicked()
{
    if(m_eMode == Card::HIDDEN)
        emit selected(x, y, N);
}

void Card::setMode(EMode e)
{
    m_eMode = e;
    switch(e)
    {
    case Card::HIDDEN:
        m_Button->setText("-");
        break;
    case Card::SHOWN:
        m_Button->setText(QString::number(N));
        break;
    case Card::ACTIVE:
        m_Button->setText(QString::number(N));
        break;
    }

    QFont f = font();
    f.setBold(m_eMode == Card::ACTIVE);
    setFont(f);
}
