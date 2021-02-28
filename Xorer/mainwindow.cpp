#include <memory>
#include <cassert>
#include <climits>
#include <map>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QIODevice>

#include "mainwindow.h"
#include "ui_mainwindow.h"


typedef unsigned long long MaskType;

template <typename T>
T swap_endian(T u)
{
    //static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

    union
    {
        T u;
        unsigned char u8[sizeof(T)];
    } source, dest;

    source.u = u;

    for (size_t k = 0; k < sizeof(T); k++)
        dest.u8[k] = source.u8[sizeof(T) - k - 1];

    return dest.u;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    normalizeWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::normalizeWindow()
{
    if ( ui->defaultXorMaskRadioButton->isChecked() ) {
        ui->customXorMaskRadioButton->setChecked( false );
        ui->customXorMaskEdit->setEnabled( false );
        ui->defaultXorMaskLabel->setEnabled( true );
    }
    if ( ui->customXorMaskRadioButton->isChecked() ) {
        ui->defaultXorMaskRadioButton->setChecked( false );
        ui->defaultXorMaskLabel->setEnabled( false );
        ui->customXorMaskEdit->setEnabled( true );
    }
    if (    (  ui->customXorMaskEdit->text().size() % 2  ) == 1    ) {
        ui->customXorMaskEdit->setText( ui->customXorMaskEdit->text() + '0' );
    }
    ui->customXorMaskEdit->setCursorPosition(0);
    ui->doXorButton->setEnabled(   QFile( ui->choosedFileOutputLineEdit->text() ).exists()   );
}

void MainWindow::on_chooseFileButton_released()
{
    normalizeWindow();
}

void MainWindow::on_defaultXorMaskRadioButton_clicked()
{
    normalizeWindow();
}

void MainWindow::on_customXorMaskEdit_editingFinished()
{
    normalizeWindow();
}

void MainWindow::on_customXorMaskRadioButton_clicked()
{
    normalizeWindow();
}

void MainWindow::on_customXorMaskEdit_inputRejected()
{
    normalizeWindow();
}

void MainWindow::on_customXorMaskEdit_returnPressed()
{
    //Qt::FocusReason fr;
    //ui->customXorMaskEdit->setFocus( Qt::OtherFocusReason  );

    normalizeWindow();
    ui->customXorMaskEdit->setEnabled( false );
}

void MainWindow::on_chooseFileButton_clicked()
{
    std::unique_ptr< QFileDialog > file_dial ( new QFileDialog( this ) );
    file_dial->selectFile( ui->choosedFileOutputLineEdit->text() );
    file_dial->setFileMode( QFileDialog::ExistingFile );
    if ( file_dial->exec() ) {
      ui->choosedFileOutputLineEdit->setText( file_dial->selectedFiles()[0] );
      normalizeWindow();
    }
}

typedef unsigned char Byte;


void doXor( Byte * mask, size_t mask_size, std::vector<Byte> & to_xor ) {
    Byte * mask_iterator = mask;
    Byte * mask_postend = mask + mask_size;
    for ( Byte & elem : to_xor ) {
        if ( mask_iterator == mask_postend ) mask_iterator = mask;
        elem ^= *mask_iterator;
        ++mask_iterator;
    }
}

void doXor( const QString name, const std::vector<Byte> & mask ) {
  //int x = 0;
  //qDebug("mask = %llX", mask);
  //qDebug("mask = %X", mask[0] );
  QFile file( name );
  file.open( QFile::ReadWrite/*|QIODevice::Unbuffered*/ );
  auto file_size = file.size();
  std::vector<Byte> cached( file_size, (Byte)0 );
  file.read( (char *)cached.data(), file_size );
  doXor( (Byte *)mask.data(), mask.size(), cached );
  //file.resize(0ull);
  file.seek( 0ull );
  file.write( (const char *) cached.data(), cached.size() );
}

bool convertHex( const QString to_convert, std::vector<Byte> & result ) {
    static const std::map<QChar, Byte> kDigitChars{
        { '0', 0 },
        { '1', 1 },
        { '2', 2 },
        { '3', 3 },
        { '4', 4 },
        { '5', 5 },
        { '6', 6 },
        { '7', 7 },
        { '8', 8 },
        { '9', 9 },
        { 'a', 10 },
        { 'b', 11 },
        { 'c', 12 },
        { 'd', 13 },
        { 'e', 14 },
        { 'f', 15 },
        { 'A', 10 },
        { 'B', 11 },
        { 'C', 12 },
        { 'D', 13 },
        { 'E', 14 },
        { 'F', 15 },
     };
    assert ( (to_convert.size() % 2) == 0 );
    result.reserve( to_convert.size()/2 );
    bool parity = 0;
    Byte current = 0;
    //auto iterator = result.begin();
    for ( auto & elem: to_convert ) {
        if ( elem == ' ' ) continue;
        if ( !kDigitChars.count(elem) ) {
            return 0;
        }
        Byte digit = kDigitChars.at( elem );
        current += digit;
        if ( parity == 0 ) {
            current =  current << 4;
        } else {
            //*iterator = current;
            //++iterator;
            result.push_back( current );
            current = 0;
        }
        parity ^= 1;
    }
    return 1;
}

void doXorUI( const QString name, const QString mask )
{
    bool success = 0;

    std::vector<Byte> mask_vector;
    success = convertHex( mask, mask_vector );

    size_t mask_size = mask_vector.size();
    if( success ) {
        if ( !QFile( name ).exists() ) {
            //int_mask = swap_endian< MaskType > ( int_mask );
            std::unique_ptr< QMessageBox > mesbox ( new QMessageBox );
            mesbox->setText( " File incorrect !!! Aborted" );
            mesbox->setIcon( QMessageBox::Warning );
            mesbox->exec();
            return;
        } else {
          doXor( name, mask_vector );
        }
    } else {
        std::unique_ptr< QMessageBox > mesbox ( new QMessageBox );
        mesbox->setText( " Mask incorrect !!! Aborted" );
        mesbox->setIcon( QMessageBox::Warning );
        mesbox->exec();
        return;
    }
}

void MainWindow::on_doXorButton_clicked()
{
    //QFuture<void> future = QtConcurrent::run( doXor, );
    QString mask = ( ui->defaultXorMaskRadioButton->isChecked() ) ?
                "c0ffee" ://ui->defaultXorMaskLabel->text() :
                ui->customXorMaskEdit->text();
    doXorUI( ui->choosedFileOutputLineEdit->text(), mask );
    //doXorUI( ui->choosedFileOutputLineEdit->text(), "ffee" );
    //doXorUI( "/ed78e437rs", "ffee" );
}
