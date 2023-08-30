#include "widget.h"
#include "ui_widget.h"

#include <QButtonGroup>
#include <QDir>
#include <QFileDialog>
#include <QSettings>
#include <QStatusBar>
#include <QTextCodec>
#include <QTextDocumentWriter>
#include <QMimeData>
#include <QMimeDatabase>
#include <QDebug>
#include <QMessageBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget), initial(0), translation(0)
{
    ui->setupUi(this);


    connect(ui->textEditInitial, &QTextEdit::textChanged,
            this, &Widget::textEditInitial_textChanged);

    QButtonGroup * buttonGroupInitial = new QButtonGroup(this);
    buttonGroupInitial->addButton(ui->AutoInitialTB);
    buttonGroupInitial->addButton(ui->CyrillicInitialTB);
    buttonGroupInitial->addButton(ui->LatinInitialTB);
    buttonGroupInitial->addButton(ui->MorseInitialTB);

    QButtonGroup * buttonGroupTranslation = new QButtonGroup(this);
    buttonGroupTranslation->addButton(ui->CyrillicTranslationTB);
    buttonGroupTranslation->addButton(ui->LatinTranslationTB);
    buttonGroupTranslation->addButton(ui->MorseTranslationTB);

    ui->pbText->setObjectName("bigButton");
    ui->pbImages->setObjectName("bigButton");
    ui->pbDocs->setObjectName("bigButton");
    ui->pbSites->setObjectName("bigButton");

    loadIniFile();
    init();
}

Widget::~Widget()
{
    saveIniFile();
    delete ui;
}

bool Widget::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        QUrl baseUrl = (f.front() == QLatin1Char(':') ? QUrl(f) : QUrl::fromLocalFile(f)).adjusted(QUrl::RemoveFilename);
        ui->textEditInitial->document()->setBaseUrl(baseUrl);
        ui->textEditInitial->setHtml(str);
    } else {
#if QT_CONFIG(textmarkdownreader)
        QMimeDatabase db;
        if (db.mimeTypeForFileNameAndData(f, data).name() == QLatin1String("text/markdown"))
            ui->textEditInitial->setMarkdown(QString::fromUtf8(data));
        else
#endif
            ui->textEditInitial->setPlainText(QString::fromUtf8(data));
    }

    setCurrentFileName(f,currentFileI);
    return true;
}

void Widget::loadIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/morse_translator/morse_translator.ini"), QSettings::IniFormat);
    setGeometry( settings.value("MainWin/geometry").toRect());

    ui->AutoInitialTB->setChecked(settings.value( "MainWin/AutoInitialTB").toBool());
    ui->CyrillicInitialTB->setChecked(settings.value( "MainWin/CyrillicInitialTB").toBool());
    ui->LatinInitialTB->setChecked(settings.value( "MainWin/LatinInitialTB").toBool());
    ui->MorseInitialTB->setChecked(settings.value( "MainWin/MorseInitialTB").toBool());

    ui->CyrillicTranslationTB->setChecked(settings.value( "MainWin/CyrillicTranslationTB").toBool());
    ui->LatinTranslationTB->setChecked(settings.value( "MainWin/LatinTranslationTB").toBool());
    ui->MorseInitialTB->setChecked(settings.value( "MainWin/MorseInitialTB").toBool());

    pahtOpenDialog = settings.value("MainWin/pahtOpenDialog", QDir::home().absolutePath()).toString();
}

void Widget::saveIniFile()
{
    QSettings settings(QDir::home().absoluteFilePath(".config/morse_translator/morse_translator.ini"), QSettings::IniFormat);
    settings.setValue  ("MainWin/geometry", geometry() );

    settings.setValue ( "MainWin/AutoInitialTB", ui->AutoInitialTB->isChecked() );
    settings.setValue ( "MainWin/CyrillicInitialTB", ui->CyrillicInitialTB->isChecked() );
    settings.setValue ( "MainWin/LatinInitialTB", ui->LatinInitialTB->isChecked() );
    settings.setValue ( "MainWin/MorseInitialTB", ui->MorseInitialTB->isChecked() );

    settings.setValue ( "MainWin/CyrillicTranslationTB", ui->CyrillicTranslationTB->isChecked() );
    settings.setValue ( "MainWin/LatinTranslationTB", ui->LatinTranslationTB->isChecked() );
    settings.setValue ( "MainWin/MorseInitialTB", ui->MorseInitialTB->isChecked() );

    settings.setValue ( "MainWin/pahtOpenDialog", pahtOpenDialog );
}

void Widget::setCurrentFileName(const QString &fileName, QString& file)
{
    file = fileName;
    ui->textEditInitial->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

bool Widget::fileSave(QTextEdit * textEdit, QString &file)
{
    if (file.isEmpty())
        return fileSaveAs(textEdit, file);

    QTextDocumentWriter writer(file);
    bool success = writer.write(textEdit->document());
    if (success)
        textEdit->document()->setModified(false);        
    else
        QMessageBox::warning(this, "Attention",tr("Could not write to file \"%1\"").arg(QDir::toNativeSeparators(file)));

    return success;
}

bool Widget::fileSaveAs(QTextEdit * textEdit, QString &file)
{
    QFileDialog fileDialog(this, tr("Save as..."), pahtOpenDialog);
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "text/plain"
             #if QT_CONFIG(textodfwriter)
              << "application/vnd.oasis.opendocument.text"
             #endif
             #if QT_CONFIG(textmarkdownwriter)
              << "text/markdown"
             #endif
              << "text/html";
    fileDialog.setMimeTypeFilters(mimeTypes);
#if QT_CONFIG(textodfwriter)
    fileDialog.setDefaultSuffix("odt");
#endif
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    pahtOpenDialog = QFileInfo(fn).absolutePath();
    setCurrentFileName(fn, file);
    return fileSave(textEdit, file);
}

void Widget::translationSlot()
{
    switch (initial)
    {
    case 4:{translationToMorseSlot();break;}
    case 5:{translationToMorseSlot();break;}
    case 6:{translationFromMorseSlot();break;}
    }

}

void Widget::translationToMorseSlot()
{
    QString tr = ui->textEditInitial->toPlainText();
    if(tr.isEmpty()) return;

    tr.replace(".", "...... ", Qt::CaseInsensitive);

    {
        QHashIterator<QString, QString> i(separator);
        while (i.hasNext()) {
            i.next();
            tr.replace(i.key(), i.value(),Qt::CaseInsensitive);
        }
    }
    //if(initial == 5)
    {
        QHashIterator<QString, QString> i(latin);
        while (i.hasNext()) {
            i.next();
            tr.replace(i.key(), i.value(),Qt::CaseInsensitive);
        }
    }
    //if(initial == 4)
    {
        QHashIterator<QString, QString> i(cyrillic);
        while (i.hasNext()) {
            i.next();
            tr.replace(i.key(), i.value(),Qt::CaseInsensitive);
        }
    }
    ui->textEditTranslation->setText(tr);
}

void Widget::translationFromMorseSlot()
{
    QString tr = ui->textEditInitial->toPlainText();
    if(tr.isEmpty()) return;
    QString rez;
    QStringList lineList = tr.split("\n");
    for(QString line : lineList)
    {

        QStringList wordsList = line.split("  ");
        for(QString words : wordsList)
        {

            QStringList letterList = words.split(" ");
            for(QString letter : letterList)
            {

                if(morse.contains(letter))
                    rez += ((translation == 1)?morse[letter].first:morse[letter].second);
                else
                    rez += letter;
            }
            if(wordsList.count() > 1)
                rez += " ";
        }
        if(lineList.count() > 1)
            rez += "\n";
    }

    ui->textEditTranslation->setText(rez);
}

void Widget::init()
{
    morse[QString(".-")  ] =   qMakePair(QString("А"),QString("A"));
    morse[QString("-...")] =   qMakePair(QString("Б"),QString("B"));
    morse[QString(".--") ] =   qMakePair(QString("В"),QString("W"));
    morse[QString("--.") ] =   qMakePair(QString("Г"),QString("G"));
    morse[QString("-..") ] =   qMakePair(QString("Д"),QString("D"));
    morse[QString(".")   ] =   qMakePair(QString("Е"),QString("E"));
    morse[QString("...-")] =   qMakePair(QString("Ж"),QString("V"));
    morse[QString("--..")] =   qMakePair(QString("З"),QString("Z"));
    morse[QString("..")  ] =   qMakePair(QString("И"),QString("I"));
    morse[QString(".---")] =   qMakePair(QString("Й"),QString("J"));
    morse[QString("-.-") ] =   qMakePair(QString("К"),QString("K"));
    morse[QString(".-..")] =   qMakePair(QString("Л"),QString("L"));
    morse[QString("--")  ] =   qMakePair(QString("М"),QString("M"));
    morse[QString("-.")  ] =   qMakePair(QString("Н"),QString("N"));
    morse[QString("---") ] =   qMakePair(QString("О"),QString("O"));
    morse[QString(".--.")] =   qMakePair(QString("П"),QString("P"));
    morse[QString(".-.") ] =   qMakePair(QString("Р"),QString("R"));
    morse[QString("...") ] =   qMakePair(QString("С"),QString("S"));
    morse[QString("-")   ] =   qMakePair(QString("Т"),QString("T"));
    morse[QString("..-") ] =   qMakePair(QString("У"),QString("U"));
    morse[QString("..-.")] =   qMakePair(QString("Ф"),QString("F"));
    morse[QString("....")] =   qMakePair(QString("Х"),QString("H"));
    morse[QString("-.-.")] =   qMakePair(QString("Ц"),QString("C"));
    morse[QString("--.-")] =   qMakePair(QString("Щ"),QString("Q"));
    morse[QString("-..-")] =   qMakePair(QString("Ь"),QString("X"));
    morse[QString("-.--")] =   qMakePair(QString("Ы"),QString("Y"));

    morse[QString("---.")] =   qMakePair(QString("Ч"),QString("H"));
    morse[QString("----")] =   qMakePair(QString("Ш"),QString("SH"));
    morse[QString("..-..")] =  qMakePair(QString("Э"),QString("E"));
    morse[QString("..--")] =   qMakePair(QString("Ю"),QString("YU"));
    morse[QString(".-.-")] =   qMakePair(QString("Я"),QString("YA"));

    morse[QString(".----")] =   qMakePair(QString("1"),QString("1"));
    morse[QString("..---")] =   qMakePair(QString("2"),QString("2"));
    morse[QString("...--")] =   qMakePair(QString("3"),QString("3"));
    morse[QString("....-")] =   qMakePair(QString("4"),QString("4"));
    morse[QString(".....")] =   qMakePair(QString("5"),QString("5"));
    morse[QString("-....")] =   qMakePair(QString("6"),QString("6"));
    morse[QString("--...")] =   qMakePair(QString("7"),QString("7"));
    morse[QString("---..")] =   qMakePair(QString("8"),QString("8"));
    morse[QString("----.")] =   qMakePair(QString("9"),QString("9"));
    morse[QString("-----")] =   qMakePair(QString("0"),QString("0"));
    morse[QString("......")] =  qMakePair(QString("."),QString("."));
    morse[QString(".-.-.-")] =  qMakePair(QString(","),QString(","));
    morse[QString("-..-." )] =  qMakePair(QString("/"),QString("/"));
    morse[QString("..--..")] =  qMakePair(QString("?"),QString("?"));
    morse[QString("--..--")] =  qMakePair(QString("!"),QString("!"));
    morse[QString(".--.-.")] =  qMakePair(QString("@"),QString("@"));


    latin["A"] = QString(".- ");
    latin["B"] = QString("-... ");
    latin["W"] = QString(".-- ");
    latin["G"] = QString("--. ");
    latin["D"] = QString("-.. ");
    latin["E"] = QString(". ");
    latin["V"] = QString("...- ");
    latin["Z"] = QString("--.. ");
    latin["I"] = QString(".. ");
    latin["J"] = QString(".--- ");
    latin["K"] = QString("-.- ");
    latin["L"] = QString(".-.. ");
    latin["M"] = QString("-- ");
    latin["N"] = QString("-. ");
    latin["O"] = QString("--- ");
    latin["P"] = QString(".--. ");
    latin["R"] = QString(".-. ");
    latin["S"] = QString("... ");
    latin["T"] = QString("- ");
    latin["U"] = QString("..- ");
    latin["F"] = QString("..-. ");
    latin["H"] = QString(".... ");
    latin["C"] = QString("-.-. ");
    latin["Q"] = QString("--.- ");
    latin["X"] = QString("-..- ");
    latin["Y"] = QString("-.-- ");

    cyrillic["А"] = QString(".- ");
    cyrillic["Б"] = QString("-... ");
    cyrillic["В"] = QString(".-- ");
    cyrillic["Г"] = QString("--. ");
    cyrillic["Д"] = QString("-.. ");
    cyrillic["Е"] = QString(". ");
    cyrillic["Ё"] = QString(". ");
    cyrillic["Ж"] = QString("...- ");
    cyrillic["З"] = QString("--.. ");
    cyrillic["И"] = QString(".. ");
    cyrillic["Й"] = QString(".--- ");
    cyrillic["К"] = QString("-.- ");
    cyrillic["Л"] = QString(".-.. ");
    cyrillic["М"] = QString("-- ");
    cyrillic["Н"] = QString("-. ");
    cyrillic["О"] = QString("--- ");
    cyrillic["П"] = QString(".--. ");
    cyrillic["Р"] = QString(".-. ");
    cyrillic["С"] = QString("... ");
    cyrillic["Т"] = QString("- ");
    cyrillic["У"] = QString("..- ");
    cyrillic["Ф"] = QString("..-. ");
    cyrillic["Х"] = QString(".... ");
    cyrillic["Ц"] = QString("-.-. ");
    cyrillic["Щ"] = QString("--.- ");
    cyrillic["Ь"] = QString("-..- ");
    cyrillic["Ы"] = QString("-.-- ");
    cyrillic["Ч"] = QString("---. ");
    cyrillic["Ш"] = QString("---- ");
    cyrillic["Э"] = QString("..-.. ");
    cyrillic["Ю"] = QString("..-- ");
    cyrillic["Я"] = QString(".-.- ");

    separator["1"] = QString(".---- ");
    separator["2"] = QString("..--- ");
    separator["3"] = QString("...-- ");
    separator["4"] = QString("....- ");
    separator["5"] = QString("..... ");
    separator["6"] = QString("-.... ");
    separator["7"] = QString("--... ");
    separator["8"] = QString("---.. ");
    separator["9"] = QString("----. ");
    separator["0"] = QString("----- ");

    separator[","] = QString(".-.-.- ");
    separator["/"] = QString("-..-. ");
    separator["?"] = QString("..--.. ");
    separator["!"] = QString("--..-- ");
    separator["@"] = QString(".--.-. ");

   // ui->textEditTranslation->setPlaceholderText("Перевод");
}

void Widget::autoText()
{
    if(!ui->textEditInitial->toPlainText().isEmpty())
    {

        int countCyrillic = 0;
        int countLatin = 0;
        for(QChar ch : ui->textEditInitial->toPlainText())
        {
            if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' &&ch <= 'z'))
            {
                ++countLatin;
            }
            else if ((ch >= "А" && ch <= "Я") || (ch >= "а" && ch <= "я") || (ch == "Ё") || (ch == "ё"))
            {
                ++countCyrillic;
            }
        }
        if((ui->textEditInitial->toPlainText().count("--") > 0)||
                (ui->textEditInitial->toPlainText().count(".-") > 0)||
                (ui->textEditInitial->toPlainText().count("-.") > 0)||
                (ui->textEditInitial->toPlainText().count("..") > 0))
        {
            ui->MorseInitialTB->setChecked(true);
        }
        else
        {
            if(countLatin > countCyrillic)
            {
                ui->LatinInitialTB->setChecked(true);
            }
            else
            {
                ui->CyrillicInitialTB->setChecked(true);
            }
        }
    }
}


void Widget::on_AutoInitialTB_toggled(bool checked)
{
    if(checked)
    {    
        ui->CyrillicTranslationTB->setEnabled(false);
        ui->LatinTranslationTB->setEnabled(false);
        ui->MorseTranslationTB->setEnabled(false);
        autoText();
    }
}

void Widget::on_CyrillicInitialTB_toggled(bool checked)
{
    if(checked)
    {
        initial = 4;
        ui->AutoInitialTB->setChecked(false);

        ui->LatinInitialTB->setChecked(false);
        ui->MorseInitialTB->setChecked(false);


        ui->CyrillicTranslationTB->setEnabled(false);
        ui->LatinTranslationTB->setEnabled(false);

        ui->MorseTranslationTB->setEnabled(true);
        ui->MorseTranslationTB->setChecked(true);
        textEditInitial_textChanged();
    }
}

void Widget::on_LatinInitialTB_toggled(bool checked)
{
    if(checked)
    {
        initial = 5;
        ui->AutoInitialTB->setChecked(false);
        ui->CyrillicInitialTB->setChecked(false);

        ui->MorseInitialTB->setChecked(false);

        ui->CyrillicTranslationTB->setEnabled(false);
        ui->LatinTranslationTB->setEnabled(false);

        ui->MorseTranslationTB->setEnabled(true);
        ui->MorseTranslationTB->setChecked(true);
        textEditInitial_textChanged();
    }
}

void Widget::on_MorseInitialTB_toggled(bool checked)
{
    if(checked)
    {

        initial = 6;
        ui->AutoInitialTB->setChecked(false);
        ui->CyrillicInitialTB->setChecked(false);
        ui->LatinInitialTB->setChecked(false);

        ui->MorseTranslationTB->setEnabled(false);
        ui->CyrillicTranslationTB->setChecked(true);
        ui->CyrillicTranslationTB->setEnabled(true);
        ui->LatinTranslationTB->setChecked(false);
        ui->LatinTranslationTB->setEnabled(true);
        textEditInitial_textChanged();
    }
}

void Widget::on_MorseTranslationTB_toggled(bool checked)
{
    if(checked)
    {
        translation = 3;
        textEditInitial_textChanged();
    }
}

void Widget::on_CyrillicTranslationTB_toggled(bool checked)
{
    if(checked)
    {
        translation = 1;
        textEditInitial_textChanged();
    }
}

void Widget::on_LatinTranslationTB_toggled(bool checked)
{
    if(checked)
    {
        translation = 2;
        textEditInitial_textChanged();
    }
}


void Widget::on_newInitialTB_clicked()
{
    ui->textEditInitial->clear();
    ui->textEditTranslation->clear();
    setCurrentFileName(QString(), currentFileI);
}

void Widget::on_openInitialTB_clicked()
{
    QFileDialog fileDialog(this, tr("Open File..."),pahtOpenDialog);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList()
                              #if QT_CONFIG(texthtmlparser)
                                  << "text/html"
                              #endif
                              #if QT_CONFIG(textmarkdownreader)
                                  << "text/markdown"
                              #endif
                                  << "text/plain");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if (load(fn))
    {       
        textEditInitial_textChanged();
    }
    else
        QMessageBox::warning(this, "Attention",tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));


    pahtOpenDialog = QFileInfo(fn).absolutePath();
}

void Widget::on_saveInitialTB_clicked()
{
    fileSave(ui->textEditInitial, currentFileI);
}

void Widget::on_saveTranslationTB_clicked()
{
    fileSave(ui->textEditTranslation, currentFileT);
}

void Widget::on_exchangeTB_clicked()
{
    QString text = ui->textEditTranslation->toPlainText();
    ui->textEditTranslation->setText("");
    disconnect(ui->textEditInitial, &QTextEdit::textChanged,
               this, &Widget::textEditInitial_textChanged);
    ui->textEditInitial->setText(text);
    ui->AutoInitialTB->setChecked(true);
    translationSlot();
    connect(ui->textEditInitial, &QTextEdit::textChanged,
            this, &Widget::textEditInitial_textChanged);
}

void Widget::textEditInitial_textChanged()
{
    ui->textEditTranslation->setText("");
    if(ui->AutoInitialTB->isChecked()) autoText();
    translationSlot();
}

/**
Знак раздела  -...-
Ошибка/перебой ........
  **/


