void Reset_LoHi_Temps()  //min_temp_max_temp()
{
//Serial.print ("Mini from Reset_Lo_Hi Temps ");Serial.println (mini);   
  if(tempIN > maxi) {maxi = tempIN;}                      
  if(tempIN < mini) {mini = tempIN;} 

  if(tempIN < miniperp) 
    {
    miniperp = tempIN;
    mininhourperp = (hour());
    mininminperp = (minute());
    In_Low_Month = (month()); 
    In_Low_Day = (day());
    In_Low_Year =(("%02d\n", (year()) % 100));
    }
    
  if(tempIN > maxiperp)
    {
    maxiperp = tempIN;
    maxinhourperp = (hour());
    maxinminperp = (minute());
    In_High_Month = (month()); 
    In_High_Day = (day());
    In_High_Year = ("%02d\n", (year()) % 100);
    } 
   
//Reset daily temps
if ((hour() == 23) && (minute() > 58) && (second() > 55)) //reset daily min/max
  {
    maxi = 0;
    mini = 99;
    maxo = 0;
    mino = 99;
  }
//Serial.println();
//Serial.print ("TempIN from Reset_Lo_Hi Temps ");Serial.println (tempIN);  
                   
} 
