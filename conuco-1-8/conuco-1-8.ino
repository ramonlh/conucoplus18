

#define INITFAB false    // si true, se resetea a fábrica, si false no se hace nada
#define versinst 1812    // se añade: Sistema de ficheros, gestión de zonas, control por 433, 32 señales remotas
#define debug true
#define debugwifi false

#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
#include "commontexts.h"              // include
#include <IoTtweet.h>                 // Local
#include <ModbusMaster.h>             // Local
//#include <SoftwareSerial.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>                // Local
#include <ESP8266HTTPClient.h>        // Local
#include <ESP8266WebServer.h>         // Local
#include <ESP8266HTTPUpdateServer.h>  // Local
#include <ESP8266httpUpdate.h>        // Local
#include "ESP8266WiFiAP.h"            // Local
#include "Wire.h"
#include <Adafruit_BMP085.h>          // Local
#include "Time.h"                     // Local
#include "TimeLib.h"                  // Local
#include "OneWire.h"                  // Local
#include "DallasTemperature.h"        // Local
#include "DHTesp.h"                   // Local
#include "defines.h"                  // include
#include "variables.h"                // include
#include "Base64.h"                   // include
#include "RCSwitch.h"                 // Local
#include "LiquidCrystal_I2C.h"        // Local
#include <FS.h>
#include <ESP8266FtpServer.h>
#include <PubSubClient.h>

ADC_MODE(ADC_TOUT);
ESP8266WebServer server(88);
OneWire owire(owPin);
DallasTemperature sensors1(&owire);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, urlNTPserverpool, 3600, 60000);
ESP8266HTTPUpdateServer httpUpdater;
ModbusMaster MBnode;
Adafruit_BMP085 bmp085;
RCSwitch mySwitch = RCSwitch();
LiquidCrystal_I2C lcd(0x27, 16, 2); // Display LCD 2x16 azul
DHTesp dht[2];
FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial
WiFiClient espClient;
PubSubClient PSclient(espClient);

#include "basicfunctions.h"            // include
#include "ajaxcode.h"                  // include
#include "htmlFunctions.h"             // include
#include "jsonfunctions.h"             // include

void ICACHE_FLASH_ATTR testTX433()
{
  mySwitch.send(conf.rfkeys.code[0], 24);
}

void ICACHE_FLASH_ATTR leevaloresDIG()
{
  for (byte i=0;i<maxSD;i++) setbit8(conf.MbC8,i,digitalRead(sdPin[i]));
  if (conf.modo45 == 0) 
    { 
    for (byte i=0;i<maxED;i++) 
      {
      setbit8(conf.MbC8,i+2,digitalRead(edPin[i]));  
      statusChange=(getbit8(conf.MbC8,i+2) != getbit8(MbC8ant,i+2));
      if (statusChange) 
        {
        setbit8(iftttchange,i+2,1);   
        if (getbit8(conf.MbC8,i+2)==1) conf.contadores[i]++;  
        }
      }
    MbC8ant[0]=conf.MbC8[0];
    }
}

void ICACHE_FLASH_ATTR leevaloresAN() { MbR[baseAna]=analogRead(A0); MbRant[baseAna]=MbR[baseAna]; }

void ICACHE_FLASH_ATTR leevaloresOW()
{
  sensors1.requestTemperatures();
  for (byte i=0; i<nTemp1; i++)  {
    int auxI=sensors1.getTempC(addr1Wire[i])*100;
    //    if ((auxI<=-10000) || (auxI=8500)) { addlog(3,auxI,"Temp error:"); return; }
    MbR[i]=auxI;
    MbRant[i]=MbR[i];
  }
}

void ICACHE_FLASH_ATTR leevaloresDHT()
{
  for (byte i=0;i<2;i++)
    {
    delay(dht[i].getMinimumSamplingPeriod());
    dhtdata[i][0]=dht[i].getTemperature();
    dhtdata[i][1]=dht[i].getHumidity();
    }
}

bool ICACHE_FLASH_ATTR autOK()
{
  return true;      // provisional
  if (conf.usepassDev == 0) return true;
  if (server.hasHeader("Cookie"))
    if (server.header("Cookie").indexOf("ESPSESSIONID=1") != -1)
      return true;
  return false;
}

void ICACHE_FLASH_ATTR serversend200() { server.send(200, texthtml, msg); msg=vacio; }

void ICACHE_FLASH_ATTR loginHTML()
{
  printremote();
  msg=vacio;
  if (server.method()==HTTP_POST)
    {
    if (server.hasArg("0") && server.hasArg("1"))
      {
      if ((server.arg(0)==conf.userDev) && (server.arg(1)==conf.passDev))
        { setCookie(1);  return; }
      }
    }
  if (server.hasArg("DISCONNECT")) { setCookie(0); return; }

  writeHeader(false,false);
  printP(c(body_i));
  printP(c(form_action), loghtm);
  printP(comillas,b);
  printP(c(Form_post), menor);
  printP(table);
  printP(b, c(tclass), ig, tnormal, mayor, tr);
  printparCP(t(usuario), 0, conf.userDev, 20, false);
  printP(tr_f, tr);
  printparCP(t(contrasena), 1, "", 20, true);
  printP(tr_f, menor, barra, table, mayor);
  printP(menor, c(tinput), b, type, ig, comillas);
  printP(submit, comillas, b, tvalue, ig, comillas);
  printP(tguardar, comillas, mayor);
  printP(menor, barra, c(tinput), mayor);
  printP(c(form_f));
  printP(c(body_f));
  printP(menor, barra, thtml, mayor);
  serversend200();
}

void ICACHE_FLASH_ATTR actualizamasters()     // envia datos a masters, normalmente 1 pero podrían ser varios
{
  statusChange=false;
  for (byte i=0; i<maxdevrem; i++) if (ListOri[i]>0) sendJson(ListOri[i],88);
}

//////////////////  DWEET  ///////////////////

int ICACHE_FLASH_ATTR getdweet(char* key)
{
  if (conf.mododweet==0) return 0;
  msg=vacio;
  printP(c(getlastdweett),conucochar,key);
  return callhttpGET("Dweet.io",80,true,conf.timeoutNTP);
}

int ICACHE_FLASH_ATTR postIoTweet()
{
  if (conf.iottweetenable==0) return 0;
  float data[4];                        //Your sending data variable.
  data[0]=MbR[0]*0.01;
  data[1]=sondaremote[3];
  data[2]=sondaremote[5];
  data[3]=getbit8(bstatremote, 2);
  myiot.WriteDashboard(conf.iottweetuser, conf.iottweetkey, data[0], data[1], data[2], data[3], private_tweet, public_tweet);
}

int ICACHE_FLASH_ATTR postdweet(char* key)
{
  if (conf.mododweet==0) return 0;
  msg=vacio;
  buildjsonext();
  strcpy(auxchar, c(dweetfor)); strcat(auxchar, conucochar); strcat(auxchar, key);
  HTTPClient http;
  http.begin(c(dweetio), 80, auxchar);
  http.addHeader(type, tPOST);
  http.addHeader(contenttype, applicationjson);
  http.addHeader(dataType, json);
  http.setTimeout(conf.timeoutNTP);
  int httpCode = http.POST(msg);
  http.end();
  msg=vacio;
  return httpCode;
}

//////////////////  IFTTT  ///////////////////
int ICACHE_FLASH_ATTR ifttttrigger(char *evento, char* key, char* value1, char* value2, char* value3)
{
  if (conf.iftttenable == 0) return 0;
  strcpy(auxchar, c(trigger)); strcat(auxchar, evento); // value1, 2 y 3 tamaño máximo = 20
  strcat(auxchar, c(withkey)); strcat(auxchar, key); strcat(auxchar, interr);
  strcat(auxchar, tvalue); strcat(auxchar, uno); strcat(auxchar, ig); strcat(auxchar, value1); strcat(auxchar, ampersand);
  strcat(auxchar, tvalue); strcat(auxchar, dos); strcat(auxchar, ig); strcat(auxchar, value2); strcat(auxchar, ampersand);
  strcat(auxchar, tvalue); strcat(auxchar, tres); strcat(auxchar, ig); strcat(auxchar, value3);
  byte i=0;
  while (auxchar[i]!='\0') { if (auxchar[i]==' ') auxchar[i]='_';  i++; }
  HTTPClient http;
  http.begin(c(makeriftttcom), 80, auxchar);
  http.setTimeout(conf.timeoutNTP);
  int httpCode=http.GET();
  if (httpCode < 0) addlog(2, httpCode, c(ifttt));
  http.end();
  return httpCode;
}
//////////////////  END IFTTT  ///////////////////

void ICACHE_FLASH_ATTR pinVAL(byte n, byte value, byte ori)
{
  if ((n==sdPin[0]) || (n==sdPin[1]))
    if (getbit8(conf.MbC8,n-12)!=value)
      {
      digitalWrite(n, valorpin[value]);
      setbit8(conf.MbC8, n-12, value);
      saveconf();
      if (value) tempact[n-sdPin[0]]=millis()/1000;
      else tempdes[n-sdPin[0]]=millis()/1000;
      setbit8(iftttchange, n-12,1);
      if (ori==conf.iddevice) statusChange=true;
      }
}

int ICACHE_FLASH_ATTR pinvalR(byte ip, int port, byte pin, byte valor) // ejecuta comando remoto
{
  createhost(hostraiz,conf.netseg,ip);
  msg=vacio;
  printP(barra, valor?on:off, interr, letrap, ig, itoa(pin+12,buff,10));
  printP(amper, letrar, ig, itoa(conf.iddevice, buff, 10));
  return callhttpGET(host,port,false,conf.timeoutrem);
}

int ICACHE_FLASH_ATTR getMyIP()
{
  msg=vacio;
  printP(barra);
  HTTPClient http;
  http.begin(conf.hostmyip, 80, msg);
  http.setTimeout(conf.timeoutNTP);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) { msg=http.getString(); msg.toCharArray(conf.myippub, msg.length());  } }
  http.end();
  msg=vacio;
  return httpCode;
}

int ICACHE_FLASH_ATTR checkMyIP()
{
  char auxip[16];
  if (strcmp(conf.myippub, auxip) != 0) // son diferentes
    {
    saveconf();
    if (conf.iftttenable) ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, "NewIP", conf.myippub);
    }
}

void ICACHE_FLASH_ATTR setactivarem(byte num, boolean estado)
{
  actirem[num] = estado;
  for (byte j=0; j<maxsalrem; j++) if (conf.idsalremote[j]==conf.idremote[num]) actisenal[j] = estado;
}

int ICACHE_FLASH_ATTR actualizaremotos(byte mododirecto)    // pide datos a remotos
{
  int auxerr = 0;
  for (byte i=0; i<maxdevrem; i++)
    {
    if ((conf.idremote[i] < 150) || (conf.idremote[i] > 166))
      {
      strcpy (auxdesc, vacio); savedescr(fileidmyjsonrem, auxdesc, i, 10);
      strcpy (auxdesc, vacio); savedescr(filemacdevrem, auxdesc, i, 14);
      }
    else
      {
      if (actirem[i])
        {
        if (mododirecto == 1) { auxerr = ReqJson(conf.idremote[i], 88); }
        if (auxerr >= 0) { parseJsonremoto();  } // pone los valores en la variable "datosremoto" y en el remoto correspondiente
        else { timerem[i]++; if (timerem[i] >= maxerrorrem) { setactivarem(i, false); timerem[i] = 0; }   }
        msg = vacio;
        }
      else { timerem[i]++; if (timerem[i] >= 1)    { setactivarem(i, true); timerem[i] = 9; }  }
      }
    }
  return auxerr;
}

void ICACHE_FLASH_ATTR onescena(byte nesc)
{
  int auxerr;
  for (byte j = 0; j < maxSD; j++) //  Salidas digitales locales
    if (getbit8(conf.bescenaact[nesc], j))
      pinVAL(sdPin[j], getbit8(conf.bescena[nesc], j), conf.iddevice);
  for (byte i = 0; i < maxsalrem; i++) // para cada salida remota
    {
    onescenaact = true;
    if (conf.idsalremote[i] > 0)
      if (conf.senalrem[i] >= 6)
        if (getbit8(conf.bescenaact[nesc], i + 2) == 1)
          {
          byte auxest = getbit8(conf.bescena[nesc], i + 2);
          auxerr = pinvalR(conf.idsalremote[i], 88, conf.senalrem[i] - 6, auxest);
          if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11))) {
            setbit8(bstatremote, i, auxest);
            contaremote[i] = 0;  }
          }
    }
}

void ICACHE_FLASH_ATTR onCmd()
{
  if (autOK())
    if (server.args() > 0)
      {
      byte auxi=server.arg(0).toInt();
      if ((auxi==sdPin[0]) || (auxi==sdPin[1]))
        pinVAL(auxi, 1, server.arg(1).toInt());   // tercer parámetro es el Id ori que lo pide
      else if (auxi <= 200)
        onescena(auxi - 100);
      }
  sendOther(panelhtm, panelact);
}

void ICACHE_FLASH_ATTR offCmd()
{
  if (autOK())
    if (server.args() > 0)
      {
      byte auxi=server.arg(0).toInt();
      if ((auxi==sdPin[0]) || (auxi==sdPin[1])) pinVAL(auxi, 0, server.arg(1).toInt());
      }
  sendOther(panelhtm, panelact);
}

void ICACHE_FLASH_ATTR htmlNotFound()
{
  msg=vacio;
  printP(c(HTTP11), b);
  printP(c(t404), b);
  printP(c(pagenotfound), crlf);
  server.send(404, textplain, msg);
  msg=vacio;
}

void ICACHE_FLASH_ATTR printOpc(boolean colorea, boolean activa, char* texto)
{
  if (activa)
    printP(th, texto, th_f);
  else  {
    printP(colorea?th:td, href_i, comillas, auxchar, comillas, mayor);
    printP(texto, href_f, colorea?th_f:td_f);
  }
}

void ICACHE_FLASH_ATTR printOpc(boolean colorea, boolean activa, PGM_P texto)
{
  if (activa)
    printP(th, texto, th_f);
  else  {
    printP(colorea?th:td, href_i, comillas, auxchar, comillas, mayor);
    printP(texto, href_f, colorea?th_f:td_f);
  }
}

void ICACHE_FLASH_ATTR printOpc(boolean colorea, boolean activa, PGM_P texto, PGM_P link)
{
  if (activa)
    printP(th, texto, th_f);
  else  {
    printP(colorea?th:td, href_i, comillas, link, comillas, mayor);
    printP(texto, href_f, colorea ? th_f : td_f);
  }
}

void ICACHE_FLASH_ATTR printOpc(boolean colorea, boolean activa, PGM_P texto, PGM_P link, byte i)
{
  if (activa)
    printP(th, texto, th_f);
  else  {
    printP(colorea?th:td, href_i, comillas, link, interr, letran);
    printP(ig);
    printI(i);
    printP(comillas, mayor, texto, href_f, colorea ? th_f : td_f);
  }
}

void ICACHE_FLASH_ATTR writeMenu(byte opcprin, byte opcsec)
{
  printP(c(body_i), menor);
  printP(table, b);
  printP(c(tclass));
  printP(ig, tmenu, mayor, tr); // formato menú
  printOpc(false, (opcprin==1), t(zonas), panelhtm); // PANEL
  printOpc(false, (opcprin==3), t(configuracion), sdhtm); // CONFIGURACIÓN
  printOpc(false, (opcprin==2), t(programas), sprghtm); // Programas
  printOpc(false, (opcprin==4), t(sistema), espsyshtm); // Sistema

  if (conf.usepassDev)
  {
    printP(td, href_i, comillas, barra, loghtm);
    if (autOK())
      printP(interr, c(DISCONNECTYES), comillas, mayor, t(des));
    else
      printP(comillas, b, c(color000000), mayor);
    printP(t(conectar), href_f, td_f);
  }
  printP(tr_f, menor, barra, table, mayor);
  printP(menor, table, b);
  printP(c(tclass), ig);
  printP(tmenu, mayor, tr);  // formato menú
  if (opcprin == 1) // PANELES
    {
    for (byte i=0; i<maxpaneles; i++)
      if (getbit8(conf.bshowpanel, i))
        printOpc(false, opcsec==i, readdescr(filezonas, i, 20), panelhtm, i);
    }
  else if (opcprin==2) // PROGRAMACIÓN
    {
    printOpc(false, opcsec==5, t(programas), sprghtm);
    printOpc(false, opcsec==1, t(semanal), ssehtm);
    printOpc(false, opcsec==2, t(condiciones), svhtm);
    printOpc(false, opcsec==3, t(fecha), sfhtm);
    printOpc(false, opcsec==4, t(escenas), seschtm);
    printOpc(false, opcsec==7, t(webcalls), swchtm);
    }
  else if (opcprin==3) // CONFIGURACIÓN
    {
    printOpc(false, opcsec==0, t(dispositivo), sdhtm);
    printOpc(false, opcsec==5, t(zonas), sphtm);
    printOpc(false, opcsec==2, t(mandorf), rfhtm);
    printOpc(false, opcsec==3, t(red), snehtm);
    printOpc(false, opcsec==4, t(servred), snshtm);
    printOpc(false, opcsec==1, c(senales), siohtm);
    printOpc(false, opcsec==10, t(remotos), slkhtm);
    printOpc(false, opcsec==11, c(salremotas), sremhtm);
    }
  else if (opcprin==4) // SISTEMA
    {
    printOpc(false, opcsec==4, t(statust), espsyshtm);
    printOpc(false, opcsec==3, t(files), fileshtm);
    printOpc(false, opcsec==5, t(seguridad), sshtm);
    printOpc(false, opcsec==1, t(actualizar), suhtm);
    printOpc(false, opcsec==2, treset, rshtm);
    }
  printP(tr_f, menor, barra, table, mayor, br);
}

void ICACHE_FLASH_ATTR printtiempo(unsigned long segundos)
{
  if (segundos < 60)  {
    printIP(segundos, comillas);
    }
  else     {
    unsigned long minutos = segundos / 60;
    segundos = segundos % 60;
    if (minutos < 60)     {
      printIP(minutos, comilla);
      printPiP(b, segundos, comillas);
      }
    else      {
      unsigned long horas = minutos / 60;
      minutos = minutos % 60;
      if (horas < 24)  {
        printIP(horas, letrah);
        printPiP(b, minutos, comilla);
      }
      else          {
        unsigned int dias = horas / 24;
        horas = horas % 24;
        printIP(dias, letrad);
        printPiP(b, horas, letrah);
        }
      }
    }
}

void ICACHE_FLASH_ATTR termostatoHTML() {
  printremote();
  boolean conectado = autOK();
  msg=vacio;
  if (server.args() > 0)
    {
    if (server.argName(0)=="up") {
      conf.evenvalA[0] = constrain(conf.evenvalA[0] + 0.5, 5, 35);
      conf.evenvalA[1] = conf.evenvalA[0] + 1.0;
      }
    else if (server.argName(0)=="dw") {
      conf.evenvalA[0] = constrain(conf.evenvalA[0] - 0.5, 5, 35);
      conf.evenvalA[1] = conf.evenvalA[0] + 1.0;
      }
    saveconf();
    procesaeventos();
    sendOther(termhtm,-1);
    return;
    }

  writeHeader(false,false);
  printP(c(body_i), menor, barra);
  printP(table, mayor);
  printP(menor, table, b);
  printP(c(tclass), ig, tmenu);
  printP(mayor);

  // TEMPERATURA
  byte val = getbit8(conf.MbC8, 0);
  printP(tr);
  printColspan(2);
  printP(c(center_i));
  printP(t(Modo), b);
  printP(t(termostato));
  printP(c(center_f), td_f);
  printP(td, c(center_i));
  printP(c(fontsize6), val == 0 ? "☼" : "☀");
  printP(c(font_f));
  printP(c(center_f));

  printP(td_f, tr_f, tr);
  printColspan(3);
  printP(c(center_i));
  printP(c(fontsize_i), ig,siete, mayor);
  printF(MbR[0] * 0.01, 2);
  printP(c(font_f));
  printP(c(fontsize_i), ig,cinco, mayor, b, celsius);
  printP(c(font_f));
  printP(c(center_f), td_f, tr_f);
  // SALIDA DIGITAL

  printP(tr, td, c(center_i));
  printP(href_i, comillas, termhtm);
  printP(interr, letrad, letraw, ig, uno, comillas);
  printP(href_f);
  printP(c(fontsize_i), ig,cinco, mayor);
  printP(c(downs));
  printP(c(font_f));
  printP(c(center_f));
  printP(td_f, td, c(center_i));
  printP(c(fontsize_i), ig,tres, mayor);
  printF(conf.evenvalA[0], 1);
  printP(c(font_f), b, celsius);
  printP(c(center_f), td_f);
  printP(td, c(center_i));
  printP(href_i, comillas, termhtm);
  printP(interr, letrau, letrap, ig, uno, comillas);
  printP(href_f, c(fontsize_i), ig,cinco, mayor);
  printP(c(ups));
  printP(c(font_f));
  printP(c(center_f), td_f, tr_f);

  // final
  printP(tr);
  printColspan(2);
  printI(day()); printPiP(barra, month(), barra); printI(year());
  printP(b);
  printhora();
  printP(td_f, td, href_i, comillas);
  printP(barra, panelhtm, comillas, href_f, c(panel), td_f);
  printP(tr_f, menor, barra, table, mayor);
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
  onescenaact = false;
}

void HtmlGetStateIn(byte ind)
{
  colorea=(getbit8(conf.MbC8, ind+2)==1);
  printP(colorea?th:td,c(resetcontp));
  printI(ind);
  printP(comillas,mayor,readdescr(filedesclocal,ind+4,20),href_f);
  printP(colorea?th_f:td_f,td);
  printI(conf.contadores[ind]);
  printP(td_f);
}

void HtmlGetStateOut(byte ind)
{
  colorea=(getbit8(conf.MbC8, ind) == 1);
  unsigned long segundos = (millis()/1000)-(colorea?tempact[ind]:tempdes[ind]);
  printP(colorea?th:td, href_i,comilla, colorea?off:on,interr, letrap);
  printP(ig);
  printI(ind+12);
  printP(comilla, mayor,readdescr(filedesclocal,ind+6,20),href_f,colorea?th_f:td_f,td);
  printtiempo(segundos);
  printP(td_f);
}

void handleStateIn(int ind) { msg=vacio; HtmlGetStateIn(ind); serversend200();  }
void handleState0In() { handleStateIn(0); }
void handleState1In() { handleStateIn(1); }

void handleStateOut(int ind) { msg=vacio; HtmlGetStateOut(ind); serversend200();  }
void handleState0Out() { handleStateOut(0); }
void handleState1Out() { handleStateOut(1); }

void HtmlGetStateR(byte rem)
{
  colorea = getbit8(bstatremote, rem);
  unsigned long segundos = (millis() / 1000) - (contaremote[rem]);
  printP(colorea?th:td, href_i, comilla);
  printP(syhtm, interr, letrar, ig);
  printI(rem);
  printP(amper, colorea?c(roff):c(ron), ig, cero);
  printP(comilla, mayor, readdescr(filesalrem, rem, 20));
  printP(href_f, colorea?th_f:td_f, td);
  printtiempo(contaremote[rem]);
  printP(td_f);
}

//void handleStater(int i) {  msg=vacio; HtmlGetStateR(i); serversend200(); }
//void handleStater0() { handleStater(0); }
//void handleStater1() { handleStater(1); }
//void handleStater2() { handleStater(2); }
//void handleStater3() { handleStater(3); }
//void handleStater4() { handleStater(4); }
//void handleStater5() { handleStater(5); }
//void handleStater6() { handleStater(6); }
//void handleStater7() { handleStater(7); }
//void handleStater8() { handleStater(8); }
//void handleStater9() { handleStater(9); }
//void handleStater10() { handleStater(10); }
//void handleStater11() { handleStater(11); }
//void handleStater12() { handleStater(12); }
//void handleStater13() { handleStater(13); }
//void handleStater14() { handleStater(14); }
//void handleStater15() { handleStater(15); }

void voicecommandHTML()
{
  printremote();
  boolean conectado = (autOK());
  msg =vacio;
  if (server.args() == 2)
    {
    if ((server.argName(0).compareTo(letraa)==0) && (server.argName(1).compareTo(letrat)==0))
      {
      server.arg(1).toLowerCase();
      server.arg(1).toCharArray(auxchar, sizeof(auxchar));
      byte i=6;  boolean encontrado = false; // buscar en salidas locales
      while ((i<=7) && (!encontrado))
        {
        msg=vacio;
        printP(readdescr(filedesclocal, i++, 20));
        encontrado=strcharcomp();
        }
      if (encontrado) pinVAL(i+5, server.arg(0).toInt(), conf.iddevice);
      
      i=0; encontrado=false; // buscar en salidas remotas
      while ((i<=maxsalrem) && (!encontrado))
        {
        msg=vacio;
        printP(readdescr(filesalrem, i++, 20));
        encontrado=strcharcomp();
        }
      if (encontrado)
        if (conf.senalrem[i-1]>=6) pinvalR(conf.idsalremote[i-1],88,conf.senalrem[i-1]-6, server.arg(0).toInt());

      i=0; encontrado=false; // buscar en escenas
      while ((i<=maxEsc) && (!encontrado))
        {
        msg=vacio;
        printP(readdescr(filedescesc, i++, 20));
        encontrado=strcharcomp();
        }
      if (encontrado) onescena(i-1);
      }
    }
  msg=vacio;
  serversend200();
}

void ICACHE_FLASH_ATTR panelHTML() {
  printremote();
  boolean conectado = (autOK());
  msg=vacio;
  if (server.method()==HTTP_POST) return; 
  writeHeader(false,true);
  byte auxI=server.arg(0).toInt();
  panelact=auxI;
  writeMenu(1, auxI);
  printP(menor, table);
  printP(b, c(tclass), ig, tpanel, mayor, tr);
  printColspan(2);
  printP(readdescr(filezonas, auxI, 20), td_f, tr_f);

  /////////////  CONTENIDO   ///////////
  if (conectado)
    {
    // ESCENAS
    for (byte i=0; i<maxEsc; i++)
      if (getbit8(conf.bshowEsc, i))
        {
        printP(tr);
        printColspan(2);
        printP(href_i);
        printPiP(hrefon, i+100, comillas);
        printP(c(colorffffcc), mayor);
        printP(readdescr(filedescesc, i, 20), href_f, td_f, tr_f);
        }
    }

  // TEMPERATURAS DS18B20 locales
  for (byte i = 0; i < maxTemp; i++)
    if (getbit8(conf.bshowbypanel[auxI], i))
      {
      printP(tr,td);
      if (conf.showN) { printparentesis(letraS, i + 1);  for (byte j = 0; j < 8; j++) printH(addr1Wire[i][j]);   }
      printP(b, readdescr(filedesclocal,i,20), td_f, td);
      printF(MbR[i]*0.01,2);
      printP(b, celsius, td_f, tr_f);
      }

  for (byte i=0; i<maxsalrem; i++) // TEMPERATURAS DS18B20 remotas
  {
    if (getbit8(conf.bshowbypanel[auxI], i + 8))
      {
      if ((conf.idsalremote[i] >= 150) && (conf.idsalremote[i] <= 166) && (conf.senalrem[i] <= 2)) // remoto conuco
        {
        printP(tr, td);
        if (conf.showN) {
          printparentesis(letrar, i + 1);
          printparentesis(letraR, conf.idsalremote[i]);
        }
        printP(b, readdescr(filesalrem, i, 20), td_f, td);
        if (!actisenal[i]) printP(aster, b);
        printF(sondaremote[i], 2);
        printP(b, celsius, td_f, tr_f);
        }
      }
  }

  // ENTRADA ANALÓGICA
  if (getbit8(conf.bshowbypanel[auxI], 3))
  {
    printP(tr, td);
    if (conf.showN) printparentesis(letraa, 0);
    printP(readdescr(filedesclocal, 3, 20), td_f, td);
    printF((MbR[baseAna]*conf.factorA[0]) + conf.offsetA[0], 2);
    printP(b, conf.unitpinA, td_f, tr_f);
  }
  for (byte i=0; i<maxsalrem; i++) // entrada analógica remota
    if (getbit8(conf.bshowbypanel[auxI], i + 8))
      if (conf.senalrem[i] == 3)
      {
        printP(tr, td);
        if (conf.showN) {
          printparentesis(letrar, i + 1);
          printparentesis(letraR, conf.idsalremote[i]);
        }
        printP(b, readdescr(filesalrem, i, 20), td_f, td);
        if (!actisenal[i]) printP(aster, b);
        printF(sondaremote[i], 2);
        printP(b,readdescr(fileunitsalrem,i,13), td_f,tr_f);
      }

  // ENTRADAS DIGITALES
  if (conf.modo45 == 0)
    for (byte i=0; i<maxED; i++)
      if (getbit8(conf.bshowbypanel[auxI], i + 4))
        {
        if (conf.tipoED[i]==0)  // entrada digital ON/OFF
          {
          printP(menor, PSTR("tr"), b);
          printP(c(tid), ig, comilla, letral); 
          printI(i+2);
          printP(comilla, mayor);
          HtmlGetStateIn(i);
          printP(tr_f);
          }
        else if (conf.tipoED[i]==1)
          {
          printP(tr);
          printP(td, readdescr(filedesclocal, i + 4, 20), td_f, td);
          printF(dhtdata[i][0],1);
          printP(celsius, barraesp);
          printF(dhtdata[i][1],0);
          printP(porcen);
          printP(td_f);
          printP(tr_f);
          }
        }

  for (byte i = 0; i < maxsalrem; i++) // entradas digitales remotas
    if (getbit8(conf.bshowbypanel[auxI], i + 8))
      if ((conf.senalrem[i] >= 4) && (conf.senalrem[i] <= 5))
        {
        byte val = getbit8(bstatremote, i);
        printP(tr, (val == 0) ? td : th);
        if (conf.showN) { printparentesis(letrar,i+1); printparentesis(letraR,conf.idsalremote[i]); }
        if (!actisenal[i]) printP(aster, b);
        printP(readdescr(filesalrem, i, 20), (val == 0) ? td_f : th_f, td);
        printI(contaremote[i]);
        printP(td_f, tr_f);
        }

  // SALIDAS DIGITALES
  if (conectado)
    {
    for (byte i = 0; i < maxSD; i++)
      if (getbit8(conf.bshowbypanel[auxI], i + 6))
        {
        printP(menor, PSTR("tr"), b);
        printP(c(tid), ig, comilla, letral); 
        printI(i);
        printP(comilla, mayor);
        HtmlGetStateOut(i);
        printP(tr_f);
        }

    for (byte i = 0; i < maxsalrem; i++) // Salidas digitales remotas
      if (getbit8(conf.bshowbypanel[auxI], i + 8))
        if (conf.senalrem[i] >= 6)
          {
          byte val = getbit8(bstatremote, i);
          printP(menor, "tr", b);
          printP(c(tid), ig, comilla, letrar); printI(i);
          printP(comilla, mayor);
          HtmlGetStateR(i);
          printP(tr_f);
          }

    for (byte i = 0; i < maxsalrem; i++) // Sensores remotos Modbus o I2C
      {
      if (getbit8(conf.bshowbypanel[auxI], i + 8))
        {
        if ((conf.idsalremote[i] > 0) && ((conf.idsalremote[i] <= 149) || (conf.idsalremote[i] >= 167))) // modbus o I2C
          {
          if (posrem(conf.idsalremote[i]) > 0)
            {
            if (conf.tipoi2cmbrem[posrem(conf.idsalremote[i]) - 1] == 1)  // BMP085
              {
              printP(tr, td);
              if (conf.showN) { printparentesis(letrar, i+1); printparentesis(letraR, conf.idsalremote[i]);  }
              printP(b, readdescr(filesalrem, i, 20), td_f, td);
              //              printF(bmp085.readTemperature(),1);
              printP(b, celsius, barraesp);
              //              printF(bmp085.readPressure()/100,0);
              printP(b, letram, letrab, td_f, tr_f);
              }
            if (conf.tipoi2cmbrem[posrem(conf.idsalremote[i]) - 1] == 7)   // T-32-P
              {
              printP(tr, getbit8(mbstatus, i) == 1 ? th : td);
              if (conf.showN) { printparentesis(letrar, i+1); printparentesis(letraR, conf.idsalremote[i]);   }
              printP(b, readdescr(filesalrem, i, 20), getbit8(mbstatus, i) == 1 ? th_f : td_f, td);
              printF(mbtemp[i] * 0.01, 2);
              printP(barra);
              printF(mbcons[i] * 0.01, 2);
              printP(b, celsius, td_f, tr_f);
              }
            }
          }
        }
      }
    }

  if (conectado)    // webcalls
    for (byte i = 0; i < maxwebcalls; i++)
      if (getbit8(conf.bPRGwebcall, i) == 1)
        {
        printP(tr);
        printColspan(2);
        printP(href_i, comillas, syhtm, interr);
        printP(c(actwc), amper);
        printP(letran);
        printPiP(ig, i, comillas);
        printP(c(colorffffcc), mayor);
        printP(readdescr(filewebcall, i, 20), href_f, td_f, tr_f);
        }
  // final
  printP(tr);
  printColspan(2);
  printFecha();
  printP(b, c(PRG), b, (countfaulttime < conf.TempDesactPrg) ? ON : OFF, b);
  printI(ESP.getFreeHeap());
  printP(td_f, tr_f, menor, barra, table, mayor);
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
}

void ICACHE_FLASH_ATTR indexHTML() { if (conf.modoterm==0) panelHTML(); else termostatoHTML(); }

void ICACHE_FLASH_ATTR AddOri(byte numori)
{
  byte i=0;
  boolean encontrado=false;
  while ((i<16) && (ListOri[i]!= numori) && (!encontrado)) encontrado=(ListOri[i++]==numori);
  if (!encontrado) ListOri[i]=numori;
}

void ICACHE_FLASH_ATTR rjsonHTML() {
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=server.arg(0);
  if (!onescenaact) parseJsonremoto();
  msg=vacio;
  server.send(200, texthtml, msg);
}

void ICACHE_FLASH_ATTR rjsonconfHTML() {    // "/rjc"->parsejconf->saveconf
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg = server.arg(0);
  parseJsonConf();
  saveconf();
  msg=vacio;
  serversend200();
  if (hacerresetrem==1) ESP.reset();
}

void ICACHE_FLASH_ATTR jsonHTML() {
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  if (server.args() > 0)
    if (server.argName(0) == letrar)
      AddOri(server.arg(0).toInt());
  msg=vacio;
  buildJson();
  serversend200();
}

void ICACHE_FLASH_ATTR jsonconfHTML() {
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  if (server.args() > 0)
    if (server.argName(0) == letrar) AddOri(server.arg(0).toInt());
  msg=vacio;
  buildJsonConf(false, server.argName(0) == letram, false);
  serversend200();
}

void ICACHE_FLASH_ATTR jsonextHTML() {
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  buildjsonext();
  serversend200();
}

void ICACHE_FLASH_ATTR initPRG(void)
{
  memset(conf.actPrg,0,sizeof(conf.actPrg));        // todos los programas desactivados
  //////////////////////////////////  SEMANAL
  memset(conf.bprgval, 0, sizeof(conf.bprgval));
  memset(conf.bPRGsem, 0, sizeof(conf.bPRGsem));
  memset(conf.prgsal, 0, sizeof(conf.prgsal));
  memset(conf.prgdia, 0, sizeof(conf.prgdia));
  memset(conf.prghor, 0, sizeof(conf.prghor));
  memset(conf.prgmin, 0, sizeof(conf.prgmin));
  //////////////////////////////////// CONDICIONES
  memset(conf.bconacttipo,0,sizeof(conf.bconacttipo));
  memset(conf.condact,0,sizeof(conf.condact));
  memset(conf.bconactmode,0,sizeof(conf.bconactmode));
  memset(conf.condvalD,0,sizeof(conf.condvalD));
  memset(conf.evencomp,0,sizeof(conf.evencomp));
  memset(conf.evenvalA,0,sizeof(conf.evenvalA));
  memset(conf.evenhis,0,sizeof(conf.evenhis));
  memset(conf.bconsaltipo,0,sizeof(conf.bconsaltipo));
  memset(conf.evensal, 0, sizeof(conf.evensal));
  memset(conf.bevenniv, 0, sizeof(conf.bevenniv));
  memset(conf.bPRGeve, 0, sizeof(conf.bPRGeve));
  ///////////////////////////////////// FECHAS
  memset(conf.bactfec, 0, sizeof(conf.bactfec));
  memset(conf.fecsal, 0, sizeof(conf.fecsal));
  memset(conf.fecdia, 0, sizeof(conf.fecdia));
  memset(conf.fecmes, 0, sizeof(conf.fecmes));
  memset(conf.fecano, 0, sizeof(conf.fecano));
  memset(conf.fechor, 0, sizeof(conf.fechor));
  memset(conf.fecmin, 0, sizeof(conf.fecmin));
  memset(conf.bfecval, 0, sizeof(conf.bfecval));
  ////////////////////////////////////// ESCENAS
  memset(conf.bshowEsc,0,sizeof(conf.bshowEsc));
  for (byte i = 0; i < maxEsc; i++) {
    strcpy(auxdesc, t(escena)); strcat(auxdesc, b); strcat(auxdesc, itoa(i+1,buff,10));
    savedescr(filedescesc, auxdesc, i, 20);
  }
  memset(conf.bescena,0,sizeof(conf.bescena));
  memset(conf.bescenaact,0,sizeof(conf.bescenaact));
  memset(conf.bPRGeve,0,sizeof(conf.bPRGeve));

  conf.bPRGwebcall[0]=0;       //  desactiva WEBCALLS
  saveconf();
}

void ICACHE_FLASH_ATTR seleccionatipoi2cmb(byte i)
{
  printP(c(Select_name),comillas);
  printIP(mpi + 3, comillas);
  printP(mayor);
  for (byte j=0; j<30; j++)  { // añade tipo de remoto I2C o Modbus
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (i==j) printP(b, selected, ig, comillas, selected, comillas);
    printP(mayor,readdescr(filei2ctypes,j,20));
    printP(c(option_f));
    }
  printP(c(select_f));
}

void ICACHE_FLASH_ATTR setupremHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=4; // número de parámetros por fila
  if (server.method()==HTTP_POST)
    {
    for (byte i=0; i<maxdevrem; i++)
      { strcpy(auxdesc, vacio); savedescr(fileidmyjsonrem, auxdesc, i, 10);
      strcpy(auxdesc, vacio);  savedescr(filemacdevrem, auxdesc, i, 14);
      }
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (resto==1) { conf.idremote[indice] = server.arg(i).toInt(); } // Id del dispositivo
      else if (resto==3) { conf.tipoi2cmbrem[indice] = server.arg(i).toInt(); }
      }
    actualizaremotos(1);    // modo directo
    saveconf();
    //    initPRG();    // inicializa programaciones
    sendOther(slkhtm,-1);
    return;
    }
  // headers
  if (server.args()>0) { posactrem = constrain(server.arg(0).toInt(), 0, 15); }
  writeHeader(false,false);
  writeMenu(3, 10);
  writeForm(slkhtm);

  printP(tr, td, ID, barraesp, c(addrt), td_f);
  printP(td, t(ttipo), td_f);
  printP(td, t(configuracion), td_f, td, c(salremotas), td_f);
  printP(tr_f);
  for (byte  i=0; i<maxdevrem; i++)
  {
    mpi=mp*i;
    colorea = (posactrem == i);
    printP(tr);
    if (posactrem == i)   {
      printP(colorea ? th : td);
      printcampoI(mpi + 1, conf.idremote[i], 3, true);
      printP(colorea ? th_f : td_f);
      }
    else
      {
      strcpy(auxchar, slkhtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
      printOpc(false, false, (conf.idremote[i]==0) ? c(notdefined):itoa(conf.idremote[i], buff, 10), auxchar);
      }

    if (conf.idremote[i] != 0)
      {
      if ((conf.idremote[i] >= 150) && (conf.idremote[i] <= 166)) { // conuco
        printP(td, href_i, comillas);
        printP(c(thttp));
        printP(hostraiz);
        printIP(conf.netseg,punto);
        printIP(conf.idremote[i], dp);
        printI(88);
        printP(comillas, b, c(newpage), mayor);
        printP(c(conuco), href_f);
        printP(td_f,td, href_i, comillas, sdremhtm, interr);
        printPiP(c(trem), conf.idremote[i], comillas);
        printP(b, c(newpage), mayor);
        printP(readdescr(filedevrem, i, 20), href_f, td_f);
        printP(td, href_i, comillas, sdremiohtm, interr);
        printPiP(c(trem), conf.idremote[i], comillas);
        printP(b, c(newpage), mayor, io, href_f, td_f);
        }
      else if (conf.idremote[i] <= 31)          // modbus
        {
        printP(td, modbust, td_f);
        printColspan(2);
        if (posactrem == i)
          seleccionatipoi2cmb(conf.tipoi2cmbrem[i]);
        else
          printP(readdescr(filei2ctypes,conf.tipoi2cmbrem[i],20));
        printP(td_f);
        }
      else            // I2C
        {
        printP(td, i2c);
        if (posactrem == i)
          {
          printP(td_f);
          printColspan(2);
          seleccionatipoi2cmb(conf.tipoi2cmbrem[i]);
          }
        else
          {
          printP(b);
          printH(conf.idremote[i]);
          printP(letrah, td_f);
          printColspan(2);
          printP(readdescr(filei2ctypes,conf.tipoi2cmbrem[i],20));
          }
        printP(td_f);
        }
      }
    else
      {
      printColspan(3);
      if (posactrem==i)
        { printP(href_i, sdrem150htm, mayor);
          printP(t(nuevoremoto), href_f); }
      printP(td_f);
      }
    printP(tr_f);
    }
  printP(tr);
  printColspan(4);
  printP(t(pietiporem), td_f, tr_f);
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupsalremHTML()
{
  printremote();
  msg=vacio;
  if (!autOK()) { sendOther(loghtm,-1); return; }
  mp=5;  // número de parámetros por fila:
  prisalrem = constrain(server.arg(0).toInt(), 0, 24);
  posactsalrem = constrain(server.arg(1).toInt(), 0, 32);
  if (server.method() == HTTP_POST)
  {
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (resto == 2) {
        conf.idsalremote[indice] = ((server.arg(i).toInt()==0)?0:conf.idremote[server.arg(i).toInt() - 1]);
        }
      else if (resto==3) {
        server.arg(i).toCharArray(auxdesc,20);
        savedescr(filesalrem, auxdesc, indice, 20);  // sólo si no es remoto conuco
        }
      else if (resto == 4)
        if (conf.idsalremote[indice] > 32)       {
          conf.senalrem[indice] = server.arg(i).toInt();     // señal de salida, 0 a 7
          if (conf.senalrem[indice] >= 6) conf.pinremote[indice] = conf.senalrem[indice] - 6; else conf.pinremote[indice] = 99;
        }
      }
    for (byte j=0; j<maxpaneles; j++)
      for (byte i=0; i<maxsalrem; i++)
        if (conf.idsalremote[i]==0)
          setbit8(conf.bshowbypanel[j], i+8, 0);
    saveconf();
    //    initPRG();    // inicializa programaciones
    strcpy(auxchar, sremhtm);
    strcat(auxchar, igualp); strcat(auxchar, itoa(prisalrem, buff, 10));
    strcat(auxchar, amper); strcat(auxchar, letran); strcat(auxchar, ig); strcat(auxchar, itoa(posactsalrem, buff, 10));
    sendOther(auxchar,-1);  return;
  }

  writeHeader(false,false);
  writeMenu(3, 11);
  printP(c(form_action), sremhtm, interr, letrap, ig);
  printI(prisalrem);
  printP(amper, letran, ig);
  printI(posactsalrem);
  printP(comillas,b);
  printP(c(Form_post), menor);
  printP(table, b);
  printP(c(tclass), ig, tnormal, mayor);

  if (prisalrem > 0)
    {
    printP(tr);
    printColspan(4);
    printP(href_i, sremhtm, interr, letrap, ig);
    printI(prisalrem - 8);
    printP(mayor, c(ups), b);
    printI(prisalrem);
    printP(td_f, tr_f);
    }
  printP(tr, td, td_f, td, t(disp), td_f);
  printP(td, t(descripcion));
  printP(td_f, td, t(senal), td_f, tr_f);
  for (byte i=prisalrem; i<prisalrem+8; i++)
    {
    colorea=(i==posactsalrem);
    mpi=mp*i;
    printP(tr, td);
    printPiP(letraR, i, td_f);
    if (i==posactsalrem)  // código de remoto
      {
      printP(th);   // número de remoto
      printP(c(Select_name),comillas);
      printIP(mpi+2, comillas);
      printP(mayor);
      printP(c(optionvalue));
      printPiP(comillas, 0, comillas);
      if (conf.idsalremote[i] == 0) printP(b, selected, ig, comillas, selected, comillas);
      printPiP(mayor, 0, c(option_f));
      for (byte j = 0; j < maxdevrem; j++)  { // añade remotos
        if (conf.idremote[j] > 0) {
          printP(c(optionvalue));
          printPiP(comillas, j+1, comillas);
          if (conf.idremote[j] == conf.idsalremote[i]) printP(b, selected, ig, comillas, selected, comillas);
          printPiP(mayor, conf.idremote[j], c(option_f));
          }
        }
      printP(c(select_f), th_f);

      printP(colorea ? th : td); // descripción de señal remota
      if ((conf.idsalremote[i] == 0) || ((conf.idsalremote[i] >= 150) && (conf.idsalremote[i] <= 166)))
        printP(readdescr(filesalrem, i, 20));
      else
        {
        printP(menor, c(tinput), b, type, ig, comillas);
        printP(c(ttext), comillas, b);
        printP(c(namet), ig, comillas);
        printI(mpi+3);
        printP(comillas, b, tvalue);
        printP(ig, comillas, readdescr(filesalrem, i, 20));
        printP(comillas,b,c(max_length));
        printIP(19, size_i);
        printI(19);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        }
      printP(colorea ? th_f : td_f);

      printP(th);   // señal de salida
      if ((conf.idsalremote[i]==0) || ((conf.idsalremote[i]>=150) && (conf.idsalremote[i]<=166)))
        {
        printP(c(Select_name), comillas);
        printIP(mpi + 4, comillas);
        printP(mayor);
        for (byte j=0; j<8; j++)  { // añade señales del remoto
          printP(c(optionvalue));
          printPiP(comillas, j, comillas);
          if (conf.senalrem[i] == j) printP(b, selected, ig, comillas, selected, comillas);
          printP(mayor);
          if (j<=2) { printP(t(sonda), b); printI(j+1);  }
          else if (j==3) { printP(t(ent), b); printP(t(analogica));  }
          else if (j==4) { printP(t(ent), b); printP(c(digital), b, uno);  }
          else if (j==5) { printP(t(ent), b); printP(c(digital), b, dos);  }
          else if (j==6) { printP(t(saldig), b, uno);  }
          else if (j==7) { printP(t(saldig), b, dos);  }
          printP(c(option_f));
          }
        printP(c(select_f));
        }
      printP(th_f);
      }
    else
      {
      strcpy(auxchar, sremhtm); strcat(auxchar, igualp); strcat(auxchar, itoa(prisalrem, buff, 10));
      strcat(auxchar, amper); strcat(auxchar, letran); strcat(auxchar, ig); strcat(auxchar, itoa(i, buff, 10));
      printOpc(false, false, conf.idsalremote[i] == 0 ? c(notdefined ): itoa(conf.idsalremote[i], buff, 10));
      printP(colorea ? th : td); // descripción de señal remota
      if (conf.idsalremote[i] > 0) printP(readdescr(filesalrem, i, 20));
      printP(colorea ? th_f : td_f);
      printP(td);
      if (conf.idsalremote[i]>32) // escribir señal de conuco
        {
        if (conf.idsalremote[i]>0)
          {
          if (conf.senalrem[i]<=2) { printP(t(sonda), b); printI(conf.senalrem[i] + 1);  }
          else if (conf.senalrem[i]==3) { printP(t(ent), b, t(analogica)); }
          else if (conf.senalrem[i]==4) { printP(t(ent), b, c(digital), b, uno); }
          else if (conf.senalrem[i]==5) { printP(t(ent), b, c(digital), b, dos); }
          else if (conf.senalrem[i]==6) { printP(t(saldig), b, uno); }
          else if (conf.senalrem[i]==7) { printP(t(saldig), b, dos); }
          }
        }
      printP(td_f);
      }
    printP(tr_f);
    }
  if (maxsalrem-prisalrem-8>0)
   {
    printP(tr);
    printColspan(4);
    printP(href_i, sremhtm, interr, letrap, ig);
    printI(prisalrem + 8);
    printP(mayor, c(downs), b);
    printI(maxsalrem - prisalrem - 8);
    printP(td_f, tr_f);
    }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupioHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=6;  // número de parámetros por fila
  if (server.method() == HTTP_POST)
  {
    if ((posactio >= 4) && (posactio <= 5)) { setbit8(conf.iftttpinED, posactio-4,0); setbit8(conf.iftttpinED, posactio+4,0); }
    if ((posactio >= 6) && (posactio <= 7)) { setbit8(conf.iftttpinSD, posactio-6,0); setbit8(conf.iftttpinSD, posactio+2,0); }
    for (int i = 0  ; i < server.args(); i++)
      {
      calcindices(i);
      if (indice <= 2)  // temperaturas
        {
        if (resto==0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesclocal, auxdesc, indice, 20); }
        else if (resto==1) conf.setpoint[indice]=server.arg(i).toFloat();  // valor consigna
        else if (resto==2) conf.salsetpoint[indice]=server.arg(i).toInt(); // salida asociada
        else if (resto==3) conf.accsetpoint[indice]=server.arg(i).toInt(); // acción consigna
        }
      else if (indice == 3) // entrada analógica
        {
        if (resto == 0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesclocal, auxdesc, indice, 20); }
        else if (resto == 1) { server.arg(i).toCharArray(conf.unitpinA, 4); } // unidades
        else if (resto == 2) conf.factorA[indice - 3] = server.arg(i).toFloat();
        else if (resto == 3) conf.offsetA[indice - 3] = server.arg(i).toFloat();
        else if (resto == 4) setbit8(conf.bsumatA, indice - 3, 1); // Mostrar si/no suma
        }
      else if ((indice >= 4) && (indice <= 5)) // entradas digitales
        {
        if (resto == 0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesclocal, auxdesc, indice, 20); }
        else if (resto == 1) conf.tipoED[indice - 4] = server.arg(i).toInt();
        else if (resto == 4) setbit8(conf.iftttpinED, indice - 4, server.arg(i).toInt()); // Notificar si/no
        else if (resto == 5) setbit8(conf.iftttpinED, indice - 4 + 8, server.arg(i).toInt()); // Notificar si/no
        }
      else if ((indice >= 6) && (indice <= 7)) // salidas digitales
        {
        if (resto == 0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesclocal, auxdesc, indice, 20); }
        else if (resto == 1) conf.valinic[indice-6] = server.arg(i).toInt();            // valor inicial
        else if (resto == 2) conf.tempdefact[indice-6] = server.arg(i).toInt();         // Seg. ON
        else if (resto == 3) conf.tempdefdes[indice-6] = server.arg(i).toInt();         // Seg. OFF
        else if (resto == 4) setbit8(conf.iftttpinSD, indice-6, server.arg(i).toInt()); // Notificar si/no
        else if (resto == 5) setbit8(conf.iftttpinSD, indice+2, server.arg(i).toInt()); // Notificar si/no
          }
        }
    if (conf.modoterm == 1) { conf.tempdefact[0] = 0; conf.tempdefdes[0] = 0;  }
    saveconf();
    sendOther(siohtm,-1); return;
    }
  if (server.args() > 0) { posactio = constrain(server.arg(0).toInt(), 0, 7); }
  writeHeader(false,false);
  writeMenu(3,1);
  writeForm(siohtm);

  printP(tr,td,t(descripcion),td_f,tr_f);
  for (byte i=0; i<8; i++)
    {
    printP(tr);
    mpi=mp*i;
    if (i<=2) // Sondas temperatura
      {
      if (posactio==i)
        {
        printP(td, menor, c(tinput), b, type, ig);
        printP(comillas, c(ttext), comillas, b);
        printP(c(namet), ig);
        printPiP(comillas, mpi, comillas);
        printP(b, tvalue, ig, comillas);
        printP(readdescr(filedesclocal, i, 20));
        printP(comillas,b,c(max_length));
        printIP(19, size_i);
        printI(19);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        printP(td_f);

        printP(td);
        printP(t(consigna));
        printcampoF(mpi+1, conf.setpoint[i], 1); // valor de consigna
        printP(td_f);

        printP(td);
        printP(t(salida));
        printP(c(Select_name),comillas);
        printI(mpi+2);
        printP(comillas, mayor);       // señal de salida
        for (byte j=0; j < maxSD; j++)   { // salidas digitales locales
          printP(c(optionvalue));
          printPiP(comillas, j, comillas);
          if (conf.salsetpoint[i] == j) printP(b, selected, ig, comillas, selected, comillas);
          if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
          printP(readdescr(filedesclocal,j+6,20));
          printP(c(option_f));
          }
        for (byte j=0; j<maxEsc; j++) // escenas, 2
          {
          printP(c(optionvalue));
          printPiP(comillas, j+2, comillas);
          if (conf.salsetpoint[i]==j+2) printP(b, selected, ig, comillas, selected, comillas);
          if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
          printP(readdescr(filedescesc, j, 20));
          printP(c(option_f));
          }
        for (byte j=0; j<maxsalrem; j++) // salidas remotas, 32
          if (conf.idsalremote[j]>0)
            if (conf.senalrem[j]>=6)
              {
              printP(c(optionvalue));
              printPiP(comillas, j+4, comillas);
              if (conf.salsetpoint[i]==j+4) printP(b, selected, ig, comillas, selected, comillas);
              if (conf.showN) printPiP(mayorparen, j+4, parenguion); else printP(mayor);
              printP(readdescr(filesalrem,j,20));
              printP(c(option_f));
              }
        printP(c(select_f));
        printP(td_f);
        
        printColspan(2);
        printP("ON/OFF");
        printP(c(Select_name),comillas);
        printIP(mpi+3, comillas);
        printP(mayor);
        for (byte k=0; k<3; k++) {
          printP(c(optionvalue));
          printPiP(comillas, k, comillas);
          if (k==conf.accsetpoint[i]) printP(b, selected, ig, comillas, selected, comillas);
          printP(mayor, k==0?guion:k==1?OFF:ON,c(option_f));
          }
        printP(c(select_f));
        }
      else
        {
        readdescr(filedesclocal,i,20);
        strcpy(auxchar, siohtm); strcat(auxchar, igualp); strcat(auxchar,itoa(i,buff,10));
        printOpc(false, false, auxdesc);
        printColspan(4);
        for (byte j=0; j<8; j++) {printH(addr1Wire[i][j]);printP(b);}
        }
      printP(td_f, tr_f);
      }
    if (i==3) // entrada analógica
      {
      if  (posactio==i)
        {
        printP(td, menor, c(tinput), b, type, ig);
        printP(comillas, c(ttext), comillas, b);
        printP(c(namet), ig);
        printPiP(comillas, mpi, comillas);
        printP(b, tvalue, ig, comillas);
        printP(readdescr(filedesclocal, i, 20));
        printP(comillas,b,c(max_length));
        printIP(19, size_i);
        printI(19);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        printP(td_f, td, c(unit), b);
  
        printP(menor, c(tinput), b, type, ig);
        printP(comillas, c(ttext), comillas, b);
        printP(c(namet), ig);
        printPiP(comillas, mpi + 1, comillas);
        printP(b, tvalue, ig, comillas, conf.unitpinA);
        printP(comillas,b,c(max_length));
        printIP(3, size_i);
        printI(3);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        printP(td_f, td, c(factor), b);
  
        printcampoF(mpi+2, conf.factorA[i - 3], 5);
        printP(td_f, td, c(offset), b);
        printcampoF(mpi + 3, conf.offsetA[i - 3], 5);
        colorea = getbit8(conf.bsumatA, i - 3);
        printP(td_f, td, sumat, b);
        checkBox(mpi+4, (getbit8(conf.bsumatA, i - 3)));
        }
      else
        {
        readdescr(filedesclocal,i,20);
        strcpy(auxchar, siohtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
        printOpc(false, false, auxdesc);
        printP(td, conf.unitpinA, td_f, td);
        printF(conf.factorA[i-3], 5);
        printP(td_f, td);
        printF(conf.offsetA[i-3], 5);
        printP(td_f, td, getbit8(conf.bsumatA,i-3)?c(si):guion);
        }
      printP(td_f, tr_f);
      }
    if (conf.modo45 == 0)
      {
      if ((i>=4) && (i<=5))    // entradas digitales
        {
        if (posactio==i) 
          {
          printP(td, menor, c(tinput), b, type, ig);
          printP(comillas);
          printP(c(ttext), comillas, b);
          printP(c(namet), ig, comillas);
          printI(mpi);
          printP(comillas, b, tvalue, ig, comillas);
          printP(readdescr(filedesclocal, i, 20));
          printP(comillas,b,c(max_length));
          printIP(19, size_i);
          printI(19);
          printP(comillas, mayor, menor, barra, c(tinput), mayor);
          printP(td_f, td, t(ttipo), b);
          printcampoCB(mpi + 1, conf.tipoED[i - 4], ONOFF, dhtt);
          printP(td_f);
          printColspan(2);
          printP(td, c(ifttt), b, ON, barra, OFF);
          checkBox(mpi+4, getbit8(conf.iftttpinED, i-4)); // checkbox Notificar ON
          printP(barra);
          checkBox(mpi+5, getbit8(conf.iftttpinED, i+4)); // checkbox Notificar OFF
          }
        else
          {
          readdescr(filedesclocal,i,20);
          strcpy(auxchar, siohtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
          printOpc(false, false, auxdesc);
          printP(td, conf.tipoED[i - 4] == 0 ? ONOFF : dhtt, colorea ? th_f : td_f);
          printColspan(2);
          printP(td, getbit8(conf.iftttpinED,i-4)?c(si):guion);
          printP(barra, getbit8(conf.iftttpinED,i+4)?c(si):guion);
          }
        printP(td_f,tr_f);
        }
      }
   if ((i>=6) && (i<=7))    // salidas digitales
      {
      if (posactio==i) 
        {
        printP(td, menor, c(tinput), b, type, ig);
      printP(comillas, c(ttext), comillas, b);
      printP(c(namet), ig);
        printPiP(comillas, mpi, comillas);
        printP(b, tvalue, ig, comillas);
        printP(readdescr(filedesclocal, i, 20));
        printP(comillas,b,c(max_length));
        printIP(19, size_i);
        printI(19);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        printP(td_f, td, c(defaultt), b);
        printcampoCB(mpi + 1, conf.valinic[i - 6], OFF, ON, t(ultimovalor));
        printP(td_f, td, c(tdefact), b);
        printcampoL(mpi+2, conf.tempdefact[i - 6], 8, true);
        printP(td_f, td, c(tdefdes), b);
        printcampoL(mpi+2, conf.tempdefdes[i - 6], 8, true);
        printP(td_f, td, c(ifttt));
        printP(b, ON, barra, OFF);
        checkBox(mpi + 4, getbit8(conf.iftttpinSD, i - 6)); // checkbox Notificar ON
        printP(barra);
        checkBox(mpi + 5, getbit8(conf.iftttpinSD, i - 6 + 8)); // checkbox Notificar OFF
        }
      else
        {
        readdescr(filedesclocal,i,20);
        strcpy(auxchar, siohtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
        printOpc(false, false, auxdesc);
        printP(td, conf.valinic[i-6]==0?OFF : conf.valinic[i - 6] == 1 ? ON : t(ultimovalor), td_f);
        printPiP(td, conf.tempdefact[i-6], td_f);
        printPiP(td, conf.tempdefdes[i-6], td_f);
        printP(td, getbit8(conf.iftttpinSD,i-6)?c(si):guion, barra, getbit8(conf.iftttpinSD,i+2)?c(si):guion);
        }
      printP(td_f, tr_f);
      }
    }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupDevHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=1;
  if (server.method()==HTTP_POST)
    {
    conf.showN=0; conf.modoterm=0; conf.actualizaut=0;
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (param_number==0)
        {
        conf.iddevice = server.arg(i).toInt(); // núm. dispositivo
        conf.EEip[3] = conf.iddevice;
        strcpy(conf.ssidAP, c(CONUCO)); strcat(conf.ssidAP, subray); strcat(conf.ssidAP, itoa(conf.iddevice, buff, 10));
        }
      else if (param_number==1) { conf.showN=server.arg(i).toInt(); } // mostrar datos pin
      else if (param_number==2) { conf.peractpan=server.arg(i).toInt();  }  // Per. actualización
      else if (param_number==3) { conf.prectemp=server.arg(i).toInt(); sensors1.setResolution(conf.prectemp); }  // Precision sondas
      else if (param_number==6) { conf.TempDesactPrg = server.arg(i).toInt(); } // seg. desact. prog.
      else if (param_number==8) { server.arg(i).toCharArray(conf.aliasdevice, sizeof(conf.aliasdevice)); } // aliasdevice
      else if (param_number==9) { conf.peractrem = server.arg(i).toInt(); }  // Per. actualización remotos
      else if (param_number==10) { server.arg(i).toCharArray(conf.instname, server.arg(i).length()+1); }  // instname
      else if (param_number==14) { conf.modoterm=server.arg(i).toInt(); if (conf.modoterm==1) { conf.tempdefact[0]=0; conf.tempdefdes[0]=0; }}  // modo termostato
      else if (param_number==15) { conf.latitud = server.arg(i).toFloat();  }
      else if (param_number==16) { conf.longitud = server.arg(i).toFloat(); }
      else if (param_number==17) { conf.actualizaut=server.arg(i).toInt(); } // actualización automática
      else if (param_number==18) { server.arg(i).toCharArray(conf.fwUrlBase, server.arg(i).length()+1); }  // fwUrlBase
      else if (param_number==19) { conf.modo45 = server.arg(i).toInt(); } // modo45
      else if (param_number==20) { conf.lang = server.arg(i).toInt(); } // idioma
      }
    saveconf();
    sendOther(sdhtm,-1);
    return;
    }

  writeHeader(false,false);
  writeMenu(3, 0);
  writeForm(sdhtm);

  printP(tr, td, t(instalacion),barraesp);
  printP(c(alias), barraesp);
  printP(t(dispositivo), td_f,td);
  printcampoC(10, conf.instname, 10, true, true, false);
  printP(td_f,td);
  printcampoC(8, conf.aliasdevice, 20, true, true, false);
  printP(td_f);
  printColspan(2);
  printcampoCB(0, conf.iddevice, 150, 166);
  printP(td_f, tr_f,tr, td, t(Modo),b);
  printP(t(pines), b);
  printP(cuatro, guion, cinco, td_f, td);
  
  printcampoCB(19, conf.modo45,diginput,i2c,modbust);
  printP(td_f, td, t(mostrarpines), td_f, conf.showN?th:td);
  checkBox(1, conf.showN);
  printP(conf.showN?th_f:td_f, tr_f);

  printP(tr, td, t(Act), b);
  printP(t(aut), barra);
  printP(t(versiont), td_f, conf.actualizaut ? th : td);
  checkBox(17, conf.actualizaut);
  colorea = (verserver > versinst);
  printP(conf.actualizaut ? th_f : td_f, colorea ? th : td);
  printI(versinst);
  printP(colorea ? th_f : td_f, colorea ? th : td);
  printI(verserver);
  printP(colorea?th_f:td_f,tr_f);

  printP(tr, td, href_i, comillas, conf.fwUrlBase, comillas);
  printP(b, c(newpage), mayor);
  printP(c(Firmware), b);
  printP(c(tserver), href_f, td_f);
  printColspan(3);
  printcampoC(18, conf.fwUrlBase, 49, true, true, false);  printP(td_f, tr_f);

  printP(tr, td, t(periodoact255), barra);
  printP(t(periodoactrem), barra);
  printP(t(tdp), td_f, td);
  printcampoI(2, conf.peractpan, 5, true);  printP(td_f, td);
  printcampoI(9, conf.peractrem, 5, true);  printP(td_f, td);
  printcampoL(6, conf.TempDesactPrg, 5, true);  printP(td_f, tr_f);

  printP(tr, td, href_i, comillas);
  printP(c(thttp));
  printP(c(gmaps));
  printP(comillas, b, c(newpage), mayor);
  printP(t(latitudt), barraesp);
  printP(t(longitudt), href_f, td_f, td);
  printcampoF(15, conf.latitud, 6);  printP(td_f, td);
  printcampoF(16, conf.longitud, 6);  printP(td_f, td, td_f, tr_f);

  printP(tr, td, PSTR("Idioma"),b);
  printP(t(pines), b);
  printP(cuatro, guion, cinco, td_f, td);
  printcampoCB(20, conf.lang, PSTR("Español"), PSTR("English"));  printP(td_f);
  printColspan(2);
  printP(td_f,tr_f);

  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupPanelHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=2;
  if (server.method() == HTTP_POST)
    {
    memset(conf.bshowpanel,0,sizeof(conf.bshowpanel));
    for (int i = 0; i < server.args(); i++)
      {
      calcindices(i);
      if (resto==0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filezonas, auxdesc, indice, 20); }
      else if (resto==1) { setbit8(conf.bshowpanel, indice, server.arg(i).toInt()); }
      }
    saveconf();
    sendOther(sphtm,-1);
    return;
    }

  writeHeader(false,false);
  writeMenu(3, 5);
  writeForm(sphtm);
  printP(tr, c(td_if), td);
  printP(t(descripcion), td_f);
  printP(td, t(activo), td_f, tr_f);
  for (byte i=0; i<maxpaneles; i++)  {
    mpi=mp*i;
    colorea=(getbit8(conf.bshowpanel, i));
    printP(tr, td, href_i, sbphtm, interr, letran);
    printPiP(ig, i, b);
    printP(c(newpage), mayor);
    printPiP(letraZ, i + 1, td_f);
    printP(td);
    printP(menor, c(tinput), b, type, ig, comillas);
    printP(c(ttext), comillas, b);
    printP(c(namet), ig, comillas);
    printI(mpi);    // número de parámetro
    printP(comillas, b, tvalue, ig, comillas);
    printP(readdescr(filezonas, i, 20));
    printP(comillas,b,c(max_length));
    printIP(19, size_i);
    printI(19);
    printP(comillas, b, mayor, menor, barra);
    printP(c(tinput), mayor, colorea ? th_f : td_f, colorea ? th : td);
    checkBox(mpi + 1, colorea);
    printP(colorea ? th_f : td_f, tr_f);
    }
  writeFooter(tguardar, false);
  serversend200(); 
}

void ICACHE_FLASH_ATTR setupbyPanelHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=1;
  byte auxI = server.arg(0).toInt();
  byte pribypanel = constrain(server.arg(1).toInt(), 0, 32);
  if (server.method() == HTTP_POST)
    {
    for (byte i=pribypanel; i<pribypanel+8; i++) setbit8(conf.bshowbypanel[auxI], i, 0);
    for (int i=0; i<server.args(); i++)
      {
      if ((server.argName(i)!="n") && (server.argName(i)!="p"))
        { calcindices(i); setbit8(conf.bshowbypanel[auxI], indice, 1);   }
      }
    saveconf();
    strcpy(auxchar, sbphtm); strcat(auxchar, paramn); strcat(auxchar, itoa(auxI, buff, 10));
    strcat(auxchar, amper); strcat(auxchar, letrap); strcat(auxchar, ig); strcat(auxchar, itoa(pribypanel, buff, 10));
    sendOther(auxchar,-1); return;
    }

  writeHeader(false,false);
  writeMenu(3, 5);
  printP(c(form_action), sbphtm, paramn);
  printI(auxI);
  printP(amper, letrap, ig);
  printI(pribypanel);
  printP(comillas,b);
  printP(c(Form_post), menor);
  printP(table, b);
  printP(c(tclass), ig, tnormal, mayor);

  if (pribypanel > 0)
    {
    printP(tr);
    printColspan(3);
    printP(href_i, sbphtm, interr, letran);
    printPiP(ig, auxI, amper);
    printP(letrap, ig);
    printI(pribypanel - 8);
    printP(mayor, c(ups), b);
    printI(pribypanel);
    printP(href_f, td_f, tr_f);
    }
  printP(tr);
  printColspan(3);
  printP(readdescr(filezonas, auxI, 20), td_f, tr_f);
  for (byte i = pribypanel; i < pribypanel + 8; i++)  {
    mpi= mp*i;
    colorea = (getbit8(conf.bshowbypanel[auxI], i));
    printP(tr, td, i <= 7 ? letraL : letraR);
    printI(i <= 7 ? i : i - 8);
    printP(td_f, colorea ? th : td);
    if (i <= 2) printP(readdescr(filedesclocal, i, 20));
    else if (i == 3) printP(readdescr(filedesclocal, i, 20));
    else if (i <= 5) printP(readdescr(filedesclocal, i, 20));
    else if (i <= 7) printP(readdescr(filedesclocal, i, 20));
    else if (i <= 39) printP(conf.idsalremote[i - 8] > 0 ? readdescr(filesalrem, i - 8, 20) : c(notdefined));
    printP(colorea ? th_f : td_f, colorea ? th : td);
    checkBox(mpi, colorea);
    printP(colorea ? th_f : td_f, tr_f);
    }
  if (40-pribypanel-8 > 0)
    {
    printP(tr);
    printColspan(3);
    printP(href_i, sbphtm, interr, letran);
    printPiP(ig, auxI, amper);
    printP(letrap, ig);
    printI(pribypanel + 8);
    printP(mayor, c(downs), b);
    printI(40 - pribypanel - 8);
    printP(href_f, td_f, tr_f);
    }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupNetServHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=1;
  if (server.method() == HTTP_POST)
    {
    conf.mododweet=0; conf.iftttenable=0; conf.modomyjson=0; 
    conf.ftpenable=0; conf.mqttenable=0; conf.iottweetenable=0;
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (param_number==0) { server.arg(i).toCharArray(conf.hostmyip, 30); }
      else if (param_number==1) { conf.ftpenable=server.arg(i).toInt();  } // ftp server enabled      }
      else if (param_number==2) { conf.iftttenable=server.arg(i).toInt(); } // enable IFTTT
      else if (param_number==3) { server.arg(i).toCharArray(conf.iftttkey, 30); }
      else if (param_number==4) { conf.mododweet=server.arg(i).toInt(); }   // modo Dweet.io
      else if (param_number==5) { conf.modomyjson=server.arg(i).toInt(); if (conf.modomyjson==0) conf.idmyjsonST=0; }  // modo myjson.com
      else if (param_number==6) { conf.iottweetenable=server.arg(i).toInt();  } // iottweet enabled      }
      else if (param_number==7) { server.arg(i).toCharArray(conf.iottweetuser, 10); }
      else if (param_number==8) { server.arg(i).toCharArray(conf.iottweetkey, 15);  }
      else if (param_number==9) { conf.mqttenable=server.arg(i).toInt(); } // enable MQTT
      else if (param_number==10) { server.arg(i).toCharArray(conf.mqttserver, 40);  }
      else if (param_number>=11) { server.arg(i).toCharArray(conf.mqttpath[param_number-11], 10);  }
      }
    saveconf();
    sendOther(snshtm,-1);
    return;
    }

  writeHeader(false,false);
  writeMenu(3, 4);
  writeForm(snshtm);

  printP(tr, td, t(ippubserver), td_f, td, td_f);
  printColspan(2);
  printcampoC(0, conf.hostmyip, 30, true, true, false);
  printP(td_f, tr_f);

  printP(tr,td,t(ftpserver),td_f,td);
  checkBox(1, conf.ftpenable);
  printP(td_f,c(td_if));
  printP(c(td_if),tr_f);

  printP(tr, td, href_i, comillas);
  printP(c(thttps));
  printP(c(iftttcom));
  printP(comillas, b, c(newpage), mayor);
  printP(c(ifttt), barraesp);
  printP(c(Key), href_f, td_f, conf.iftttenable ? th : td);
  checkBox(2, conf.iftttenable);
  printP(conf.iftttenable?th_f:td_f);
  printColspan(2);
  if (clientremote()) printP(c(hidden)); else printcampoC(3, conf.iftttkey, 30, true, true, false);
  printP(td_f, tr_f);

  printP(tr, td, href_i, comillas);
  printP(c(thttp));
  printP(c(dweetio));
  printP(comillas, b, c(newpage), mayor);
  printP(c(dweet), barraesp);
  printP(c(dweetname), href_f, td_f, conf.mododweet ? th : td);
  checkBox(4, conf.mododweet);
  printP(td_f);
  printColspan(2);
  if (conf.mododweet == 1)
    if (clientremote()) { printP(c(hidden));  }
    else   {
      printP(href_i);
      printP(c(urldweet));
      printP(c(conuco)); // 5 últimos dweets 24h
      printP(mac, mayor, c(conuco), mac, href_f);
    }
  printP(td_f, tr_f);

  printP(tr, td, href_i, comillas);
  printP(c(thttp));
  printP(c(myjsoncom), comillas);
  printP(b, c(newpage), mayor);
  printP(c(modomyjsont));
  printP(barraesp, c(Key), href_f, td_f, conf.modomyjson ? th : td);
  checkBox(5, conf.modomyjson);
  printP(conf.modomyjson ? th_f : td_f);
  printColspan(2);
  if (conf.modomyjson == 1) {
    if (clientremote())
      printP(c(hidden));
    else {
      printP(href_i, comillas);
      printP(c(thttps));
      printP(c(api), punto);
      printP(c(myjsoncom));
      printP(c(bins), conf.idmyjson, interr);
      printP(c(pretty), comillas);
      printP(mayor, conf.idmyjson, href_f);
      }
    }
  printP(td_f, tr_f);

  printP(tr, td, href_i, comillas);
  printP(c(thttp));
  printP(c(iottweetcom));
  printP(comillas, b, c(newpage), mayor);
  printP(c(iottweett));
  printP(barraesp, t(usuario), barraesp);
  printP(c(Key), href_f, td_f, conf.iottweetenable == 1 ? th : td);
  checkBox(6, conf.iottweetenable);
  printP(conf.iottweetenable==1?th_f:td_f,td);
  if (clientremote()) printP(c(hidden)); else printcampoC(7, conf.iottweetuser, 10, true, true, false);
  printP(td_f, td);
  if (clientremote()) printP(c(hidden)); else printcampoC(8, conf.iottweetkey, 15, true, true, false);
  printP(td_f, tr_f);

  printP(tr,td,c(mqtt),b);
  printP(c(tserver),td_f);
  printP(conf.mqttenable==1?th:td);
  checkBox(9, conf.mqttenable);
  printP(td_f);
  printColspan(2);
  printcampoC(10, conf.mqttserver, 40, true, true, false);
  printP(conf.mqttenable==1?th_f:td_f,tr_f);
  
  for (byte i=0;i<3;i++)
    {
    printP(tr);
    printColspan(2);
    if (i==0) { printP(c(mqtt),b); printP(c(tpath)); }
    printP(td_f,td);
//    printcampoC(11+(i*2), conf.mqttpath[i*2], 10, true, (i*2>=6), false);
    printcampoC(11+(i*2), conf.mqttpath[i*2], 10, true, true, false);
    printP(barra,td_f,td);
//    printcampoC(12+(i*2), conf.mqttpath[i*2+1], 10, true, (i*2>=6), false);
    printcampoC(12+(i*2), conf.mqttpath[i*2+1], 10, true, true, false);
    printP(barra,td_f,tr_f); 
    }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR lineasetuprfonoff(boolean remote, byte num, char* texto)
{
  printP(tr, td, href_i, syhtm, interr, letran);
  printPiP(ig, num, amper);
  printP(letrar, ig);
  if (remote) printP(letrar);
  printP(rfon, mayor);
  printP(texto, b, ON, td_f,td);
  printL(conf.code433.codeon[remote?num+2:num]);
  printP(td_f,td, href_i, syhtm, interr, letran);
  printPiP(ig, num, amper);
  printP(letrar,ig);
  if (remote) printP(letrar);
  printP(rfoff, mayor);
  printP(texto, b, OFF, td_f,td);
  printL(conf.code433.codeoff[remote?num+2:num]);
  printP(td_f,tr_f);
}

void ICACHE_FLASH_ATTR lineasetuprfkey(byte num, PGM_P texto, unsigned long valor)
{
  printP(td, href_i, syhtm, interr, letran);
  printPiP(ig, num, amper);
  printP(letrar, ig, trestart, mayor);
  printP(texto, td_f,td);
  printL(valor);
  printP(td_f);
}

void ICACHE_FLASH_ATTR setuprfHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  if (server.method()==HTTP_POST) { saveconf(); sendOther(rfhtm,-1); return; }
  writeHeader(false,false);
  writeMenu(3,2);
  writeForm(rfhtm);
  printP(tr);
  lineasetuprfkey(1, c(intro), conf.rfkeys.code[0]);
  lineasetuprfkey(2, c(esc), conf.rfkeys.code[1]);  printP(tr_f, tr);
  lineasetuprfkey(3, c(ups), conf.rfkeys.code[2]);
  lineasetuprfkey(4, c(downs), conf.rfkeys.code[3]);  printP(tr_f, tr);
  lineasetuprfkey(5, c(lefts), conf.rfkeys.code[4]);
  lineasetuprfkey(6, c(rigths), conf.rfkeys.code[5]);  printP(tr_f);

  espacioSep(4);
  for (byte i=0;i<2;i++) { lineasetuprfonoff(false,i,readdescr(filedesclocal,i+6,20));  }
  for (byte i=0; i<maxsalrem; i++) 
    if (conf.idsalremote[i]!=0) 
      if (conf.senalrem[i]>=6) { lineasetuprfonoff(true, i,readdescr(filesalrem,i,20)); }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR linearoomba(PGM_P texto, byte comm)
{
  printP(tr, td, href_i, syhtm, interr, letrap);
  printPiP(ig, comm, amper);
  printP(letrar, ig, ro, mayor, texto);
  printP(td_f, tr_f);
}

void ICACHE_FLASH_ATTR roombaHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  writeHeader(false,false);
  writeMenu(3, 2);
  writeForm(rohtm);
  linearoomba(c(START), 128);
  linearoomba(c(BAUD), 129);
  linearoomba(c(CONTROL), 130);
  linearoomba(c(SAFE), 131);
  linearoomba(c(FULL), 132);
  linearoomba(c(POWER), 133);
  linearoomba(c(SPOT), 134);
  linearoomba(c(CLEAN), 135);
  linearoomba(c(MAX), 136);
  linearoomba(c(DRIVE), 137);
  linearoomba(c(MOTORS), 138);
  linearoomba(c(LEDS), 139);
  linearoomba(c(SONG), 140);
  linearoomba(c(PLAY), 141);
  linearoomba(c(SENSORS), 142);
  linearoomba(c(DOCK), 143);
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR downloadHTML() {
  File download=SPIFFS.open(server.arg(0),letrar);
  if (download) {
    server.sendHeader(contenttype, texttext);     // "Content-Type", "text"
    server.sendHeader(c(contentdisposition), attachfilename+server.arg(0));    //"Content-Disposition","attachment; filename=xxxx" 
    server.sendHeader(c(connection), closet);          // "Connection", "close"
    server.streamFile(download, c(applicationoctet));  // "application/octet-stream"
    download.close();
  }
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
    printP(t(faltafichero),crlf,t(useftp),crlf);
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

void ICACHE_FLASH_ATTR extraevaloresTempConf(boolean withpass)
{                       // extrae datos de json.conf
  if (withpass)
    {
    extrae(true, msg, PSTR("ss")).toCharArray(ssidSTAtemp, 20);
    extrae(true, msg, PSTR("ps")).toCharArray(passSTAtemp, 20);
    }
  iddevicetemp = extrae(false, msg, PSTR("DV")).toInt();
  extrae(true, msg, PSTR("al")).toCharArray(aliasdevicetemp, 20);
  versinsttemp = extrae(false, msg, PSTR("v")).toInt();
  actualizauttemp = extrae(false, msg, PSTR("aa")).toInt();
  iftttenabletemp = extrae(false, msg, PSTR("if")).toInt();
  extrae(true, msg, PSTR("k")).toCharArray(iftttkeytemp, 30);
  mododweettemp = extrae(false, msg, PSTR("dw")).toInt();
  modomyjsontemp = extrae(false, msg, PSTR("my")).toInt();
  extrae(true, msg, PSTR("MJ")).toCharArray(idmyjsontemp, 10);
  modo45temp = extrae(false, msg, PSTR( "m45")).toInt();
  peractpantemp = extrae(false, msg, PSTR( "pap")).toInt();
  peractremtemp = extrae(false, msg, PSTR( "par")).toInt();
  TempDesactPrgtemp = extrae(false, msg, PSTR("tdp")).toInt();
  iottweetenabletemp = extrae(false, msg, PSTR("iot")).toInt();
  extrae(true, msg, PSTR("iotu")).toCharArray(iottweetusertemp, 10);
  extrae(true, msg, PSTR("iotk")).toCharArray(iottweetkeytemp, 15);
  latitudtemp = extrae(true, msg, PSTR("lat")).toFloat();
  longitudtemp = extrae(true, msg, PSTR("lon")).toFloat();

  for (byte i=0;i<2;i++) { extrae(true,msg,idpin[i]).toCharArray(auxdesc,20); savedescr(filedesctemp,auxdesc,i,20); }
  for (byte i=0;i<1;i++) { extrae(true,msg,idpin[i+4]).toCharArray(auxdesc,20); savedescr(filedesctemp,auxdesc,i+4,20); }
  for (byte i=0;i<1;i++) { extrae(true,msg,idpin[i+6]).toCharArray(auxdesc,20); savedescr(filedesctemp,auxdesc,i+6,20); }

  extrae(true, msg, PSTR("ad0")).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 3, 20);
  extrae(true, msg, PSTR("au0")).toCharArray(unitpinAtemp, 4);
  factorAtemp[0] = extrae(true, msg, PSTR("af0")).toFloat();
  offsetAtemp[0] = extrae(true, msg, PSTR("ao0")).toFloat();
  bsumatAtemp[0] = extrae(true, msg, PSTR("asu0")).toInt();
  tipoEDtemp[0] = extrae(true, msg, PSTR("et0")).toInt();
  tipoEDtemp[1] = extrae(true, msg, PSTR("et1")).toInt();
  valinictemp[0] = extrae(false, msg, PSTR( "vi0")).toInt();
  valinictemp[1] = extrae(false, msg, PSTR("vi1")).toInt();
  tempdefacttemp[0] = extrae(false, msg, PSTR("on0")).toInt();
  tempdefacttemp[1] = extrae(false, msg, PSTR("on1")).toInt();
  tempdefdestemp[0] = extrae(false, msg, PSTR("off0")).toInt();
  tempdefdestemp[1] = extrae(false, msg, PSTR("off1")).toInt();
  iftttpinEDtemp[0] = extrae(false, msg, PSTR("ife0")).toInt();
  iftttpinEDtemp[1] = extrae(false, msg, PSTR("ife1")).toInt();
  iftttpinSDtemp[0] = extrae(false, msg, PSTR("ifs0")).toInt();
  iftttpinSDtemp[1] = extrae(false, msg, PSTR("ifs1")).toInt();
  extrae(true, msg, PSTR("fsv")).toCharArray(fwUrlBasetemp, 80);
  msg=vacio;
}

void ICACHE_FLASH_ATTR setupdev150HTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }

  if (server.method()==HTTP_POST)
  {
    hacerresetrem = 0;
    sendOther(slkhtm,-1);
    for (byte i = 0; i<server.args(); i++)
      {
      if (server.argName(i).toInt()==0) iddevicetemp = server.arg(i).toInt();
      if (server.argName(i).toInt()==1) server.arg(i).toCharArray(aliasdevicetemp, sizeof(aliasdevicetemp));
      if (server.argName(i).toInt()==2) server.arg(i).toCharArray(ssidSTAtemp, sizeof(ssidSTAtemp));
      if (server.argName(i).toInt()==3) server.arg(i).toCharArray(passSTAtemp, sizeof(passSTAtemp));
      if (server.argName(i).toInt()==4) wifimodetemp=server.arg(i).toInt();
      if (server.argName(i).toInt()==5) hacerresetrem=server.arg(i).toInt();
      }
    /////////////////////////////////
    // conectar a red conuco150
    conf.staticIP = 1;
    conf.EEip = {192,168,4,9}; conf.EEip[3]=9;
    conf.EEgw = {192,168,4,1};
    WiFi.config(conf.EEip, conf.EEgw, conf.EEmask, conf.EEdns, conf.EEdns2);
    WiFi.begin("CONUCO_150", t12341234, true);
    byte cont = 0;
    dPrint(t(conectando)); dPrint(b); dPrint(WiFi.SSID()); dPrint(barra); dPrint(WiFi.psk()); dPrint(b);
    while ((!WiFi.isConnected()) && (cont++ < 20))  {
      delay(1000);
      dPrint(punto);
    }
    dPrint(crlf); dPrint(t(tconectado)); dPrint(b); dPrint(WiFi.isConnected() ? ok : c(terror)); dPrint(crlf);
    dPrint(c(IP)); Serial.print(WiFi.localIP()); dPrint(crlf);

    if (WiFi.isConnected())
    {
      // enviar nuevos datos
      sendJsonConf(1, 88, true, hacerresetrem == 1); // envia jsonConf al remoto para configurar y guardar, incluído ssid y pass
      hacerresetrem = 0;
    }
    else
      {Serial.print(t(NO)); Serial.print(b);Serial.print(t(tconectado));}
    // volver a red original
    conf.EEip = {192, 168, 1, 150};  conf.EEip[3] = conf.iddevice;
    conf.EEgw = {192, 168, 1, 1};
    WiFi.config(conf.EEip, conf.EEgw, conf.EEmask, conf.EEdns, conf.EEdns2);
    WiFi.begin(conf.ssidSTA, conf.passSTA, true);
    cont = 0;
    dPrint(t(conectando)); dPrint(b); dPrint(WiFi.SSID()); dPrint(barra); dPrint(WiFi.psk()); dPrint(b);
    while ((!WiFi.isConnected()) && (cont++ < 20))  {
      delay(1000);
      dPrint(punto);
    }
    dPrint(crlf); dPrint(t(tconectado)); dPrint(b); dPrint(WiFi.isConnected() ? ok : c(terror)); dPrint(crlf);
    dPrint(c(IP)); Serial.print(WiFi.localIP()); dPrint(crlf);
    ////////////////////////////////
    //    sendOther(sdrem150htm,-1);
    delay(10000);
    sendOther(slkhtm,-1);
    return;
  }

  if (server.argName(0).compareTo(PSTR("r")) == 0)
    if (server.arg(0).toInt() == 1)
      {
      /////////////////////////////////
      // conectar a red conuco150
      byte auxdevice = conf.iddevice;
      conf.staticIP = 1;
      conf.EEip = {192, 168, 4, 9};  conf.EEip[3] = 9;
      conf.EEgw = {192, 168, 4, 1};
      WiFi.config(conf.EEip, conf.EEgw, conf.EEmask, conf.EEdns, conf.EEdns2);
      WiFi.begin("CONUCO_150", t12341234, true);
      byte cont = 0;
      dPrint(t(conectando)); dPrint(b); dPrint(WiFi.SSID()); dPrint(barra); dPrint(WiFi.psk()); dPrint(b);
      while ((!WiFi.isConnected()) && (cont++ < 20))  {
        delay(1000);
        dPrint(punto);
      }
      dPrint(crlf); dPrint(t(tconectado)); dPrint(b); dPrint(WiFi.isConnected()?ok:c(terror)); dPrint(crlf);
      dPrint(c(IP)); Serial.print(WiFi.localIP()); dPrint(crlf);

      if (WiFi.isConnected())
        {
        int auxerr = 0;
        auxerr = ReqJsonConf(1, 88);
        Serial.print(c(treqjson)); Serial.print(dp);  Serial.println(auxerr);
        if (auxerr == HTTP_CODE_OK) extraevaloresTempConf(true);
        msg=vacio;
        }
      else {
        Serial.print(t(NO));
        Serial.print(b);
        Serial.print(t(tconectado));  }
      // volver a red original
      conf.iddevice = auxdevice;
      conf.EEip = {192, 168, 1, 150};  conf.EEip[3] = conf.iddevice;
      conf.EEgw = {192, 168, 1, 1};
      WiFi.config(conf.EEip, conf.EEgw, conf.EEmask, conf.EEdns, conf.EEdns2);
      WiFi.begin(conf.ssidSTA, conf.passSTA, true);
      cont=0;
      dPrint(t(conectando)); dPrint(b); dPrint(WiFi.SSID()); dPrint(barra); dPrint(WiFi.psk()); dPrint(b);
      while ((!WiFi.isConnected()) && (cont++ < 20))  { delay(1000); dPrint(punto); }
      dPrint(crlf); dPrint(t(tconectado)); dPrint(b); dPrint(WiFi.isConnected()?ok:c(terror)); dPrint(crlf);
      dPrint(c(IP)); (WiFi.localIP()); dPrint(crlf);
      ////////////////////////////////
      sendOther(sdrem150htm,-1);
      return;
      }

  writeHeader(false,false);
  writeMenu(3, 5);
  writeForm(sdrem150htm);
  printP(tr);
  printColspan(2);
  printP(t(nuevoremoto), td_f, tr_f);
  printP(tr, td, t(dispositivo), td_f, td);
  printcampoL(0, iddevicetemp, 3, true);  printP(td_f, tr_f);
  printP(tr, td, c(alias), td_f, td);
  printcampoC(1, aliasdevicetemp, 20, true, true, false);  printP(td_f, tr_f);
  printP(tr, td, c(tssid), td_f, td);
  printcampoC(2, conf.ssidSTA, 20, true, true, false);  printP(td_f, tr_f);
  printP(tr, td, c(tpass), td_f, td);
  printcampoC(3, conf.passSTA, 20, true, true, true);  printP(td_f, tr_f);
  printP(tr, td, treset, interr, td_f, td);
  checkBox(5, hacerresetrem);  printP(td_f, tr_f);
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupDevRemHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  iddevicetemp = server.arg(0).toInt(); // núm. dispositivo
  int auxerr = ReqJsonConf(iddevicetemp, 88);
  byte posdevice = 99;
  if (auxerr == HTTP_CODE_OK) {
    extraevaloresTempConf(false);
    for (byte i = 0; i < maxdevrem; i++)  {
      if (conf.idremote[i] == iddevicetemp) posdevice = i;
      if (posdevice != 99) {
        strcpy(auxdesc, aliasdevicetemp);  savedescr(filedevrem, auxdesc, posdevice, 20);
        for (byte j = 0; j < maxsalrem; j++)       {
          if (conf.idsalremote[j] == conf.idremote[posdevice])
            if ((conf.senalrem[j] >= 0) && (conf.senalrem[j] <= 7)) {
              readdescr(filedesctemp, conf.senalrem[j], 20); savedescr(filesalrem, auxdesc, j, 20);
            }
          }
        saveconf();
        }
      }
    } 
  msg=vacio;
  mp=1;
  if (server.method() == HTTP_POST)
    {
    mododweettemp=0; iftttenabletemp=0; modomyjsontemp=0; iottweetenabletemp=0;
    actualizauttemp=0;
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (param_number==1) { iddevicetemp = server.arg(i).toInt(); } // núm. dispositivo
      else if (param_number==2) { server.arg(i).toCharArray(aliasdevicetemp, sizeof(aliasdevicetemp)); } // aliasdevicetemp
      else if (param_number==3) { server.arg(i).toCharArray(iftttkeytemp, sizeof(iftttkeytemp)); } // iftttkeytemp
      else if (param_number==4) { iftttenabletemp=server.arg(i).toInt(); } // iftttenabletemp
      else if (param_number==5) { modomyjsontemp=server.arg(i).toInt(); } // enable modomodomyjsontemp
      else if (param_number==6) { mododweettemp=server.arg(i).toInt(); } // enable mododweettemp
      else if (param_number==7) { iottweetenabletemp=server.arg(i).toInt(); } // enable iottweet
      else if (param_number==8) { modo45temp=server.arg(i).toInt(); } // modo45
      else if (param_number==9) { actualizauttemp=server.arg(i).toInt(); } 
      else if (param_number==10) { server.arg(i).toCharArray(fwUrlBasetemp, 80); }
      else if (param_number==11) { peractpantemp=server.arg(i).toInt();  }
      else if (param_number==12) { peractremtemp=server.arg(i).toInt();  }
      else if (param_number==13) { TempDesactPrgtemp=server.arg(i).toInt(); }
      else if (param_number==14) { latitudtemp=server.arg(i).toFloat(); }
      else if (param_number==15) { longitudtemp=server.arg(i).toFloat(); }
      }
    sendJsonConf(iddevicetemp, 88, false, false); // envia jsonConf al remoto para configurar
    strcpy(auxchar, sdremhtm);  strcat(auxchar, interr); strcat(auxchar, c(trem)); strcat(auxchar, itoa(iddevicetemp, buff, 10));
    sendOther(auxchar,-1);
    return;
    }

  writeHeader(false,false);
  writeMenu(3, 5);
  writeForm(sdremhtm);

  if (auxerr != HTTP_CODE_OK)
    {
    printP(t(noconf), menor, barra);
    printP(table, mayor);
    printP(c(form_f));
    printP(c(body_f), menor, barra);
    printP(thtml, mayor);
    }
  else
    {
    printP(tr);
    printColspan(4);
    printP(c(devremotot), td_f, tr_f);
    printP(td, t(dispositivo), barraesp, c(alias), td_f, td);
    printcampoL(1, iddevicetemp, 3, true);
    printP(td_f);
    printColspan(2);
    printcampoC(2, aliasdevicetemp, 20, true, true, false);
    printP(td_f, tr_f, tr, td, t(Modo), b);
    printP(t(pines));
    printP(b, cuatro, guion, cinco, td_f, td);
    printcampoCB(8, modo45temp, diginput,i2c,modbust);
    printP(td_f);
    printColspan(2);
    printP(td_f, tr_f, tr, td_f, td);
    printP(t(Act), b, t(aut), barraesp, t(versiont), actualizauttemp ? th : td);
    checkBox(9, actualizauttemp);
    printP(actualizauttemp?th_f:td_f, td);
    printI(versinsttemp);
    printP(td_f, c(td_if), tr_f, tr, td);
    printP(c(Firmware), b);
    printP(c(tserver), td_f);
    printColspan(3);
    printcampoC(10, fwUrlBasetemp, 49, true, true, false);
    printP(td_f, tr_f, tr, td, t(periodoact255));

    printP(barra, t(periodoactrem), barra, t(tdp), td_f, td);
    printcampoI(11, peractpantemp, 5, true);
    printP(td_f, td);
    printcampoI(12, peractremtemp, 5, true);
    printP(td_f, td);
    printcampoL(13, TempDesactPrgtemp, 5, true);
    printP(td_f, tr_f);

    espacioSep(4);
    printP(tr);
    printP(td, c(ifttt), barraesp);
    printP(c(Key), td_f, iftttenabletemp ? th : td);
    checkBox(4, iftttenabletemp);
    printP(iftttenabletemp ? th_f : td_f);
    printColspan(2);
    printcampoC(3, iftttkeytemp, 30, true, true, false);
    printP(td_f, tr_f, tr);

    printP(td, c(dweet), barraesp);
    printP(c(dweetname), td_f, mododweettemp ? th : td);
    checkBox(6, mododweettemp);
    printP(mododweettemp ? th_f : td_f);
    printColspan(2);
    if (mododweettemp==1) printP(c(conuco), readdescr(filemacdevrem, posdevice, 14));
    printP(td_f, tr_f, tr);

    printP(td, c(modomyjsont), barraesp);
    printP(c(Key), td_f, modomyjsontemp ? th : td);
    checkBox(5, modomyjsontemp);
    printP(modomyjsontemp ? th_f : td_f);
    printColspan(2);
    if (modomyjsontemp == 1) printP(idmyjsontemp);
    printP(td_f, tr_f, tr);

    printP(tr, td, href_i, comillas);
    printP(c(urliotwweet), comillas);
    printP(b, c(newpage), mayor);
    printP(c(iottweett));
    printP(barraesp, t(usuario), barraesp);
    printP(c(Key), href_f, td_f, iottweetenabletemp==1?th:td);
    checkBox(7, iottweetenabletemp);
    printP(conf.iottweetenable==1?th_f:td_f,td);
    printP(td_f, td);
    if (clientremote()) printP(c(hidden)); else printP(iottweetusertemp);
    printP(td_f, tr_f);

    printP(td, t(latitudt), barraesp, t(longitudt), td_f, td);
    printcampoF(14, latitudtemp, 6);   printP(td_f, td);
    printcampoF(15, longitudtemp, 6);   printP(td_f, c(td_if), tr_f);

    writeFooter(tguardar, false);
    }
  serversend200();
}

void ICACHE_FLASH_ATTR setupDevRemioHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  iddevicetemp = server.arg(0).toInt(); // núm. dispositivo

  int auxerr = ReqJsonConf(iddevicetemp, 88);
  if (auxerr == HTTP_CODE_OK) {
    extraevaloresTempConf(false);
    byte posdevice=99;
    for (byte i=0; i<maxdevrem; i++)  {
      if (conf.idremote[i]==iddevicetemp) posdevice=i;
      if (posdevice!=99)
        {
        strcpy(auxdesc, aliasdevicetemp);  savedescr(filedevrem, auxdesc, posdevice, 20);
        for (byte j=0; j<maxsalrem; j++)
          {
          if (conf.idsalremote[j] == conf.idremote[posdevice])
            {
            if ((conf.senalrem[j] >= 0) && (conf.senalrem[j] <= 7))
              {
              readdescr(filedesctemp, conf.senalrem[j], 20);
              savedescr(filesalrem, auxdesc, j, 20);
              }
            }
          }
        saveconf();
        }
    }
  }
  msg=vacio;
  mp=1;
  if (server.method()==HTTP_POST)
  {
    iftttpinEDtemp[0]=0; iftttpinSDtemp[0]=0;
    iftttpinEDtemp[1]=0; iftttpinSDtemp[1]=0;
    bsumatAtemp[0] = 0;
    for (int i = 0; i < server.args(); i++)
    {
      calcindices(i);
      if (param_number==0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 0, 20); }
      else if (param_number==5) { server.arg(i).toCharArray(auxdesc, 20);  savedescr(filedesctemp, auxdesc, 1, 20); }
      else if (param_number==10) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 2, 20); }
      else if (param_number==15) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 4, 20); }
      else if (param_number==16) { server.arg(i).toCharArray(unitpinAtemp, 3); }
      else if (param_number==17) { factorAtemp[0] = server.arg(i).toFloat(); }
      else if (param_number==18) { offsetAtemp[0] = server.arg(i).toFloat(); }
      else if (param_number==19) { setbit8(bsumatAtemp, 0, 1); }
      else if (param_number==20) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 4, 20); }
      else if (param_number==24) { setbit8(iftttpinEDtemp,0,1); }
      else if (param_number==124) { setbit8(iftttpinEDtemp,8,1); }
      else if (param_number==25) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 5, 20); }
      else if (param_number==29) { setbit8(iftttpinEDtemp, 1, 1); }
      else if (param_number==129) { setbit8(iftttpinEDtemp,9, 1); }
      else if (param_number==30) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 6, 20); }
      else if (param_number==31) { valinictemp[0] = server.arg(i).toInt(); }
      else if (param_number==32) { tempdefacttemp[0] = server.arg(i).toInt(); }
      else if (param_number==33) { tempdefdestemp[0] = server.arg(i).toInt(); }
      else if (param_number==34) { setbit8(iftttpinSDtemp, 0, 1); }
      else if (param_number==134) { setbit8(iftttpinSDtemp, 8, 1); }
      else if (param_number==35) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedesctemp, auxdesc, 7, 20); }
      else if (param_number==36) { valinictemp[1] = server.arg(i).toInt(); }
      else if (param_number==37) { tempdefacttemp[1] = server.arg(i).toInt(); }
      else if (param_number==38) { tempdefdestemp[1] = server.arg(i).toInt(); }
      else if (param_number==39) { setbit8(iftttpinSDtemp, 1, 1); }
      else if (param_number==139) { setbit8(iftttpinSDtemp, 9, 1); }
    }
    sendJsonConf(iddevicetemp, 88, false, false); // envia jsonConf al remoto para configurar
    strcpy(auxchar, sdremiohtm);  strcat(auxchar, interr); strcat(auxchar, c(trem));
    strcat(auxchar, itoa(iddevicetemp, buff, 10));
    sendOther(auxchar,-1);   // "sdrem?rem=xxx";
    return;
  }

  writeHeader(false,false);
  writeMenu(3, 5);
  writeForm(sdremiohtm);

  if (auxerr != HTTP_CODE_OK)
    {
    printP(t(noconf), menor, barra);
    printP(table, mayor);
    printP(c(form_f));
    printP(c(body_f), menor, barra);
    printP(thtml, mayor);
    }
  else
    {
    printP(tr);
    printP(td, t(dispositivo), barra, c(alias), td_f, td);
    printcampoL(2, iddevicetemp, 3, true);
    printP(guion, aliasdevicetemp, td_f, tr_f);

    printP(tr, td, t(sonda), b, uno, barra);
    printP(dos, barra, tres, td_f);
    for (byte i = 0; i < maxTemp; i++)
      {
      if (i == 0) printP(td); else printColspan(2);
      printP(menor, c(tinput), b, type, ig);
      printP(comillas, c(ttext), comillas, b);
      printP(c(namet), ig);
      printPiP(comillas, i * 5, comillas);
      printP(b, tvalue, ig, comillas);
      printP(readdescr(filedesctemp, i, 20));
      printP(comillas,b,c(max_length));
      printIP(19, size_i);
      printI(19);
      printP(comillas, mayor, menor, barra, c(tinput), mayor);
      printP(td_f);
      }
    printP(tr_f,tr, td, t(entanalog));
    printPiP(b, 1, td_f);

    printP(td, menor, c(tinput), b, type, ig);
    printP(comillas, c(ttext), comillas, b);
    printP(c(namet), ig);
    printPiP(comillas, 15, comillas);
    printP(b, tvalue, ig, comillas);
    printP(readdescr(filedesctemp, 3, 20));
    printP(comillas,b,c(max_length));
    printIP(19, size_i);    
    printI(19);
    printP(comillas, mayor, menor, barra, c(tinput), mayor);
    printP(td_f, td);

    printP(t(units), menor);
    printP(c(tinput), b, type, ig);
    printP(comillas,c(ttext), comillas, b);
    printP(c(namet));
    printP(ig);
    printPiP(comillas, 16, comillas);
    printP(b, tvalue, ig, comillas, unitpinAtemp);
    printP(comillas,b,c(max_length));
    printIP(3, size_i); printI(3);
    printP(comillas, mayor, menor, barra, c(tinput), mayor);
    printP(td_f, td, c(factor));
    printcampoF(17, factorAtemp[0], 5);
    printP(td_f, td, c(offset));
    printcampoF(18, offsetAtemp[0], 5);
    printP(td, sumat);
    checkBox(19, (getbit8(bsumatAtemp, 0)));
    printP(td_f, tr_f);

    if (modo45temp == 0)
      for (byte i = 0; i < maxED; i++) // entradas digitales
      {
        printP(tr, td, t(entradasdig), b);
        printI(i + 1);
        printP(td_f, td);
        printP(menor, c(tinput), b, type, ig, comillas);
        printP(c(ttext), comillas, b);
        printP(c(namet), ig, comillas);
        printI(20 + (i * 5));
        printP(comillas, b, tvalue, ig, comillas);
        printP(readdescr(filedesctemp, i+4, 20));
        printP(comillas,b,c(max_length));
        printIP(19, size_i);
        printI(19);
        printP(comillas, mayor, menor, barra, c(tinput), mayor);
        printP(td_f, td, t(ttipo), b);
        printcampoCB(21 + (i * 5), conf.tipoED[i-4], ONOFF, dhtt); // número de parámetro ????

        printP(td_f, c(td_if));
        printP(c(td_if), td);
        checkBox(24 + (i * 5), getbit8(iftttpinEDtemp, i)); // checkbox Notificar
        printP(barra);
        checkBox(124 + (i * 5), getbit8(iftttpinEDtemp, i + 8)); // checkbox Notificar
        printP(td_f, tr_f);
      }

    printP(tr, c(td_if));
    printP(c(td_if), td_f);
    printP(td, letraV, punto, b, t(inicial), td_f);
    printP(td, c(tdefact), td_f, td);
    printP(c(tdefdes), td_f);
    printP(td, c(ifttt), td_f, tr_f);
    for (byte i=0; i<maxSD; i++) // salidas digitales
      {
      printP(tr, td, t(saldig));
      printI(i+1);
      printP(td_f, td);

      printP(menor, c(tinput), b, type, ig, comillas);
      printP(c(ttext), comillas, b);
      printP(c(namet), ig, comillas);
      printI(30+(i*5));
      printP(comillas, b, tvalue, ig, comillas);
      printP(readdescr(filedesctemp, i + 6, 20));
      printP(comillas,b,c(max_length));
      printIP(19, size_i);
      printI(19);
      printP(comillas, mayor, menor, barra, c(tinput), mayor);
      printP(td_f, td);
      printcampoCB(31+(i*5), valinictemp[i], OFF, ON, t(ultimovalor));
      printP(td_f, td);
      printcampoL(32+(i*5), tempdefacttemp[i], 8, true);
      printP(td_f, td);
      printcampoL(33+(i*5), tempdefdestemp[i], 8, true);
      printP(td_f, td);
      checkBox(34+(i*5), getbit8(iftttpinSDtemp, i)); // checkbox Notificar
      printP(barra);
      checkBox(134+(i*5), getbit8(iftttpinSDtemp, i + 8)); // checkbox Notificar
      printP(td_f, tr_f);
      }
    writeFooter(tguardar, false);
    }
  serversend200();
}

/*********************************************************************************************/
void ICACHE_FLASH_ATTR scanapHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  nAPact=0;
  nAP=WiFi.scanNetworks(false, false);
  msg=vacio;
  writeHeader(false,false);
  printP(menor,table, b);
  printP(c(tclass), ig, tnormal, mayor);

  for (int i=0; i<nAP; i++)
  {
    WiFi.SSID(i).toCharArray(auxchar, 20);
    printP(tr, td);
    printP(href_i, syhtm,interr,letran,ig);
    printI(i);
    printP(amper, letras,letrai, ig, cero, mayor);
    printP(auxchar, td_f, td);
    printI(WiFi.RSSI(i));
    printP(b, c(dbm), td_f, tr_f);
  }
  printP(menor, barra, table, mayor);  
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
}

/*********************************************************************************************/
void ICACHE_FLASH_ATTR setupNetHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=1;
  if (server.method()==HTTP_POST)
    {
    conf.staticIP = 0;
    for (int i = 0; i < server.args(); i++)
      {
      calcindices(i);
      if (param_number>=0 && param_number <= 5) { server.arg(i).toCharArray(conf.EEmac[i], 3);  }
      else if (param_number>=6 && param_number <= 8)
        {
        conf.EEip[param_number-6] = server.arg(i).toInt();
        conf.EEgw[2] = conf.EEip[2];
        strcpy(hostraiz,itoa(conf.EEip[0],buff,10)); strcat(hostraiz, punto);
        strcat(hostraiz,itoa(conf.EEip[1],buff,10)); strcat(hostraiz, punto);
        strcat(hostraiz,itoa(conf.EEip[2],buff,10)); strcat(hostraiz, punto);
        }
      else if (param_number>=10 && param_number <= 13) { conf.EEmask[param_number - 10] = server.arg(i).toInt(); }
      else if (param_number>=14 && param_number <= 17) { conf.EEgw[param_number - 14] = server.arg(i).toInt();   }
      else if (param_number>=18 && param_number <= 21) { conf.EEdns[param_number - 18] = server.arg(i).toInt();  }
      else if (param_number==22) { conf.webPort = server.arg(i).toInt();  }
      else if (param_number==41) { conf.wifimode = server.arg(i).toInt(); }
      //      else if (param_number==42) {if (nAP != 0) WiFi.SSID(StrtoInt(server.arg(i))).toCharArray(conf.ssidSTA, 20);}
      else if (param_number==43) { server.arg(i).toCharArray(conf.passSTA, 20); }
      //      else if (param_number == 44) {server.arg(i).toCharArray(conf.ssidAP,20);}
      else if (param_number==45) { server.arg(i).toCharArray(conf.passAP, 9); }
      else if (param_number==46) { conf.staticIP = server.arg(i).toInt(); }
      else if (param_number>=47 && param_number <= 50) { conf.EEdns2[param_number - 47] = server.arg(i).toInt(); }
      else if (param_number==53) { conf.timeoutrem = server.arg(i).toInt(); }
      else if (param_number==54) { conf.timeoutNTP = server.arg(i).toInt(); }
      else if (param_number==56) { conf.canalAP = server.arg(i).toInt()+1; }
      }
    //
    //    nAP = 0;
    saveconf();
    sendOther(snehtm,-1); return;
    }

  writeHeader(false,false);
  writeMenu(3, 3);
  writeForm(snehtm);

  printP(tr, td, t(Modo), b, td_f, td);
  printcampoCB(41, conf.wifimode, sta, ap, apsta);
  printP(td_f, tr_f);

  // ssid
  printP(tr, td, c(routerssid), td_f, td);
  printcampoC(42, conf.ssidSTA, 20, true, false, false);
  printP(href_i, comillas, scanap, comillas);
  printP(mayor, b);
  printP(t(explorar), href_f, td_f, tr_f);

  printP(tr);  printparCP(c(routerpass), 43, conf.passSTA, 20, clientremote());  printP(tr_f);

  printP(tr, td, c(apssid), td_f, td);
  printP(conf.ssidAP, td_f, tr_f, tr);
  printparCP(c(appass), 45, conf.passAP, 9, clientremote());
  printP(tr_f, tr, td, t(canal), td_f, td);
  printP(c(Select_name),comillas);
  printIP(56, comillas);
  printP(mayor);
  for (byte j = 0; j < 13; j++)   { // canales
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (conf.canalAP-1==j) printP(b, selected, ig, comillas, selected, comillas);
    printPiP(mayor, j+1, c(option_f));
  }
  printP(c(select_f), td_f, tr_f);

  espacioSep(2);
  printP(tr, td, c(MAC), td_f, td);
  if (clientremote())  printP(c(hidden));
  else { for (byte i=0; i<5; i++) printP(conf.EEmac[i]); printP(conf.EEmac[5]);  }
  printP(td_f, tr_f);
  printP(tr, td, t(staticip), td_f, conf.staticIP ? th : td);
  checkBox(46, conf.staticIP);
  printP(conf.staticIP ? th_f : td_f, tr_f);

  // print the current IP
  printP(tr, td, t(DIRIP), td_f, td);
  if (clientremote())  printP(c(hidden));
  else  { for (byte i=0; i<3; i++) { printI(conf.EEip[i]); printP(punto);  }  printI(conf.EEip[3]);  }
  printP(td_f, tr_f);

  printP(tr, td, c(IP), td_f, td);
  for (byte i=0; i<4; i++) printcampoI(6+i, conf.EEip[i],3,i!=3);
  printP(td_f, tr_f);
  printP(tr, td, c(mask), td_f, td);
  for (byte i=0; i<4; i++) printcampoI(i+10, conf.EEmask[i], 3, true);
  printP(td_f, tr_f);
  printP(tr, td, c(gateway), td_f, td);
  for (byte i=0; i<4; i++) printcampoI(14+i, i==2?conf.EEip[i]:conf.EEgw[i],3,i!=2);
  printP(td_f, tr_f);

  printP(tr, td, t(ippublica), td_f, td);
  printP(clientremote() ? c(hidden) : conf.myippub, td_f, tr_f);

  //  printP(tr,td,ttimeoutrem,td_f,td);
  //  printcampoL(53, timeoutrem, 5,true);
  //  printP(td_f,tr_f);
  //  printP(tr,td,c(ttimeoutNTP),td_f,td);
  //  printcampoL(54, conf.timeoutNTP, 5,true);
  //  printP(td_f,tr_f);

  printP(menor, barra, table, mayor, menor, c(tinput));
  printP(b, type, ig, comillas, submit, comillas);
  printP(b, tvalue, ig, comillas);
  printP(tguardar, comillas);
  printP(mayor, menor, barra, c(tinput), mayor);
  printP(c(form_f));
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
  serversend200();
}

void ICACHE_FLASH_ATTR setupSegHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=1;
  char passDevtemp1[20];
  char passDevtemp2[20];
  if (server.method()==HTTP_POST)
    {
    conf.usepassDev = 0;
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (param_number==0) conf.usepassDev = server.arg(i).toInt();
      else if (param_number==1) server.arg(i).toCharArray(conf.userDev, 20);
      else if (param_number==2) server.arg(i).toCharArray(passDevtemp1, 20);
      else if (param_number==3) server.arg(i).toCharArray(passDevtemp2, 20);
      }
    if (conf.usepassDev)    // contraseña activa
      {
      if (strcmp(passDevtemp1, passDevtemp2)==0)   // si coinciden ambas password se almacena
        strcpy(conf.passDev, passDevtemp1);
      else
        conf.usepassDev=0; // no se guarda y se desactiva contraseña
      }
    else    // contraseña NO activa
      if (strcmp(passDevtemp1, conf.passDev) != 0)  // si no se da la contraseña correcta, no se desactiva
        conf.usepassDev = 1;
    saveconf();
    sendOther(sshtm,-1); return;
    }

  /////////////////////
  writeHeader(false,false);
  writeMenu(4, 5);
  writeForm(sshtm);

  printP(tr, td, t(autenticacion), td_f, conf.usepassDev ? th : td);
  checkBox(0, conf.usepassDev);
  if (conf.usepassDev) printP(th_f, tr_f); else printP(td_f, tr_f);
  printP(tr);
  printparCP(t(usuario), 1, conf.userDev, 20, false);
  printP(tr_f, tr);
  printparCP(t(contrasena), 2, "", 20, true);
  printP(tr, tr_f);
  printparCP(t(confcontrasena), 3, "", 20, true);
  printP(tr);
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupPrgHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg = vacio;
  mp = 2;
  if (server.method()==HTTP_POST)
  {
    memset(conf.actPrg, 0, sizeof(conf.actPrg));
    for (int i = 0; i < server.args(); i++)
      {
      calcindices(i);
      if (resto==0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedescprg, auxdesc, indice, 20);   }
      else if (resto==1) conf.actPrg[indice] = server.arg(i).toInt();
      }
    saveconf();
    sendOther(sprghtm,-1);
    return;
  }

  writeHeader(false,false);
  writeMenu(2, 5);
  writeForm(sprghtm);
  printP(tr, c(td_if));
  printP(td, t(descripcion), td_f);
  printP(td, t(activo), td_f, tr_f);
  for (byte i=0; i<maxPrg; i++)  {
    mpi=mp*i;
    colorea=(conf.actPrg[i]);
    printP(tr);
    printPiP(td, i+1, td_f);
    printP(colorea?th:td);
    printP(menor, c(tinput), b, type, ig, comillas);
    printP(c(ttext), comillas, b);
    printP(c(namet), ig, comillas);
    printI(mpi);    // número de parámetro
    printP(comillas, b, tvalue, ig, comillas);
    printP(readdescr(filedescprg, i, 20));
    printP(comillas,b,c(max_length));
    printIP(19, size_i);
    printI(19);
    printP(comillas, mayor, menor, barra, c(tinput), mayor);
    printP(colorea?th_f:td_f, colorea?th:td);
    checkBox(mpi+1, colorea);
    printP(colorea ? th_f : td_f, tr_f);
  }
  writeFooter(tguardar, false);
  server.send(200, texthtml, msg);
  msg=vacio;
}

void ICACHE_FLASH_ATTR setupWebCallHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=3;
  if (server.method()==HTTP_POST)
   {
    memset(conf.bPRGwebcall, 0, sizeof(conf.bPRGwebcall));
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (resto==0) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filewebcall, auxdesc, indice, 20); }
      else if (resto==1) { server.arg(i).toCharArray(auxdesc, 60); savedescr(fileurlwebcall, auxdesc, indice, 60);  }
      else if (resto==2) if (server.arg(i).toInt() == 1) setbit8(conf.bPRGwebcall, indice, 1);
      }
    saveconf();
    sendOther(swchtm,-1);
    return;
   }

  writeHeader(false,false);
  writeMenu(2, 7);
  writeForm(swchtm);
  printP(tr, c(td_if), td);
  printP(t(descripcion), td_f, td);
  printP(c(urltext), td_f, td);
  printP(t(activo), td_f, tr_f);
  for (byte i=0; i<maxwebcalls; i++)  {
    mpi = mp*i;
    colorea=(getbit8(conf.bPRGwebcall, i)==1);
    printP(tr);
    printPiP(td,i+1,td_f);

    printP(colorea?th:td);
    printP(menor, c(tinput), b, type, ig, comillas);
    printP(c(ttext), comillas, b);
    printP(c(namet), ig, comillas);
    printI(mpi);    // número de parámetro
    printP(comillas, b, tvalue, ig, comillas);
    printP(readdescr(filewebcall, i, 20));
    printP(comillas,b,c(max_length));
    printIP(19, size_i);
    printI(19);
    printP(comillas, mayor, menor, barra, c(tinput), mayor);
    printP(colorea ? th_f : td_f);

    printP(colorea ? th : td);
    printP(menor, c(tinput), b, type, ig, comillas);
    printP(c(ttext), comillas, b);
    printP(c(namet), ig, comillas);
    printI(mpi + 1);  // número de parámetro
    printP(comillas, b, tvalue, ig, comillas);
    printP(readdescr(fileurlwebcall, i, 60));
    printP(comillas,b,c(max_length));
    printIP(59, size_i);
    printI(59);
    printP(comillas, mayor, menor, barra, c(tinput), mayor);
    printP(colorea ? th_f : td_f, colorea ? th : td);

    checkBox(mpi + 2, colorea);
    printP(colorea ? th_f : td_f, tr_f);
  }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupSemHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp = 19;  // número de parámetros por fila
  if (server.method()==HTTP_POST)
  {
    for (byte i = 0; i < maxPrg; i++) setbit8(conf.bPRGsem[posactsem], i, 0);
    memset(conf.prgdia[posactsem], 0, sizeof(conf.prgdia[posactsem]));
    for (int i = 0; i < server.args(); i++)
    {
      calcindices(i);
      if ((resto>=0) && (resto <= 7)) {setbit8(conf.bPRGsem[indice], resto, mival); } // asociacion PRG
      if (resto==8) { conf.prgsal[indice] = mival; }  // Número de salida
      if (resto==9) { setbit8(conf.bprgval, indice, mival);  } // valor de la salida
      if ((resto>=10) && (resto<=16)) { setbit8(conf.prgdia[indice], resto-10,mival); }  // domingo-sábado
      if (resto==17) { conf.prghor[indice] = constrain(mival, 0, 23); }   // Hora
      if (resto==18) { conf.prgmin[indice] = constrain(mival, 0, 59); }   // Minuto
    }
    saveconf();
    sendOther(ssehtm,-1);
    return;
  }
  if (server.args() > 0) { posactsem=constrain(server.arg(0).toInt(),0,7); }
  writeHeader(false,false);
  writeMenu(2, 1);
  writeForm(ssehtm);

  printP(tr, td, td_f);
  printP(td);
  printP(t(asociara), b, t(programa), td_f);
  printColspan(2);
  printP(t(salida), td_f, td, t(dia), letras, td_f);
  printColspan(2);
  printP(t(hora), td_f, tr_f, tr, td, td_f);

  printP(td, c(pre_i));
  for (byte i=0; i<maxPrg; i++) { printPiP(b, i+1, b); }
  printP(c(pre_f), td_f);
  printP(td, t(nombre), td_f);
  printP(td, t(tvalor), td_f, td);
  printP(c(pre_i));
  printP(b, letraD, b, b);
  printP(letraL, b, b, letraM, b, b);
  printP(letraX, b, b, letraJ, b, b);
  printP(letraV, b, b, letraS);
  printP(c(pre_f), td_f);
  printP(td, t(hora), td_f, td, c(minuto), td_f);
  printP(tr_f);

  for (byte i=0; i<maxPrgSem; i++)
    {
    mpi=mp*i;
    printP(tr);
    strcpy(auxchar, ssehtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
    printOpc(false, (i == posactsem), itoa(i + 1, buff, 10));
    if (posactsem==i)
      {
      printP(td, c(pre_i));
      for (byte j=0; j<maxPrg; j++) checkBox(mpi+j, getbit8(conf.bPRGsem[i],j)); // asociar a programa
      printP(c(pre_f),td_f,td);
      printP(c(Select_name),comillas);
      printIP(mpi+8, comillas);
      printP(mayor);
      for (byte j=0; j<maxSD; j++)   { // salidas digitales, 2
        printP(c(optionvalue));
        printPiP(comillas, j, comillas);
        if (conf.prgsal[i] == j) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, sdPin[j], parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j + 6, 20));
        printP(c(option_f));
       }
      for (byte j=0; j<maxEsc; j++) // escenas, 2
        {
        printP(c(optionvalue));
        printPiP(comillas, j+2, comillas);
        if (conf.prgsal[i]==j + 2) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j + 2, parenguion); else printP(mayor);
        printP(readdescr(filedescesc, j, 20));
        printP(c(option_f));
        }
      for (byte j=0; j<maxsalrem; j++) // señales remotas, 32
        if ((conf.idsalremote[j]>0) && (conf.senalrem[j]>=6))     {
          printP(c(optionvalue));
          printPiP(comillas, j+4, comillas);
          if (conf.prgsal[i]==j+4) printP(b, selected, ig, comillas, selected, comillas);
          if (conf.showN) printPiP(mayorparen, j + 4, parenguion); else printP(mayor);
          printP(readdescr(filesalrem,j,20));
          printP(c(option_f));
        }
      for (byte j = 0; j < maxPrg; j++) // programas, 2
        {
        printP(c(optionvalue));
        printPiP(comillas, j+20, comillas);
        if (conf.prgsal[i] == j + 20) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j + 20, parenguion); else printP(mayor);
        printP(readdescr(filedescprg, j, 20));
        printP(c(option_f));
        }
      printP(c(select_f));
      }
    else
      {
      printP(td);
      if (conf.bPRGsem[i][0] > 0)
      {
        printP(c(pre_i));
        for (byte j = 0; j < maxPrg; j++) printP(b, getbit8(conf.bPRGsem[i], j) ? itoa(j + 1, buff, 10) : b, b); // asociar a programa
        printP(c(pre_f));
      }
      printP(td_f, td);
      if (conf.bPRGsem[i][0] > 0)
        {
        if (conf.prgsal[i] <= 1) { printP(readdescr(filedesclocal, conf.prgsal[i]+6, 20)); }
        else if (conf.prgsal[i] < 3) { printP(readdescr(filedescesc, conf.prgsal[i]-2, 20)); }
        else if (conf.prgsal[i] < 20) { printP(readdescr(filesalrem, conf.prgsal[i]-4, 20)); }
        else if (conf.prgsal[i] < 22) { printP(readdescr(filesalrem, conf.prgsal[i]-20, 20)); }
        }
      }
    printP(td_f, td);
    if (posactsem==i)  printcampoSiNo(mpi+9, getbit8(conf.bprgval, i)); // valor on/off
    else if (conf.bPRGsem[i][0] > 0) printP(getbit8(conf.bprgval,i)?ON:OFF);
    printP(td_f);

    printP(td, c(pre_i));
    for (byte j=0; j<7; j++)
      if (posactsem==i)
        checkBox(mpi+10+j, getbit8(conf.prgdia[i], j)); // días de la semana
      else
      {
        printP(b);
        if (getbit8(conf.prgdia[i], j) == 1) printDiaSem(j); else printP(b);
        printP(b);    // días de la semana
      }
    printP(c(pre_f), td_f, td);

    if (posactsem == i)
    {
      printcampoI(mpi+17, conf.prghor[i], 2, true);
      printP(td_f,td);
      printcampoI(mpi+18, conf.prgmin[i], 2, true); // minuto
    }
    else
    {
      if (conf.bPRGsem[i][0] > 0) printI(conf.prghor[i]);
      printP(td_f, td);
      if (conf.bPRGsem[i][0] > 0) printI(conf.prgmin[i]);
    }
    printP(td_f, tr_f);
  }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupEveHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp = 22; // número de parámetros por fila
  if (server.method()==HTTP_POST)
    {
    for (byte i=0; i<maxPrg; i++) setbit8(conf.bPRGeve[posacteve], i, 0);
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (resto==1) setbit8(conf.bconacttipo, indice, mival);       // Tipo activadora: local o remota
      else if (resto==2) conf.condact[indice] = mival;              // señal activadora
      else if (resto==3) setbit8(conf.bconactmode, indice, mival);  // modo activación: cambio/estado
      else if (resto==4) conf.condvalD[indice] = mival;             // valor activador si digital
      else if (resto==5) conf.evencomp[indice] = mival;             // tipo comparador: menor-igual o mayor-igual
      else if (resto==6) conf.evenvalA[indice] = server.arg(i).toFloat();
      else if (resto==7) conf.evenhis[indice] = server.arg(i).toFloat();
      else if (resto==8) setbit8(conf.bconsaltipo, indice, mival);    // Tipo salida: local o remota
      else if (resto==9) conf.evensal[indice] = mival;                // señal a activar
      else if (resto==10) setbit8(conf.bevenniv, indice, mival);      // valor que debe tomar la señal de salida
      else if (resto>=11) setbit8(conf.bPRGeve[indice], resto - 11, mival); // asociacion PRG
      }
    saveconf();
    sendOther(svhtm,-1); return;
    }
  writeHeader(false,false);
  writeMenu(2, 2);
  writeForm(svhtm);

  printP(tr, td, td_f);
  printP(td, t(asociara), b);
  printP(t(programa), td_f);
  printColspan(2);  printP(t(senal), b);
  printP(t(activadora));
  printP(td_f);
  printColspan(3);  printP(t(ana), td_f);
  printColspan(2);  
  printP(t(senal), b);
  printP(t(de),b);
  printP(t(salida), td_f);
  printP(tr_f, tr, c(td_if), td);
  printP(c(pre_i));
  for (byte i=0; i<maxPrg; i++) printPiP(b, i + 1, b);
  printP(c(pre_f));
  printP(td_f, td, t(senal), td_f);
  printP(td, t(tvalor), td_f);
  printP(td, t(compa), td_f, td, t(tvalor), td_f);
  printP(td, t(hist), td_f);
  printP(td, t(senal), td_f);
  printP(td, ON, barra, OFF, td_f, tr_f);
  //
  /////////////////////////////////////////////////////////
  if (server.args() > 0) { posacteve = constrain(server.arg(0).toInt(), 0, 7); }
  for (byte i=0; i<nEVE; i++) // nEVE=8
    {
    colorea=false;
    boolean actdigital = (conf.condact[i] < 150);
    mpi=mp*i;
    indice=(i*12)+420; // parámetros del 420/426 en adelante
    printP(tr);
    strcpy(auxchar, svhtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
    printOpc(false, (i==posacteve), itoa(i+1,buff,10));
    if (i==posacteve)
      {
      printP(td, c(pre_i));
      for (byte j = 0; j < maxPrg; j++) checkBox(mpi + 11 + j, getbit8(conf.bPRGeve[i], j));
      printP(c(pre_f), td_f, td);
      printP(c(Select_name),comillas);
      printIP(mpi + 2, comillas);
      printP(mayor);
      for (byte j=0; j<maxED; j++)  { // añade entradas digitales locales
        printP(c(optionvalue));
        printPiP(comillas, j, comillas);
        if (j==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j+4, 20));
        printP(c(option_f));
        }
      for (byte j=0; j<maxSD; j++)  { // añade salidas digitales locales
        printP(c(optionvalue));
        printPiP(comillas, j+2, comillas);
        if (j+2==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j+2, parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j+6, 20));
        printP(c(option_f));
        }
      for (byte j=0; j<maxsalrem; j++) // señales digitales remotas
        {
        if (conf.idsalremote[j]>0)
          {
          if (conf.senalrem[j]>=4)   {
            printP(c(optionvalue));
            printPiP(comillas, j+100, comillas);
            if (j+100==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
            if (conf.showN) printPiP(mayorparen, j+100, parenguion); else printP(mayor);
            printP(readdescr(filesalrem, j, 20));
            printP(c(option_f));
            }
          }
        }
      printP(c(optionvalue));       // entrada analógica local
      printPiP(comillas,150,comillas);
      if (150==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
      if (conf.showN) printPiP(mayorparen, 150, parenguion); else printP(mayor);
      printP(readdescr(filedesclocal,3,20));
      printP(c(option_f));
      for (byte j=0; j<maxsalrem; j++) // entradas analógicas remotas
        if (conf.idsalremote[j]>0)
          if (conf.senalrem[j]==3)   {
            printP(c(optionvalue));
            printPiP(comillas, j+160, comillas);
            if (j+160 == conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
            if (conf.showN) printPiP(mayorparen, j+160, parenguion); else printP(mayor);
            printP(readdescr(filesalrem, j,20));
            printP(c(option_f));
          }
      for (byte j=0; j<maxTemp; j++)   { // añade temperaturas locales
        printP(c(optionvalue));
        printPiP(comillas, j+180, comillas);
        if (j + 180 == conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j+180, parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j, 20));
        printP(c(option_f));
        }
      for (byte j=0; j<maxsalrem; j++) // temperaturas remotas
        {
        if (conf.idsalremote[j] > 32)  {
          if (conf.senalrem[j] < 3)     {
            printP(c(optionvalue));
            printPiP(comillas, j+200, comillas);
            if (j+200==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
            if (conf.showN) printPiP(mayorparen, j + 200, parenguion); else printP(mayor);
            printP(readdescr(filesalrem, j, 20));
            printP(c(option_f));
            }
          }
        else if (conf.idsalremote[j] > 0)  {
          printP(c(optionvalue));
          printPiP(comillas, j+220, comillas);
          if (j + 220 == conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
          if (conf.showN) printPiP(mayorparen, j + 220, parenguion); else printP(mayor);
          printP(readdescr(filesalrem, j, 20));
          printP(c(option_f));
          }
        }
      printP(c(optionvalue));
      printPiP(comillas, 254, comillas);
      if (254==conf.condact[i]) printP(b, selected, ig, comillas, selected, comillas);
      if (conf.showN) printPiP(mayorparen, 254, parenguion); else printP(mayor);
      printP(t(preciokwh));
      printP(c(option_f));
      printP(c(select_f));
      }
    else
      {
      printP(td, c(pre_i));
      if (conf.bPRGeve[i][0] > 0) for (byte j = 0; j < maxPrg; j++) printP(b, getbit8(conf.bPRGeve[i], j) ? itoa(j + 1, buff, 10) : b, b);
      printP(c(pre_f), td_f, td);
      if (conf.bPRGeve[i][0] > 0)
        {
        if (conf.condact[i]<2) printP(readdescr(filedesclocal, conf.condact[i]+4, 20));
        else if (conf.condact[i]<4) printP(readdescr(filedesclocal, conf.condact[i]+4, 20));
        else if (conf.condact[i]<150) printP(readdescr(filesalrem, conf.condact[i]-100, 20));
        else if (conf.condact[i]<160) printP(readdescr(filesalrem, conf.condact[i]-150, 20));
        else if (conf.condact[i]<180) printP(readdescr(filesalrem, conf.condact[i]-160, 20));
        else if (conf.condact[i]<200) printP(readdescr(filesalrem, conf.condact[i]-180, 20));
        else if (conf.condact[i]<254) printP(readdescr(filesalrem, conf.condact[i]-200, 20));
        else if (conf.condact[i]==254) printP(t(preciokwh));
        }
      }
    printP(td_f);
    if (actdigital)    // la señal activadora es digital
      {
      printP(td);
      if (i==posacteve)
        printcampoSiNo(mpi + 4, conf.condvalD[i]);
      else if (conf.bPRGeve[i][0] > 0)
        printP(conf.condvalD[i] ? ON : OFF);
      printP(td_f);
      printColspan(3);
      }
    else
      {
      if (i==posacteve)
        {
        printP(c(td_if), td);
        printP(c(Select_name),comillas);
        printI(mpi + 5);
        printP(comillas, mayor);
        for (byte j=0; j<2; j++)  { // añade salidas posibles
          printP(c(optionvalue));
          printPiP(comillas, j, comillas);
          if (conf.evencomp[i] == j) printP(b, selected, ig, comillas, selected, comillas);
          printP(mayor, (j == 0) ? mayoroigual : menoroigual, c(option_f));
          }
        printP(c(select_f), td);
        printcampoF(mpi + 6, conf.evenvalA[i], 5); // valor analógico de comparación
        printP(td_f, td);
        printcampoF(mpi + 7, conf.evenhis[i], 5); // histéresis
        printP(td_f);
        }
      else
        {
        printP(c(td_if));
        if (conf.bPRGeve[i][0] > 0)
          {
          printP(conf.evencomp[i] ? menoroigual : mayoroigual);
          printP(td); printF(conf.evenvalA[i], 5); printP(td_f);
          printP(td); printF(conf.evenhis[i], 5); printP(td_f);
        }
        else
          printColspan(3);
      }
      printP(td_f);
    }
    printP(td);
    if (i==posacteve)
      {
      printP(c(Select_name),comillas);
      printI(mpi+9);
      printP(comillas, mayor);      // señal de salida
      for (byte j=0; j < maxSD; j++)   { // salidas digitales locales
          printP(c(optionvalue));
          printPiP(comillas, j, comillas);
        if (conf.evensal[i] == j) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j + 6, 20));
        printP(c(option_f));
        }
      for (byte j = 0; j < maxEsc; j++) // escenas, 2
        {
          printP(c(optionvalue));
          printPiP(comillas, j+2, comillas);
        if (conf.evensal[i]==j+2) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
        printP(readdescr(filedescesc, j, 20));
        printP(c(option_f));
        }
      for (byte j=0; j<maxsalrem; j++) // salidas remotas, 32
        if (conf.idsalremote[j]>0)
          if (conf.senalrem[j]>=6)
            {
            printP(c(optionvalue));
            printPiP(comillas, j+4, comillas);
            if (conf.evensal[i]==j+4) printP(b, selected, ig, comillas, selected, comillas);
            if (conf.showN) printPiP(mayorparen, j+4, parenguion); else printP(mayor);
            printP(readdescr(filesalrem,j,20));
            printP(c(option_f));
            }
      printP(c(select_f));
      }
    else
      {
      if (conf.bPRGeve[i][0] > 0)
        {
        if (conf.evensal[i] < 2) printP(readdescr(filedesclocal, conf.evensal[i] + 6, 20)); // 1-2
        else if (conf.evensal[i] < 4) printP(readdescr(filedescesc, conf.evensal[i] - 2, 20)); // 3-4
        else if (conf.evensal[i] < 100) printP(readdescr(filesalrem, conf.evensal[i] - 4, 20));
        else if (conf.evensal[i] == despIFTTT) printP(t(notifttt));  // 252
        }
      }
    printP(td_f, td);
    if (i==posacteve) printcampoSiNo(mpi + 10, getbit8(conf.bevenniv, i));
    else if (conf.bPRGeve[i][0]>0) printP(getbit8(conf.bevenniv, i)?ON:OFF);
    printP(td_f, tr_f);
    }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupFecHTML()
{
  printremote();
  if (!autOK()) {sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=7; // número de parámetros por fila
  if (server.method() == HTTP_POST)
  {
    memset(conf.bactfec, 0, sizeof(conf.bactfec));
    for (int i = 0; i < server.args(); i++)
    {
      calcindices(i);
      if (resto==0) setbit8(conf.bactfec, indice, server.arg(i).toInt());  // programa activo si/no
      else if (resto==1) conf.fecsal[indice] = server.arg(i).toInt();         // Número de salida
      else if (resto==2) setbit8(conf.bfecval, indice, server.arg(i).toInt());  // valor de la salida
      else if (resto==3) conf.fecdia[indice] = constrain(server.arg(i).toInt(), 1, 31); // día
      else if (resto==4) conf.fecmes[indice] = constrain(server.arg(i).toInt(), 1, 12); // mes
      else if (resto==5) conf.fechor[indice] = constrain(server.arg(i).toInt(), 0, 23); // Hora
      else if (resto==6) conf.fecmin[indice] = constrain(server.arg(i).toInt(), 0, 59); // minuto
    }
    saveconf();
    sendOther(sfhtm,-1); return;
  }

  if (server.args() > 0) {
    posactfec = constrain(server.arg(0).toInt(), 0, 3);
  }
  writeHeader(false,false);
  writeMenu(2, 3);
  writeForm(sfhtm);

  printP(tr,td,td_f);
  printP(td,t(Act),td_f,td);
  printP(t(salida), td_f, td);
  printP(t(tvalor), td_f,td);
  printP(t(dia), td_f, td);
  printP(t(mes), td_f,td);
  printP(t(hora), td_f, td);
  printP(c(minuto), td_f,tr_f);
  for (byte i = 0; i < maxPrgFec; i++)
  {
    mpi = mp * i;
    printP(tr);
    strcpy(auxchar, sfhtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
    printOpc(false, (i == posactfec), itoa(i + 1, buff, 10));
    printP(td);
    checkBox(mpi, getbit8(conf.bactfec, i));
    printP(td_f);

    if (i == posactfec)
      {
      printP(td);
      printP(c(Select_name),comillas);
      printIP(mpi+1, comillas);
      printP(mayor);
      for (byte j = 0; j < maxSD; j++)   { // salidas digitales, 2
          printP(c(optionvalue));
          printPiP(comillas, j, comillas);
        if (conf.fecsal[i] == j) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, sdPin[j], parenguion); else printP(mayor);
        printP(readdescr(filedesclocal, j + 6, 20));
        printP(c(option_f));
        }
      for (byte j = 0; j < maxEsc; j++)  { // escenas,2
          printP(c(optionvalue));
          printPiP(comillas, j+2, comillas);
        if (conf.fecsal[i] == j + 2) printP(b, selected, ig, comillas, selected, comillas);
        if (conf.showN) printPiP(mayorparen, j, parenguion); else printP(mayor);
        printP(readdescr(filedescesc, j, 20));
        printP(c(option_f));
        }
      for (byte j = 0; j < maxsalrem; j++) // salidas remotas, 32
        if ((conf.idsalremote[j] > 0) && (conf.senalrem[j] >= 6))
          {
          printP(c(optionvalue));
          printPiP(comillas, j+4, comillas);
          if (conf.fecsal[i] == j + 4) printP(b, selected, ig, comillas, selected, comillas);
          if (conf.showN) printPiP(mayorparen, j + 4, parenguion); else printP(mayor);
          printP(readdescr(filesalrem, j, 20));
          printP(c(option_f));
          }
      printP(c(select_f), td_f, td);
      printcampoSiNo(mpi+2, getbit8(conf.bfecval, i));
      printP(td_f, td);
      printcampoI(mpi+3, conf.fecdia[i], 2, true); // día
      printP(td_f, td);
      printcampoI(mpi+4, conf.fecmes[i], 2, true); // día
      printP(td_f, td);
      printcampoI(mpi+5, conf.fechor[i], 2, true);
      printP(td_f, td);
      printcampoI(mpi+6, conf.fecmin[i], 2, true); // minuto
      printP(td_f);
      }
    else
      {
      if (getbit8(conf.bactfec, i) > 0)
        {
        printP(td);
        if (conf.fecsal[i] < 2) printP(readdescr(filedesclocal, conf.fecsal[i] + 6, 20));
        else if (conf.fecsal[i] < 4) printP(readdescr(filedescesc, conf.fecsal[i] - 2, 20));
        else if (conf.fecsal[i] < 20) printP(readdescr(filesalrem, conf.fecsal[i] - 4, 20));
        printP(td_f);
        printP(td, getbit8(conf.bfecval, i) ? ON : OFF, td_f);
        printPiP(td,conf.fecdia[i],td_f);
        printPiP(td,conf.fecmes[i],td_f);
        printPiP(td,conf.fechor[i],td_f);
        printPiP(td,conf.fecmin[i],td_f);
        }
      else
        printColspan(6);
      }
    printP(tr_f);
  }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR setupEscHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  mp=8;  // número de parámetros por fila
  if (server.method()==HTTP_POST)
    {
    setbit8(conf.bshowEsc, posactesc, 0);
    for (int i=0; i<server.args(); i++)
      {
      calcindices(i);
      if (indice==0) { setbit8(conf.bshowEsc, resto, server.arg(i).toInt()); }  // mostrar escena
      else if (indice==1) { server.arg(i).toCharArray(auxdesc, 20); savedescr(filedescesc, auxdesc, resto, 20); }   // descripción escena
      else if (indice<=34) {
        setbit8(conf.bescenaact[resto], indice - 2, server.arg(i).toInt() == 0 ? 0 : 1);
        setbit8(conf.bescena[resto], indice - 2, server.arg(i).toInt() == 2 ? 1 : 0);
        }
      }
    saveconf();
    sendOther(seschtm,-1); return;
    }

  if (server.args()>0) { posactesc=constrain(server.arg(0).toInt(),0,7); }
  writeHeader(false,false);
  writeMenu(2, 4);
  writeForm(seschtm);

  printP(tr, td, t(escena), td_f);
  for (byte i=0; i<maxEsc; i++)    {
    strcpy(auxchar, seschtm); strcat(auxchar, igualp); strcat(auxchar, itoa(i, buff, 10));
    printOpc(false, (i == posactesc), itoa(i + 1, buff, 10));
    }

  printP(tr, td, c(ver), td_f);  // fila ver
  for (byte i=0; i<maxEsc; i++)
    {
    mpi=i;
    printP(td);
    if (posactesc==i) checkBox(mpi, getbit8(conf.bshowEsc, i));
    else printP(getbit8(conf.bshowEsc, i)?c(si):b);
    printP(td_f);
    }
  printP(tr_f);

  printP(tr, td, t(descripcion), td_f); // fila descripción
  for (byte i = 0; i < maxEsc; i++)
   {
    printP(td);
    if (posactesc == i)
      {
      mpi=mp+i;
      printP(menor, c(tinput), b, type, ig, comillas);
      printP(c(ttext), comillas, b);
      printP(c(namet), ig, comillas);
      printI(mpi);
      printP(comillas, b, tvalue, ig, comillas);
      printP(readdescr(filedescesc, i, 20));
      printP(comillas,b,c(max_length));
      printIP(19, size_i);
      printI(19);
      printP(comillas, mayor, menor, barra, c(tinput), mayor);
      }
    else
      printP(getbit8(conf.bshowEsc, i) ? readdescr(filedescesc, i, 20) : puntossus);
    printP(td_f);
    }
  printP(tr_f);

  for (byte i=0; i<maxSD; i++) // para cada salida local
    {
    printP(tr, td, readdescr(filedesclocal, i + 6, 20), td_f);
    for (byte j = 0; j < maxEsc; j++) // para cada escena de cada salida
      {
      mpi = (mp * (i + 2) + j);
      colorea = false;
      byte valaux = (getbit8(conf.bescenaact[j],i) == 0 ? 0 : getbit8(conf.bescena[j], i) + 1);
      printP(td);
      if (posactesc==j)
        {
        printP(c(Select_name),comillas);
        printIP(mpi, comillas);
        printP(mayor);
        for (byte k=0; k<3; k++) {
          printP(c(optionvalue));
          printPiP(comillas, k, comillas);
          if (k == valaux) printP(b, selected, ig, comillas, selected, comillas);
          printP(mayor, k==0?guion:k==1?OFF:ON,c(option_f));
          }
        printP(c(select_f));
        }
      else
        printP(valaux==0?guion:(valaux==1?OFF:ON));
      printP(td_f);
      }
    printP(tr_f);
    }

  for (byte i = 0; i < maxsalrem; i++) // para cada salida remota
    if (conf.idsalremote[i] > 0)
      if (conf.senalrem[i] >= 6)
      {
        printP(tr, td, readdescr(filesalrem, i, 20), td_f);
        for (byte j = 0; j < maxEsc; j++) // para cada escena de cada salida
        {
          mpi = (mp * (i + 4) + j); // número de parámetro
          colorea = false;
          byte valaux = (getbit8(conf.bescenaact[j], i + 2) == 0 ? 0 : getbit8(conf.bescena[j], i + 2) + 1);
          printP(td);
          if (posactesc == j)
          {
            printP(c(Select_name),comillas);
            printIP(mpi, comillas);
            printP(mayor);
            for (byte k = 0; k < 3; k++)  {
          printP(c(optionvalue));
          printPiP(comillas, k, comillas);
              if (k == valaux) printP(b, selected, ig, comillas, selected, comillas);
              printP(mayor, k == 0 ? guion : k == 1 ? OFF : ON, c(option_f));
            }
            printP(c(select_f), td_f);
          }
          else
            printP(valaux == 0 ? guion : valaux == 1 ? OFF : ON);
          printP(td_f);
        }
        printP(tr_f);
      }
  writeFooter(tguardar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR initWiFi(void)
{
  dPrint(t(reiniciando)); dPrint(b); dPrint(c(twifi)); dPrint(crlf);
  conf.wifimode = 1;                  // AP
  strcpy(conf.ssidSTA, c(tssid)); strcat(conf.ssidSTA, subray); strcat(conf.ssidSTA, sta);
  strcpy(conf.passSTA, c(tpass)); strcat(conf.passSTA, subray); strcat(conf.passSTA, sta);
  strcpy(conf.ssidAP, c(CONUCO)); strcat(conf.ssidAP, subray); strcat(conf.ssidAP, itoa(conf.iddevice, buff, 10));
  strcpy(conf.passAP, t12341234);
  conf.canalAP=3;
  for (byte i=0; i<6; i++) strcpy(conf.EEmac[i], vacio);
  conf.staticIP = 1;
  conf.EEip={192,168,1,150};  conf.EEip[3]=conf.iddevice;
  conf.EEmask={255,255,255,0};
  conf.EEgw={192,168,1,1};
  conf.EEdns={8,8,8,8};
  conf.EEdns={8,8,4,4};
  conf.webPort=88;
  strcpy(conf.hostmyip,c(icanhazip));
  saveconf();
}

void initConf()
{
  ////////////////////////////////// RF
  memset(conf.rfkeys.pro, 0, sizeof(conf.rfkeys.pro));      // teclas de control: up,down,left,rigth,esc,intro
  memset(conf.rfkeys.code, 0, sizeof(conf.rfkeys.code));
  memset(conf.rfkeys.len, 0, sizeof(conf.rfkeys.len));
  memset(conf.code433.proon, 0, sizeof(conf.code433.proon));   // teclas de on/off
  memset(conf.code433.codeon, 0, sizeof(conf.code433.codeon));
  memset(conf.code433.lenon, 0, sizeof(conf.code433.lenon));
  memset(conf.code433.prooff, 0, sizeof(conf.code433.prooff));
  memset(conf.code433.codeoff, 0, sizeof(conf.code433.codeoff));
  memset(conf.code433.lenoff, 0, sizeof(conf.code433.lenoff));
  //////////////////////////////////////// DEVICE
  conf.iddevice=150;
  strcpy(conf.aliasdevice, t(NUEVO));
  conf.bestado=0;
  conf.valinic[0]=2; conf.valinic[1]=2;         // último valor como defecto
  conf.showN=0;
  conf.peractpan=15;
  conf.prectemp=12;
  memset(conf.tipoED,0,sizeof(conf.tipoED));
  conf.usepassDev=0;                                // control usuario desactivado
  strcpy(conf.userDev,admin);                       // 20 bytes, usuario device
  strcpy(conf.passDev,admin);                       // 20 bytes, password device
  conf.modo45=0;                                    // modo pines 4 y 5 como entradas digitales
  conf.iottweetenable=0;                            // IoTTweet desactivado
  conf.factorA[0]=1.0;                              // coeficiente entrada analógica
  conf.offsetA[0]=0.0;                              // offset entrada analógica
  strcpy(conf.unitpinA,vacio);                     // unidades entrada analógica
  conf.bsumatA[0]=0;                                // acumular valores entrada analógica
  memset(conf.tempdefact, 0, sizeof(conf.tempdefact));
  memset(conf.tempdefdes, 0, sizeof(conf.tempdefdes));
  conf.staticIP=1;                                  // en initWiFi
  conf.TempDesactPrg=600;                           // tiempo desactivación programas si falla la hora en segundos  
  memset(conf.contadores,0,sizeof(conf.contadores));// contadores de número de activaciones de entradas digitales
  conf.webPort=88;                                  // en initWiFi
  memset(conf.bshowbypanel,0,sizeof(conf.bshowbypanel)); conf.bshowbypanel[0][0]=193;
  conf.wifimode=1;                                  // en initWifi, modo AP
  strcpy(conf.ssidSTA,"SSID_AP");
  strcpy(conf.passSTA,"PASS_AP");
  strcpy(conf.ssidAP,"CONUCO_150");
  strcpy(conf.passAP,t12341234);
  ///////////////////////////////////// REMOTOS Y SALIDAS REMOTAS
  memset(conf.pinremote,0,sizeof(conf.pinremote));      // pin de cada salida remota
  memset(conf.idsalremote,0,sizeof(conf.idsalremote));  // dispositivo de cada salida remota
  memset(conf.ipremote,0,sizeof(conf.ipremote));
  memset(conf.bshowpanel,0,sizeof(conf.bshowpanel)); 
  memset(conf.idremote,0,sizeof(conf.idremote));
  memset(conf.senalrem,0,sizeof(conf.senalrem));
  strcpy(conf.iottweetkey,vacio);              // IoTtweet account user ID (6 digits, included zero pre-fix)
  conf.timeoutrem=500;
  conf.timeoutNTP=1000;
  strcpy(conf.iottweetuser,vacio);             // IoTtweet account user ID (6 digits, included zero pre-fix)
  conf.peractrem=10;                           // período de actualización automática a dispositivo maestro
  memset(conf.tipoi2cmbrem, 0, sizeof(conf.tipoi2cmbrem));
  conf.iftttenable=0;                          // ifttt desactivado
  strcpy(conf.iftttkey, vacio);                // IoTtweet account user ID (6 digits, included zero pre-fix)
  memset(conf.iftttpinED,0,sizeof(conf.iftttpinED));
  memset(conf.iftttpinSD,0,sizeof(conf.iftttpinSD));
  conf.mododweet=0;                            // dweet.io desactivado
  conf.modomyjson=0;                           // myjson desactivado
  conf.idmyjsonST=0;
  strcpy(conf.idmyjson, vacio);
  conf.modoterm=0;
  conf.latitud=0.0;  conf.longitud=0.0;
  strcpy(conf.myippub,vacio);
  conf.actualizaut = 0;
  conf.netseg=1;                                        // segmento de red local
  memset(conf.setpoint,0,sizeof(conf.setpoint));        // setpoint temperaturas
  memset(conf.salsetpoint,0,sizeof(conf.salsetpoint));  // salida asociada al setpoint (0,1 salidas locales; 2-17 salidas remotas)
  memset(conf.accsetpoint,0,sizeof(conf.accsetpoint));  // acción asociada al setpoint (0=OFF,1=ON,2=ninguna)
  conf.ftpenable=1;                         // FTP activado
  conf.lang=0;                              // español, por defecto
  ///////////////////////////////////// REMOTOS
  memset(mbtemp, 0, sizeof(mbtemp));         // estado relés remotos modbus (1 bit cada uno);
  memset(mbcons, 0, sizeof(mbcons));         // estado relés remotos modbus (1 bit cada uno);
  memset(mbstatus, 0, sizeof(mbstatus));     // estado relés remotos modbus (1 bit cada uno);
  
}

void ICACHE_FLASH_ATTR initFab(void)
{
  dPrint(t(reiniciando)); dPrint(b); dPrint(t(fabrica)); dPrint(crlf);
  initConf();                  // variables de estructura Conf
  initWiFi();                  // WiFi y Red
  initPRG();                   // PROGRAMAS
  saveconf();
}

void ICACHE_FLASH_ATTR resetHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg=vacio;
  if (server.method() == HTTP_POST)
    {
    for (int i=0; i<server.args(); i++)
      {
      if (server.argName(i).toInt()==0)
        {
        boolean idaccion = server.arg(i).toInt();
        if (idaccion > 0)
          {
          writeHeader(false,false);
          server.sendHeader("Connection", "close");
          server.sendHeader("Access-Control-Allow-Origin", "*");
          server.send(200, "text/html", espere);
          if (idaccion==1)      { ESP.reset(); }
          else if (idaccion==2) { ESP.restart(); }
          else if (idaccion==3) { initWiFi();  }
          else if (idaccion==4) { initFab(); ESP.reset(); }
          }
        }
      }
    return;
    }
  writeHeader(false,false);
  writeMenu(4, 2);
  writeForm(rshtm);
  printP(tr);
  printP(td, treset, barra);
  printP(trestart, td_f, td);
  printcampoCB(0, 0, nohacernada, treset, trestart, tresetwifi,tresetfab);
  printP(td_f,tr_f);
  writeFooter(tejecutar, false);
  serversend200();
}

void ICACHE_FLASH_ATTR resetcontador(byte n)
{
  printremote();
  conf.contadores[n] = 0;
  saveconf();
  sendOther(barra,-1);
}

void ICACHE_FLASH_ATTR systemHTML()
{
  printremote();
  if (!autOK()) { sendOther(loghtm,-1); return; }
  msg =vacio;
  if (server.method()==HTTP_GET)
  {
    for (int i=0; i<server.args(); i++)
    {
      if (server.arg(i).compareTo("conuco")==0) {saveconf();        return;
      }
      else if (server.arg(i).compareTo(PSTR("rp0"))==0)   {
        resetcontador(0);
        return;
      }
      else if (server.arg(i).compareTo(PSTR("rp1")) == 0)   {
        resetcontador(1);
        return;
      }
      else if (server.argName(i).compareTo(PSTR("pon")) == 0)
        {
        int auxerr = pinvalR(conf.idsalremote[server.arg(i - 1).toInt()], 88, conf.senalrem[server.arg(i - 1).toInt()] - 6, 1);
        if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))
          {
          setbit8(bstatremote, server.arg(i - 1).toInt(), 1);
          contaremote[server.arg(i - 1).toInt()] = 0;
          }
        else
          addlog(3, auxerr, c(tpinvalr));
        }
      else if (server.argName(i).compareTo(PSTR("pof")) == 0)
        {
        int auxerr = pinvalR(conf.idsalremote[server.arg(i - 1).toInt()], 88, conf.senalrem[server.arg(i - 1).toInt()] - 6, 0);
        if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))
          {
          setbit8(bstatremote, server.arg(i - 1).toInt(), 0);
          contaremote[server.arg(i - 1).toInt()] = 0;
          }
        else
          addlog(3, auxerr, c(tpinvalr));
        }
      else if (server.arg(i).compareTo(PSTR("rfon")) == 0)
        {
        if ((server.arg(i - 1).toInt() >= 0) && (server.arg(i - 1).toInt() <= 1))
          {
          conf.code433.proon[server.arg(i - 1).toInt()] = lastpro;
          conf.code433.codeon[server.arg(i - 1).toInt()] = lastcode;
          conf.code433.lenon[server.arg(i - 1).toInt()] = lastlen;
          }
        sendOther(rfhtm,-1); return;
        }
      else if (server.arg(i).compareTo(PSTR("rfoff")) == 0)
        {
        if ((server.arg(i - 1).toInt() >= 0) && (server.arg(i - 1).toInt() <= 1))
          {
          conf.code433.prooff[server.arg(i - 1).toInt()] = lastpro;
          conf.code433.codeoff[server.arg(i - 1).toInt()] = lastcode;
          conf.code433.lenoff[server.arg(i - 1).toInt()] = lastlen;
          }
        sendOther(rfhtm,-1); return;
        }
      else if (server.arg(i).compareTo(PSTR("rrfon")) == 0)
        {
        if ((server.arg(i - 1).toInt() >= 0) && (server.arg(i - 1).toInt() <= 15))
          {
          conf.code433.proon[server.arg(i - 1).toInt() + 2] = lastpro;
          conf.code433.codeon[server.arg(i - 1).toInt() + 2] = lastcode;
          conf.code433.lenon[server.arg(i - 1).toInt() + 2] = lastlen;
          }
        sendOther(rfhtm,-1); return;
        }
      else if (server.arg(i).compareTo(PSTR("rrfoff")) == 0)
        {
        if ((server.arg(i - 1).toInt() >= 0) && (server.arg(i - 1).toInt() <= 15))
          {
          conf.code433.prooff[server.arg(i - 1).toInt() + 2] = lastpro;
          conf.code433.codeoff[server.arg(i - 1).toInt() + 2] = lastcode;
          conf.code433.lenoff[server.arg(i - 1).toInt() + 2] = lastlen;
          }
        sendOther(rfhtm,-1); return;
        }
      else if (server.arg(i).compareTo(PSTR("rfkey")) == 0)
        {
        if ((server.arg(i - 1).toInt() >= 1) && (server.arg(i - 1).toInt() <= 6))
          {
          conf.rfkeys.pro[server.arg(i-1).toInt()-1] = lastpro;
          conf.rfkeys.code[server.arg(i-1).toInt()-1] = lastcode;
          conf.rfkeys.len[server.arg(i-1).toInt()-1] = lastlen;
          }
        sendOther(rfhtm,-1); return;
        }
      else if (server.arg(i).compareTo(PSTR("wc")) == 0)   {
        msg=readdescr(fileurlwebcall, server.arg(i + 1).toInt(), 60);
        HTTPClient http;
        int port=msg.substring(msg.indexOf(":")+1, msg.indexOf("/")).toInt(); if (port==0) port=80;
        http.begin(msg.substring(0, msg.indexOf(":")), port, msg.substring(msg.indexOf("/"), msg.length()));
        http.setTimeout(conf.timeoutrem);
        int httpCode = http.GET();
        http.end();
        msg=vacio;
        }
      else if (server.arg(i).compareTo(PSTR("ro")) == 0)   {        // ROOMBA
        Serial.print(server.arg(0).toInt());
        sendOther(rohtm,-1); return; 
        }
      else if (server.argName(i).compareTo(PSTR("si")) == 0)   {
        nAPact = server.arg(i - 1).toInt();
        WiFi.SSID(nAPact).toCharArray(conf.ssidSTA, 20);
        saveconf();
        sendOther(snehtm,-1); return;
        }
      }
    sendOther(panelhtm, panelact);
  }
}

void ICACHE_FLASH_ATTR checkForUpdates() {
  String fwFileName = String("conuco-1-8");
  String fwFileVer = String(conf.fwUrlBase);
  fwFileVer.concat(fwFileName); fwFileVer.concat(".ver");
  HTTPClient httpClient;
  addlog(0, 0, "fwFileVer:");
  httpClient.begin(fwFileVer);
  int httpCode = httpClient.GET();
  if ( httpCode == 200) {
    String newFWVersion = httpClient.getString();
    verserver = newFWVersion.toInt();
    if (conf.actualizaut == 1)   {
      if (verserver > versinst) {
        addlog(0, 0, "Updating...");
        String fwFileBin = String(conf.fwUrlBase);
        fwFileBin.concat(fwFileName);   fwFileBin.concat(inobin);
        t_httpUpdate_return ret = ESPhttpUpdate.update(fwFileBin);
        switch (ret) {
          case HTTP_UPDATE_FAILED:
            addlog(2, ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            //            Serial.printf("UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
            break;
          case HTTP_UPDATE_NO_UPDATES:
            addlog(2, 2, "UPDATE_NO_UPDATES");
            break;
        }
      }
      else
      {
        addlog(0, 0, t(versionact));
      }
    }
  }
  else {
    addlog(2, httpCode, t(versioncheckfail));
  }
  httpClient.end();
}

uint8_t portArray[] = {16, 5, 4, 0, 2, 14, 12, 13};
//String portMap[] = {"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7"}; //for Wemos
String portMap[] = {"GPIO16", "GPIO5", "GPIO4", "GPIO0", "GPIO2", "GPIO14", "GPIO12", "GPIO13"};

void ICACHE_FLASH_ATTR check_if_exist_I2C() {
  byte error, address;
  int nDevices;
  nDevices = 0;
  for (address = 1; address < 127; address++ )  {
    // The i2c_scanner uses the return value of the Write.endTransmisstion to see if a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print(c(i2cdevnotfound));
      if (address < 16) Serial.print(cero);
      Serial.print(address, HEX);
      Serial.println(PSTR("  !"));
      nDevices++;
    } else if (error == 4) {
      Serial.print(c(unkerrorat));
      if (address < 16) Serial.print(cero);
      Serial.println(address, HEX);
    }
  } //for loop
  if (nDevices == 0)
    Serial.println(c(i2cnodevices));
  else
    printlinea(ig);
}

//void ICACHE_FLASH_ATTR scanPorts() {
//  for (uint8_t i = 0; i < sizeof(portArray); i++) {
//    for (uint8_t j = 0; j < sizeof(portArray); j++) {
//      if (i != j){
//        Serial.print("Scanning (SDA : SCL) - " + portMap[i] + " : " + portMap[j] + " - ");
//        Wire.begin(portArray[i], portArray[j]);
//        check_if_exist_I2C();
//      }
//    }
//  }
//}

void ICACHE_FLASH_ATTR lcdshowstatus()
{
  lcd.clear();
  for (byte i=0;i<2;i++) { lcd.setCursor(i*6,0); lcd.print(MbR[i]*0.01,1); lcd.print(letrac); }
  for (byte i=0;i<3;i++) { lcd.setCursor(i*4,1); lcd.print(getbit8(conf.MbC8,i)==1?ON:OFF); lcd.print(b); }
}

void ICACHE_FLASH_ATTR lcdshowconf(boolean editing)
{
  lcd.clear(); if (pendsave) lcd.print(aster); lcd.print(paract);
  lcd.setCursor(4, 0);
  if (paract == 0) lcdshowstatus();
  else if (paract == 1) {
    if (!WiFi.isConnected()) lcd.print(NO); lcd.print(b);
    lcd.print(t(tconectado)); lcd.print(b); lcd.print(letraa);
    lcd.setCursor(0, 1); lcd.print(conf.ssidSTA);
    }
  else if (paract == 2) { lcd.print(t(dispositivo)); lcd.setCursor(0,1); lcd.print(conf.iddevice); lcd.print(b); lcd.print(conf.aliasdevice); }
  else if (paract == 3) { lcd.print(t(versiont)); lcd.setCursor(0,1); lcd.print(versinst); }
  else if (paract == 4) { lcd.print(t(Modo)); lcd.setCursor(0,1); lcd.print(conf.wifimode); lcd.print(b);
                          lcd.print(conf.wifimode==0?sta:conf.wifimode==1?ap:apsta);  }
  else if (paract == 5) { lcd.print(c(IP)); lcd.setCursor(0,1); for (byte i=0;i<4;i++) { lcd.print(conf.EEip[i]); if (i<3) lcd.print(punto); } }
  else if (paract == 6) { lcd.print(t(ippublica)); lcd.setCursor(0,1); lcd.print(conf.myippub); }
  else if (paract == 7) { lcd.print(c(routerssid)); lcd.setCursor(0,1); lcd.print(conf.ssidSTA); }
  else if (paract == 8) { lcd.print(c(routerpass)); lcd.setCursor(0,1); lcd.print(conf.passSTA); }
  else if (paract == 9) { lcd.print(t(usuario)); lcd.setCursor(0,1); lcd.print(conf.userDev); }
  else if (paract == 10) { lcd.print(c(password)); lcd.setCursor(0,1); lcd.print(conf.passDev); }
  else if (paract == 11) { lcd.print(c(apssid)); lcd.setCursor(0,1); lcd.print(conf.ssidAP); }
  else if (paract == 12) { lcd.print(c(appass)); lcd.setCursor(0,1); lcd.print(conf.passAP); }
  else if (paract == 13) { lcd.print(t(canal)); lcd.setCursor(0,1); lcd.print(conf.canalAP); }
  else if (paract == 14) { lcd.print(c(MAC)); lcd.setCursor(0,1); lcd.print(mac); }
  else if (paract == 15) { lcd.print(c(watermarkt)); lcd.setCursor(0,1); lcd.print(conf.watermark); }
}

void ICACHE_FLASH_ATTR initHTML()
{
  server.onNotFound (htmlNotFound);
  httpUpdater.setup(&server,"/firm", update_username, update_password);
  server.on("/f", filesHTML);
  server.on("/rs", resetHTML);
  server.on("/sy", systemHTML);
  
  if (!checkfiles()) { server.on("/", filesHTML); return;  }
  server.on("/", indexHTML);
  server.on("/dw", downloadHTML);
  server.on("/es", espsysHTML);
  server.on("/j", jsonHTML);
  server.on("/jc", jsonconfHTML);
  server.on("/je", jsonextHTML);
  server.on("/l", loginHTML);
  server.on("/on", onCmd);
  server.on("/of", offCmd);
  server.on("/p", panelHTML);
  server.on("/rj", rjsonHTML);
  server.on("/rjc", rjsonconfHTML);
  server.on("/rf", setuprfHTML);
  server.on("/ro", roombaHTML);
  server.on("/sbp", setupbyPanelHTML);
  server.on("/sc", scanapHTML);
  server.on("/sd", setupDevHTML);
  server.on("/sdr", setupDevRemHTML);
  server.on("/sdrio", setupDevRemioHTML);
  server.on("/se", setupEscHTML);
  server.on("/sf", setupFecHTML);
  server.on("/sio", setupioHTML);
  server.on("/sne", setupNetHTML);
  server.on("/sns", setupNetServHTML);
  server.on("/sp", setupPanelHTML);
  server.on("/spr", setupPrgHTML);
  server.on("/sr", setupremHTML);
  server.on("/s150", setupdev150HTML);
  server.on("/ssr", setupsalremHTML);
  server.on("/ss", setupSegHTML);
  server.on("/sse", setupSemHTML);
  server.on("/sv", setupEveHTML);
  server.on("/swc", setupWebCallHTML);
  server.on("/t", termostatoHTML);
  server.on("/v", voicecommandHTML);
  
  server.on("/l0", handleState0Out);
  server.on("/l1", handleState1Out);
  server.on("/l2", handleState0In);
  server.on("/l3", handleState1In);
//  server.on("/r0", handleStater0);
//  server.on("/r1", handleStater1);
//  server.on("/r2", handleStater2);
//  server.on("/r3", handleStater3);
//  server.on("/r4", handleStater4);
//  server.on("/r5", handleStater5);
//  server.on("/r6", handleStater6);
//  server.on("/r7", handleStater7);
//  server.on("/r8", handleStater8);
//  server.on("/r9", handleStater9);
//  server.on("/r10", handleStater10);
//  server.on("/r11", handleStater11);
//  server.on("/r12", handleStater12);
//  server.on("/r13", handleStater13);
//  server.on("/r14", handleStater14);
//  server.on("/r15", handleStater15);
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

void ICACHE_FLASH_ATTR setup(void) {
  memset(contaremote, 0, sizeof(contaremote));
  for (byte i=0; i<maxdevrem; i++) {
    strcpy(auxdesc,vacio); savedescr(filemacdevrem,auxdesc, i,14);
    strcpy(auxdesc,vacio); savedescr(fileidmyjsonrem,auxdesc, i,10); }

  pinMode(bt2Pin, INPUT);
  pinMode(rx433, INPUT_PULLUP);
  pinMode (sdPin[0], OUTPUT);  pinMode (sdPin[1], OUTPUT);
  pinMode(ledSt, OUTPUT); digitalWrite(ledSt, 0);
  //  pinMode (rs485enpin, OUTPUT); digitalWrite(rs485enpin,0);

  Serial.begin (115200);  delay(10);
  EEPROM.begin(ROMsize);

  // comprobar si es la primera vez que se ejecuta conuco
  //  Serial.println();Serial.print(c(watermarkt));Serial.print(dp); Serial.println(conf.watermark);
  //  if (strcmp(conf.watermark,"conuco")!=0)
  //    {
  //    saveconf();
  //    initFab();
  //    }

  ///////  SPIFFS
  Serial.println(); Serial.println(t(files));
  Serial.print(c(spiffs));Serial.print(b);
  if (SPIFFS.begin()) Serial.println(ok); else { Serial.println(c(terror)); ; return; }
  Dir dir=SPIFFS.openDir(barra);
  while (dir.next())   {
    Serial.print(dir.fileName());  Serial.print(b);
    File f=dir.openFile(letrar);
    Serial.println(f.size());  }
  ///////  SPIFFS FIN
  ftpSrv.begin(admin,admin);    //username, password for ftp.  set ports in ESP8266FtpServer.h  (default 21, 50009 for PASV)

  sensors1.begin();
  sensors1.setResolution(conf.prectemp);
  nTemp1 = sensors1.getDeviceCount();

  readconf();
  if (conf.netseg==255) conf.netseg=1;      // provisional
  strcpy(conf.mqttpath[0],c(conuco));
  strcpy(conf.mqttpath[1],conf.instname);
  strcpy(conf.mqttpath[2],itoa(conf.iddevice,buff,10));

  memset(mbtemp,0,sizeof(mbtemp));         // estado relés remotos modbus (1 bit cada uno);
  memset(mbcons,0,sizeof(mbcons));         // estado relés remotos modbus (1 bit cada uno);
  memset(mbstatus,0,sizeof(mbstatus));     // estado relés remotos modbus (1 bit cada uno);
  for (byte i=0; i<3; i++) MbRant[i]=MbR[i];
  MbC8ant[0]=conf.MbC8[0];
  memset(ListOri,0,sizeof(ListOri));
//  serialprintParESP();

  dPrint(t(dispositivo)); dPrint(dp); dPrintI(conf.iddevice); dPrint(crlf);
  dPrint(t(versiont)); dPrint(dp); dPrintI(versinst); dPrint(crlf);
  dPrint(t(sondastemp)); dPrint(dp); dPrintI(nTemp1); dPrint(b);
  dPrint(b); dPrint(t(Modo)); dPrint(dp);
  dPrint((sensors1.isParasitePowerMode())?c(parasite):c(power)); dPrint(crlf);
  for (byte i=0; i<nTemp1; i++)
    {
    if (sensors1.getAddress(addr1Wire[i], i))
      {
      dPrint(b);
      for (uint8_t j=0; j<8; j++) { if (addr1Wire[i][j]<16) dPrint(cero); Serial.print(addr1Wire[i][j], HEX); }
      dPrint(crlf);
      }
    }

  if (conf.wifimode==0) WiFi.mode(WIFI_STA);
  else if (conf.wifimode==1) WiFi.mode(WIFI_AP);
  else if ((conf.wifimode==2) or (conf.wifimode==12)) WiFi.mode(WIFI_AP_STA);
  WiFi.setPhyMode(WIFI_PHY_MODE_11N);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  dPrint(t(wifimodet)); dPrint(dp); dPrintI(conf.wifimode); dPrint(crlf);
  dPrint(c(MAC)); dPrint(dp); dPrint(WiFi.softAPmacAddress()); dPrint(crlf);
  for (byte i=0; i<6; i++) {
    WiFi.softAPmacAddress().substring(i*3,i*3+2).toCharArray(conf.EEmac[i], 3);
    strcat(mac, conf.EEmac[i]);
    }
  if ((conf.wifimode==1) || (conf.wifimode==2) || (conf.wifimode==12)) // AP o AP+STA
    {
    WiFi.channel(conf.canalAP);
    WiFi.softAP(conf.ssidAP, conf.passAP, conf.canalAP, false);
    dPrint(t(canal)); dPrint(dp); dPrintI(WiFi.channel()); dPrint(crlf);
    dPrint(c(IP)); Serial.print(WiFi.softAPIP()); dPrint(crlf);
    }
  if ((conf.wifimode==0) || (conf.wifimode==2) || (conf.wifimode==12)) // STA o AP+STA
    {
    dPrint(t(staticip));  dPrint(dp); dPrint(conf.staticIP ? t(SI) : t(NO)); dPrint(coma);
    if (conf.staticIP==1)
      {
      dPrint(WiFi.config(conf.EEip, conf.EEgw, conf.EEmask, conf.EEdns, conf.EEdns2) == 1 ? ok : c(terror));
      }
    dPrint(crlf);
    WiFi.begin(conf.ssidSTA, conf.passSTA, true);
    if (debugwifi) Serial.setDebugOutput(true);
    byte cont = 0;
    dPrint(t(conectando)); dPrint(b); dPrint(WiFi.SSID()); dPrint(barra); dPrint(WiFi.psk()); dPrint(b);
    while ((!WiFi.isConnected()) && (cont++ < 20))  {
      delay(1000);
      dPrint(punto);
    }
    dPrint(crlf); dPrint(t(tconectado)); dPrint(b); dPrint(WiFi.isConnected()?ok:c(terror)); dPrint(crlf);
    dPrint(c(IP)); Serial.print(WiFi.localIP()); dPrint(crlf);
    dPrint(c(thost)); dPrint(WiFi.hostname()); dPrint(crlf);
    }
  for (byte i=0; i<maxSD; i++)
    {
    pinMode(sdPin[i], OUTPUT);
    digitalWrite(sdPin[i], valorpin[conf.valinic[i]>1?getbit8(conf.MbC8,sdPin[i]-12):conf.valinic[i]]);
    }

  //here the list of headers to be recorded
  const char * headerkeys[]={"User-Agent","Cookie"};
  size_t headerkeyssize=sizeof(headerkeys)/sizeof(char*);
  server.collectHeaders(headerkeys, headerkeyssize);
  initHTML();
  server.begin();
  timeClient.begin();
  if (conf.modo45==0) {
    Serial.print(c(tinput)); Serial.print(b); Serial.println(modet);
    pinMode(edPin[0], INPUT_PULLUP);
    pinMode(edPin[1], INPUT_PULLUP);    }
  else if (conf.modo45 == 1) {
    Serial.print(i2c); Serial.print(b); Serial.println(modet);
    Wire.begin(edPin[0], edPin[1]);
    if (!bmp085.begin()) { Serial.print(b);  Serial.println(BMP085notfound);    }  }
  else if (conf.modo45==2) {
    Serial.print(modbust); Serial.print(b); Serial.println(modet);
    //    SoftSerial.begin(modbusbaud);
    //    SoftSerial.setTransmitEnablePin(rs485enpin);
    //    MBnode.begin(1, SoftSerial);
    pinMode (edPin[0], INPUT_PULLUP);
    pinMode (edPin[1], OUTPUT);  }

  memset(timerem, 0, sizeof(timerem));
  for (byte i=0; i<maxdevrem; i++) { actirem[i]=true; actisenal[i]=true; }
  dPrint(c(tport)); dPrint(dp); Serial.print(88); dPrint(crlf);
  dPrint(conf.userDev); dPrint(barra); dPrint(conf.passDev); dPrint(crlf);
  leevaloresOW();
  if (WiFi.isConnected()) {
    timeClient.setTimeOffset(7200);
    if (timeClient.update() == 1) {
      countfaulttime = 0;
      setTime(timeClient.getEpochTime());    }
    getMyIP();
//    checkMyIP();
//    dPrint(t(ippublica)); dPrint(dp); dPrint(conf.myippub); dPrint(crlf);
    delay(100);
    if (mododirecto==1) { actualizamasters(); }
    actualizaremotos(mododirecto);
    for (byte i=0; i<maxdevrem; i++)
      if ((conf.idremote[i]>=150) && (conf.idremote[i]<=166))
        {
        dPrint(b); dPrintI(conf.idremote[i]); dPrint(b);
        int auxerr=ReqJsonConf(conf.idremote[i], 88);
        if (auxerr==HTTP_CODE_OK) {
          Serial.println(ok);
          extraevaloresTempConf(false);
          strcpy(auxdesc, aliasdevicetemp); savedescr(filedevrem, auxdesc, i, 20);
          for (byte j = 0; j < maxsalrem; j++)
            if (conf.idsalremote[j] == conf.idremote[i])
              if ((conf.senalrem[j] >= 0) && (conf.senalrem[j] <= 7)) {
                readdescr(filedesctemp, conf.senalrem[j], 20); savedescr(filesalrem, auxdesc, j, 20);  }
          saveconf();
          }
        else
          Serial.println(c(terror));
        msg=vacio;
        }
    Serial.println();
  }
  mact1=0; mact2=0; mact10=0; mact60=0; mact3600=0; mact86400=0;
  for (byte j=0;j<1;j++) for (byte i=0; i<8; i++) bevenENABLE[j][i]=255;
  if (WiFi.isConnected()) {
    if (conf.iftttenable) ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, "Reset", conf.myippub);
    if (conf.modomyjson==1) putmyjson();
    if (conf.mododweet==1) postdweet(mac);
    if (conf.iottweetenable==1) postIoTweet();
  }
  dPrint(c(runningt)); dPrint(crlf);
  printhora(); dPrint(crlf);
  PSclient.setServer(conf.mqttserver, 1883);
  PSclient.setCallback(mqttcallback);

  mySwitch.enableReceive(rx433);  // Receiver on interrupt 0 => that is pin #2,
//  mySwitch.enableTransmit(tx433);  // 15
  lcd.init();  lcd.backlight();
  lcdshowstatus();

  ////////////////////// ZONA DE PRUEBAS ////////////
  //  if (conf.tipoED[0]==1) dht0.setup(edPin[0]); // Connect DHT sensor to GPIO 4 (ED0)
  //  if (conf.tipoED[1]==1) dht1.setup(edPin[1]); // Connect DHT sensor to GPIO 5 (ED1)

  //  fauxmo.enable(true);
  //
  //// Fauxmo
  //    fauxmo.addDevice("relay");
  //    fauxmo.addDevice("relay 2");
  //    fauxmo.onSetState(callback);

// for (int i=0;i<150;i++)  {  Serial.print("i:");   Serial.print(i);   Serial.print("  t:");   Serial.println(t(i));   }
// for (int i=0;i<250;i++)  {  Serial.print("i:");   Serial.print(i);   Serial.print("  c:");   Serial.println(c(i));   }
}   // setup()

void mqttcallback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived ["); Serial.print(topic);  Serial.print("] ");
  for (int i=0; i<length; i++) { Serial.print((char)payload[i]); }  Serial.println();
//  MbR[0]=(int)payload;
// Switch on the LED if an 1 was received as first character
//  if ((char)payload[0] == '1') {
//    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
//    // but actually the LED is on; this is because
//    // it is acive low on the ESP-01)
//    } else {
//      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
//    }
}

boolean mqttreconnect() 
  { 
  if (PSclient.connect("conucoplus")) { PSclient.publish("conuco/temperatura","conectado"); }
  return PSclient.connected(); 
  }

void mqttpublishvalues()
{
  for (byte i=0;i<8;i++)
    {
    strcpy(auxdesc,conf.mqttpath[0]); strcat(auxdesc,"/");
    for (byte j=1;j<6;j++) { if (strlen(conf.mqttpath[j])>0) {  strcat(auxdesc,conf.mqttpath[j]); strcat(auxdesc,"/"); } }
    strcat(auxdesc,idpin[i]);
    if (i<=2) { strcpy(auxchar,itoa(MbR[i],buff,10));  }
    else if (i==3) { strcpy(auxchar,itoa(MbR[i],buff,10));  }
    else if (i<=5) { strcpy(auxchar,itoa(getbit8(conf.MbC8,i-2),buff,10));  }
    else if (i<=7) { strcpy(auxchar,itoa(getbit8(conf.MbC8,i-6),buff,10));  }
    PSclient.publish(auxdesc, auxchar);
    strcpy(auxdesc,"");strcpy(auxchar,"");
    }
}

void ICACHE_FLASH_ATTR procesaSemanal()
{
  if ((conf.actPrg[0]) || (conf.actPrg[1]))   // algún programa activo
    {
    for (byte i=0; i<maxPrgSem; i++)
      {
      if (((conf.actPrg[0]) && (getbit8(conf.bPRGsem[0], i))) ||
          ((conf.actPrg[1]) && (getbit8(conf.bPRGsem[1], i))))
        {
        if (conf.prgdia[i] != 0)                    // hay algún día marcado
          {
          if (getbit8(conf.prgdia[i], weekday()-1)==1) // coincide el día de la semana
            {
            if (conf.prghor[i] == hour())           // coincide la hora
              {
              if (conf.prgmin[i] == minute())       // coincide el minuto
                {
                if (conf.prgsal[i] <= 1)              // salida digital local
                  pinVAL(sdPin[conf.prgsal[i]], getbit8(conf.bprgval, i), conf.iddevice); // señal local
                else                              // escena: prgsal[i]-2
                  if (conf.prgsal[i] <= 3)           // escena
                    onescena(conf.prgsal[i] - 2);
                  else                         // salida remota
                    {
                    int auxerr = pinvalR(conf.idsalremote[conf.prgsal[i] - 4], 88, conf.pinremote[conf.prgsal[i] - 4], getbit8(conf.bprgval, i));
                    if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))
                      {
                      setbit8(bstatremote, conf.prgsal[i] - 4, getbit8(conf.bprgval, i) );
                      contaremote[conf.prgsal[i] - 4] = 0;
                      }
                    else
                      { Serial.print(c(tpinvalr)); Serial.println(auxerr); }
                    }
                }
              }
            }
          }
        }
      }
    }
  for (byte i=0; i<maxPrgSem; i++)
    if (conf.prgdia[i]!=0)                    // hay algún día marcado
      if (getbit8(conf.prgdia[i], weekday()-1)==1) // coincide el día de la semana
        if (conf.prghor[i]==hour())           // coincide la hora
          if (conf.prgmin[i]==minute())       // coincide el minuto
            if (conf.prgsal[i]>=20)         // activar/desactivar programa
              conf.actPrg[conf.prgsal[i]-20]=getbit8(conf.bprgval, i);
}

void ICACHE_FLASH_ATTR procesaFechas()
{
  for (byte i = 0; i < maxPrgFec; i++)
    if (getbit8(conf.bactfec, i))
      if (conf.fecmes[i] == month())
        if (conf.fecdia[i] == day())
          if (conf.fechor[i] == hour())
            if (conf.fecmin[i] == minute())      {
              if (conf.fecsal[i] <= 3)
                pinVAL(sdPin[conf.fecsal[i]], getbit8(conf.bfecval, i), conf.iddevice);
              else
                {
                int auxerr = pinvalR(conf.idsalremote[conf.fecsal[i] - 4], 88, conf.pinremote[conf.fecsal[i] - 4], getbit8(conf.bfecval, i));
                if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))  {
                  setbit8(bstatremote, conf.fecsal[i] - 4, getbit8(conf.bfecval, i));
                  contaremote[conf.fecsal[i] - 4] = 0;
                  }
                else  {
                  Serial.print(c(tpinvalr)); Serial.println(auxerr); }
                }
              }
}

void procesaeventos()
{
  for (byte i=0; i<nEVE; i++) // número de condición, de 0 a 7
    {
    if (((conf.actPrg[0]) && (getbit8(conf.bPRGeve[i], 0))) ||
        ((conf.actPrg[1]) && (getbit8(conf.bPRGeve[i], 1)))) // si algún programa activo en la condición
    {
      if (conf.condact[i] < 150)  // activadora entrada o salida digital (0-1)
      {
        boolean cumple=false;
        if (conf.condact[i] <= 1) cumple = (getbit8(conf.MbC8, conf.condact[i] + 2) == conf.condvalD[i]);
        //          if ((!modo45) && (conf.condact[i]<=1)) cumple=(getbit8(conf.MbC8,conf.condact[i]+2) == condvalD[i]);
        if ((conf.condact[i] >= 2) && (conf.condact[i] <= 3)) cumple = (valorpin[getbit8(conf.MbC8, conf.condact[i] - 2)] == conf.condvalD[i]);
        if (conf.condact[i] >= 100) cumple = getbit8(bstatremote, conf.condact[i] - 100);
        if (cumple)                   // si valor pin = valor en condición // CUMPLE
          {
          if (conf.evensal[i] <= 3)         // señal local o escena
            {
            pinVAL(sdPin[conf.evensal[i]], getbit8(conf.bevenniv, i), conf.iddevice);  // señal local
            }
          else if (conf.evensal[i] <= 200)  // señal remota
            {
            if (getbit8(conf.bconactmode, i) == 0) // modo ESTADO
              {
              int auxerr = pinvalR(conf.idsalremote[conf.evensal[i] - 4], 88, conf.pinremote[conf.evensal[i] - 4], getbit8(conf.bevenniv, i));
              if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))
                {
                setbit8(bstatremote, conf.evensal[i] - 4, getbit8(conf.bevenniv, i));
                contaremote[conf.evensal[i] - 4] = 0;
                }
              else
                { Serial.print(c(tpinvalr)); Serial.println(auxerr);   }
              }
            else                             // modo CAMBIO
              {
              if (getbit8(bconactcumple, i) == 0) // antes no cumplía
                pinVAL(sdPin[conf.evensal[i]], getbit8(conf.bevenniv, i), conf.iddevice); // señal local
              }
            }
          else if (conf.evensal[i] == despIFTTT) // mandar notificación
            {
            if (getbit8(bevenENABLE[0], i) == 1)
              {
              // ENVIAR NOTIFICACIÓN y SI NOTIFICACION ENVIADA
              setbit8(bevenENABLE[0], i, 0);
              }
            }
          setbit8(bconactcumple, i, 1);
          }  // fin CUMPLE
        else    // no CUMPLE
          {
          setbit8(bconactcumple, i, 0);
          }     // fin no CUMPLE
        }   // fin activadora digital Local
      else     // activadora analógica o sonda (>=150)
        {
        float vact;
        if (conf.condact[i] == 150) vact = float(MbR[3]); // entradas analógicas locales
        else if (conf.condact[i] < 180) vact = float(sondaremote[conf.condact[i] - 160]); // entradas analógicas remotas
        else if (conf.condact[i] < 200) vact = float(MbR[conf.condact[i] - 180] / 100);  // temperaturas locales
        else if (conf.condact[i] < 220) vact = float(sondaremote[conf.condact[i] - 200] / 100); // temperaturas remotas
        else if (conf.condact[i] < 250) vact = float(mbtemp[conf.condact[i] - 220] / 100); // temperaturas modbus
        //          else if (condact[i]==254)
        float tcomp = float(conf.evenvalA[i]);
        if (((conf.evencomp[i] == 0) && (vact >= tcomp)) || ((conf.evencomp[i] != 0) && (vact <= tcomp)))
          { // CUMPLE
          if (getbit8(conf.bconsaltipo, i) == 0) // señal de salida local
            {
            if (conf.evensal[i] <= 3)
              {
              pinVAL(sdPin[conf.evensal[i]], getbit8(conf.bevenniv, i), conf.iddevice);  // señal local
              }
            else if (conf.evensal[i] <= 200)
              {
              int auxerr = pinvalR(conf.idsalremote[conf.evensal[i] - 4], 88, conf.pinremote[conf.evensal[i] - 4], getbit8(conf.bevenniv, i));
              if ((auxerr == 200) || (auxerr == 303) || (auxerr == (-11)))
                {
                setbit8(bstatremote, conf.evensal[i] - 4, getbit8(conf.bevenniv, i));
                contaremote[conf.evensal[i] - 4] = 0;
                }
              else
                { Serial.print(c(tpinvalr)); Serial.println(auxerr); }
              }
            else if (conf.evensal[i] == despIFTTT)
              {
              if (getbit8(bevenENABLE[0], i) == 1)
                {
                // enviar notif y si notif enviada
                setbit8(bevenENABLE[0], i, 0);
                }
             }
            }
          setbit8(bconactcumple, i, 1);
          }
        else
          { // NO CUMPLE
          setbit8(bconactcumple, i, 0);
          }
        }
    }
    }
}

void ICACHE_FLASH_ATTR procesaTimeMax() // procesa tiempos máximos de ON/OFF de las salidas
{
  for (byte i=0;i<2;i++)
    if (getbit8(conf.MbC8,i)==valorpin[1]) // está ON
      if (conf.tempdefact[i] != 0) if (((millis()/1000)-tempact[i])>=conf.tempdefact[i]) pinVAL(sdPin[i], 0, conf.iddevice);
    else
      if (conf.tempdefdes[i]!=0) if (((millis()/1000)-tempdes[i])>=conf.tempdefdes[i]) pinVAL(sdPin[i], 1, conf.iddevice);
}

void ICACHE_FLASH_ATTR leevaloresMB()
{
  for (byte i = 0; i < maxsalrem; i++)
  {
    if ((conf.idsalremote[i] > 0) && (conf.idsalremote[i] <= 32))
    {
      static uint32_t k;   k++;
      MBnode.setTransmitBuffer(0, lowWord(k));  MBnode.setTransmitBuffer(1, highWord(k));
      uint8_t auxerr2 = MBnode.readHoldingRegisters(315, 1);
      Serial.print(F("Reg 315:")); Serial.print(auxerr2, 16);
      if (auxerr2 == MBnode.ku8MBSuccess)  {
        mbtemp[i] = ((MBnode.getResponseBuffer(0) * 100) - 8000) / 2;
      }

      auxerr2 = MBnode.readHoldingRegisters(317, 1);
      Serial.print(F("  Reg 317:")); Serial.print(auxerr2, 16);
      //        auxerr2=MBnode.readHoldingRegisters(14, 1);   // en modo manual
      if (auxerr2 == MBnode.ku8MBSuccess) mbcons[i]=((MBnode.getResponseBuffer(0) * 100) - 2000) / 2;
      delay(1);
      auxerr2=MBnode.readCoils(5, 1);
      Serial.print(F("  coil 1:")); Serial.println(auxerr2, 16);
      if (auxerr2 == MBnode.ku8MBSuccess)  {
        setbit8(mbstatus, i, MBnode.getResponseBuffer(0));
      }
    }
  }
}

void ICACHE_FLASH_ATTR procesaRF()
{
  byte i=0;
  boolean encontrado=false;
  while ((i<6) && (!encontrado))
    {
    if (lastcode==conf.rfkeys.code[i])
      {
      encontrado = true;
      lcd.init();
      lcd.clear();
      if (i==0) { if (paract==1) { saveconf(); pendsave=0; } }    // Intro
      else if (i==1) { paract=0; }                                        // Esc
      else if (i==2) { paract++; if (paract > maxparam) paract=1; }       // Up
      else if (i==3) { paract--; if (paract < 1) paract = maxparam; }     // Down
      else if (i==4) { if (paract==1) { // Left
          conf.iddevice--; conf.EEip[3] = conf.iddevice;
          strcpy(conf.ssidAP, c(CONUCO)); strcat(conf.ssidAP, subray); strcat(conf.ssidAP, itoa(conf.iddevice, buff, 10));
          pendsave = true;       }  }
      else if (i==5) { if (paract==1) {   // Rigth
          conf.iddevice++; conf.EEip[3] = conf.iddevice;
          strcpy(conf.ssidAP, c(CONUCO)); strcat(conf.ssidAP, subray); strcat(conf.ssidAP, itoa(conf.iddevice, buff, 10));
          pendsave = true;  }  }
      lcdshowconf(true);
      return;
      }
    i++;
    }

  i=0;
  encontrado=false;
  while ((i<18) && (!encontrado))
    {
    if (lastcode==conf.code433.codeon[i])   {
      encontrado=true;
      if (i<=1) {
        pinVAL (sdPin[i], 1, conf.iddevice);   }
      else    {
        int auxerr=pinvalR(conf.idsalremote[i-2], 88, conf.senalrem[i-2]-6, 1);
        if ((auxerr==200) || (auxerr==303) || (auxerr == (-11))) setbit8(bstatremote,i-2,1);
        }
      }
    if (lastcode==conf.code433.codeoff[i])    {
      encontrado=true;
      if (i<=1) {
        pinVAL (sdPin[i], 0, conf.iddevice);   }
      else    {
        int auxerr = pinvalR(conf.idsalremote[i-2], 88, conf.senalrem[i-2]-6, 0);
        if ((auxerr==200) || (auxerr==303) || (auxerr==(-11))) setbit8(bstatremote,i-2,0);
        }
      }
    i++;
    }
}

void handleRF()
{
  if (mySwitch.available())    {
    int value=mySwitch.getReceivedValue();
    Serial.print("rf:"); Serial.println(value);
    if (value!=0)
      {
      lastpro=mySwitch.getReceivedProtocol();
      lastcode=mySwitch.getReceivedValue();
      lastlen=mySwitch.getReceivedBitlength();
      if (lastcode!=0) procesaRF();
      }
    mySwitch.resetAvailable();
    }
}

void ICACHE_FLASH_ATTR loop(void)
{
  handleRF();
  unsigned long tini = millis();
  //////////////////////////////////////////////////  
  if (conf.ftpenable) ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!  
  server.handleClient();    // atiende peticiones http

  if (conf.mqttenable) 
    {
    if (!PSclient.connected())
      {
      long tnow = millis();
      if (tnow-lastReconnectAttempt>5000) {
        lastReconnectAttempt=tnow;
        if (mqttreconnect()) lastReconnectAttempt=0;    // Attempt to reconnect
        }
      } 
    else 
      PSclient.loop();
    }
  //  fauxmo.handle();
  ////////////////////////////////////////////////////  

  if ((millis() - tini) > 5000) { Serial.print(c(handleclientt)); Serial.print(dp); Serial.println(millis()-tini); }
  leevaloresDIG();
  if ((millis() > (mact1 + 1000)))    // tareas que se hacen cada segundo
    {
    tini=millis();
    countfaulttime++;   // si se hace mayor que TempDesactPrg,desactiva ejecucion programas dependientes de fecha
    leevaloresAN();
    while (digitalRead(bt2Pin) == 0)  // pulsado botón 2, reset fábrica 20 seg, Reset Wifi: 10 seg.
      {
      tempbt2++;
      if ((tempbt2==10) || (tempbt2==20)) tictac(ledSt, 5, 100);
      delay(500);
      }
    if (tempbt2>=20) { initFab(); ESP.reset(); } // Reset Fábrica
    else if (tempbt2>=10) { initWiFi(); ESP.reset(); } // mod Reset Wifi
    tempbt2=0;
    if (countfaulttime < conf.TempDesactPrg) procesaeventos();
    procesaTimeMax();
    for (byte j=0; j<maxsalrem; j++)
      if (conf.idsalremote[j] > 0)
        if ((conf.senalrem[j]==6) || (conf.senalrem[j]==7)) contaremote[j]++;
    if (WiFi.isConnected())
      {
      if (statusChange) {
        lcdshowstatus();
        if (conf.modomyjson==1) putmyjson();
        if (conf.mododweet==1) postdweet(mac);
        if (conf.iottweetenable==1) postIoTweet();
        if (mododirecto==1) actualizamasters();
        }
      if (iftttchange[0]>0)
        {
        if (getbit8(iftttchange,0)==1)
          {
          if ((getbit8(conf.iftttpinSD, 0) == 1) && (getbit8(conf.MbC8, 0) == 1))
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 6, 20), textonoff(1));
          if ((getbit8(conf.iftttpinSD, 8) == 1) && (getbit8(conf.MbC8, 0) == 0))
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 6, 20), textonoff(0));
          }
        if (getbit8(iftttchange, 1) == 1)
          {
          if ((getbit8(conf.iftttpinSD, 1) == 1) && (getbit8(conf.MbC8, 1) == 1))
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 7, 20), textonoff(1));
          if ((getbit8(conf.iftttpinSD, 9) == 1) && (getbit8(conf.MbC8, 1) == 0))
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 7, 20), textonoff(0));
          } 
        if (getbit8(iftttchange, 1) == 1) if (getbit8(conf.iftttpinSD, 1) == 1) {
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 7, 20), textonoff(getbit8(conf.MbC8, 1)));
          }
        if (getbit8(iftttchange, 2) == 1) if (getbit8(conf.iftttpinED, 0) == 1) {
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 4, 20), textonoff(getbit8(conf.MbC8, 2)));
          }
        if (getbit8(iftttchange, 3) == 1) if (getbit8(conf.iftttpinED, 1) == 1) {
            ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, readdescr(filedesclocal, 5, 20), textonoff(getbit8(conf.MbC8, 3)));
          }
        iftttchange[0] = 0;
        }
      }
    else  // no conectado
      {
      //      if ((wifimode==1) || (wifimode==2))    // AP o AP+STA
      //        tictac(ledSt,4,50);
      //      else if (wifimode==0)                 // modo STA
      //        {
      //        tictac(ledSt,2,50);         // alarma no conectado
      //        wifimode=12;
      //        WiFi.mode(WIFI_AP_STA);;    // poner en modo AP+STA
      //        if (wifimode==12) ESP.reset();
      //        }
      }
    //////////////////////
    //    if (modo45==1)
    //      {
    //      Serial.print("Temperature = ");  Serial.print(bmp085.readTemperature());   Serial.println(" *C");
    //      Serial.print("Pressure = ");     Serial.print(bmp085.readPressure());      Serial.println(" Pa");
    //      // Calculate altitude assuming 'standard' barometric pressure of 1013.25 millibar = 101325 Pascal
    //      Serial.print("Altitude = ");  Serial.print(bmp085.readAltitude());   Serial.println(" meters");
    //      Serial.print("Pressure at sealevel (calculated) = ");  Serial.print(bmp085.readSealevelPressure());  Serial.println(" Pa");
    //    // you can get a more precise measurement of altitude if you know the current sea level pressure which will
    //    // vary with weather and such. If it is 1015 millibars that is equal to 101500 Pascals.
    //      Serial.print("Real altitude = ");  Serial.print(bmp085.readAltitude(101500));   Serial.println(" meters");
    //      }
    ////////////////////////
    //    if ((millis()-tini)>5000) {Serial.print(F("1 SEG:"));  Serial.println(millis()-tini);}
    mact1 = millis();
    }

  if ((millis() > (mact10 + (conf.peractrem * 1000))))  // tareas que se hacen cada "peractrem" segundos
    {
    tini = millis();
//////////////////////////////////////////////
    if (conf.mqttenable) {  mqttpublishvalues();    }
/////////////////////////////////////////////   
//    if (!pendsave) lcdshowstatus();
    lastpro = 0; lastcode = 0; lastlen = 0; // pone a cero el último código 433
    leevaloresOW();
    if ((conf.tipoED[0] == 1) || (conf.tipoED[1] == 1)) leevaloresDHT();
    if (conf.modo45 == 2) leevaloresMB();
    if (WiFi.isConnected()) {
      unsigned long tini = millis();
      if (conf.modomyjson == 1) { putmyjson(); }
      if (conf.mododweet == 1) { postdweet(mac); }
      if (conf.iottweetenable == 1) { postIoTweet(); }
      if (mododirecto == 1) { actualizamasters(); }
      actualizaremotos(mododirecto);
    }
    //    if ((millis()-tini)>5000) {printhora(); Serial.print(F(" 10 SEG:")); Serial.println(millis()-tini);}
    mact10 = millis();
  }

  if ((millis() > (mact60 + 60000)))    // tareas que se hacen cada 60 segundos:1 minuto
  {
    tini = millis();
    if (countfaulttime < conf.TempDesactPrg)      {
      procesaSemanal();
      procesaFechas();
      }
    else
      {
      timeClient.setTimeOffset(7200);
      if (timeClient.update() == 1) {
        countfaulttime = 0;
        setTime(timeClient.getEpochTime());
        }
      }
    if ((millis() - tini) > 5000) { Serial.print(60); Serial.print(F(" SEG:")); Serial.println(millis() - tini); }
    mact60 = millis();
  }
  if ((millis() > (mact3600 + 3600000)))    // tareas que se hacen cada 3600 segundos:1 hora
  {
    tini = millis();
    if (WiFi.isConnected()) {
      timeClient.setTimeOffset(7200);
      if (timeClient.update() == 1) {
        countfaulttime = 0;
        setTime(timeClient.getEpochTime());
      }
      getMyIP();
      checkMyIP();
      checkForUpdates();
    }
    if (millis() - tini > 1000) { Serial.print(3600); Serial.print(F(" seg.:")); Serial.println(millis() - tini); }
    mact3600 = millis();
  }

  if ((millis() > (mact86400 + 86400000)))    // tareas que se hacen cada 86400 segundos: 1 día
  {
    tini = millis();
    if (WiFi.isConnected()) {
      if (conf.iftttenable) ifttttrigger(conucochar, conf.iftttkey, conf.aliasdevice, "MyIP", conf.myippub);
    }
    //    if (millis()-tini>1000) {Serial.print(F("1 día.:")); Serial.println(millis()-tini);}
    mact86400 = millis();
  }

  //////////////////////////  ZONA DE PRUEBAS //////////////////////
  //////////////////////////  FIN ZONA DE PRUEBAS //////////////////////
}


