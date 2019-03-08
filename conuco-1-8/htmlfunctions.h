

void ICACHE_FLASH_ATTR dPrint(String texto) { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrint(char* texto) { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrint(PGM_P texto)  { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrintI(int valor)   { if (debug) Serial.print(valor); }

void ICACHE_FLASH_ATTR printColspan(int ancho)
  { printP(c(tdcolspan_i)); printI(ancho); printP(comillas, mayor);}
  
void ICACHE_FLASH_ATTR cell(PGM_P texto) { printP(td,texto,td_f); }
void ICACHE_FLASH_ATTR cell(int num) { printPiP(td,num,td_f); }
void ICACHE_FLASH_ATTR cell(float num, byte deci) { printP(td); printF(num,deci); printP(td_f); }

void ICACHE_FLASH_ATTR ccell(int ntexto) { printP(td,c(ntexto),td_f); }
void ICACHE_FLASH_ATTR tcell(int ntexto) { printP(td,t(ntexto),td_f); }

void ICACHE_FLASH_ATTR pt(int pos)
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
  printP(auxdesc);
}


void ICACHE_FLASH_ATTR pc(int pos)
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
  printP(auxdesc);
}

void ICACHE_FLASH_ATTR espacioSep(int col)  //  espacio separación
  { printP(tr); printColspan(col);printP(td_f,tr_f);  }

void printfincampo() { printP(mayor, menor, barra, c(tinput), mayor); }
void printdisabled() { printP(c(ttext)); printP(comillas,b,c(disabled)); printP(ig,comillas, c(disabled)); }
void printselected(boolean check) { printP(b, check?checked:selected); }

void ICACHE_FLASH_ATTR printcampoI(int numpar, int valactual, byte tam, boolean enabled,boolean printtd)
{
  if (printtd) printP(td);
  printP(menor, c(tinput), b, type, ig, comillas);  
  if (!enabled) printdisabled();
  printP(c(ttext), comillas, b);
  printP(c(namet), ig,comillas);
  printI(numpar);
  printP(comillas,b, tvalue, ig,comillas);
  printI(valactual);
  printP(comillas,b,c(max_length));
  printIP(tam, size_i);
  printIP(tam,comillas);
  printfincampo();
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR printcampoL(int numpar, long valactual, byte tam, boolean enabled,boolean printtd)
{
  if (printtd) printP(td);
  printP(menor, c(tinput), b, type, ig, comillas);
  if (!enabled) printdisabled();
  printP(c(ttext), comillas, b);
  printP(c(namet), ig,comillas);
  printI(numpar);
  printP(comillas,b, tvalue, ig,comillas);
  printI(valactual);
  printP(comillas,b,c(max_length));
  printIP(tam, size_i);
  printIP(tam, comillas);
  printfincampo();
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR printcampoF(int numpar, float valactual, int deci)
{
  printP(menor, c(tinput), b, type, ig, comillas);
  printP(c(ttext), comillas, b);
  printP(c(namet), ig,comillas);
  printI(numpar);
  printP(comillas,b, tvalue, ig,comillas);
  printF(valactual, deci);
  printP(comillas,b,c(max_length));
  printIP(10,size_i);
  printIP(10,comillas);
  printfincampo();
}

void ICACHE_FLASH_ATTR printcampoC(int numpar, char *valactual, byte tam, boolean visible, boolean enabled, boolean pass,boolean printtd)
{
  if (printtd) printP(td);
  printP(menor, c(tinput), b, type, ig, comillas);
  if (visible)  {
    printP(pass?c(tpassword):c(ttext));
    if ((!enabled) && (!pass)) { printP(comillas, b, c(disabled)); printP(ig, comillas, c(disabled)); }
    printP(comillas);   }
  else
    {
    printP(c(ttext), comillas, b);
    printP(c(invis), b);  }
  printP(b, c(namet), ig );
  printPiP(comillas,numpar, comillas);
  printP(b, tvalue, ig, comillas, valactual);
  printP(comillas,b, c(max_length));
  printIP(tam-1, size_i);
  printIP(tam-1, comillas);
  printfincampo();
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR printparCP(const char *titulo, int numpar, char valactual[], byte tam, boolean pass)
{
  printP(tr);
  cell(titulo);
  printcampoC(numpar, valactual, tam, true, true, pass,true);
  printP(tr_f);
}

void ICACHE_FLASH_ATTR printcampoSiNo(int numpar, int valactual)
  {
  printP(c(Select_name), comillas);
  printIP(numpar, comillas);
  printP(mayor, c(optionvalue),comillas, cero, comillas);
  if (valactual==0) printselected(false);
  printP(mayor, OFF, c(option_f));
  printP(c(optionvalue));
  printPiP(comillas, 1, comillas);
  if (valactual) printselected(false);
  printP(mayor, ON, c(option_f));
  printP(c(select_f));
  }

void ICACHE_FLASH_ATTR printparentesis(PGM_P letra, int numero)
{
  printP(paren_i,letra);
  printI(numero);
  printP(paren_f, b, b);
}

void ICACHE_FLASH_ATTR checkBox(int numpar, bool selected, boolean printtd)
{
  if (printtd) printP(td);
  printP(menor, c(tinput), b, type, ig, comillas);
  printP(c(checkbox), comillas, b);
  printP(c(namet), ig,comillas);
  printI(numpar);
  printP(comillas,b, tvalue, ig,comillas);
  printP(uno,comillas);
  if (selected) printselected(true);
  printfincampo();
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR writeHeader(boolean refreshmode, boolean ajaxmode)
{
  printP(menor, thtml);
  printP(b, c(xmlns), mayor);
  printP(menor, thead, mayor, menor, c(title), mayor);
  printP(c(conuco));
  printP(c(web));
  printP(c(tserver), menor, barra);
  printP(c(title));
  printP(mayor,htmlHead_3);
  if (ajaxmode)
    printP(ajaxscript);
  else
    if (refreshmode)
      if (conf.peractpan>0) printPiP(c(htmlRefresh_i), conf.peractpan, comillascierre);
  printP(c(head_f));
}

void printinicampoCB(int numpar)
{
  printP(c(Select_name), comillas);
  printIP(numpar, comillas);
  printP(mayor);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, int pri, int ult, boolean printtd)
{
  if (printtd) printP(td);
  printinicampoCB(numpar);
  for (byte j=pri; j<=ult; j++)   {
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (valact==j) printselected(false);
    printPiP(mayor, j, c(option_f));   }
  printP(c(select_f));
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, byte lon, PGM_P t[], boolean printtd)
{
  if (printtd) printP(td);
  printinicampoCB(numpar);
  for (byte j=0;j<lon;j++)   {
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (valact==j) printselected(false);
    printP(mayor,t[j]);
    printP(c(option_f));   }
  printP(c(select_f));
  if (printtd) printP(td_f);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, boolean printtd)
{
  PGM_P t[]={t0,t1};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t,printtd);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, boolean printtd)
{
  PGM_P t[]={t0,t1,t2};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t,printtd);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3, boolean printtd)
{
  PGM_P t[]={t0,t1,t2,t3};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t,printtd);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3, PGM_P t4, boolean printtd)
{
  PGM_P t[]={t0,t1,t2,t3,t4};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t,printtd);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3, PGM_P t4, PGM_P t5, boolean printtd)
{
  PGM_P t[]={t0,t1,t2,t3,t4,t5};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t,printtd);
}

void ICACHE_FLASH_ATTR tituloFila(PGM_P texto, int num, PGM_P letra, int indice)
{
  printP(td, paren_i);
  printPiP(letra, indice, paren_f);
  printP(b, b, texto);
  printPiP(b, num, td_f);
}


void printTime()
{
  printI(day()); printPiP(barra, month(), barra); printIP(year(),b);
  if (hour()<10) printP(cero); printI(hour()); printP(dp);
  if (minute()<10) printP(cero); printI(minute()); printP(dp);
  if (second()<10) printP(cero); printI(second());
}

void ICACHE_FLASH_ATTR HtmlGetStateTime()
{
  printColspan(2);
  printTime();
  printP(b, c(PRG), b, (countfaulttime < conf.TempDesactPrg) ? ON : OFF, b);
  printI(ESP.getFreeHeap());
  printP(td_f);
}

char* ICACHE_FLASH_ATTR textonoff(float valor)
{ if (valor==1) return "ON"; else return "OFF";  }

void ICACHE_FLASH_ATTR writeForm(PGM_P phtm)
{
  printP(c(form_action), phtm);
  printP(comillas,b);
  printP(c(Form_post), menor);
  printP(table, b);
  printP(c(tclass), ig, tnormal, mayor);
}

void ICACHE_FLASH_ATTR writeFooter(PGM_P texto, boolean cerrar)
{
  printP(menor, barra, table, mayor);               // final <table>
  printP(menor, c(tinput), b, type, ig, comillas);
  printP(submit);                                   // submit
  printP(comillas, b, tvalue, ig, comillas);      // " value="texto
  printP(texto);                                    
  if (cerrar) printP(comillas, b, c(onclickclose));
  printP(comillas);
  printfincampo();
  printP(c(form_f));
  printP(c(body_f), menor, barra);
  printP(thtml, mayor);
}

void ICACHE_FLASH_ATTR setCookie(byte valor)
{
  msg=vacio;
  printP(c(HTTP11), b);
  printP(c(t301), b, ok, crlf);
  printP(c(setcookie),dp, b);
  printP(c(espsessionid), ig, valor==0?cero:uno);
  printP(crlf,c(location), dp, b, barra, crlf);
  printP(c(cachecontrol),dp, b);
  printP(c(nocache), crlf, crlf);
  server.sendContent(msg);
  msg=vacio;
}

void ICACHE_FLASH_ATTR sendOther(const char *otherURL, int param)
{
  msg=vacio;
  printP(c(HTTP11),b);
  printP(c(t303),b);
  printP(c(seeother),crlf);
  printP(c(location),dp,b,otherURL);
  if (param>=0) { printP(paramn);printI(param);}
  printP(crlf,crlf);
  server.sendContent(msg);
  msg=vacio;
}

void ICACHE_FLASH_ATTR printDiaSem(byte i)
  {
  if (i==0) printP(letraD);  
  if (i==1) printP(letraL);  
  if (i==2) printP(letraM);  
  if (i==3) printP(letraX);  
  if (i==4) printP(letraJ);  
  if (i==5) printP(letraV);  
  if (i==6) printP(letraS);  
  }

