#include "widget.h"

#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QApplication>
#include <QFile>
#include <QColorDialog>

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   QTranslator qtTranslator;
   qtTranslator.load("qt_" + QLocale::system().name(),
            QLibraryInfo::location(QLibraryInfo::TranslationsPath));
   a.installTranslator(&qtTranslator);

   QTranslator translator;
   translator.load(":/transfer/morse_translator_ru");
   a.installTranslator(&translator);

   QFile styleF;

   styleF.setFileName(":/qss/qss/style.css");
   styleF.open(QFile::ReadOnly);
   QString qssStr = styleF.readAll();

   a.setStyleSheet(qssStr);

//   QColorDialog  dialog;
//   dialog.exec();

   Widget w;
   w.show();
   return a.exec();
}
