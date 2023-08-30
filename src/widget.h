#ifndef WIDGET_H
#define WIDGET_H

#include <QStatusBar>
#include <QTextEdit>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
   Q_OBJECT

public:
   Widget(QWidget *parent = nullptr);
  ~Widget();

   bool load(const QString &f);

protected:
   void loadIniFile();
   void saveIniFile();

   void setCurrentFileName(const QString &fileName, QString& file);
   bool fileSave(QTextEdit*, QString& file);
   bool fileSaveAs(QTextEdit *, QString& file);

   void translationSlot();
   void translationToMorseSlot();
   void translationFromMorseSlot();
   void init();

   void autoText();


private slots:
   void on_AutoInitialTB_toggled(bool checked);

   void on_CyrillicInitialTB_toggled(bool checked);

   void on_LatinInitialTB_toggled(bool checked);

   void on_MorseInitialTB_toggled(bool checked);

   void on_MorseTranslationTB_toggled(bool checked);

   void on_CyrillicTranslationTB_toggled(bool checked);

   void on_LatinTranslationTB_toggled(bool checked);

   void on_openInitialTB_clicked();

   void on_saveInitialTB_clicked();

   void on_saveTranslationTB_clicked();

   void on_exchangeTB_clicked();

   void textEditInitial_textChanged();

   void on_newInitialTB_clicked();

private:
   Ui::Widget *ui;

   QHash<QString, QPair<QString, QString>> morse;
   QHash<QString, QString> latin;
   QHash<QString, QString> cyrillic;

   QHash<QString, QString> numbers;
   QHash<QString, QString> separator;

   uint initial;
   uint translation;
   QString pahtOpenDialog;
   QString currentFileI;
   QString currentFileT;
};
#endif // WIDGET_H
