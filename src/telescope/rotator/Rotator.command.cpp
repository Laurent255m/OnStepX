//--------------------------------------------------------------------------------------------------
// OnStepX rotator control

#include "../../lib/convert/Convert.h"
#include "../Telescope.h"
#include "Rotator.h"

#if AXIS3_DRIVER_MODEL != OFF

bool Rotator::command(char *reply, char *command, char *parameter, bool *supressFrame, bool *numericReply, CommandError *commandError) {

  // :RA#    rotator Active?
  //            Return: 0 on failure (no rotator)
  //                    1 on success
  if (cmd("RA")) {
    // empty command for success response 1
  } else

  // r - rotator Commands
  if (command[0] == 'R') {

    // :rT#       Get rotator sTatus
    //            Returns: M# (for moving) or S# (for stopped)
    if (command[1] == 'T') {
      if (axis3.autoSlewActive()) strcpy(reply,"M"); else strcpy(reply,"S");
      *numericReply = false;
    } else

    // :rI#       Get rotator mInimum position (in degrees)
    //            Returns: n#
    if (command[1] == 'I') {
      sprintf(reply,"%ld",(long)round(axis3.settings.limits.min));
      *numericReply = false;
    } else

    // :rM#       Get rotator Max position (in degrees)
    //            Returns: n#
    if (command[1] == 'M') {
      sprintf(reply,"%ld",(long)round(axis3.settings.limits.max));
      *numericReply = false;
    } else

    // :rD#       Get rotator degrees per step
    //            Returns: n.n#
    if (command[1] == 'u') {
      sprintF(reply, "%7.5f", 1.0/axis3.getStepsPerMeasure());
      *numericReply = false;
    } else

    // :rb#       Get rotator Backlash amount (in steps)
    //            Return: n#
    if (command[1] == 'b' && parameter[0] == 0) {
      sprintf(reply,"%ld",(long)round(getBacklash()));
      *numericReply = false;
    } else

    // :rb[n]#    Set rotator Backlash amount (in steps)
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'b') {
      setBacklash(round(atol(parameter)));
      axis3.setBacklashSteps(getBacklash());
    } else

    // :rQ#       Stop (Quit) rotator movement
    //            Returns: Nothing
    if (command[1] == 'Q') {
      axis3.autoSlewStop();
      *numericReply = false;
    } else

    // :r[n]#     Set rotator move rate where n = 1 for .01 deg/s, 2 for 0.1 deg/s, 3 for 1.0 deg/s, 4 for 5.0 deg/s
    //            Returns: Nothing
    if (command[1] >= '1' && command[1] <= '4') {
      float p[] = {0.01F, 0.1F, 1.0F, 5.0F};
      moveRate = p[command[1] - '1'];
      *numericReply = false;
    } else

    // :rc#       Set continious move
    //            Returns: Nothing
    if (command[1] == 'c') {
      // this command is ignored, all rotator movement in OnStepX is continious
      *numericReply = false;
    } else

    // :r>#       Move rotator CW
    //            Returns: Nothing
    if (command[1] == '>') {
      axis3.setFrequencySlew(moveRate);
      axis3.autoSlew(DIR_FORWARD);
      *numericReply = false;
    } else

    // :r<#       Move rotator CCW
    //            Returns: Nothing
    if (command[1] == '<') {
      axis3.setFrequencySlew(moveRate);
      axis3.autoSlew(DIR_REVERSE);
      *numericReply = false;
    } else

    // :rG#       Get rotator current angle
    //            Returns: sDD*MM#
    if (command[1] == 'G') {
      convert.doubleToDms(reply, axis3.getInstrumentCoordinate(), true, true, PM_LOW);
      *numericReply = false;
    } else

    // :rR[sDDD*MM, etc.]#
    //            Set rotator target angle Relative (in degrees)
    //            Returns: Nothing
    if (command[1] == 'R') {
      double r;
      convert.dmsToDouble(&r, parameter, true);
      long t = axis3.getTargetCoordinate();
      axis3.setTargetCoordinateSteps(t + r);
      axis3.setFrequencySlew(AXIS3_SLEW_RATE_DESIRED);
      axis3.autoSlewRateByDistance(AXIS3_SLEW_RATE_DESIRED);
      *numericReply = false;
    } else

    // :rS[sDDD*MM, etc.]#
    //            Set rotator target angle (in degrees)
    //            Return: 0 on failure
    //                    1 on success
    if (command[1] == 'S') {
      double t;
      convert.dmsToDouble(&t, parameter, true);
      axis3.setTargetCoordinate(t);
      axis3.setFrequencySlew(AXIS3_SLEW_RATE_DESIRED);
      axis3.autoSlewRateByDistance(AXIS3_SLEW_RATE_DESIRED);
    //  *commandError = CE_SLEW_ERR_IN_STANDBY;
    } else

    // :rZ#       Set rotator position as Zero degrees
    //            Returns: Nothing
    if (command[1] == 'Z') {
      axis3.setMotorCoordinate(0.0);
      *numericReply = false;
    } else

    // :rF#       Set rotator position as Half-travel
    //            Returns: Nothing
    if (command[1] == 'F') {
      float p = round((axis3.settings.limits.max + axis3.settings.limits.min)/2.0F);
      axis3.setMotorCoordinate(p);
      *numericReply = false;
    } else

    // :rC#       Set rotator target position at half-travel
    //            Returns: Nothing
    if (command[1] == 'C') {
      long t = round((axis3.settings.limits.max + axis3.settings.limits.min)/2.0F);
      axis3.setTargetCoordinateSteps(t);
      axis3.setFrequencySlew(AXIS3_SLEW_RATE_DESIRED);
      axis3.autoSlewRateByDistance(AXIS3_SLEW_RATE_DESIRED);
      *numericReply = false;
    } else

    // :r+#       Derotator Enable
    //            Returns: Nothing
    if (command[1] == '+') {
      derotatorEnable(true);
      *numericReply = false;
    } else

    // :r-#       Derotator Disable 
    //            Returns: Nothing
    if (command[1] == '-') {
      derotatorEnable(false);
      *numericReply = false;
    } else

    // :rP#       Rotator move to parallactic angle
    //            Returns: Nothing
    if (command[1] == 'P') {
      setDerotatorPA(&telescope.mount.getPosition());
      *numericReply = false;
    } else

    // :rR#       Derotator Reverse direction
    //            Returns: Nothing
    if (command[1] == 'R') {
      setDerotatorReverse(!getDerotatorReverse());
      *numericReply = false;
    } else *commandError = CE_CMD_UNKNOWN;

  } else return false;

  return true;
}

#endif