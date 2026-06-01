#include "gameselector.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    GameSelector w;
    w.show();
    return a.exec();
}
