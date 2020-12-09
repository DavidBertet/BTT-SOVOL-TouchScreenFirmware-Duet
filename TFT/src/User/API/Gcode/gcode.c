#include "gcode.h"
#include "includes.h"

REQUEST_COMMAND_INFO requestCommandInfo;
bool WaitingGcodeResponse = 0;

static void resetRequestCommandInfo(void)
{
  requestCommandInfo.cmd_rev_buf = malloc(CMD_MAX_REV);
  while (!requestCommandInfo.cmd_rev_buf)
    ; // malloc failed
  memset(requestCommandInfo.cmd_rev_buf, 0, CMD_MAX_REV);
  requestCommandInfo.inWaitResponse = true;
  requestCommandInfo.inResponse = false;
  requestCommandInfo.done = false;
  requestCommandInfo.inError = false;
}

bool RequestCommandInfoIsRunning(void)
{
  return WaitingGcodeResponse; //i try to use requestCommandInfo.done but does not work as expected ...
}

void clearRequestCommandInfo(void)
{
  free(requestCommandInfo.cmd_rev_buf);
}

/*
  * SENDING:M20
  * Begin file list
  * PI3MK2~1.GCO 11081207
  * /YEST~1/TEST2/PI3MK2~1.GCO 11081207
  * /YEST~1/TEST2/PI3MK2~3.GCO 11081207
  * /YEST~1/TEST2/PI3MK2~2.GCO 11081207
  * /YEST~1/TEST2/PI3MK2~4.GCO 11081207
  * /YEST~1/TEST2/PI3MK2~5.GCO 11081207
  * /YEST~1/PI3MK2~1.GCO 11081207
  * /YEST~1/PI3MK2~3.GCO 11081207
  * /YEST~1/PI3MK2~2.GCO 11081207
  * End file list
*/
char *request_M20(char *nextdir)
{
  uint32_t timeout = ((uint32_t)0x000FFFFF);
  if (nextdir == NULL)
  {
    strcpy(requestCommandInfo.command, "M20 S2\n\n");
  }
  else
  {
    sprintf(requestCommandInfo.command, "M20 S2 P\"/gcodes/%s\"\n\n", nextdir);
  }

  //  strcpy(requestCommandInfo.startMagic, "{\"dir\"");
  strcpy(requestCommandInfo.startMagic, "{");
  //strcpy(requestCommandInfo.stopMagic, ",\"err\"");
  strcpy(requestCommandInfo.stopMagic, "}");
  strcpy(requestCommandInfo.errorMagic, "Error");
  resetRequestCommandInfo();
  mustStoreCmd("\n");
  mustStoreCmd(requestCommandInfo.command);
  // in case of some issue of user srs5694, I store a extra \n
  mustStoreCmd("\n");
  // Wait for response
  WaitingGcodeResponse = 1;
  while ((!requestCommandInfo.done) && (timeout > 0x00))
  {
    loopProcess();
    timeout--;
  }
  WaitingGcodeResponse = 0;
  if (timeout <= 0x00)
  {
    clearRequestCommandInfo();
  }
  //clearRequestCommandInfo(); //shall be call after copying the buffer ...
  return requestCommandInfo.cmd_rev_buf;
}

/*
 * M33 retrieve long filename from short file name
 *   M33 miscel~1/armchair/armcha~1.gco
 * Output:
 *   /Miscellaneous/Armchair/Armchair.gcode
*/
char *request_M33(char *filename)
{
  sprintf(requestCommandInfo.command, "M33 %s\n", filename);
  strcpy(requestCommandInfo.startMagic, "/"); //a character that is in the line to be treated
  strcpy(requestCommandInfo.stopMagic, "ok");
  strcpy(requestCommandInfo.errorMagic, "Cannot open subdir");
  resetRequestCommandInfo();
  mustStoreCmd(requestCommandInfo.command);
  // Wait for response
  WaitingGcodeResponse = 1;
  while (!requestCommandInfo.done)
  {
    loopProcess();
  }
  WaitingGcodeResponse = 0;
  //clearRequestCommandInfo(); //shall be call after copying the buffer ...
  return requestCommandInfo.cmd_rev_buf;
}

/**
 * Select the file to print
 *
 * >>> m23 YEST~1/TEST2/PI3MK2~5.GCO
 * SENDING:M23 YEST~1/TEST2/PI3MK2~5.GCO
 * echo:Now fresh file: YEST~1/TEST2/PI3MK2~5.GCO
 * File opened: PI3MK2~5.GCO Size: 11081207
 * File selected
 * RRF3 не отдаёт в ответ ничего
 **/
bool request_M23(char *filename)
{
  char command[100];
  sprintf(command, "M23 %s\n", filename);
  mustStoreCmd(command);

  return true;
}

/**
 * Start o resume print
 **/
bool request_M24(int pos)
{
  if (pos == 0)
  {
    mustStoreCmd("M24\n");
  }
  else
  {
    char command[100];
    sprintf(command, "M24 S%d\n", pos);
    mustStoreCmd(command);
  }
  return true;
}

/**
 * Abort print
 **/
bool request_M524(void)
{
  request_M25();
  mustStoreCmd("M0 H1\n");
  return true;
}
/**
 * Pause print
 **/
bool request_M25(void)
{
  mustStoreCmd("M25\n");
  return true;
}

char *request_M20_macros(char *nextdir)
{
  uint32_t timeout = ((uint32_t)0x000FFFFF);
  if ((nextdir == NULL) || strchr(nextdir, '/') == NULL)
  {
    strcpy(requestCommandInfo.command, "M20 S2 P\"/macros/\"\n\n");
  }
  else
  {
    sprintf(requestCommandInfo.command, "M20 S2 P\"/macros/\"%s\n\n", nextdir);
  }

  strcpy(requestCommandInfo.startMagic, "{");
  strcpy(requestCommandInfo.stopMagic, "}");
  strcpy(requestCommandInfo.errorMagic, "Error");
  resetRequestCommandInfo();
  mustStoreCmd("\n");
  mustStoreCmd(requestCommandInfo.command);
  // in case of some issue of user srs5694, I store a extra \n
  mustStoreCmd("\n");
  // Wait for response
  WaitingGcodeResponse = 1;
  while ((!requestCommandInfo.done) && (timeout > 0x00))
  {
    loopProcess();
    timeout--;
  }
  WaitingGcodeResponse = 0;
  if (timeout <= 0x00)
  {
    clearRequestCommandInfo();
  }
  //clearRequestCommandInfo(); //shall be call after copying the buffer ...
  return requestCommandInfo.cmd_rev_buf;
}
