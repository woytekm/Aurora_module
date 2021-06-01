#ifndef TOUCH_H
#define TOUCH_H


//
// example sequences of touch sensor events used to communicate with device
// there can be any sequence of three subsequent events: single, double ant tripple taps on single sensor,
// or sequences of single taps on different sensors (ie: left - right - left: T_L_R_L), DT - double tap, TT - tripple tap
//

enum touch_events 
 {T_L,
  T_M,
  T_R,
  T_L_DT,
  T_M_DT,
  T_R_DT,
  T_L_R,
  T_L_M,
  T_L_R_L,
  T_M_R_M,
  T_R_TT};

#endif TOUCH_H
