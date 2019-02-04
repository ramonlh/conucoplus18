#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i=0; i<sizeof(value); i++) EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p=(byte*)(void*)&value;
    unsigned int i;
    for (i=0; i<sizeof(value); i++) *p++=EEPROM.read(ee++);
    return i;
}

boolean strcharcomp() { msg.toLowerCase(); msg.toCharArray(auxdesc, msg.length()+1); return (strcmp(auxchar,auxdesc)==0); }

char* ICACHE_FLASH_ATTR readdescr(char* namefile, byte ind, byte len)
{
  File auxfile=SPIFFS.open(namefile,letrar);
  if (auxfile)
    {
    auxfile.seek(len*(ind), SeekSet);
    auxfile.readBytes(auxdesc,len);
    auxfile.close();
    auxdesc[len-1]='\0';
    byte n=strlen(auxdesc);
    while ((n>0) && ((auxdesc[n-1]==' ')||(auxdesc[n-1]=='\n')||(auxdesc[n-1]=='\r'))) n--;
    auxdesc[n]='\0';
    }
  return auxdesc;
}

char* ICACHE_FLASH_ATTR t(int pos)
{
  char auxlang[20]="";
  if (conf.lang==0) strcpy(auxlang,filespanish);
  if (conf.lang==1) strcpy(auxlang,fileenglish);
  File auxfile=SPIFFS.open(auxlang,letrar);
  if (auxfile)
    {
    auxfile.seek(42*(pos-1), SeekSet);
    auxfile.readBytes(auxdesc,42);
    auxfile.close();
    auxdesc[41]='\0';
    byte n=strlen(auxdesc);
    while ((n>0) && ((auxdesc[n-1]==' ')||(auxdesc[n-1]=='\n')||(auxdesc[n-1]=='\r'))) n--;
    auxdesc[n]='\0';
    }
  return auxdesc;
}

char* ICACHE_FLASH_ATTR c(int pos)
{
  File auxfile=SPIFFS.open(filecommon,letrar);
  if (auxfile)
    {
    auxfile.seek(42*(pos-1), SeekSet);
    auxfile.readBytes(auxdesc,42);
    auxfile.close();
    auxdesc[41]='\0';
    byte n=strlen(auxdesc);
    while ((n>0) && ((auxdesc[n-1]==' ')||(auxdesc[n-1]=='\n')||(auxdesc[n-1]=='\r'))) n--;
    auxdesc[n]='\0';
    }
  return auxdesc;
}

void ICACHE_FLASH_ATTR savedescr(char* namefile, char* descr, byte ind, byte len)
{
  File auxfile=SPIFFS.open(namefile, rmas);
  if (!auxfile) return;
  if (auxfile.seek(len*(ind), SeekSet)) 
    {
    for (byte i=strlen(descr);i<len-1;i++) descr[i]=' ';  
    descr[len-1]='\0';
    auxfile.print(descr);
    }
  auxfile.close();   
}

void ICACHE_FLASH_ATTR addlog(byte tipo, int code, char *texto)
{
//  File auxfile=SPIFFS.open("/log.txt",amas);
//  if (auxfile)  
//    { 
//    if (hour()<10) auxfile.print(cero); auxfile.print(hour()); auxfile.print(dp);
//    if (minute()<10) auxfile.print(cero); auxfile.print(minute()); auxfile.print(dp);
//    if (second()<10) auxfile.print(cero); auxfile.print(second()); auxfile.print(b);
//    auxfile.print(code); auxfile.print(b);
//    auxfile.print(texto);
//    auxfile.println(crlf); 
//    auxfile.close();   }
}

void ICACHE_FLASH_ATTR addlog(byte tipo, int code, PGM_P texto)
{
//  File auxfile=SPIFFS.open("/log.txt",amas);
//  if (auxfile)  
//    { 
//    if (hour()<10) auxfile.print(cero); auxfile.print(hour()); auxfile.print(dp);
//    if (minute()<10) auxfile.print(cero); auxfile.print(minute()); auxfile.print(dp);
//    if (second()<10) auxfile.print(cero); auxfile.print(second()); auxfile.print(b);
//    auxfile.print(code); auxfile.print(b);
//    auxfile.println(texto); 
//    auxfile.close();   }
}

byte ICACHE_FLASH_ATTR getbit8(byte tabla[], byte pin)
  {  return ((tabla[pin/8] & tab[(pin % 8)])>0)?1:0;}

void ICACHE_FLASH_ATTR setbit8(byte tabla[], byte pin, byte value)  {
  tabla[pin/8]=((value==0)?tabla[pin/8]&(255^tab[(pin%8)]):tabla[pin/8]|tab[(pin%8)]);}

String ICACHE_FLASH_ATTR extrae(boolean eschar, String cad, String subcad)
  { subcad.concat("\"");
    return cad.substring(cad.indexOf(subcad)+subcad.length()+(eschar?2:1), cad.indexOf(",",cad.indexOf(subcad))-(eschar?1:0));      }

char* ICACHE_FLASH_ATTR ftoa(int valor)
{  
  char buff1[10],buff2[10];
  itoa(valor/100,buff1,10);
  itoa(valor%100,buff2,10);
  strcat(buff1,".");
  strcat(buff1,buff2);
  return buff1;
}  

void ICACHE_FLASH_ATTR tictac(int pin, int n, int delayed)
  { for (int i=0;i<n;i++) 
    {digitalWrite(pin,1); delay(delayed); digitalWrite(pin, 0); delay(delayed);} }

void ICACHE_FLASH_ATTR printconceros(int value)  { Serial.print(value<10?0:value); }

void ICACHE_FLASH_ATTR printhora() {
  printconceros(hour()); Serial.print(dp);
  printconceros(minute()); Serial.print(dp);
  printconceros(second());  
  }

boolean ICACHE_FLASH_ATTR clientremote(){
  return ((server.remoteIP[0]!=192) && (server.remoteIP[0]!=0)); }

void ICACHE_FLASH_ATTR printremote() {
  if (!clientremote()) return;
  printhora();
  for (byte i=0;i<3;i++) {Serial.print(server.remoteIP[i]);Serial.print(punto);}
  Serial.println(server.remoteIP[3]);
  }

int ICACHE_FLASH_ATTR posrem(int numrem)  {
  int p=0;
  boolean encontrado=false;
  while ((p<maxsalrem) && (!encontrado)) { encontrado=(conf.idremote[p++]==numrem); }
  return encontrado?p:0;
  }

int ICACHE_FLASH_ATTR readconf()
{
  File auxfile=SPIFFS.open(fileconf,letrar);
  if (!auxfile) return 0;
  int count=0;
  for (count=0;count<sizeof(conf);count++)*(buffconf+count)=auxfile.read();
  auxfile.close();
  return count;
}

void ICACHE_FLASH_ATTR saveconf()
{
  File auxfile=SPIFFS.open(fileconf, rmas);
  if (auxfile) { auxfile.write(buffconf,sizeof(conf)); auxfile.close();  }
}

void ICACHE_FLASH_ATTR printlinea(PGM_P texto) { for (byte i=0;i<20;i++) Serial.print(texto);Serial.println(); }

void ICACHE_FLASH_ATTR createhost(char *hostraiz, byte seg, byte ip)
{
  strcpy(host,hostraiz); 
  strcat(host,itoa(conf.netseg, buff, 10)); 
  strcat(host,punto); 
  strcat(host, itoa(ip, buff, 10));
}

void calcindices(int i)
{
  param_number=server.argName(i).toInt();
  if (mp<=0) mp=1;
  indice=param_number/mp;
  resto=param_number%mp;
  mival=server.arg(i).toInt();
}

int callhttpGET(char *host, int port, boolean readresp, unsigned long timeout)
{
  HTTPClient http;
  http.begin(host,port,msg);
  http.setTimeout(timeout);
  int httpCode=http.GET();
  msg="";
  if (readresp) if(httpCode==HTTP_CODE_OK) { msg=http.getString(); }
  http.end();
  return httpCode;
}

boolean checkfile(char* namefile)
{  if (!SPIFFS.exists(namefile)) { Serial.print(namefile); Serial.println(" no existe"); return false; }  return true; }

boolean checkfiles()
{
  boolean auxB=true;
  auxB=auxB && checkfile(fileconf); 
  auxB=auxB && checkfile(filezonas);
  auxB=auxB && checkfile(filedevrem);
  auxB=auxB && checkfile(filesalrem); 
  auxB=auxB && checkfile(filewebcall); 
  auxB=auxB && checkfile(fileurlwebcall); 
  auxB=auxB && checkfile(filedescprg); 
  auxB=auxB && checkfile(filedescesc); 
  auxB=auxB && checkfile(filemacdevrem); 
  auxB=auxB && checkfile(fileidmyjsonrem); 
  auxB=auxB && checkfile(fileunitsalrem); 
  auxB=auxB && checkfile(fileurl); 
  auxB=auxB && checkfile(filedesclocal); 
  auxB=auxB && checkfile(filedesctemp); 
  auxB=auxB && checkfile(filei2ctypes); 
  auxB=auxB && checkfile(filecommon); 
  auxB=auxB && checkfile(filespanish); 
  auxB=auxB && checkfile(fileenglish); 
  filesok=auxB;
  return auxB;
}

