#include <FPS_GT511C1R.h>
#include "SoftwareSerial.h"
#include "Nextion.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include <Agenda.h>

Agenda scheduler;
int tiempoCasillas;
int tiempoCarga;

//Huellero
FPS_GT511C1R fps(12, 11);
int zero = 9; //pin boton borrar db
int temp = 0;//Comparador usuario
int enrollid;//Conteo general usuarios. EEPROM 128
int pasaPuerta;
int contador = 0;

bool estadoPuerta = 0;
bool verificaEstadoCarga = 0;

//unsigned long interval=1000; // the time we need to wait
//unsigned long previousMillis=0; // millis() returns an unsigned long.


//Inicio definicion Entorno Grafico
NexPage inicio = NexPage(0,0, "inicio");
NexButton retiraCelu = NexButton(0,2, "b1");

NexPage inscribir = NexPage(1,0, "inscribir");
NexPage b7 = NexPage(1,1, "b0");

NexPage retirar = NexPage(2,0, "retirar");

NexPage huellas = NexPage(3,0, "huellas");
//NexButton cerrarHuellas = NexButton(3,1, "b0");

NexPage casillas = NexPage(4,0, "casillas");
NexButton b0 = NexButton(4,2, "b0");
NexButton b1 = NexButton(4,3, "b1");
NexButton b2 = NexButton(4,4, "b2");
NexButton b3 = NexButton(4,5, "b3");
NexButton b4 = NexButton(4,6, "b4");
NexButton b5 = NexButton(4,7, "b5");
NexButton b8 = NexButton(4,8, "b6");
//NexNumber n0 = NexNumber(4,9, "n0");

NexPage carga = NexPage(5,0, "carga");
NexButton b9 = NexButton(5,1, "b0");
//NexButton b6 = NexButton(5,2, "b1");
//NexNumber n1 = NexNumber(5,3, "n0");

NexPage salir = NexPage(6,0, "salir");
NexPage finalizar = NexPage(7,0, "finalizar");
NexPage errorPuerta = NexPage(8,0, "error");

NexTouch *nex_listen_list[] = 
{
    &retiraCelu,
    //&cerrarHuellas,
    &b0,
    &b1,
    &b2,
    &b3,
    &b4,
    &b5,
    //&b6,
    &b7,
    &b8,
    &b9,
    //&errorPuerta,
    NULL
};
//Fin definicion entorno grafico

int relay = 0;
const byte PUESTOS = 6; //Máximo numero de casillas, hardware.
byte utilizados; //Numero de casillas en uso


struct usoCasillas
{
  int userId;
  int seleccionada;
};

usoCasillas selecUser[PUESTOS];//revisa carga eeprom de valor inicial EEPROM 24

int pinCandados[PUESTOS] = {38, 42, 46, 50, 39, 43};
int pinCarga[PUESTOS]= {40, 44, 48, 52, 41, 45};
int pinEstadoCandado[PUESTOS] = {22, 24, 26, 28, 30, 32};
int pinEstadoCarga[PUESTOS] = {A0, A1, A2, A3, A4, A5};

boolean casillasEnUso[PUESTOS];//EEPROM 0
boolean puertasRevisado = 0; //Estado de verificacion inicial de puertas abiertas.


byte contarCasillas(){
  byte cuentaCasilla = 0;
  for(int i=0;i< PUESTOS; i++){
    if (casillasEnUso[i] == 0){
      continue;  
    }
    else {
      cuentaCasilla += 1;
    }
  }
  return cuentaCasilla;
}

void retiraCeluPopCallback(void *ptr)
{
  delay(3000);
  fps.SetLED(true);
 // t2.setText("Presione el dedo registrado");
  delay(2220);
  fps.CaptureFinger(false);//Realiza a leitura da digital no scanner
  int id = fps.Identify1_N();//Salva na variavel id o codido de identificação da digital
  
  fps.SetLED(false);
  if (id >= 0 && id <=199)
  {
    //Printa as informações no LCD se houver reconhecimento de dados
    //dbSerialPrint("Huella ID:");
    //dbSerialPrint(id);
    //delay(600);
    sendCommand("vis p4,1");
    delay(30);
    int resultado = encontrarUser(selecUser, id, PUESTOS);
    if(resultado != -1)
    {

      //t2.setText("Abriendo Casilla:");
      
      activaRelevo(selecUser[resultado].seleccionada);
      delay(100);
      desactivaRelevoCarga(selecUser[resultado].seleccionada);
      
      fps.DeleteID(id);
      delay(100);
      utilizados--;
      pasaPuerta = selecUser[resultado].seleccionada;
      casillasEnUso[selecUser[resultado].seleccionada] = 0;
      EEPROM_writeAnything(0, casillasEnUso);
      selecUser[resultado] = {-1,-1};
      EEPROM_writeAnything(24, selecUser);
      delay(66);
      finalizar.show();
      delay(6666);
      //revisarPuertas(pasaPuerta);
      errorPuerta.show();
      estadoPuerta = 1;

    }
    else
    {
      //t2.setText("Resultado no encontrado"); 
      sendCommand("vis p0,1"); 
      delay(3000); 
      inicio.show();
    }
  }
  else
  {
    //Printa as informações no LCD se nao houve reconhecimento de dados
    //t2.setText("Huella no reconocida, no incluida");
    sendCommand("vis p0,1");
    delay(2500);//Aguarda 2 segundos
    inicio.show();
  }
    
}
void b0PopCallback(void *ptr)
{
    int n = 0;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}

void b1PopCallback(void *ptr)
{
    int n = 1;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}

void b2PopCallback(void *ptr)
{
    int n = 2;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}

void b3PopCallback(void *ptr)
{
    int n = 3;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}
void b4PopCallback(void *ptr)
{
    int n = 4;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}
void b5PopCallback(void *ptr)
{
    int n = 5;
    activaRelevo(n);
    delay(100);
    iniciaCarga(n);
    scheduler.remove(tiempoCasillas);
}

void b7PopCallback(void *ptr)
{
  huellas.show();
  sendCommand("vis b0,0");
  delay(1000);
  Enroll();
}

void b8PopCallback(void *ptr)
{
  reversarHuella();
  delay(333);
  scheduler.remove(tiempoCasillas);
  delay(33);
}

void b9PopCallback(void *ptr)
{
  scheduler.remove(tiempoCarga);
  reversarHuella();
  contador = 0;  
  delay(33);
  //sendCommand("cls RED");
  pasaPuerta = relay;
  //revisarPuertas(relay);
  errorPuerta.show();
  verificaEstadoCarga = 0;
  estadoPuerta = 1;
  //dbSerialPrint("B9 mensaje prueba...");
  delay(33);
}

void reversarHuella(){
   fps.DeleteID(enrollid-1);
   delay(111);
   enrollid--;
   EEPROM_writeAnything(128, enrollid);   
   delay(33);
}

/*
void errorPuertaPopCallback(void *ptr){
  revisarPuertas(pasaPuerta);
  delay(100);
}
*/

void estadoCarga(){
  activaRelevoCarga(relay);
  delay(3000);
  
  int sensorValue = analogRead(pinEstadoCarga[relay]);
  float voltage = sensorValue * (5.0 / 1023.0);
/*  
  if (voltage < 1){
    desactivaRelevoCarga(relay);
    activaRelevo(relay);
    //temporizadorCarga();
    //carga.show();        
    contador +=1;
        
    if (contador == 3){
      //Error de casilla, abrir  otra diferente manualmente

      contador = 0;
      reversarHuella();
      delay(33);
      scheduler.remove(tiempoCarga);
      delay(33);
      pasaPuerta = relay;
      errorPuerta.show();
      estadoPuerta = 1;
      delay(33);
      inicio.show();

    } 
    else {
      verificaEstadoCarga = 1;
    }    
  }
  else
  {*/
    selecUser[relay] = {enrollid-1, relay};
    EEPROM_writeAnything(24, selecUser);
    casillasEnUso[relay] = 1;
    EEPROM_writeAnything(0, casillasEnUso);
    delay(33);
    utilizados++;
    //previousMillis = millis();
    contador = 0;
    //dbSerialPrint("Salida Forzada...");
    scheduler.remove(tiempoCarga);      
    salir.show();
    delay(6666);
    inicio.show();

}

void temporizadorCasillas(){
      //dbSerialPrintln("Timer Casillas iniciado luego de 15 seconds.");
      reversarHuella();
      delay(100);
      inicio.show();
      scheduler.remove(tiempoCasillas);

}

void temporizadorCarga(){
    //dbSerialPrintln("Timer Carga iniciado luego de 45 seconds.");  
    contador = 0;
    reversarHuella();
    delay(100);
    pasaPuerta = relay;
    errorPuerta.show();
    verificaEstadoCarga = 0;
    estadoPuerta = 1;
    scheduler.remove(tiempoCarga);
}


void iniciaCarga(int n)
{
  relay = n;
  tiempoCarga = scheduler.insert(temporizadorCarga, 45000000, true);//45000000
  carga.show();
  verificaEstadoCarga = 1;  

}


void activaRelevo(int num){
  digitalWrite(pinCandados[num], HIGH);
  delay(300);
  //dbSerialPrint(pinCandados[num]);
  digitalWrite(pinCandados[num], LOW);
  delay(300);    
}
void activaRelevoCarga(int num){
  digitalWrite(pinCarga[num], HIGH);
  //dbSerialPrint(pinCarga[num]);
  delay(300);
   
}
void desactivaRelevoCarga(int num){
  digitalWrite(pinCarga[num], LOW);
  //dbSerialPrint(pinCarga[num]);
  delay(300);
   
}

void verificarCasillas(boolean arreglo[]){
    int i = 0;
    if (arreglo[i] == 0 ) {
      sendCommand("vis b0,1");         
                  
    }
    i++;
    if (arreglo[i] == 0 ) {
        sendCommand("vis b1,1");
                  
    }
    i++;
    if (arreglo[i] == 0 ) {
        sendCommand("vis b2,1");
                  
    }
    i++;
    if (arreglo[i] == 0 ) {
        sendCommand("vis b3,1");
                  
    }
    i++;
    if (arreglo[i] == 0 ) {
        sendCommand("vis b4,1");
                  
    }
    i++;
    if (arreglo[i] == 0 ) {
        sendCommand("vis b5,1");
                  
    }

}

int encontrarUser(usoCasillas arreglo[], int valorBuscado, int maximo){
  for (int i = 0; i < maximo; i++) {
      if (arreglo[i].userId == valorBuscado) {
        return i;          
      }
    }

  return -1;
}

void Enroll()
{
  if (utilizados == PUESTOS) {
    //t0.setText("No hay casillas disponibles");
    sendCommand("cls RED");
    delay(2000);
    inicio.show();
  }
  else {
    delay(300);
    fps.SetLED(true);
    delay(200);
  
    //É verificado quantos digitais tem cadastradas, e selecionandoa proxima posição disponivel para armazenamento
    //======================================================================================
    bool usedid = true;
    while (usedid == true)
    {
      usedid = fps.CheckEnrolled(enrollid);
      //dbSerialPrint(enrollid);
      if (usedid==true) enrollid++;
    }
    fps.EnrollStart(enrollid);
    temp=enrollid;
    //======================================================================================
  
    //Aguarda até que o dedo seja retirado do scanner
    while(fps.IsPressFinger() == false) delay(100);
    bool bret = fps.CaptureFinger(true);
    int iret = 0;
    //Se conseguir realizar alguma leitura a variavel "bret" valerá 1
    if (bret != false)
    {
      //Se for diferente de 0 irá printar a informação para que o dedo seja removido do scanner
      //t0.setText("Presione dedo, 1a vez...");
      fps.SetLED(false); 
      delay(200);
      fps.Enroll1();//Salva a primeira informacao da digital
      //Aguarda até que o dedo seja retirado do scanner
      sendCommand("vis p1,1");
        delay(300);
        while(fps.IsPressFinger() == true) delay(100);
    
        //Solicita que o dedo seja pressionado no scanner novamente
        //t0.setText("Presione dedo, 2a vez...");
        
        fps.SetLED(true); 
        //Aguarda até que o dedo seja retirado do scanner
        while(fps.IsPressFinger() == false) delay(100);
    
        //Repete tudo por mais 2 vezes
        //=====================================================================================
        bret = fps.CaptureFinger(true);
        if (bret != false)
        {
          fps.SetLED(false);
          sendCommand("vis p6,1");
          delay(300); 
          
          fps.Enroll2();
          while(fps.IsPressFinger() == true) delay(100);
          fps.SetLED(true);
          //t0.setText("Presione dedo, 3a vez...");
    
          while(fps.IsPressFinger() == false) delay(100);
          //====================================================================================
          //====================================================================================
          bret = fps.CaptureFinger(true);
          if (bret != false)
          {
            fps.SetLED(false);        
            //t0.setText("Retire el dedo ...");
            delay(100);
            sendCommand("vis p7,1");
            iret = fps.Enroll3();
            
            //===================================================================================
            //Nessa parte é verificado se se o cadastro foi realizado com sucesso
            //enrollid=0;          
            usedid = true;
            while (usedid == true)
            {
              usedid = fps.CheckEnrolled(enrollid);
              //dbSerialPrint("Enroll ID:");
              //dbSerialPrintln(enrollid);
              if (usedid==true) enrollid++;
            } 
            //Compara se o dedo foi pressionado 3 vezes e se a posição anterior da memoria é igualk a atual
            if (iret == 0&&temp!=enrollid)
            {
              //t0.setText("Inscripcion realizada con exito");
              EEPROM_writeAnything(128, enrollid);
              delay(100);
              //temporizadorCasillas();
                            
              casillas.show();
              verificarCasillas(casillasEnUso);
              delay(12);
              tiempoCasillas = scheduler.insert(temporizadorCasillas, 15000000, true);
              delay(12);  
                                 
            }
            else
            {
              //t0.setText("Inscripcion de huella, fallida...");
              sendCommand("vis p4,1");
              delay(1500);//Aguarda 1 segundo
              inscribir.show();
    
          }
        }
        // Se houver falhas durante a captura printará umas das 3 mensagens abaixo, de acordo com cada posição
        //Erro 1
        else {
          //t0.setText("Fallo captura, primer dedo.");
          sendCommand("vis p4,1");
          fps.SetLED(false);
          delay(1000);
          inscribir.show();
        }
        //====================================================================================
      }
      //Erro 2
      else {
        //t0.setText("Fallo captura, 2o dedo.");
        sendCommand("vis p4,1");
        fps.SetLED(false);
        delay(1000);
        inscribir.show();
      }
      //=====================================================================================
    }
    //Erro 3
    else {
      //t0.setText("Fallo captura ...");
      sendCommand("vis p4,1");
      fps.SetLED(false);
      delay(1000);
      inscribir.show();
    }
    //======================================================================================
   }   
}  
  
void borrar_todo()
{
    fps.DeleteAll();// Apaga todo o registro de memoria de digitais
    for (int i=0; i< PUESTOS; i++){
      selecUser[i] = {-1,-1}; 
    }
    EEPROM_writeAnything(24, selecUser);    
    utilizados = 0;
    enrollid =0;
    EEPROM_writeAnything(128, enrollid);
    boolean casillasEnUso[PUESTOS] = {0,0,0,0,0,0};
    EEPROM_writeAnything(0, casillasEnUso);
    dbSerialPrintln("Datos eliminados exitosamente");
    delay(222);//Aguarda 2 segundos
    
}

void danza_fps(int veces) {
  for(int x=0; x < veces; x++){
    fps.SetLED(true);
    delay(99);
    fps.SetLED(false);
    delay(99);
  }
}
  
void revisarPuertas (int puertaAbierta){
        
    activaRelevoCarga(puertaAbierta);
    delay(3000);
    int sensorValue = analogRead(pinEstadoCarga[puertaAbierta]);       
    float voltage = sensorValue * (5.0 / 1023.0);
    //dbSerialPrint(voltage);            
     
    if (voltage < 1) {
      desactivaRelevoCarga(puertaAbierta);
      delay(111);
      //dbSerialPrint("Mensaje revisar Puertas...");
      inicio.show();     
    } 
    else {
       desactivaRelevoCarga(puertaAbierta);
       activaRelevo(puertaAbierta);
       pasaPuerta = puertaAbierta;
       errorPuerta.show();
       estadoPuerta = 1;  
       
    }

}


void setup() {
  EEPROM_readAnything(0, casillasEnUso);
  EEPROM_readAnything(24, selecUser);
  EEPROM_readAnything(128, enrollid);
  utilizados = contarCasillas();
  delay(100);
  pinMode(zero, INPUT);
  digitalWrite(zero, LOW);  
  
  for (int i=0; i<PUESTOS; i++){
    pinMode(pinCandados[i], OUTPUT);
    pinMode(pinCarga[i], OUTPUT);
    digitalWrite(pinCarga[i], casillasEnUso[i]);
     
    //ENTRADAS
    pinMode(pinEstadoCarga[i], INPUT);
    digitalWrite(pinEstadoCarga[i], HIGH);
    
    pinMode(pinEstadoCandado[i], INPUT);
    digitalWrite(pinEstadoCandado[i], HIGH);
  }
      
  nexInit();

  b0.attachPop(b0PopCallback);
  b1.attachPop(b1PopCallback);
  b2.attachPop(b2PopCallback);
  b3.attachPop(b3PopCallback);
  b4.attachPop(b4PopCallback);
  b5.attachPop(b5PopCallback);
    
  //b6.attachPop(b6PopCallback);
  b7.attachPop(b7PopCallback);
  b8.attachPop(b8PopCallback);
  b9.attachPop(b9PopCallback);
  retiraCelu.attachPop(retiraCeluPopCallback);
  //cerrarHuellas.attachPop(cerrarHuellasPopCallback);
  //errorPuerta.attachPop(errorPuertaPopCallback);
  
  fps.Open();
  delay(111);
  danza_fps(3);
  //sendCommand("cls RED");
  sendCommand("dim=30");
  //revisarPuertas(6);
  dbSerialPrintln("Configuracion finalizada con exito...");

}

void loop() {
  // put your main code here, to run repeatedly:
  nexLoop(nex_listen_list);
  scheduler.update();
    
  int zero1 = digitalRead(zero);
  if(zero1==1){
    borrar_todo();
  } 

  if(estadoPuerta==1){
    if(digitalRead(pinEstadoCandado[pasaPuerta]) == 1){
      delay(33);
      estadoPuerta = 0;
      revisarPuertas(pasaPuerta);      
    } 
    
  }

  if(verificaEstadoCarga==1){
    if (digitalRead(pinEstadoCandado[relay]) == 1) {
      delay(33);
      verificaEstadoCarga = 0;      
      estadoCarga();
    }    
  }
  
}
