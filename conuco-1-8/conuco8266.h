
boolean ICACHE_FLASH_ATTR clientremote(){
  return ((server.remoteIP[0]!=192) && (server.remoteIP[0]!=0)); }

void ICACHE_FLASH_ATTR printremote() {
  if (!clientremote()) return;
  printhora(); Serial.print(b);
  for (byte i=0;i<3;i++) {Serial.print(server.remoteIP[i]);Serial.print(punto);}
  Serial.println(server.remoteIP[3]);
  }

void ICACHE_FLASH_ATTR filesHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  writeHeader(false,false);
  if (filesok)
    writeMenu(4, 3);
  else
    printP(t(faltafichero),crlf);
    printP(t(useftp),crlf);
  printP(menor,table, b);
  printP(c(tclass), ig, tnormal, mayor);
  Dir dir = SPIFFS.openDir("/");
  while (dir.next())   {
    File f=dir.openFile("r");
    printP(tr, td, href_i, comillas, letrad, letraw);
    printP(interr, letraf, ig);
    msg=msg+dir.fileName();
    printP(comillas, mayor);
    msg=msg+dir.fileName();
    printP(href_f, td_f, td);
    printI(f.size());
    printP(td_f, tr_f);
    }
  printP(menor, barra, table, mayor);  
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
}

void ICACHE_FLASH_ATTR espsysHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }

  msg=vacio;
  writeHeader(false,false);
  writeMenu(4, 4);
  printP(menor,table, b);
  printP(c(tclass), ig, tnormal, mayor);
  printP(tr, td, c(Time), td_f, td); printtiempo(millis() / 1000); printP(td_f, tr_f);
  printP(tr, td, c(Chipid), td_f, td); printL(system_get_chip_id()); printP(td_f, tr_f);
  printP(tr, td, c(ChipFlashSize), td_f, td);  printL(ESP.getFlashChipRealSize()); printP(td_f, tr_f);
  printP(tr, td, c(Chipspeed), td_f, td);  printL(ESP.getFlashChipSpeed()); printP(td_f, tr_f);
  printP(tr, td, c(sdkversion), td_f, td); printP(system_get_sdk_version()); printP(td_f, tr_f);
  printP(tr, td, c(vdd33), td_f, td); printL(system_get_vdd33()); printP(td_f, tr_f);
  printP(tr, td, c(adc_read), td_f, td); printL(system_adc_read()); printP(td_f, tr_f);
  printP(tr, td, c(boot_version), td_f, td); printL(system_get_boot_version());   printP(td_f, tr_f);
  printP(tr, td, c(Time), td_f, td); printI(conf.wifimode); printP(td_f, tr_f);
  printP(tr, td, c(boot_mode), td_f, td); printI(system_get_boot_mode());   printP(td_f, tr_f);
  printP(tr, td, c(userbin_addr), td_f, td); printL(system_get_userbin_addr());   printP(td_f, tr_f);
  printP(tr, td, c(cpu_freq), td_f, td); printL(system_get_cpu_freq());   printP(td_f, tr_f);
  printP(tr, td, c(flash_get_id), td_f, td); printL(spi_flash_get_id());   printP(td_f, tr_f);
  printP(tr, td, c(opmode), td_f, td); printI(wifi_get_opmode());   printP(td_f, tr_f);
  printP(tr, td, c(opmode_default), td_f, td); printI(wifi_get_opmode_default());   printP(td_f, tr_f);
  printP(tr, td, c(auto_connect), td_f, td); printI(wifi_get_opmode());   printP(td_f, tr_f);
  printP(tr, td, c(sleep_type), td_f, td); printI(wifi_get_sleep_type());   printP(td_f, tr_f);
  printP(tr, td, c(broadcast_if), td_f, td); printI(wifi_get_broadcast_if());   printP(td_f, tr_f);
  printP(tr, td, c(user_limit_rate_mask), td_f, td); printL(wifi_get_user_limit_rate_mask());   printP(td_f, tr_f);
  printP(tr, td, c(channelt), td_f, td); printI(wifi_get_channel());   printP(td_f, tr_f);
  printP(tr, td, c(dhcps_status), td_f, td); printI(wifi_softap_dhcps_status());   printP(td_f, tr_f);
  printP(tr, td, c(phy_mode), td_f, td); printI(wifi_get_phy_mode());   printP(td_f, tr_f);
  if (conf.wifimode!=1)
    {
    printP(tr, td, c(connect_status), td_f, td); printI(wifi_station_get_connect_status());   printP(td_f, tr_f);
    printP(tr, td, c(hostnamet), td_f, td); printP(wifi_station_get_hostname());   printP(td_f, tr_f);
    printP(tr, td, c(station_num), td_f, td); printI(wifi_softap_get_station_num());   printP(td_f, tr_f);
    printP(tr, td, c(get_current_ap_id), td_f, td); printI(wifi_station_get_current_ap_id());   printP(td_f, tr_f);
    }
  printP(menor, barra, table, mayor);  
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
}

void serialprintParESP()
{
  printlinea(ig);
  Serial.print(c(sdkversion)); Serial.print(dp); Serial.println(system_get_sdk_version());
  Serial.print(c(Chipid)); Serial.print(dp); Serial.println(system_get_chip_id());
  Serial.print(c(vdd33));  Serial.print(dp); Serial.println(system_get_vdd33());
  Serial.print(c(adc_read));  Serial.print(dp); Serial.println(system_adc_read());
  Serial.print(c(free_heap_size));  Serial.print(dp); Serial.println(system_get_free_heap_size());
  Serial.print(c(rtc_time));  Serial.print(dp); Serial.println(system_get_rtc_time());
  Serial.print(c(boot_version));  Serial.print(dp); Serial.println(system_get_boot_version());
  Serial.print(c(userbin_addr));  Serial.print(dp); Serial.println(system_get_userbin_addr());
  Serial.print(c(boot_mode));  Serial.print(dp); Serial.println(system_get_boot_mode());
  Serial.print(c(cpu_freq));  Serial.print(dp); Serial.println(system_get_cpu_freq());
  Serial.print(c(flash_get_id));  Serial.print(dp); Serial.println(spi_flash_get_id());
  Serial.print(c(opmode));  Serial.print(dp); Serial.println(wifi_get_opmode());
  Serial.print(c(opmode_default));  Serial.print(dp); Serial.println(wifi_get_opmode_default());
  Serial.print(c(connect_status));  Serial.print(dp); Serial.println(wifi_station_get_connect_status());
  Serial.print(c(get_current_ap_id));  Serial.print(dp); Serial.println(wifi_station_get_current_ap_id());
  Serial.print(c(auto_connect));  Serial.print(dp); Serial.println(wifi_station_get_auto_connect());
  Serial.print(c(dhcpc_status));  Serial.print(dp); Serial.println(wifi_station_dhcpc_status());
  Serial.print(c(hostnamet));  Serial.print(dp); Serial.println(wifi_station_get_hostname());
  Serial.print(c(station_num));  Serial.print(dp); Serial.println(wifi_softap_get_station_num());
  Serial.print(c(dhcps_status));  Serial.print(dp); Serial.println(wifi_softap_dhcps_status());
  Serial.print(c(phy_mode));  Serial.print(dp); Serial.println(wifi_get_phy_mode());
  Serial.print(c(sleep_type));  Serial.print(dp); Serial.println(wifi_get_sleep_type());
  Serial.print(c(broadcast_if));  Serial.print(dp); Serial.println(wifi_get_broadcast_if());
  Serial.print(c(user_limit_rate_mask));  Serial.print(dp); Serial.println(wifi_get_user_limit_rate_mask());
  Serial.print(c(channelt));  Serial.print(dp); Serial.println(wifi_get_channel());
  printlinea(ig);
}

void initupdateserver()
{
  httpUpdater.setup(&server,"/firm", conf.userDev, conf.passDev);
}
