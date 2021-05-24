

uint8_ system_init(void)
 {
 
   twi_init();
   MPR121_init();
   
   touch_IRQ_init();
   
 }
 
 
