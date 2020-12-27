void InSidetemp() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  T2 = (sensors.getTempCByIndex(i)); //Centigrade
   if ((T2 < -100) || (T2 > 125)) {             // loop until error resolves
      Serial.println (T2);
      delay (200);
      InSidetemp();
    }
  tempIN =(((T2 * 1.8) + 32)); // F
  }
