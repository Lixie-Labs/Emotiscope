void init_ota(){
  FOTA.setManifestURL("https://raw.githubusercontent.com/hoeken/yarrboard/main/firmware/firmware.json");
  FOTA.setRootCA(MyRootCA);
  FOTA.setPubKey(MyPubKey);

  FOTA.setUpdateBeginFailCb( [](int partition) {
    Serial.printf("[ota] Update could not begin with %s partition\n", partition==U_SPIFFS ? "spiffs" : "firmware" );
  });

  // usage with lambda function:
  FOTA.setProgressCb( [](size_t progress, size_t size)
  {
      if( progress == size || progress == 0 )
        Serial.println();
      Serial.print(".");

      //let the clients know every second and at the end
      if (millis() - ota_last_message > 1000 || progress == size)
      {
        float percent = (float)progress / (float)size * 100.0;
        sendOTAProgressUpdate(percent);
        ota_last_message = millis();
      }
  });

  FOTA.setUpdateEndCb( [](int partition) {
    Serial.printf("[ota] Update ended with %s partition\n", partition==U_SPIFFS ? "spiffs" : "firmware" );
      sendOTAProgressFinished();
  });

  FOTA.setUpdateCheckFailCb( [](int partition, int error_code) {
    Serial.printf("[ota] Update could not validate %s partition (error %d)\n", partition==U_SPIFFS ? "spiffs" : "firmware", error_code );
    // error codes:
    //  -1 : partition not found
    //  -2 : validation (signature check) failed
  });

  FOTA.printConfig();
}