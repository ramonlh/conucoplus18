

void ICACHE_FLASH_ATTR dPrint(String texto) { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrint(char* texto) { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrint(PGM_P texto)  { if (debug) Serial.print(texto); }
void ICACHE_FLASH_ATTR dPrintI(int valor)   { if (debug) Serial.print(valor); }

void ICACHE_FLASH_ATTR printP(PGM_P texto1) { char c;  while ((c = pgm_read_byte(texto1++))) msg += c; }
void ICACHE_FLASH_ATTR printP(PGM_P texto1, PGM_P texto2) { printP(texto1); printP(texto2);}
void ICACHE_FLASH_ATTR printP(PGM_P texto1, PGM_P texto2, PGM_P texto3) { printP(texto1, texto2); printP(texto3);}
void ICACHE_FLASH_ATTR printP(PGM_P texto1, PGM_P texto2, PGM_P texto3, PGM_P texto4)
  { printP(texto1,texto2);  printP(texto3,texto4); }
void ICACHE_FLASH_ATTR printP(PGM_P texto1, PGM_P texto2, PGM_P texto3, PGM_P texto4, PGM_P texto5)
  { printP(texto1, texto2, texto3, texto4);  printP(texto5); }
void ICACHE_FLASH_ATTR printP(PGM_P texto1, PGM_P texto2, PGM_P texto3, PGM_P texto4, PGM_P texto5, PGM_P texto6)
  { printP(texto1, texto2, texto3, texto4, texto5); printP(texto6); }

void ICACHE_FLASH_ATTR printI(int value)  { printP(itoa(value, buff, 10));  }
void ICACHE_FLASH_ATTR printH(int value)  { printP(itoa(value, buff, 16));  }
void ICACHE_FLASH_ATTR printL(long value)  { printP(ltoa(value, buff, 10));  }

void ICACHE_FLASH_ATTR printF(float value, byte deci) {
  float pdec=value-int(value);
  printI(int(value)); if (deci>0) printP(punto);  
  for (byte i=0;i<deci;i++) if (int(pdec*pow(10,i+1))%10==0) printP(cero); else printI(abs(int(pdec*pow(10,i+1))%10));
  }
  
void ICACHE_FLASH_ATTR printIP(long valor, const  char *texto) { printI(valor); printP(texto); }

void ICACHE_FLASH_ATTR printPiP(const char *texto1, int num, const char *texto2)
  { printP(texto1); printI(num); printP(texto2);}

void ICACHE_FLASH_ATTR printColspan(int ancho)
  { printP(c(tdcolspan_i)); printI(ancho); printP(comillas, mayor);}
  
void ICACHE_FLASH_ATTR espacioSep(int col)  //  espacio separación
  { printP(tr); printColspan(col);printP(td_f,tr_f);  }

//void printinicampo() { printP(menor, c(tinput), b, type, ig, comillas); }
void printfincampo() { printP(mayor, menor, barra, c(tinput), mayor); }
void printdisabled() { printP(c(ttext)); printP(comillas,b,c(disabled)); printP(ig,comillas, c(disabled)); }
void printselected(boolean check) { printP(b, check?checked:selected); }

void ICACHE_FLASH_ATTR printcampoI(int numpar, int valactual, byte tam, boolean enabled)
{
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
}

void ICACHE_FLASH_ATTR printcampoL(int numpar, long valactual, byte tam, boolean enabled)
{
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

void ICACHE_FLASH_ATTR printcampoC(int numpar, char *valactual, byte tam, boolean visible, boolean enabled, boolean pass)
{
  printP(menor, c(tinput), b, type, ig, comillas);
  if (visible)  {
    printP(pass?c(password):c(ttext));
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
  printIP(tam, size_i);
  printIP(tam, comillas);
  printfincampo();
}

void ICACHE_FLASH_ATTR printparCP(const char *titulo, int numpar, char valactual[], byte tam, boolean pass)
{
  printP(td, titulo, td_f, td);
  printcampoC(numpar, valactual, tam, true, true, pass);
  printP(td_f);
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

void ICACHE_FLASH_ATTR checkBox(int numpar, bool selected)
{
  printP(menor, c(tinput), b, type, ig, comillas);
  printP(c(checkbox), comillas, b);
  printP(c(namet), ig,comillas);
  printI(numpar);
  printP(comillas,b, tvalue, ig,comillas);
  printP(uno,comillas);
  if (selected) printselected(true);
  printfincampo();
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

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, int pri, int ult)
{
  printinicampoCB(numpar);
  for (byte j=pri; j<=ult; j++)   {
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (valact==j) printselected(false);
    printPiP(mayor, j, c(option_f));   }
  printP(c(select_f));
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, byte lon, PGM_P t[])
{
  printinicampoCB(numpar);
  for (byte j=0;j<lon;j++)   {
    printP(c(optionvalue));
    printPiP(comillas, j, comillas);
    if (valact==j) printselected(false);
    printP(mayor,t[j]);
    printP(c(option_f));   }
  printP(c(select_f));
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1)
{
  PGM_P t[]={t0,t1};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2)
{
  PGM_P t[]={t0,t1,t2};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3)
{
  PGM_P t[]={t0,t1,t2,t3};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3, PGM_P t4)
{
  PGM_P t[]={t0,t1,t2,t3,t4};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t);
}

void ICACHE_FLASH_ATTR printcampoCB(int numpar, int valact, PGM_P t0, PGM_P t1, PGM_P t2, PGM_P t3, PGM_P t4, PGM_P t5)
{
  PGM_P t[]={t0,t1,t2,t3,t4,t5};
  printcampoCB(numpar,valact,sizeof(t)/sizeof(t[0]),t);
}

void ICACHE_FLASH_ATTR tituloFila(PGM_P texto, int num, PGM_P letra, int indice)
{
  printP(td, paren_i);
  printPiP(letra, indice, paren_f);
  printP(b, b, texto);
  printPiP(b, num, td_f);
}

void ICACHE_FLASH_ATTR printFecha()
{
  printI(day()); printPiP(barra, month(), barra); printIP(year(),b);
  if (hour()<10) printP(cero); printI(hour()); printP(dp);
  if (minute()<10) printP(cero); printI(minute()); printP(dp);
  if (second()<10) printP(cero); printI(second());
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

