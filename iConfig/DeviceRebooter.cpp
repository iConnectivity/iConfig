/*
;iConfig source code and documentation is released under a GPLv3 license. 
;
; A copy is available from the Open Source Initiative site at:
;	https://opensource.org/licenses/gpl-3.0.html
*/

#include "DeviceRebooter.h"

#include "ACK.h"
#include "Device.h"
#include "DeviceID.h"
#include "Reset.h"

#include <QMutex>
#include <QWaitCondition>
#include <QDebug>

#ifndef Q_MOC_RUN
#include <boost/bind.hpp>
#endif

using namespace GeneSysLib;
using namespace boost;

DeviceRebooter::DeviceRebooter(CommPtr _comm, DeviceInfoPtr _device,
                               QObject *_parent)
    : QObject(_parent), comm(_comm), device(_device) {
  searchTimer = new QTimer(this);
  searchTimer->setInterval(400);

  connect(searchTimer, SIGNAL(timeout()), this, SLOT(timerTick()));
  connect(this, SIGNAL(resetSent()), this, SLOT(startSearch()),
          Qt::QueuedConnection);

  connect(this, SIGNAL(rebootComplete(int)), this, SLOT(cleanup()));
  /*m_HandleMio10 = ((DevicePID::Enum) device->getPID() == DevicePID::MIO10); //zx,2017-06-22
  if(m_HandleMio10 == true)
  {
     //searchTimer->setInterval(3000);
    qDebug() << "DeviceRebooter handle MIO10: ";
  }*/
}

void DeviceRebooter::reboot(BootModeEnum bootMode) {
  Q_ASSERT(comm);
  Q_ASSERT(device);

  //qDebug() << "DeviceRebooter reboot";

  // get the current PID, transID and serial number
  DeviceID deviceID;
  Word transID;
  tie(deviceID, transID) = device->getInfo();

  // store the desired boot mode
  desiredBootMode = bootMode;

  // register an ACK callback
  comm->registerExclusiveHandler(
      Command::ACK, bind(&DeviceRebooter::ackHandler, this, _1, _2, _3, _4));

  // register a getDevice callback
  auto getDeviceHandlerPtr =
      boost::bind(&DeviceRebooter::getDeviceHandler, this, _1, _2, _3, _4);
  getDeviceHandlerID =
      comm->registerHandler(Command::RetDevice, getDeviceHandlerPtr);

  // send the reset command
  device->send<ResetCommand>(bootMode);
}

void DeviceRebooter::startSearch() {
  //qDebug() << "DeviceRebooter startSearch()";

  // get the current PID, transID and serial number
  comm->unRegisterExclusiveHandler();

  bool opened = false;
  if ((opened = comm->openAllInputs()) == true)
  {
    opened = comm->openAllOutputs();
  }

  if (opened)
  {
    int outCount = comm->getOutCount();

    //zx, 2017-06-20
    //qDebug() << "DeviceRebooter Communicator Ouput count: " << outCount;
    /*if((DevicePID::Enum) device->getPID() == DevicePID::MIO10) //zx, 2017-06-20
    {
       ++outCount;
    }*/

    for (int port = 0; port < outCount; ++port)
    {
      pendingSearchSysex.push(make_pair(sysex(GetDeviceCommand(device->getDeviceID(), port)), port));
      //qDebug() << "DeviceRebooter put sending GetDeviceCommand into sysex command queue: DeviceID: " << device->getPID() << " transID: " << device->getTransID() << " portID: " << port;
      //if(m_HandleMio10 == true) //zx, 2017-06-22
      //{
      //  pendingSearchSysex.push(make_pair(sysex(GetDeviceCommand(DeviceID(), port)), port));
      //}
    }

    searchTimer->start();

    //if(m_HandleMio10 == true)//zx,2017-06-22
    //{
    //   sendNextDiscovery();
    //}
  }
  else
  {
      emit error(CouldNotOpen);
  }
}

void DeviceRebooter::sendNextDiscovery()
{
    //qDebug() << "DeviceRebooter::sendNextDiscovery called: ";

    if (!pendingSearchSysex.empty())
    {
         auto next = pendingSearchSysex.front();
         pendingSearchSysex.pop();

         auto port = next.second;
         auto message = next.first;

         //qDebug() << "sendNextDiscovery Send Sysex msg to portID: " << port;

         comm->setCurrentOutput(port);
         comm->sendSysex(message);
    }
}

void DeviceRebooter::timerTick() {
  // stop the timer
  //qDebug() << "DeviceRebooter::timerTick called: ";
  searchTimer->stop();

  if (!pendingSearchSysex.empty()) {
    auto next = pendingSearchSysex.front();
    pendingSearchSysex.pop();

    auto port = next.second;
    auto message = next.first;

    //qDebug() << "timerTick Send Sysex msg to portID: " << port;

    comm->setCurrentOutput(port);
    comm->sendSysex(message);
//????????
  //  message = sysex(GetDeviceCommand(device->getDeviceID(), 0x00FF));
  //  comm->sendSysex(message);

    searchTimer->start();
  } else {
    /*DeviceID deviceID;
    Word transID;
    tie(deviceID, transID) = device->getInfo();
    if((DevicePID::Enum) device->getPID() == DevicePID::MIO10) //zx, 2017-06-20
    {
      //comm->setCurrentOutput(transID);
      // let the listener know that we have found a device
      //emit rebootComplete(transID);
      //comm->unRegisterExclusiveHandler();

      //device->send<ResetCommand>(BootMode::AppMode);
      //cleanup();
      //rebootComplete(transID);
      qDebug() << "DeviceRebooter timeout";
      //emit error(TimedOut);
      searchTimer->stop();

      //comm->setCurrentOutput(transID);

      device->send<ResetCommand>(BootMode::AppMode);

      // let the listener know that we have found a device
      emit rebootComplete(transID);

    }*/
    //else
    //{
    //qDebug() << "DeviceRebooter::timerTick emit TimeOut ";
       emit error(TimedOut);
    //}
  }
}

void DeviceRebooter::ackHandler(CmdEnum, DeviceID, Word,
                                commandData_t commandData) {
  const auto &ackData = commandData.get<ACK>();
   qDebug() << "Get ACK msg";
  if (ackData.commandID() == Command::Reset) {
    QWaitCondition waitCondition;
    QMutex mutex;

    waitCondition.wait(&mutex, 10000); //fix 20171117: was 7000, need 10 seconds for mio10
    emit resetSent();
  }
}

void DeviceRebooter::getDeviceHandler(CmdEnum, DeviceID, Word transID, commandData_t commandData)
{
  const auto &devInfo = commandData.get<Device>();
  //qDebug() << "DeviceRebooter::getDeviceHandler called: " << desiredBootMode;

  if (devInfo.mode() == desiredBootMode) {
    // stop searching for another device, we have found the right device
    searchTimer->stop();

    comm->setCurrentOutput(transID);

    // let the listener know that we have found a device
    emit rebootComplete(transID);
  }
  else
  {
    //zx, 2017-06-21
    //qDebug() << "desiredBootMode: " << desiredBootMode;
    //qDebug() << "current mode: " << devInfo.mode();

  }
}

void DeviceRebooter::cleanup() {
  //qDebug() << "DeviceRebooter::cleanup() called";
  comm->unRegisterHandler(Command::RetDevice, getDeviceHandlerID);
}
