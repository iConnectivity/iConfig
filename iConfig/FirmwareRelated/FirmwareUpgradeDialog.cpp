/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "FirmwareUpgradeDialog.h"
#include "ui_FirmwareUpgradeDialog.h"

#include "MyAlgorithms.h"
#include "Reset.h"
#include "DevicePID.h"

#include <ACK.h>

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QTime>
#include <QXmlStreamReader>
#define CHECK_MD5
#ifdef CHECK_MD5
#include <QCryptographicHash>
#endif

#include <QObject>
#include <QDialog>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QUrl>
#include <QNetworkReply>
#include <QDebug>
#include <QPointer>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#endif

//New feature, zx 2017-03-22
#define FW_UPDATE_TIMEOUT   90000
#define FW_UPDATE_TIMEOUT_UPDATEPERIOD   3000

using namespace boost;
using namespace MyAlgorithms;
using namespace GeneSysLib;

FirmwareUpgradeDialog::FirmwareUpgradeDialog(CommPtr _comm,
                                             DeviceInfoPtr _device,
                                             FirmwareMode::Enum _mode,
                                             QWidget *_parent)
    : QDialog(_parent), ui(new Ui::FirmwareUpgradeDialog), firmwareMode(_mode) {
  ackRegistered = false;

  comm = _comm;
  device = _device;
  ui->setupUi(this);

  Q_ASSERT(comm);
  Q_ASSERT(device);

  deviceRebooter = QSharedPointer<DeviceRebooter>(
      new DeviceRebooter(_comm, _device), &DeviceRebooter::deleteLater);

  nam = QSharedPointer<QNetworkAccessManager>(
      new QNetworkAccessManager, &QNetworkAccessManager::deleteLater);
  connect(nam.data(), SIGNAL(finished(QNetworkReply *)), this,
          SLOT(finishedRequest(QNetworkReply *)));

  connect(this, SIGNAL(firmwareUpdateRequired(QString, QString)), this,
          SLOT(downloadFirmware(QString, QString)), Qt::QueuedConnection);

  connect(this, SIGNAL(sendNext()), this, SLOT(messagePump()),
          Qt::QueuedConnection);

  connect(deviceRebooter.data(), SIGNAL(rebootComplete(int)), this,
          SLOT(rebootComplete()));

  connect(comm->timerThread.get(), SIGNAL(timedOut()), this, SLOT(close()));

  connect(this, SIGNAL(firmwareUpdateTimeOut()), this, SLOT(close())); //zx, 2017-06-19

  noUpgradeNeeded =  new QMessageBox(QMessageBox::Information, "Up to date",
                      "No upgrade required", QMessageBox::Ok, this);

  connect(this, SIGNAL(firmwareUpToDate(QString)), noUpgradeNeeded,
          SLOT(exec()));
  connect(noUpgradeNeeded, SIGNAL(buttonClicked(QAbstractButton *)),
          this, SLOT(close()));
  connect(this, SIGNAL(checkComplete()), this, SLOT(close()));

  //New feature, zx 2017-03-22
  //int timerId = startTimer(FW_UPDATE_TIMEOUT_UPDATEPERIOD);


  //connect(this, SIGNAL(redirectDownloadURL()), this, SLOT(reQueryDownloadSite()));

  //Bugfixing for new download website, zx 2017-03-03
  //QUrl url("http://www.iConnectivity.com/downloads/firmwares.xml");
//  connect(this, SIGNAL(redirectDownloadURL()), this, SLOT(close()));
  m_downloadURL = "http://downloads.iConnectivity.com/firmwares.xml";
//    m_downloadURL = "http://www.iConnectivity.com/downloads/testfirmwares.xml";
  QUrl url(m_downloadURL);
  qDebug() << "Request URL: " << url.toString();
  nam->get(QNetworkRequest(url));
  //QNetworkReply *reply = nam->get(QNetworkRequest(url));
  //connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(reQueryDownloadSite()));

  ui->progressBar->setMinimum(0);
  ui->progressBar->setMaximum(0);
  //New feature, zx 2017-03-23
  ui->timeoutBar->setMinimum(0);
  ui->timeoutBar->setMaximum(FW_UPDATE_TIMEOUT);
  m_TimerCount = 0;
  m_bLocalFile = false;

}

//Support local firmware file updated, zx, 2017-05-08
FirmwareUpgradeDialog::FirmwareUpgradeDialog(GeneSysLib::CommPtr _comm,
                                DeviceInfoPtr _device,
                                QString fwFileName,
                                QWidget* _parent)
    : QDialog(_parent), ui(new Ui::FirmwareUpgradeDialog), firmwareMode(FirmwareMode::UpgradeMode) //zx,2017-06-19
{
  ackRegistered = false;

  comm = _comm;
  device = _device;
  ui->setupUi(this);

  Q_ASSERT(comm);
  Q_ASSERT(device);

  deviceRebooter = QSharedPointer<DeviceRebooter>(
      new DeviceRebooter(_comm, _device), &DeviceRebooter::deleteLater);
/*
  nam = QSharedPointer<QNetworkAccessManager>(
      new QNetworkAccessManager, &QNetworkAccessManager::deleteLater);
  connect(nam.data(), SIGNAL(finished(QNetworkReply *)), this,
          SLOT(finishedRequest(QNetworkReply *)));

  connect(this, SIGNAL(firmwareUpdateRequired(QString, QString)), this,
          SLOT(downloadFirmware(QString, QString)), Qt::QueuedConnection);
*/
  connect(this, SIGNAL(sendNext()), this, SLOT(messagePump()),
          Qt::QueuedConnection);

  connect(deviceRebooter.data(), SIGNAL(rebootComplete(int)), this,
          SLOT(rebootComplete()));

  //if(((DevicePID::Enum) device->getPID()) != DevicePID::MIO10) //zx,2017-06-20
  //  connect(comm->timerThread.get(), SIGNAL(timedOut()), this, SLOT(close()));
   connect(comm->timerThread.get(), SIGNAL(timedOut()), this, SLOT(fileUpdateTimeOut()));

  connect(this, SIGNAL(firmwareUpdateTimeOut()), this, SLOT(close())); //zx, 2017-06-19

   m_LocalFirmwareFile = fwFileName;
   m_bLocalFile = true;
   ui->progressBar->setVisible(false);
   QString slabel = tr("Update firmware from file: ") + m_LocalFirmwareFile;
   ui->statusLabel
       ->setText(slabel);

/*
  noUpgradeNeeded =  new QMessageBox(QMessageBox::Information, "Up to date",
                      "No upgrade required", QMessageBox::Ok, this);

  connect(this, SIGNAL(firmwareUpToDate(QString)), noUpgradeNeeded,
          SLOT(exec()));
  connect(noUpgradeNeeded, SIGNAL(buttonClicked(QAbstractButton *)),
          this, SLOT(close()));
  connect(this, SIGNAL(checkComplete()), this, SLOT(close()));
*/
  //New feature, zx 2017-03-22
  //int timerId = startTimer(FW_UPDATE_TIMEOUT_UPDATEPERIOD);

  //ui->progressBar->setMinimum(0);
  //ui->progressBar->setMaximum(0);
  //New feature, zx 2017-03-23
  ui->timeoutBar->setMinimum(0);
  ui->timeoutBar->setMaximum(FW_UPDATE_TIMEOUT);
  m_TimerCount = 0;

}

void FirmwareUpgradeDialog::fileUpdateTimeOut()//zx,2017-06-21
{
    //???
/*  if(((DevicePID::Enum) device->getPID()) == DevicePID::MIO10&& m_bLocalFile == true && !m_LocalFirmwareFile.isEmpty()) //zx,2017-06-22
  {
     firmwareMode = FirmwareMode::RecoveryMode;
     if(!m_LocalFirmwareFile.isEmpty())
     {
         QFile file(m_LocalFirmwareFile);
         if (!file.open(QIODevice::ReadOnly))
         {
             this->close();
             return;
         }
         QByteArray bytes;// = file.readAll();
         bytes = file.readAll();
         if(!bytes.isEmpty())
         {
            qDebug() << QString("Midi data:") << bytes.constData();
            parseMIDI(bytes);
         }
         else
           this->close();
     }
     else
     {
        this->close();
     }
  }*/
  //else
  //{
     this->close();
  //}
}

void FirmwareUpgradeDialog::ShowFWUpdateDialogForLocalFile()
{
    this->showNormal();
   //firmwareMode = FirmwareMode::RecoveryMode;//???????
    if(!m_LocalFirmwareFile.isEmpty())
    {
        QFile file(m_LocalFirmwareFile);
        if (!file.open(QIODevice::ReadOnly))
        {
            this->close();
            return;
        }
        QByteArray bytes;// = file.readAll();
        //QByteArray bytes = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
        //while (file.waitForReadyRead(100))
        bytes = file.readAll();
        if(!bytes.isEmpty())
        {
           qDebug() << QString("Midi data:") << bytes.constData();
           parseMIDI(bytes);
        }
        else
          this->close();
    }
    else
    {
       this->close();
    }
}

//bugfixing, zx, 2017-03-22
void FirmwareUpgradeDialog::timerEvent(QTimerEvent *event) {
  ++m_TimerCount;
  int nCurrentTime = m_TimerCount*FW_UPDATE_TIMEOUT_UPDATEPERIOD;
  ui->timeoutBar->setValue(nCurrentTime);
  if(FW_UPDATE_TIMEOUT <= nCurrentTime) {
    this->close();
  }
}


//Bugfixing for new download website, zx 2017-03-03
void FirmwareUpgradeDialog::reQueryDownloadSite() {
  QUrl url("http://www.iConnectivity.com/downloads/firmwares.xml");
  qDebug() << "Requery URL: " << url.toString();
  nam->get(QNetworkRequest(url));
}

void FirmwareUpgradeDialog::finishedRequest(QNetworkReply *reply) {
  // on error received?
  if (reply->error() == QNetworkReply::NoError && reply->isFinished() == true) {
    // read data from QNetworkReply here

    if (reply->url().toString().endsWith(".xml", Qt::CaseInsensitive)) {
      // Handle the firmware xml file
      QByteArray bytes = reply->readAll();     // bytes
      QString xml = QString::fromUtf8(bytes);  // string

      qDebug() << "firewares.xml: \n" << xml;

      parseXML(xml);
      checkFirmware();
    } else if (reply->url().toString().endsWith(".mid", Qt::CaseInsensitive)) {
      // Handle the firmware upgrading
      ui->statusLabel
          ->setText("Download Complete. Rebooting to bootloader mode.");

      QByteArray bytes = reply->readAll();  // bytes
      parseMIDI(bytes);
    }
  } else if (firmwareMode == FirmwareMode::CheckMode) {
    this->close();
  } else {
    QMessageBox::critical(this, tr("Error"),
                          tr("Could not download required files.\n"
                             "Please try again."));
    this->close();
  }

  // We receive ownership of the reply object and therefore need to handle
  // deletion.
  reply->deleteLater();
}

void FirmwareUpgradeDialog::downloadFirmware(QString version, QString url) {
  QString upgrade = QString("Do you want to upgrade the firmware to version %1?")
                    .arg(version);

  switch ((DevicePID::Enum) device->getPID()) {
    case DevicePID::iConnect4Audio:
    case DevicePID::iConnect2Audio:
  case DevicePID::PlayAudio12:
      upgrade = QString("Do you want to upgrade the firmware to version %1?\n\nPlease ensure before update that your computer is connected to the interface's USB port #1")
        .arg(version);
      break;
    case DevicePID::MIO:
    default:
      upgrade = QString("Do you want to upgrade the firmware to version %1?")
        .arg(version);
      break;

    case DevicePID::iConnect1:
      upgrade = QString("Do you want to upgrade the firmware to version %1?")
        .arg(version);
      break;

    case DevicePID::iConnect2Plus:
    case DevicePID::MIO2:
  case DevicePID::MIO4:
    upgrade = QString("Do you want to upgrade the firmware to version %1?\n\nPlease ensure before update that your computer is connected to the interface's USB port #2")
        .arg(version);
      break;
  case DevicePID::MIO10: //zx, 2017-06-19
    upgrade = QString("Do you want to upgrade the firmware to version %1?\n\nPlease ensure before update that your computer is connected to the interface's USB port #1")
        .arg(version);
      break;

    case DevicePID::iConnect4Plus:

      upgrade = QString("Do you want to upgrade the firmware to version %1?\n\nPlease ensure before update that your computer is connected to the interface's USB port #3")
        .arg(version);
      break;
  }

  if (QMessageBox::question(
          this, tr("Firmware upgrade available"),
          upgrade,
          QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
    ui->statusLabel->setText("Downloading firmware version " + version);
    nam->get(QNetworkRequest(QUrl(url)));
  } else {
    this->close();
  }
}

void FirmwareUpgradeDialog::parseXML(QString xml) {
  // create the xml reader
  QXmlStreamReader reader(xml);

  // clear the firmware list
  firmwareList.clear();

  // loop through the xml file
  while ((!reader.atEnd()) && (!reader.hasError())) {

    // read the next
    const auto &token = reader.readNext();

    // if the token is StartElement then attempt to read it
    if (token == QXmlStreamReader::StartElement) {
      // look for the firmware element
      if (reader.name() == "firmware") {
        // create default strings;
        QString pid;
        QString version;
        QString url;
        QMap<QString, QString> firmware;

        auto attributes = reader.attributes();

        if (attributes.hasAttribute("pid")) {
          pid = attributes.value("pid").toString();
        }
        if (attributes.hasAttribute("version")) {
          version = attributes.value("version").toString();
        }
        if (attributes.hasAttribute("url")) {
          url = attributes.value("url").toString();
          qDebug() << "Firmware download url:" << url;
        }

        firmware["pid"] = pid;
        firmware["version"] = version;
        firmware["url"] = url;

        firmwareList.append(firmware);
      }
          // continue for every other element
          else {
        continue;
      }
    }
        // continue reading if you don't know the element
        else {
      continue;
    }
  }
}

void FirmwareUpgradeDialog::parseMIDI(QByteArray midi) {
  sysexMessages.clear();

// Check the MD5
#ifdef CHECK_MD5
  // create the MD5 hash container
  QCryptographicHash md5Hash(QCryptographicHash::Md5);
  md5Hash.reset();

  // get the stored MD5
  QByteArray storedMD5;
  unsigned char startOfMD5[] = { 0xFF, 0x7F, 0x14, 0x00, 0x01, 0x73, 0x00 };

  int md5Index = midi.indexOf((char *)startOfMD5);

  if ((md5Index != -1) && ((md5Index + (int) sizeof(startOfMD5) + 16) < midi.size())) {
    // 16 hexadecimal digits MD5
    storedMD5 = midi.mid(md5Index + sizeof(startOfMD5), 16);
  }
#endif

  // Look for the header
  int startIndex = 0;
  int endIndex = 0;

  while ((startIndex = midi.indexOf(0xF0, startIndex)) != -1) {
    if (((endIndex = midi.indexOf(0xF7, startIndex)) != -1) &&
        ((endIndex - startIndex) > 0)) {
      auto sysex = midi.mid(startIndex, (endIndex - startIndex) + 1);

#ifdef CHECK_MD5
      md5Hash.addData(sysex);
#endif

      if (sysex.size() > 2) {
        // removed length byte (midi file only) before transmission
        sysex.remove(1, 1);
        sysexMessages.append(sysex);
      }
    }

    // slightly improve the search
    startIndex = endIndex;
  }

#ifdef CHECK_MD5
  auto calculatedMD5 = md5Hash.result();
  if ((storedMD5.size() == calculatedMD5.size()) &&
      (calculatedMD5 != storedMD5)) {
    if(m_bLocalFile == false)
    {
        QMessageBox::critical(this, tr("File Corrupt."), tr("The downloaded firmware file is corrupted. Please try again."));
    }
    else
    {
      QMessageBox::critical(this, tr("File Corrupt."), tr("The local firmware file, ") + m_bLocalFile + tr(", is corrupted. Please try other file."));
    }
    close();
  } else {
    if ((firmwareMode == FirmwareMode::UpgradeMode) || (firmwareMode == FirmwareMode::CheckMode))
    {
        deviceRebooter->reboot(BootMode::BootLoaderMode);
    } else if (firmwareMode == FirmwareMode::RecoveryMode) {
      // No reboot required
      //QMessageBox::critical(this, tr("test123."), tr(", test124.")); //Test
      rebootComplete();
    }
  }
#else
  if ((firmwareMode == FirmwareMode::UpgradeMode) ||
      (firmwareMode == FirmwareMode::CheckMode)) {
    deviceRebooter->reboot(BootMode::BootLoaderMode);
  } else if (firmwareMode == FirmwareMode::RecoveryMode) {
    // No reboot required
    rebootComplete();
  }
#endif
}

void FirmwareUpgradeDialog::checkFirmware() {
  if (firmwareList.empty())
    return;

  auto item = firmwareList.end();

  // find the right item in the list
  for (auto firmware = firmwareList.begin(); firmware != firmwareList.end();
       ++firmware) {

    if ((*firmware).contains("pid") == true && device->getDeviceID().pid() == (*firmware)["pid"].toInt()) {  //Fixing crash. zx, 2017-04-27
      item = firmware;
      break;
    }
  }

  if (item == firmwareList.end() || (*item).isEmpty()) { //Fixing crash. zx, 2017-04-27
    emit checkComplete();
    return;
  }

  QString currentVersion = "";

  if (device->containsInfo(InfoID::FirmwareVersion)) {
    auto versionInfo = device->infoData(InfoID::FirmwareVersion);
    currentVersion = QString::fromStdString(versionInfo.infoString());
  }

  QString currentVersionWithoutBeta = QString(currentVersion);
  int currentBetaVersion = 9999;
  if (currentVersion.contains('b')) {
    currentVersionWithoutBeta = currentVersion.split('b').at(0);
    currentBetaVersion = QString(currentVersion.split('b').at(1)).toInt();
  }

  QString webVersionWithoutBeta = currentVersionWithoutBeta; //??QString((*item)["version"]); //Fixing crash. zx, 2017-04-27
  if((*item).contains("version"))
  {
     webVersionWithoutBeta = QString((*item)["version"]);
  }
  int webBetaVersion = 9999;
  //Fixing crash. zx, 2017-04-27
  if ((*item).contains("version") && QString((*item)["version"]).contains('b'))
  {
    webVersionWithoutBeta = QString((*item)["version"]).split('b').at(0);
    webBetaVersion = QString(QString((*item)["version"]).split('b').at(1)).toInt();
  }

  //printf("version: %s, web: %s\n", currentVersionWithoutBeta.toStdString().c_str(), webVersionWithoutBeta.toStdString().c_str());
  //printf("beta: %d, web: %d\n");

  if (device->containsInfo(InfoID::SerialNumber))
  {
    if (item != firmwareList.end())
    {
      if ((currentVersionWithoutBeta.compare(webVersionWithoutBeta) == 0 && currentBetaVersion >= webBetaVersion) || (currentVersionWithoutBeta.compare(webVersionWithoutBeta) > 0))
      {
        if (firmwareMode == FirmwareMode::CheckMode)
        {
          emit checkComplete();
        }
        else
        {
          emit firmwareUpToDate(currentVersion);
        }
      }
      else
      {
        //Fixing crash. zx, 2017-04-27
        if((*item).contains("version") && (*item).contains("url"))
        {
          emit firmwareUpdateRequired((*item)["version"], (*item)["url"]);
        }
        else
        {
           emit firmwareUpToDate(currentVersion);
        }
      }
    } else {
      // We didn't find an entry in the list of available firmwares, assume up
      // to date
      if (firmwareMode == FirmwareMode::CheckMode) {
        emit checkComplete();
      } else {
        emit firmwareUpToDate(currentVersion);
      }
    }
  } else if (firmwareMode == FirmwareMode::RecoveryMode) {
    // There was an issue with querying the required information for this form.
    if (item != firmwareList.end())
    {
      //Fixing crash. zx, 2017-04-27
      if((*item).contains("version") && (*item).contains("url"))
      {
        emit firmwareUpdateRequired((*item)["version"], (*item)["url"]);
      }
      else
      {
         emit checkComplete();
      }
    }
    else
    {
      // We didn't find an entry in the list of available firmwares, assume up
      // to date
      emit checkComplete();
    }
  }
}

void FirmwareUpgradeDialog::handleACK(CmdEnum, DeviceID, Word,
                                      commandData_t _commandData) {
  const auto &ackData = _commandData.get<ACK>();
  if (ackData.errorCode() == 0) {
    // wait a bit then try to connect to a new device
    emit sendNext();
  }
}

void FirmwareUpgradeDialog::rebootComplete() {
  ui->statusLabel->setText(tr("Reboot complete. Loading firmware."));
  ui->progressBar->setMaximum(sysexMessages.count());

  messagePump();
}

void FirmwareUpgradeDialog::messagePump() {
  if (!ackRegistered) {
    auto ackBind =
        bind(&FirmwareUpgradeDialog::handleACK, this, _1, _2, _3, _4);
    comm->registerExclusiveHandler(Command::ACK, ackBind);
    ackRegistered = true;
  }

  if (sysexMessages.size() > 0) {
    const auto &sysex = sysexMessages.first();
    const auto &message = Bytes(sysex.begin(), sysex.end());
    sysexMessages.removeFirst();
    comm->sendSysex(message);

    ui->progressBar->setValue(ui->progressBar->maximum() - sysexMessages.size());
  } else {
    // DONE
    QMessageBox::information(this, "Complete", "Upgrade Complete");

    comm->unRegisterExclusiveHandler();

    device->send<ResetCommand>(BootMode::AppMode);

    this->accept();
  }
}
