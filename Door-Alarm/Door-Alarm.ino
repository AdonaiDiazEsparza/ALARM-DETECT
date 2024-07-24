#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

#define PIN_ECHO      A1
#define PIN_TRIGGER   A0
#define PIN_BUZZER    A2

const int filas = 4;
const int columnas = 3;

String contrasena = "12345";
String contrasena_introducida = "";

char hexaKeys[filas][columnas]{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte filasPines[filas] = {2,3,4,5};
byte columnaPines[columnas] = {6,7,8};

bool limpiar = false;
bool limpiado = false;

unsigned long tiempo = 0;
unsigned long tiempo_desbloqueo = 0;
unsigned long tiempo_alarma = 0;

int letrapos = 0;

char tecla = ' ';
bool puerta = false;
bool bloqueado = false;
bool alarma = false;
bool suena = false;
bool desbloqueo = false;

LiquidCrystal_I2C lcd(0x27,20,4);
Keypad matriz ( makeKeymap(hexaKeys), filasPines, columnaPines, filas, columnas); 


// deteccion de puerta
bool puerta_cerrada(){
  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(12);
  digitalWrite(PIN_TRIGGER, LOW);
  long tiempo = pulseIn(PIN_ECHO, HIGH);
  float distancia = tiempo / 58;

  delay(500);

  if(distancia>4){
    return false;
  }
  else{
    return true;
  }
}

// bloqueo
bool bloqueo(){
  unsigned long tiempo_bloqueo = millis();
  while((tiempo_bloqueo+7000)>millis()){
    char tecleado = matriz.getKey();
    if(tecleado){
      if(tecleado == '0'){
        return true;
      }
      if(tecleado != '0'){
        return false;
      }
    }
  }

  return false;
}

// PARA LAS PANTALLAS DE CADA UNO
void pantalla_desbloqueado(){
  lcd.setCursor(0,0);
  lcd.print(" ALARMA ");
  lcd.setCursor(0,1);
  lcd.print(" DESACTIVADA ");
}

void pantalla_bloqueado(){
  lcd.setCursor(0,0);
  lcd.print("   ALARMA   ");
  lcd.setCursor(0,1);
  lcd.print("   ACTIVADA   ");
}

void pantalla_bloqueando(){
  lcd.setCursor(0,0);
  lcd.print("   ACTIVANDO   ");
  lcd.setCursor(0,1);
  lcd.print("   ALARMA   ");
}

void pantalla_desbloqueando(){
  lcd.setCursor(0,0);
  lcd.print(" DESACTIVANDO   ");
  lcd.setCursor(0,1);
  lcd.print("   ALARMA   ");
}

void pantalla_alarma(){
  lcd.setCursor(0,0);
  lcd.print("   ALARMA   ");
  lcd.setCursor(0,1);
  lcd.print("   SONANDO   ");
}


void setup() {
  Serial.begin(115200);

  pinMode(PIN_ECHO,INPUT);
  pinMode(PIN_TRIGGER,OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  
  lcd.init();
  lcd.setCursor(0,0);
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print(" ALARMA DE ");
  lcd.setCursor(0,1);
  lcd.print(" PUERTA ");

  delay(1000);
  lcd.clear();
  
  tiempo = millis();
  tiempo_alarma = millis();
}

void loop() {

  tecla = matriz.getKey();
  
  if((millis()- tiempo)>500) {
    puerta = puerta_cerrada();
    tiempo = millis();
  }

  if(!bloqueado && (tecla == '*')){
    Serial.println("-----------------------");
    Serial.println("Entra a bloqueo");
    bloqueado = bloqueo();
    tecla = 0;
    Serial.println("Bloqueo ="+(String)bloqueado);

    if(bloqueado){
      lcd.clear();
      pantalla_bloqueando();
      delay(3000);
      lcd.clear();
    }
  }

  if(bloqueado && !puerta && !alarma){
    Serial.println("-----------------------");
    Serial.println("Alarma empieza a sonar");
    alarma = true;
    tiempo_alarma = millis();
  }

  if(alarma && (millis()-tiempo_alarma)>500){
    suena = !suena;
    digitalWrite(PIN_BUZZER,suena);
    tiempo_alarma = millis();
  }

  if(bloqueado && (tecla=='*') && !desbloqueo){
    desbloqueo = true;
    tecla = 0;
    tiempo_desbloqueo = millis();
    lcd.clear();
  }

  if(desbloqueo && ((tiempo_desbloqueo+10000)>millis())){
    if(tecla){
      contrasena_introducida = contrasena_introducida+(String)tecla;
      Serial.println("Letra "+(String)letrapos+": "+(String)tecla);
      lcd.setCursor(0,1);
      lcd.print(contrasena_introducida);
      letrapos++;
    }
    
    if(letrapos>4){
      delay(300);
      Serial.println(contrasena_introducida);
      if(contrasena_introducida == contrasena){
        Serial.println("contrasena correcta");
        alarma = false;
        bloqueado = false;
        suena = false;
        digitalWrite(PIN_BUZZER, LOW);


        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Contrasena");
        lcd.setCursor(0,1);
        lcd.print("Correcta");
        delay(1000);
        lcd.clear();
        pantalla_desbloqueando();
        delay(3000);
        lcd.clear();
      }else{
        Serial.println("Contrasena Incorrecta");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Contrasena");
        lcd.setCursor(0,1);
        lcd.print("Incorrecta");
        delay(1000);
      }
      desbloqueo = false;
      contrasena_introducida = "";
      letrapos = 0;
    }
  }else{
    desbloqueo = false;
    contrasena_introducida = "";
    letrapos = 0;
  }

  if(desbloqueo){
    lcd.setCursor(0,0);
    lcd.print("Contrasena: ");
  }

  if(alarma && !desbloqueo){
    pantalla_alarma();
  }

  if(bloqueado && puerta && !desbloqueo){
    pantalla_bloqueado();
  }

  if(!bloqueado){
    pantalla_desbloqueado();
  }
  
}
