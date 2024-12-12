/*-----------------------------------------------------------------------
 Control_DS1631

 Programme à téléverser sur Arduino Mega.
 En utilisant Arduino IDE version 1.8.13
 Exporté sur l'extension de Visual Studio Code PlatformIO version 3.3.3

 constituant:
  Carte Arduino Mega
  Capteur de température thermostat DS1631

  Mis à jour le: 19 Novembre
  -----------------------------------------------------------------------
 */
#include <Wire.h>
#include <Arduino.h>
#define DS1631_ADDR 0x90 >> 1   //On définit l'addresse du capteur, ici les pin A0 A1 et A2 sont reliés au gnd, donc à 0 --> 0x90 (1001 0000) avec le >> qui décale vers la droite pour avoir 0x48 (0100 1000)et avoir le nombre sur 7 bits
void setup() {
  // On démarre le moniteur série (liaison RS232 Arduino <-> pc)
  Serial.begin(9600);
  Serial.println();
  Serial.println("In setup, configurations...");
  Serial.println();
  Serial.println("**********************************");
  Serial.println("** DS1631 Capteur de température **");
  Serial.println("**********************************");
  Serial.println();

  //On démarre l'I2C avec le maître
  Wire.begin();

  //On vérifie qu'il n'y a pas de transmission active, sinon on les arrête
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0x22); //On arrête la conversion du capteur
  int error = Wire.endTransmission(); 
  Serial.println(error);
  if (error != 0 ){
    Serial.print("Erreur détécté, valeur: ");
    Serial.println(error);
    Serial.println("Composant non détécté");
    delay(10000);
  }
  //Lecture de la configuration courante
  Wire.beginTransmission(DS1631_ADDR);  //On démarre la communication
  Wire.write(0xAC);   //donne la configuration en binnaire
  Wire.endTransmission(); //On arrête la communication pour réduite de risque de bug

  Wire.requestFrom(DS1631_ADDR, 1); //On demande 1 octet au DS1631 qui est sur 7 bits
  Wire.available();
  int ancienne_config = Wire.read(); //On stock l'ancienne configuration dans une nouvelle variable

  Serial.print("Configuration actuelle avant modification : 0x");
  Serial.print(ancienne_config, HEX); //On affiche l'ancienne configuration, on utilise le HEX pour afficher le résultat en hexadécimal
  Serial.println();

  //écriture  de la data de configuration dans le registre config
  Wire.requestFrom(DS1631_ADDR, 1);
  Wire.write(0xAC); //Code commande Acces Config    En Wire.write(), le dernier bit de paramètrage est à 0 et pour Wire.read() il est à 1
  Wire.write(0x0C); //Configuration
  Wire.endTransmission();

  //Lecture de la configuration après la modification
  Wire.requestFrom(DS1631_ADDR, 1);
  Wire.available();
  int config_actuelle = Wire.read();

  Serial.print("Configuration actuelle après modification : 0x");
  Serial.print(config_actuelle, HEX);
  Serial.println();

  //Début de la conversion de la température
  Wire.beginTransmission(DS1631_ADDR);    //On innicie une conversation avec le DS1631
  Wire.write(0x51); //Commande Start Convert
  Wire.endTransmission();     //On ferme la conversation avec le DS1631
}
void loop() {
  Serial.println("-----------------------------------");
  Serial.println();

  // Lecture de la température 
  Wire.beginTransmission(DS1631_ADDR);
  Wire.write(0xAA); // Commande de la lecture de la température 
  Wire.endTransmission();

  //Lecture des 2 octets de data
  Wire.requestFrom(DS1631_ADDR, 2); //On demande au DS1631 2 octets de data
  Serial.print("Octets en cour de lecture : ");
  Serial.println(Wire.available());   //On affiche la température à partir du moment où elle est disponible 
  int T_MSB = Wire.read();    //La fonction Wire.read ne lit qu'un octet à la fois, il faut donc faire 2 read pour lire les 2 octets dans un buffer 
  int T_LSB = Wire.read();

  //On affiche les valeurs binaires brutes
  Serial.println( "Valeurs binaires \"brutes\" lues :");    //Le \" sert à utiliser le caractère " dans une chaine de caractère sans en marquer la fin
  Serial.print("T_MSB = ");
  Serial.println(T_MSB, BIN);   //On converti la partie entière en binaire et on l'affiche
  Serial.print("T_LSB = ");
  Serial.println(T_LSB, BIN);   //On converti la parti décimale en binaire et on l'affiche
  
  //calcule de la partie entière
  int partie_entiere = T_MSB & 0b01111111 ;
/*
  /////////////////////////////////////////////////////////////
  //Première façon pour calculer la partie décimale
  /////////////////////////////////////////////////////////////
  float partie_decimale = 0.0;
  if ( (T_LSB & 0b10000000) == 0b10000000)    //on incrémente de 2^-n en fonction des bits du 2ème octet pour faire la partie décimale (si le 
    partie_decimale = partie_decimale + 0.5;
  if ( (T_LSB & 0b01000000) == 0b01000000)
    partie_decimale = partie_decimale + 0,25;
  if ( (T_LSB & 0b00100000) == 0b00100000);
    partie_decimale = partie_decimale + 0,125;
  if ( (T_LSB & 0b00010000) == 0b00010000);
    partie_decimale = partie_decimale + 0.0625;

  Serial.print("La valeure décimale correspondante :T_dec = ");
  if (T_MSB >= 0x80) Serial.print(" - ");
  Serial.println((T_MSB & 0b01111111) + partie_decimale, 4);

  /////////////////////////////////////////////////////////////
  //Deuxième façon pour calculer la partie décimale
  /////////////////////////////////////////////////////////////
  partie_decimale = 0.0;
  unsigned int masque = 0x80;
  float poids = 0.5;

  for (int i=0 ; i < 4; i++){   //On répète la boucle 4 fois      
    if ((T_LSB & masque) == masque )
      partie_decimale = partie_decimale + poids;
      masque = masque >>1;
      poids = poids / 2;
  }
  Serial.print("Valeur décimale correspondante :T_dec = ");
  if (T_MSB >= 0x80) Serial.print( "- ");
  Serial.println((T_MSB & 0b01111111) + partie_decimale, 4); 
*/
/////////////////////////////////////////////////////////////////
//Troisième façon pour calculer la partie décimale
/////////////////////////////////////////////////////////////////
  float partie_decimale = 0.0;
  partie_decimale = (T_LSB>>4)* 0.0625;   

  Serial.print("Valeur décimale correspondante :T_dec = "); //on affiche la partie entière et décimale 
  if (T_MSB >= 0x80) Serial.print( "- ");
  Serial.println((T_MSB & 0b01111111) + partie_decimale, 4);
  Serial.println(); 

  delay(5000);


}
