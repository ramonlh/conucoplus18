
void buildtipo(PGM_P cad1, PGM_P cad2, int pos)
{
  printP(comillas, cad1,cad2);
  printI(pos);
  printP(comillas,dp, llave_i);
}

void buildvalorI(PGM_P cad1, PGM_P cad2, PGM_P cad3, int pos, int value, PGM_P cadf)
{
  printP(comillas,cad1,cad2,cad3);
  if (pos>=0) printI(pos);
  printP(comillas,dp);
  printI(value);
  printP(cadf,coma);
}

void buildvalorL(PGM_P cad1, PGM_P cad2, PGM_P cad3, int pos, long value, PGM_P cadf)
{
  printP(comillas,cad1,cad2,cad3);
  if (pos>=0) printI(pos);
  printP(comillas,dp);
  printL(value);
  printP(cadf,coma);
}

void buildvalorF(PGM_P cad1, PGM_P cad2, PGM_P cad3, int pos, float value, byte deci, PGM_P cadf)
{
  printP(comillas,cad1,cad2,cad3);
  if (pos>=0) printI(pos);
  printP(comillas,dp);
  printF(value,deci);
  printP(cadf,coma);
}

void buildvalorC(PGM_P cad1, PGM_P cad2, PGM_P cad3, int pos, char *value, PGM_P cadf)
{
  printP(comillas,cad1,cad2,cad3);
  if (pos>=0) printI(pos);
  printP(comillas,dp,comillas,value,comillas);
  printP(cadf,coma);
}

void ICACHE_FLASH_ATTR buildJson()
{
  printP(llave_i,comillas,ID,comillas,dp,comillas);      for (byte i=0;i<6;i++) printP(conf.EEmac[i]);      // MAC
  printP(comillas,coma,comillas,letraD,letraV,comillas); printPiP(dp,conf.iddevice,coma);                   // DV Device
  printP(comillas,letraM,letraJ,comillas);  printP(dp,comillas,conf.idmyjson,comillas,coma);                // MJ IdMyjson
  
  for (byte i=0; i<maxTemp; ++i) buildvalorF(letrat,vacio,vacio,i,(MbR[i]*0.01),2,vacio);  // TEMPERATURAS
  for (byte i=0; i<maxEA; ++i)    // ENTRADA ANALÓGICA
    {
    buildvalorF(letraa,vacio,vacio,i,((MbR[baseAna]*conf.factorA[i])+(conf.offsetA[i])),2,vacio);
    buildvalorC(letrau,letraa,vacio,i,conf.unitpinA,vacio);
    }
  for (byte i=0; i<maxED; ++i) 
    { 
    if ((conf.tipoED[i]==0)||(conf.tipoED[i]==1))      // ON/OFF
      buildvalorI(letrae,vacio,vacio,i,getbit8(conf.MbC8,i+2),vacio);   // ENTRADAS DIGITALES
    else if (conf.tipoED[i]==2)
      {
      buildvalorI(letrae,vacio,vacio,i,2,vacio);   // DHT
      buildvalorF(letrad,letrah,letrat,i,dhtdata[i][0],2,vacio);   // temperartura
      buildvalorF(letrad,letrah,letrah,i,dhtdata[i][1],2,vacio);   // humedad
      }
    }
  for (byte i=0; i<maxSD; ++i) buildvalorI(letras,vacio,vacio,i,getbit8(conf.MbC8,i),vacio);    // SALIDAS DIGITALES
  for (byte i=0; i<maxED; ++i) buildvalorL(letrat,letrae,vacio,i,conf.contadores[i],vacio);   // contador ENTRADAS DIGITALES
  for (byte i=0; i<maxSD; ++i)    // tiempos de SALIDAS DIGITALES
    {
    long segundos = (millis()/1000)-((getbit8(conf.MbC8,i)==0)?tempdes[i]:tempact[i]);
    printP(comillas,letrat,letras);
    printIP(i,comillas);
    printP(dp);
    printL((millis()/1000)-((getbit8(conf.MbC8,i)==0)?tempdes[i]:tempact[i]));
    if (i<maxSD-1) printP(coma);
    }
  printP(llave_f);
}

void ICACHE_FLASH_ATTR buildjsonext()
{
  printremote();
  printP(llave_i,comillas,ID,comillas,dp,comillas);
  for (byte i=0;i<6;i++) printP(conf.EEmac[i]);
  printP(comillas,coma);

  buildvalorI(letraD,letraV,vacio,-1,conf.iddevice,vacio);
  buildvalorC(letraM,letraJ,vacio,-1,conf.idmyjson,vacio);
  buildvalorC(c(alias),vacio,vacio,-1,conf.aliasdevice,vacio);
  buildvalorI(c(vers),vacio,vacio,-1,versinst,vacio);
  buildvalorF(t(45),vacio,vacio,-1,conf.latitud,6,vacio);   // latitud
  buildvalorF(t(46),vacio,vacio,-1,conf.longitud,6,vacio);  // longitud

  for (byte i=0; i<maxEsc; ++i) // ESCENAS
   {
    buildtipo(letrae,letras,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowEsc, i),vacio);
    buildvalorC(letrau,letraa,vacio,i,readdescr(filedescesc,i,20),llave_f);
   }
  
  for (byte i=0; i<maxTemp; ++i) // TEMPERATURAS
   {
    buildtipo(letrat,letral,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowbypanel[0], i),vacio);
    buildvalorC(letran,vacio,vacio,-1,readdescr(filedesclocal,i,20),vacio);
    buildvalorF(letrav,vacio,vacio,-1,MbR[i]*0.01,2,llave_f);
   }
  for (byte i=0;i<maxEA; ++i)    // ENTRADA ANALÓGICA
  {
    buildtipo(letraa,vacio,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowbypanel[0], i+3),vacio);
    buildvalorC(letran,vacio,vacio,-1,readdescr(filedesclocal,i+3,20),vacio);
    buildvalorF(letrav,vacio,vacio,-1,(MbR[baseAna]*conf.factorA[i])+conf.offsetA[i],2,llave_f);
  }
  for (byte i=0;i<maxED;++i)    // ENTRADAS DIGITALES
  {
    buildtipo(letrae,letral,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowbypanel[0], i+4),vacio);
    buildvalorC(letran,vacio,vacio,-1,readdescr(filedesclocal,i+4,20),vacio);
    buildvalorI(letrav,vacio,vacio,-1,getbit8(conf.MbC8,i+2),llave_f);
  }
  for (byte i=0; i<maxSD; ++i)    // SALIDAS DIGITALES
  {
    buildtipo(letras,letral,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowbypanel[0], i+6),vacio);
    buildvalorC(letran,vacio,vacio,-1,readdescr(filedesclocal,i+6,20),vacio);
    buildvalorI(letrav,vacio,vacio,-1,getbit8(conf.MbC8,i),llave_f);
  }
  for (byte i=0; i<maxsalrem; ++i)    // Señales remotas
  {
    buildtipo(letrar,vacio,i);
    buildvalorI(letras,vacio,vacio,-1,getbit8(conf.bshowbypanel[0], i+8),vacio);
    buildvalorC(letran,vacio,vacio,-1,readdescr(filesalrem,i,20),vacio);
    if (conf.senalrem[i] <= 3) 
      buildvalorF(letrav,vacio,vacio,-1,sondaremote[i],2,vacio);
    else
      buildvalorI(letrav,vacio,vacio,-1,getbit8(bstatremote,i),vacio);
    printP(comillas, letrat, comillas, dp,comillas, conf.senalrem[i]<=2?letrac:(conf.senalrem[i]==3?letraa:(conf.senalrem[i]<=5)?letrae:letras));
    printP(comillas, llave_f);
    if (i<maxsalrem-1) printP(coma);
  }
  printP(llave_f);
}

void ICACHE_FLASH_ATTR buildJsonConf(boolean remoto, boolean sendpass, boolean resetear)
{
  printP(llave_i);
  printP(comillas,letras,letras,comillas,dp,comillas); printP(sendpass?remoto?ssidSTAtemp:conf.ssidSTA:vacio,comillas,coma);  // SSID
  printP(comillas,letrap,letras,comillas,dp,comillas); printP(sendpass?remoto?passSTAtemp:conf.passSTA:vacio,comillas,coma);  // PASS
  
  buildvalorI(letrar,letras,letrat,-1,resetear?1:0,vacio);                              // rst, Resetear
  buildvalorI(letraD,letraV,vacio,-1,remoto?iddevicetemp:conf.iddevice,vacio);          // DV, device
  buildvalorI(letrav,vacio,vacio,-1,versinst,vacio);                                    // v, versión
  buildvalorI(letraa,letraa,vacio,-1,remoto?actualizauttemp:conf.actualizaut,vacio);    // aa, actualización automática
  buildvalorC(letraa,letral,vacio,-1,remoto?aliasdevicetemp:conf.aliasdevice,vacio);    // a1, alias
  buildvalorI(letrai,letraf,vacio,-1,remoto?iftttenabletemp:conf.iftttenable,vacio);    // if, IFTTT enable
  buildvalorC(letrak,vacio,vacio,-1,remoto?iftttkeytemp:conf.iftttkey,vacio);           // k, IFTTT key
  buildvalorI(letrad,letraw,vacio,-1,remoto?mododweettemp:conf.mododweet,vacio);        // dw, dweet enable 
  buildvalorI(letram,letray,vacio,-1,remoto?modomyjsontemp:conf.modomyjson,vacio);      // my, myjson enable
  buildvalorC(letraM,letraJ,vacio,-1,remoto?conf.idmyjson:conf.idmyjson,vacio);         // MJ, id myjson
  buildvalorI(letram,cuatro,cinco,-1,remoto?modo45temp:conf.modo45,vacio);              // m45, modo entradas 4,5
  buildvalorL(letrap,letraa,letrap,-1,remoto?peractpantemp:conf.peractpan,vacio);       // pap, período refresco panel
  buildvalorL(letrap,letraa,letrar,-1,remoto?peractremtemp:conf.peractrem,vacio);       // par, período actualización remotos
  buildvalorL(letrat,letrad,letrap,-1,remoto?TempDesactPrgtemp:conf.TempDesactPrg,vacio);   // tdp, tiempo desactivación programas
  buildvalorI(letrai,letrao,letrat,-1,remoto?iottweetenabletemp:conf.iottweetenable,vacio); // iot, IoTweet enable
  printP(comillas,letrai,letrao,letrat,letrau,comillas); printP(dp,comillas,remoto?iottweetusertemp:conf.iottweetuser,comillas,coma); // iotu, IoTweet user
  printP(comillas,letrai,letrao,letrat,letrak,comillas); printP(dp,comillas,remoto?iottweetkeytemp:conf.iottweetkey,comillas,coma);   // iotk, IoTweet pass
  buildvalorF(letral,letraa,letrat,-1,remoto?latitudtemp:conf.latitud,6,vacio);             // lat, latitud
  buildvalorF(letral,letrao,letran,-1,remoto?longitudtemp:conf.longitud,6,vacio);           // lon, longitud
  for (byte i=0; i<maxTemp; ++i) // TEMPERATURAS                                            // t0,1,2, temperaturas
    buildvalorC(letrat,vacio,vacio,i,remoto?readdescr(filedesctemp,i,20):readdescr(filedesclocal,i,20),vacio);  
  buildvalorC(letraa,vacio,vacio,0,remoto?readdescr(filedesctemp,3,20):readdescr(filedesclocal,3,20),vacio);    // ad0, descriptor entrada analógica
  buildvalorC(letraa,letrau,vacio,0,remoto?unitpinAtemp:conf.unitpinA,vacio);                                   // au0, unidades entrada analógica
  buildvalorF(letraa,letraf,vacio,0,remoto?factorAtemp[0]:conf.factorA[0],5,vacio);                             // af0, factor entrada analógica
  buildvalorF(letraa,letrao,vacio,0,remoto?offsetAtemp[0]:conf.offsetA[0],5,vacio);                             // ao0, offset entrada analógica
  buildvalorI(letraa,letras,letrau,0,getbit8(remoto?bsumatAtemp:conf.bsumatA,0),vacio);                         // asu0, suma entrada analógica
  for (byte i=0; i<maxED; ++i)    // ENTRADAS DIGITALES
    {
    buildvalorC(letrae,vacio,vacio,i,remoto?readdescr(filedesctemp,i+4,20):readdescr(filedesclocal,i+4,20),vacio);  // en, descriptor
    buildvalorI(letrae,letrat,vacio,i,remoto?tipoEDtemp[i]:conf.tipoED[i],vacio);                                   // etn, tipo
    buildvalorI(letrai,letraf,letrae,i,remoto?iftttpinEDtemp[i]:conf.iftttpinED[i],vacio);                          // ifen, IFTTT activo
    }
  for (byte i=0; i<maxSD; ++i)    // SALIDAS DIGITALES
    {
    buildvalorC(letras,vacio,vacio,i,remoto?readdescr(filedesctemp,i+6,20):readdescr(filedesclocal,i+6,20),vacio);  // sn, descriptor
    buildvalorI(letrav,letrai,vacio,i,remoto?valinictemp[i]:conf.valinic[i],vacio);                                 // vin, valor inicial
    buildvalorL(letrao,letran,vacio,i,remoto?tempdefacttemp[i]:conf.tempdefact[i],vacio);                           // onn, tiempo máximo ON
    buildvalorL(letrao,letraf,letraf,i,remoto?tempdefdestemp[i]:conf.tempdefdes[i],vacio);                          // offn, tiempo máximo OFF
    buildvalorI(letrai,letraf,letras,i,remoto?iftttpinSDtemp[i]:conf.iftttpinSD[i],vacio);                          // ifsn, IFTTT activo
    }
  printP(comillas,letraf,letras,letrav,comillas);
  printP(dp,comillas,remoto?fwUrlBasetemp:conf.fwUrlBase);                                                          // fsv, URLbase
}

int ICACHE_FLASH_ATTR ReqJson(int ip, int port) // pide json a remoto 
{
  createhost(ip);
  msg=vacio;
  printP(barra,json,interr,letrar,ig);
  printI(conf.iddevice);
  return callhttpGET(host,port,true,conf.timeoutrem);
}

int ICACHE_FLASH_ATTR ReqJsonConf(int ip, int port) // pide jsonext a remoto
{
  createhost(ip);
  msg=vacio;
  printP(barra,jsonconf);
  if (ip==1) printP(interr,letram,ig,uno);
  return callhttpGET(host,port,true,conf.timeoutrem);
}

int ICACHE_FLASH_ATTR sendJsonConf(int ip, int port, boolean sendpass,boolean resetear) // envia json conf, recibe el comando "/rjc"->parsejconf->saveconf
{
  createhost(ip);
  msg=vacio;
  buildJsonConf(true,sendpass,resetear);
  
  HTTPClient http;
  http.begin(host,port,barrarjc);
  http.addHeader(type, tPOST);
  http.addHeader(contenttype, applicationjson);
  http.addHeader(dataType, json);
  http.setTimeout(conf.timeoutNTP);
  int httpCode=http.POST(msg);
  if (httpCode>0) {  msg=http.getString();  }
  http.end();
  msg=vacio;
  return httpCode;
}

int ICACHE_FLASH_ATTR sendJson(int ip, int port) // envia json al master/ o a los masters
{
  createhost(ip);
  msg=vacio;
  printP(barra,rjson,interr,c(tdata),ig);
  buildJson();
  return callhttpGET(host,port,false,conf.timeoutrem);
}

int ICACHE_FLASH_ATTR putmyjson() 
{
//  statusChange=false;
  strcpy(auxchar,c(bins));;
  msg=vacio;
  buildjsonext();
  int httpCode=0;
  HTTPClient http;
  if (conf.idmyjsonST==0)
    {
//    Serial.print("auxchar:"); Serial.print(auxchar);Serial.print(" tPOST:"); Serial.print(tPOST);
//    Serial.print(" applicationjson:"); Serial.print(applicationjson);Serial.print(" json:"); Serial.println(json);
    http.begin(c(myjsoncom),80,auxchar);
    http.addHeader(type, tPOST);
    http.addHeader(contenttype, applicationjson);
    http.addHeader(dataType, "json");
    http.setTimeout(conf.timeoutNTP);
    httpCode=http.POST(msg);
//    Serial.print("httpCode:"); Serial.print(httpCode);
//    Serial.print("  msg:"); Serial.println(msg);
    if (httpCode>0) {
      if(httpCode==HTTP_CODE_CREATED) {
        msg=http.getString();
        msg.substring(msg.indexOf("bins//")+6, msg.length()-2).toCharArray(conf.idmyjson,10);
        conf.idmyjsonST=1;
        saveconf();
        }
      }
    }
  else
    {
    strcat(auxchar,conf.idmyjson);
    http.begin(c(myjsoncom),80,auxchar);
    http.addHeader(type, PUT);
    http.addHeader(contenttype, applicationjson);
    http.addHeader(dataType, json);
    http.setTimeout(conf.timeoutNTP);
    httpCode=http.sendRequest(PUT,msg);
    msg=http.getString(); 
    }
  http.end();
  msg=vacio;
  return httpCode;
}

void ICACHE_FLASH_ATTR parseJsonremoto()
{
  ///// parte 1, extracción de valores
  extrae(true,msg, PSTR("ID")).toCharArray(datosremoto.mac,13);
  datosremoto.devori=extrae(false,msg,DV).toInt();
  extrae(true,msg,MJ).toCharArray(datosremoto.idmyj,10);

  for (byte i=0;i<3;i++) datosremoto.s[i]=extrae(false,msg,idpin[i]).toFloat();   // temperaturas
  datosremoto.a1=extrae(false,msg,idpin[3]).toFloat();                            // analógica
  extrae(true,msg,ua0).toCharArray(datosremoto.ua1,4);                            // unidades señal analógica
  for (byte i=0;i<2;i++) datosremoto.di[i]=extrae(false,msg,idpin[i+4]).toInt();  // entradas digitales
  if (datosremoto.di[0]==2)
    {
    datosremoto.dhtdata[0][0]=extrae(false,msg,PSTR("dht0")).toFloat();
    datosremoto.dhtdata[0][1]=extrae(false,msg,PSTR("dhh0")).toFloat();
    }
  if (datosremoto.di[1]==2)
    {
    datosremoto.dhtdata[1][0]=extrae(false,msg,PSTR("dht1")).toFloat();
    datosremoto.dhtdata[1][1]=extrae(false,msg,PSTR("dhh1")).toFloat();
    }
  for (byte i=0;i<2;i++) datosremoto.ds[i]=extrae(false,msg,idpin[i+6]).toInt();  // salidas digitales
  datosremoto.tdi[0]=extrae(false,msg,tel0).toInt();        // número veces entradas digitales
  datosremoto.tdi[1]=extrae(false,msg,tel1).toInt();
  datosremoto.tdo[0]=extrae(false,msg,tsl0).toInt();        // tiempo salidas digitales
  datosremoto.tdo[1]=extrae(false,msg,tsl1).toInt();
  
  ///// parte 2, asignación de valores
  for (int j=0;j<maxsalrem;j++)
    {
    if (conf.idsalremote[j]>0)
      if (conf.idsalremote[j]==datosremoto.devori)
        {
        if (conf.senalrem[j]<=2) { sondaremote[j]=datosremoto.s[conf.senalrem[j]]; }// sondas remotas 1,2 y 3
        else if (conf.senalrem[j]==3) {sondaremote[j]=datosremoto.a1; strcpy(auxdesc,datosremoto.ua1); savedescr(fileunitsalrem,auxdesc, j,13); } // analógica
        else if (conf.senalrem[j]<=5) {setbit8(bstatremote, j, datosremoto.di[conf.senalrem[j]-4]); contaremote[j]=datosremoto.tdi[conf.senalrem[j]-4];
                                       tipoedremote[j]=datosremoto.di[conf.senalrem[j]-4];}
        else if (conf.senalrem[j]<=7) {setbit8(bstatremote, j, datosremoto.ds[conf.senalrem[j]-6]); contaremote[j]=datosremoto.tdo[conf.senalrem[j]-6];   }
        }
    if (conf.idremote[j]==datosremoto.devori) { 
      strcpy(auxdesc,datosremoto.mac); savedescr(filemacdevrem,auxdesc, j,13); 
      strcpy(auxdesc,datosremoto.idmyj); savedescr(fileidmyjsonrem,auxdesc, j,10); }
    }
}

void ICACHE_FLASH_ATTR parseJsonConf()
{
  extrae(true,msg, PSTR("ss")).toCharArray(ssidSTAtemp,20); if (strlen(ssidSTAtemp)>0) strcpy(conf.ssidSTA,ssidSTAtemp);
  extrae(true,msg, PSTR("ps")).toCharArray(passSTAtemp,20); if (strlen(passSTAtemp)>0) strcpy(conf.passSTA,passSTAtemp);
  hacerresetrem=extrae(false,msg,PSTR("rst")).toInt();    
  conf.iddevice=extrae(false,msg,DV).toInt();
  strcpy(conf.ssidAP, c(CONUCO)); strcat(conf.ssidAP,subray); strcat(conf.ssidAP, itoa(conf.iddevice,buff,10));
  conf.EEip[3]=conf.iddevice;
  conf.wifimode=2;
  
  conf.actualizaut=extrae(false,msg,aa).toInt();
  extrae(true,msg,al).toCharArray(conf.aliasdevice,20);
  conf.iftttenable=extrae(false,msg,PSTR("if")).toInt();
  extrae(true,msg, PSTR("k")).toCharArray(conf.iftttkey,30);
  conf.mododweet=extrae(false,msg,PSTR("dw")).toInt();
  conf.modomyjson=extrae(false,msg,PSTR("my")).toInt();
  extrae(true,msg,MJ).toCharArray(conf.idmyjson,20);
  conf.modo45=extrae(false,msg,PSTR("m45")).toInt();
  conf.peractpan=extrae(false,msg,PSTR("pap")).toInt();
  conf.peractrem=extrae(false,msg,PSTR("par")).toInt();
  conf.TempDesactPrg=extrae(false,msg,PSTR("tdp")).toInt();
  conf.iottweetenable=extrae(false,msg,PSTR("iot")).toInt();
  extrae(true,msg, PSTR("iotu")).toCharArray(conf.iottweetuser,10);
  extrae(true,msg, PSTR("iotk")).toCharArray(conf.iottweetkey,15);
  conf.latitud=extrae(true,msg,PSTR("lat")).toFloat();
  conf.longitud=extrae(true,msg,PSTR("lon")).toFloat();

  for (byte i=0;i<8;i++)
    { extrae(true,msg, idpin[i]).toCharArray(auxdesc,20); savedescr(filedesclocal,auxdesc,i,20); }
  extrae(true,msg, PSTR("au0")).toCharArray(conf.unitpinA,20);
  conf.factorA[0]=extrae(true,msg,PSTR("af0")).toFloat();
  conf.offsetA[0]=extrae(true,msg,PSTR("ao0")).toFloat();
  conf.bsumatA[0]=extrae(true,msg,PSTR("asu0")).toInt();
  conf.tipoED[0]=extrae(true,msg,PSTR("et0")).toInt();
  conf.tipoED[1]=extrae(true,msg,PSTR("et1")).toInt();
  conf.iftttpinED[0]=extrae(false,msg,PSTR("ife0")).toInt();
  conf.iftttpinED[1]=extrae(false,msg,PSTR("ife1")).toInt();
  conf.valinic[0]=extrae(false,msg,PSTR("vi0")).toInt();
  conf.valinic[1]=extrae(false,msg,PSTR("vi1")).toInt();
  conf.tempdefact[0]=extrae(false,msg,PSTR("on0")).toInt();
  conf.tempdefact[1]=extrae(false,msg,PSTR("on1")).toInt();
  conf.tempdefdes[0]=extrae(false,msg,PSTR("off0")).toInt();
  conf.tempdefdes[1]=extrae(false,msg,PSTR("off1")).toInt();
  conf.iftttpinSD[0]=extrae(false,msg,PSTR("ifs0")).toInt();
  conf.iftttpinSD[1]=extrae(false,msg,PSTR("ifs1")).toInt();
  extrae(true,msg,PSTR("fsv")).toCharArray(conf.fwUrlBase,80);
}


