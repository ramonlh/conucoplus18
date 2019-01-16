////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////  COMUNES   //////////////////////////////////////////////////////////
    
PGM_P(htmlHead_3)=  
    "<meta name=\"viewport\" content=\"width=device-width initial-scale=1\"/>"
//    "<meta http-equiv=\"Content-Type\" content=\"text/html charset=utf-8\"/>"
    "<meta http-equiv=\"Content-Type\" content=\"text/html charset=ISO-8859-1\"/>"
    
    "<style type=\"text/css\"> a:link { text-decoration:none; }"    // quita subrayado de enlaces
    
    "table.m {font-family:Verdana;font-size:11pt;border-collapse:collapse;}"     
    "table.m th {border-width:2px;padding:8px;border-style:solid;border-color:#666666;background-color:#FDFF9A;}"
    "table.m td {border-width:2px;padding:8px;border-style:solid;border-color:#666666;background-color:#dedede;}"

    // usado en tablas de panel principal
    "table.p {font-family:Verdana;font-size:9pt;border-collapse:collapse;color:#FFFFFF;text-align:left;}"      
    "table.p th {border-color:#666666;border-width:2px;padding:8px;border-style:solid;background-color:#FFFF00;}"
    "table.p td {border-color:#666666;border-width:2px;padding:8px; border-style:solid;font-color:#000000;background-color:#515772;}"

    // usado en tablas de configuración, programación etc NO CENTRADO
    "table.n {font-family:Verdana;font-size:9pt;border-collapse:collapse;text-align:left;}"      
    "table.n th {border-style:solid;border-color:#666666;border-width:2px;padding:4px;background-color:#FFFF00;}"
    "table.n td {border-style:solid;border-color:#666666;border-width:2px;padding:4px;background-color:#DEDEDE;}"
    "</style>";


// textos html NO MODIFICAR
PGM_P(head_f)="</head>";
PGM_P(form_action)="<form action=\"";
PGM_P(espere)="<META http-equiv=\"refresh\" content=\"15; URL=/\">OK. Espere unos 15 seg.";
PGM_P(htmlRefresh_i)="<meta http-equiv=\"refresh\" content=\"";

PGM_P(hrefon)="\"on?p=";

PGM_P(t301)="301";
PGM_P(t303)="303";
PGM_P(t404)="404";

PGM_P(addrt)="Addr";
PGM_P(apssid)="AP mode SSID";
PGM_P(appass)="AP mode Password";
PGM_P(applicationjson)="application/json; charset=utf-8";
PGM_P(applicationoctet)="application/octet-stream";
PGM_P(bins)="/bins/"; 
PGM_P(body_i)="<body>"; 
PGM_P(body_f)="</body>";
PGM_P(cachecontrol)="Cache-Control";
PGM_P(checkbox)="checkbox";  
PGM_P(center_i)="<center>";
PGM_P(center_f)="</center>";
PGM_P(tclass)="class";
PGM_P(connection)="Connection";
PGM_P(closet)="close";
PGM_P(contenttype)="Content-Type";
PGM_P(contentdisposition)="Content-Disposition";
PGM_P(conuco)="conuco";
PGM_P(CONUCO)="CONUCO";
PGM_P(data)="data";
PGM_P(dataType)="dataType";
PGM_P(defaultt)="Default";
PGM_P(devremotot)="!!! DISPOSITIVO REMOTO ¡¡¡";
PGM_P(disabled)="disabled";
PGM_P(dweet)="Dweet.io";
PGM_P(error)="ERROR";
PGM_P(esp8266)="ESP8266";
PGM_P(espsessionid)="ESPSESSIONID";
PGM_P(form)="<form action\"save\" method=\"get\">";
PGM_P(form_f)="</form>";
PGM_P(handleclientt)="handleClient";
PGM_P(head)="head";
PGM_P(html)="html";
PGM_P(HTTP11)="HTTP/1.1";
PGM_P(io)="I/O";
PGM_P(ID)="ID";
PGM_P(ifttt)="IFTTT";
PGM_P(input)="input";
PGM_P(Key)="Key";
PGM_P(json)="j";
PGM_P(jsonconf)="jc";
PGM_P(jsonext)="je";
PGM_P(lengt)="length";  
PGM_P(location)="Location";
PGM_P(logt)="Log";
PGM_P(max_length)="\" maxlength=\"";
PGM_P(Form_post)="\" method=\"post\">";
PGM_P(idmyjsont)="MyJson Key";
PGM_P(namet)="name";
PGM_P(nocache)="no-cache";
PGM_P(ok)="OK";
PGM_P(optionvalue)="<option value=\"";
PGM_P(option_f)="</option>";
PGM_P(pagenotfound)="Page not found";
PGM_P(password)="password";
PGM_P(pre_i)="<pre>";
PGM_P(pre_f)="</pre>";
PGM_P(POST)="POST";
PGM_P(PRG)="PRG";
PGM_P(PUT)="PUT";
PGM_P(rem)="rem=";
PGM_P(salremotas)="Remote I/O";
PGM_P(treset)="Reset";
PGM_P(tresetwifi)="Reset WiFi";
PGM_P(trestart)="Restart";
PGM_P(rfkey)="rfkey";
PGM_P(rfon)="rfon";
PGM_P(rfoff)="rfoff";
PGM_P(ro)="ro";
PGM_P(seeother)="See other";
PGM_P(select_f)="</select>";
PGM_P(Select_name)="<select name=\"";
PGM_P(tserver)="Server";
PGM_P(setcookie)="Set-Cookie";
PGM_P(routerssid)="STA mode SSID";
PGM_P(routerpass)="STA mode Password";
PGM_P(systemt)="system";
PGM_P(submit)="submit";
PGM_P(table)="table";
PGM_P(dweetname)="ThingName";

PGM_P(td)="<td>";  
PGM_P(td_f)="</td>"; 
PGM_P(tdcolspan_i)="<td colspan=\"";  
PGM_P(textjson)="text/json";
PGM_P(text)="text/text";
PGM_P(texttext)="text";
PGM_P(texthtml)="text/html";
PGM_P(th)="<th>";  
PGM_P(th_f)="</th>";  
PGM_P(title)="title";     
PGM_P(toff)="<toff>";  
PGM_P(tr)="<tr>";  
PGM_P(tr_f)="</tr>";   
PGM_P(tmenu)="\"m\"";
PGM_P(tnormal)="\"n\"";
PGM_P(tpanel)="\"p\"";
PGM_P(value)="value";
PGM_P(ver)="Show";
PGM_P(vers)="Ver";
PGM_P(web)="Web";
PGM_P(id)="id";
PGM_P(xmlns)="xmlns=\"http://www.w3.org/1999/xhtml\"";             

PGM_P(fontsize_i)="<font size=";
PGM_P(font_f)="</font>";
PGM_P(color000000)="style=\"color:#000000;\"";
PGM_P(colorffffcc)="style=\"color:#ffffcc;\"";
PGM_P(invis)="style=\"visibility: hidden\"";
PGM_P(crlf)="\r\n";  
PGM_P(actwc)="act=wc";
PGM_P(ON)="ON";  
PGM_P(OFF)="OFF";
PGM_P(celsius)="&deg;C";
PGM_P(br)="<br />";
PGM_P(size_i)="\" size=\"";  
PGM_P(comilla)="'";
PGM_P(comillas)="\"";
PGM_P(ap)="AP";
PGM_P(apsta)="AP+STA";
PGM_P(sta)="STA";
PGM_P(comillascierre)="\"/>";
PGM_P(trcolor1_i)="<tr>";
PGM_P(colspan)="colspan=\"";
PGM_P(resetcontp)="<a href=\"sy?act=rp";     
PGM_P(selected)="selected";
PGM_P(snehtm)="sne";
PGM_P(snshtm)="sns";
PGM_P(MAC)="MAC";
PGM_P(iftttkeyt)="IFTTT Key";
PGM_P(tssid)="SSID";
PGM_P(tpass)="PASS";
PGM_P(hr)="<hr>";  
PGM_P(td_if)="<td></td>";

PGM_P(dhtt)="DHT";
PGM_P(ONOFF)="ON/OFF";
PGM_P(parenguion)=")-";
PGM_P(mayorparen)=">(";
PGM_P(type)="type";
PGM_P(urltext)="URL";
PGM_P(panelhtm)="p";
PGM_P(sshtm)="ss";
PGM_P(srlhtm)="srl";  
PGM_P(slkhtm)="sr";
PGM_P(sremhtm)="ssr";
PGM_P(sdhtm)="sd";
PGM_P(sphtm)="sp";
PGM_P(sbphtm)="sbp";
PGM_P(rfhtm)="rf";
PGM_P(rohtm)="ro";
PGM_P(sdremhtm)="sdr";
PGM_P(sdrem150htm)="s150";
PGM_P(sdremiohtm)="sdrio";
PGM_P(suhtm)="firm";
PGM_P(Firmware)="Firmware";
PGM_P(rshtm)="rs";
PGM_P(seschtm)="se";
PGM_P(ssechtm)="ssec";
PGM_P(selsalhtm)="selsal";
PGM_P(sprghtm)="spr";
PGM_P(ssehtm)="sse";
PGM_P(syhtm)="sy";
PGM_P(swchtm)="swc";
PGM_P(espsyshtm)="es";
PGM_P(fileshtm)="f";
PGM_P(sfhtm)="sf";
PGM_P(loghtm)="login";
PGM_P(scanap)="sc"; 
PGM_P(svhtm)="sv";
PGM_P(siohtm)="sio";
PGM_P(termhtm)="t";
PGM_P(factor)="Factor";
PGM_P(offset)="Offset";
PGM_P(Off)="Off";
PGM_P(off)="of";
PGM_P(On)="On";
PGM_P(on)="on";
PGM_P(checked)="checked";
PGM_P(n)="n";
PGM_P(href_i)="<a href=";
PGM_P(href_f)="</a>";
PGM_P(attachfilename)="attachment; filename=";



PGM_P(tdefact)="Seg. ON";
PGM_P(tdefdes)="Seg. OFF";
PGM_P(ron)="pon";
PGM_P(roff)="pof";
PGM_P(panel)="Panel";
PGM_P(dig)="Dig.";
PGM_P(digital)="digital";
PGM_P(DISCONNECTYES)="DISCONNECT=YES";
PGM_P(parasite)="Parasite";
PGM_P(power)="Power";
PGM_P(minuto)="Min.";
PGM_P(port)="Port";
PGM_P(file)="'file'";
PGM_P(updatet)="update";
PGM_P(device)="dev";
PGM_P(chipsize)="Chip Flash.";
PGM_P(chipspeed)="Freq.";
PGM_P(dns)="DNS";
PGM_P(mask)="Mask";
PGM_P(modet)="mode";
PGM_P(gateway)="Gateway";
PGM_P(http)="http://";
PGM_P(https)="https://";
PGM_P(alias)="Alias";
PGM_P(trigger)="/trigger/";
PGM_P(withkey)="/with/key/";
PGM_P(amper)="&amp;";
PGM_P(ampersand)="&";
PGM_P(hidden)="hidden";
PGM_P(thost)="Host:";
PGM_P(IP)="IP:";
PGM_P(runningt)="Running";
PGM_P(twifi)="WiFi";
PGM_P(ttimeoutNTP)="Timeout Http";
PGM_P(smtpPER)="SMTP Per.";
PGM_P(smtpSVR)="SMTP Server";
PGM_P(smtpPORT)="SMTP Port";
PGM_P(smtpUSER)="SMTP User";
PGM_P(smtpPASS)="SMTP Pass";
PGM_P(smtpREMI)="SMTP Sender";
PGM_P(smtpDEST1)="SMTP Dest.";
PGM_P(pinvalr)="pinvalR:";
PGM_P(unit)="Un.";
PGM_P(si)="Sí";
PGM_P(no)="No";
PGM_P(login)="l";
PGM_P(ori)="ori";
PGM_P(rjson)="rj";
PGM_P(rjsonconf)="rjc";
PGM_P(barrarjc)="/rjc";
PGM_P(notdefined)="ND";
PGM_P(textplain)="text/plain";
PGM_P(igualp)="?p=";
PGM_P(pretty)="pretty";
PGM_P(watermarkt)="watermark";
PGM_P(webcall)="WebCall";

PGM_P(letraa)="a";
PGM_P(letrab)="b";
PGM_P(letrac)="c";
PGM_P(letrad)="d";
PGM_P(letraD)="D";
PGM_P(letrae)="e";
PGM_P(letraf)="f";
PGM_P(letrag)="g";
PGM_P(letrah)="h";
PGM_P(letrai)="i";
PGM_P(letraJ)="J";
PGM_P(letral)="l";
PGM_P(letraL)="L";
PGM_P(letrak)="k";
PGM_P(letram)="m";
PGM_P(letraM)="M";
PGM_P(letran)="n";
PGM_P(letrao)="o";
PGM_P(letrap)="p";
PGM_P(letraP)="P";
PGM_P(letrar)="r";
PGM_P(letraR)="R";
PGM_P(letras)="s";
PGM_P(letraS)="S";
PGM_P(letrat)="t";
PGM_P(letrau)="u";
PGM_P(letrav)="v";
PGM_P(letraV)="V";
PGM_P(letraw)="w";
PGM_P(letraX)="X";
PGM_P(letray)="y";
PGM_P(letraZ)="Z";
PGM_P(cero)="0";
PGM_P(uno)="1";
PGM_P(dos)="2";
PGM_P(tres)="3";
PGM_P(cuatro)="4";
PGM_P(cinco)="5";
PGM_P(seis)="6";
PGM_P(siete)="7";
PGM_P(sumat)="Σ";
PGM_P(b)=" ";
PGM_P(coma)=",";
PGM_P(barra)="/";
PGM_P(barraesp)=" / ";
PGM_P(ig)="=";          
PGM_P(menor)="<";          
PGM_P(mayor)=">";
PGM_P(mayoroigual)=">=";          
PGM_P(menoroigual)="<=";          
PGM_P(dp)=":";
PGM_P(paren_i)="(";
PGM_P(paren_f)=")";
PGM_P(llave_i)="{";
PGM_P(llave_f)="}";
PGM_P(punto)=".";
PGM_P(puntoycoma)=";";
PGM_P(guion)="-";  
PGM_P(subray)="_";  
PGM_P(interr)="?";          
PGM_P(aster)="*";          
PGM_P(porcen)="%";
PGM_P(puntossus)="...";
PGM_P(modbust)="ModBus";
PGM_P(t32p)="T-32-P";
PGM_P(senales)="Local I/O";
PGM_P(iottweett)="IoTtweet";
PGM_P(modomyjsont)="MyJson";
PGM_P(api)="api";
PGM_P(newpage)="target=\"_blank\"";
PGM_P(onclickclose)="onclick=\"window.close()";
PGM_P(diginput)="Dig. Input";
PGM_P(i2c)="I2C";
PGM_P(intro)="INTRO";
PGM_P(esc)="ESC";
PGM_P(up)="UP";
PGM_P(down)="DOWN";
PGM_P(left)="LEFT";
PGM_P(rigth)="RIGTH";
PGM_P(ups)="UP";
PGM_P(downs)="DW";
PGM_P(lefts)="LF";
PGM_P(rigths)="RT";
PGM_P(paramn)="?n=";
PGM_P(rf)="RF";
PGM_P(tadmin)="admin";
PGM_P(vacio)="";
PGM_P(spiffs)="SPIFFS";
PGM_P(amas)="a+";
PGM_P(rmas)="r+";
PGM_P(wmas)="w+";

/// Sensores
PGM_P(BM1750)="BM1750";
PGM_P(BMP085)="BMP085";
PGM_P(DHT12)="DHT12";
PGM_P(INA219)="INA219";
PGM_P(PCF8551)="PCF8551";
PGM_P(PN532)="PN532";
PGM_P(SoilMisture)="Soil Misture";

PGM_P(myjsoncom)="myjson.com";
PGM_P(iottweetcom)="www.iottweet.com";
PGM_P(gmaps)="www.google.es/maps";
PGM_P(urliotwweet)="http://www.iottweet.com";
PGM_P(icanhazip)="icanhazip.com";
PGM_P(urlapimyjson)="api.myjson.com";
PGM_P(urlNTPserverpool)="europe.pool.ntp.org";
PGM_P(iftttcom)="ifttt.com";
PGM_P(makeriftttcom)="maker.ifttt.com";
PGM_P(urldweet)="https://dweet.io/get/dweets/for/";
PGM_P(getdweett)="getdweet";
PGM_P(getlastdweett)="/get/latest/dweet/for/";
PGM_P(dweetfor)="/dweet/for/";
PGM_P(dweetio)="dweet.io";
PGM_P(inobin)=".ino.bin";

// Roomba
PGM_P(START)="START";
PGM_P(BAUD)="BAUD *";
PGM_P(CONTROL)="CONTROL";
PGM_P(SAFE)="SAFE";
PGM_P(FULL)="FULL";
PGM_P(POWER)="POWER";
PGM_P(SPOT)="SPOT";
PGM_P(CLEAN)="CLEAN";
PGM_P(MAX)="MAX";
PGM_P(DRIVE)="DRIVE *";
PGM_P(MOTORS)="MOTORS *";
PGM_P(LEDS)="LEDS *";
PGM_P(SONG)="SONG *";
PGM_P(PLAY)="PLAY *";
PGM_P(SENSORS)="SENSORS *";
PGM_P(DOCK)="DOCK";

PGM_P(t12341234)="12341234";

// json codes
PGM_P(aa)="aa";
PGM_P(DV)="DV";
PGM_P(s0)="s0";
PGM_P(s1)="s1";
PGM_P(s2)="s2";
PGM_P(a0)="a0";
PGM_P(ua0)="ua0";
PGM_P(el0)="el0";
PGM_P(el1)="el1";
PGM_P(sl0)="sl0";
PGM_P(sl1)="sl1";
PGM_P(tel0)="tel0";
PGM_P(tel1)="tel1";
PGM_P(tsl0)="tsl0";
PGM_P(tsl1)="tsl1";
PGM_P(MJ)="MJ";
PGM_P(al)="al";

// ESPSYS
PGM_P(dbm)="dBm";
PGM_P(Time)="Time";
PGM_P(Chipid)="Chip id";
PGM_P(ChipFlashSize)="Chip Flash Size";
PGM_P(Chipspeed)="Chip speed";
PGM_P(sdkversion)="sdk version";
PGM_P(vdd33)="vdd33";
PGM_P(adc_read)="adc_read";
PGM_P(boot_version)="boot_version";
PGM_P(boot_mode)="boot_mode";
PGM_P(userbin_addr)="userbin_addr";
PGM_P(cpu_freq)="cpu_freq";
PGM_P(flash_get_id)="flash_get_id";
PGM_P(opmode)="opmode";
PGM_P(opmode_default)="opmode_default";
PGM_P(connect_status)="connect_status";
PGM_P(get_current_ap_id)="get_current_ap_id";
PGM_P(auto_connect)="auto_connect";
PGM_P(dhcpc_status)="dhcpc_status";
PGM_P(hostnamet)="_hostname";
PGM_P(station_num)="station_num";
PGM_P(dhcps_status)="dhcps_status";
PGM_P(phy_mode)="phy_mode";
PGM_P(sleep_type)="sleep_type";
PGM_P(broadcast_if)="broadcast_if";
PGM_P(user_limit_rate_mask)="user_limit_rate_mask";
PGM_P(channelt)="channel";
PGM_P(free_heap_size)="free_heap_size";
PGM_P(rtc_time)="rtc_time";
PGM_P(BMP085notfound)="BMP085 not found";
PGM_P(sensori2c)="sensor I2C";
PGM_P(mqtt)="MQTT";
PGM_P(path)="path";

