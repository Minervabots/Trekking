#include "CommandHandlers.h"
#include "Variables.h"
#include "Constants.h"

void onRecvMpuData(CmdMessenger *cmdMesseger)
{
  currentTransform.position.x = cmdMesseger->readBinArg<float>();
  currentTransform.position.y = cmdMesseger->readBinArg<float>();
  currentTransform.heading = cmdMesseger->readBinArg<float>();
}

void onRecvMpuLog(CmdMessenger *cmdMesseger)
{
  rPiCmdMessenger.sendCmdStart(MessageCodesRPi::kRPiLog);
  while(cmdMesseger->available())
  {
    rPiCmdMessenger.sendCmdBinArg(cmdMesseger->readStringArg());
  }
  rPiCmdMessenger.sendCmdEnd();
}

void onRecvTargetFound(CmdMessenger *cmdMesseger)
{
  targetDirection = cmdMesseger->readBinArg<double>();
  targetDirectionFiltered.add(targetDirection);
  
  //int x = cmdMesseger->readBinArg<int>();
  //int y = cmdMesseger->readBinArg<int>();
  //int w = cmdMesseger->readBinArg<int>();
  int h = cmdMesseger->readBinArg<int>();

  targetDirectionFiltered.add((FOCAL_LENGHT * CONE_REAL_HEIGHT * IMAGE_PIXEL_HEIGHT) / (h * SENSOR_HEIGHT));
}

void onRecvTargetLost(CmdMessenger *cmdMesseger)
{
}

void onRecvUnknownCommand(CmdMessenger *cmdMesseger)
{
  rPiCmdMessenger.sendCmdStart(MessageCodesRPi::kRPiLog);
  rPiCmdMessenger.sendCmdArg("Mensagem desconhecida");
  rPiCmdMessenger.sendCmdEnd();
}

