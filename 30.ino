// Reveil fait avec:
// Arduino Nano, Afficheur LED Alpha-numérique en I2C (https://learn.adafruit.com/adafruit-led-backpack/0-54-alphanumeric)
// Module Real Time Clock (http://adafru.it/3296)

#include <Wire.h>
#include <Adafruit_GFX.h> // pas utilisée mais celle ci-dessous en dépend
#include "Adafruit_LEDBackpack.h" // librairie pour afficheur alphanumerique
#include "RTClib.h" // librairie du module RTC
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// on initialise les variables:
int heure = 0; // contient l'heure sur 2 chiffres (ex: 23)
int heure1 = 0; // le premier chiffre (ex: 2)
int heure2 = 0; // le 2ème (ex: 3)
int minut = 0; // contient les minutes sur 2 chiffres (ex: 59)
int minut1 = 0;
int minut2 = 0;
int reglage = 0; // si 3 -> réglage des heures de l'heure actuelle (avec le bouton setup) Si 4 -> réglage des minutes de l'heure actuelle
                //si 1 -> réglage des heures de l'alarme (avec le chamignon + le bouton setup) Si 2 -> réglage des minutes de l'alarme
int alarme = 0; // si 1 -> alarme activée (le point est affiché sur le digit) Si 0 -> alarme est désactivée
int heure_alarme = 2350; // contient l'heure de l'alarme sur 4 chiffres (ex: 2359)
int enfonce_tmp = 0; // utilisé lorsque l'alarme sonne, quand on enfonce le champignon il passe à 1
int nb_buzz = 0; // compte le nombre de bip pendant l'alarme (s'arrete à 10)
int buzzer = 7; // pin utilisée par le buzzer (pin 7)
int bouton_alarm = 8; // pin utilisée par le bouton Alarme (pin 8)
int bouton_champi = 9; // pin utilisée par le bouton Champignon (pin 9)
int bouton_setup = 10; // pin utilisée par le bouton Setup (pin 10)
int temps = 0; // utilisé dans réglage pour compter le temps d'attente entre les appuis sur le bouton


RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"}; // ne sont affichés que dans la console

void setup() {
  Serial.begin(57600); // pour vérifier que la date est juste: lancer la console à 57600
  rtc.adjust(DateTime(2017, 12, 31, 14, 59, 0)); // On définit la date au : 31 Decembre 2017 à 14h59
  pinMode(buzzer, OUTPUT);// on met la pin du buzzer en mode sortie
  pinMode(bouton_alarm, INPUT_PULLUP); // on met la pin du bouton Alarme en mode lecture AVEC la résistance pull-up activée (afin de ne pas devoir souder de résistance)
  pinMode(bouton_champi, INPUT_PULLUP); // idem pour le bouton Champignon
  pinMode(bouton_setup, INPUT_PULLUP); // idem pour le bouton Setup

  digitalWrite(buzzer,HIGH); // on test le buzzer
  delay(500);
  digitalWrite(buzzer,LOW);

  alpha4.begin(0x70);  // adresse I2C de l'affichage
  alpha4.clear();
  alpha4.writeDisplay(); // toujours rafraichir pour afficher

  //test
  //alpha4.writeDigitAscii(3, 'P');
  //alpha4.writeDigitAscii(3, 14); // le point pour le temoin d'alarme
  //alpha4.writeDisplay(); // toujours rafraichir pour afficher
  //delay(50);

}

char displaybuffer[4] = {' ', ' ', ' ', ' '}; // utile??

void loop() {
  DateTime now = rtc.now(); // on définit DateTime avec celle du module RTC
  heure = now.hour();
  minut = now.minute();

  // On vérifie si l'heure actuelle correspond à l'heure de l'alarme (et que l'alarme est activée)
  if (alarme == 1 and heure_alarme == ((heure * 100) + minut)){
    Serial.println("Driiiiing..");
    Serial.println(" ");
    enfonce_tmp = 1; // si on appuie sur le champignon, on met $enfonce_tmp=0 et ca arrete la sonnerie
    nb_buzz = 1; // variable qui sert à compter le nombre de fois que le reveil a sonné
    // la première sonnerie est un peu + courte que les autres:
    digitalWrite(buzzer,HIGH); // biiip
    delay(800);
    for(int i = 1; i < 10; i++) { // boucle de 9
      digitalWrite(buzzer,LOW); // on coupe le buzzer et on attend 1 min
      for(int ii = 1; ii < 200; ii++) { // boucle de 200 pour le sleep 200x0.3sec donc 60sec
        if (digitalRead(bouton_champi) == LOW) { // si bouton Champignon est appuyé -> arret de la sonnerie
          Serial.println("Vous appuyez sur le champignon -> Arret de la sonnerie..");
          enfonce_tmp = 0;
        } // fin de si champignon appuyé
      delay(300);
      } // fin for 200
      if (enfonce_tmp != 0){ // si $enfonce_tmp = 0 on laisse le buzzer couper
        digitalWrite(buzzer,HIGH); // on lance le buzzer durant 2sec
        nb_buzz = (nb_buzz + 1); // on ajoute 1 au compteur de sonnerie
        for(int i = 0; i < 6; i++) { // boucle de 6 pour le sleep 6x0.3sec donc 1.8sec
          if (digitalRead(bouton_champi) == LOW) { // check bouton champignon pour arreter la sonnerie
            Serial.println("Vous appuyez sur le champignon -> Arret de la sonnerie..");
            enfonce_tmp = 0;
          } // fin check du champignon
          delay(300);
        } // fin for 6
      } // fin de si $enfonce_tmp = 0
    } // fin de la boucle for 9x
    digitalWrite(buzzer,LOW); // on coupe le buzzer si ce n'est deja fait
  } // fin de si l'heure actuelle correspond à l'heure de l'alarme



  // Sonnerie de secours: on verifie si l'heure actuelle correspond a l'heure d'alarme + 20min ET nb_buzz=11
  if(nb_buzz >= 10){
    enfonce_tmp = ((heure_alarme % 100) + 20); // on reutilise enfonce_tmp pour récupérer les minutes de heure_alarme et mettre + 20 min
    if(enfonce_tmp >= 60){ // si egal ou sup à 60
      enfonce_tmp = (enfonce_tmp - 60);
    }
    if (alarme == 1 and minut == enfonce_tmp){ // si les minutes actuelles = minutes de l'alarme + 20min
      Serial.println("L'heure actuelle correspond a celle de l'alarme + 20min, on active la sonnerie de secours");
      for(int i = 2; i < 10; i++) { // boucle de 8 (4.8sec)
        digitalWrite(buzzer,HIGH); // on lance le buzzer
        delay(400);
        digitalWrite(buzzer,LOW); // on coupe le buzzer
        delay(200);
      }
      nb_buzz = 0;
      Serial.println("Fin de l'alarme de secours.");
    }
  }



  // Si on appuie sur le champignon ->
  if (digitalRead(bouton_champi) == LOW) { // si bouton Champignon est appuyé
    // J'ai eu bcp de mal pour l'affichage à cause du formatage, du coup mes variables heure1 s'afficheront en hexadécimal
    
    // on récupère le 1er chiffre dans $heure
    heure1 = 0;
    if (heure > 9 and heure < 20) {
      heure1 = 1;
    }
    if (heure > 19) {
      heure1 = 2;
    }
    
    // on récupère le 2e chiffre dans $heure
    heure2 = heure;
    while (heure2 > 9) { // boucle tant que > 9
      heure2 = (heure2 - 10);
    }
    
    
    // on récupère le 1er chiffre dans $minut
    minut1 = 0;
    if (minut > 9 and minut < 20) {
      minut1 = 1;
    }
    if (minut > 19 and minut < 30) {
      minut1 = 2;
    }
    if (minut > 29 and minut < 40) {
      minut1 = 3;
    }
    if (minut > 39 and minut < 50) {
      minut1 = 4;
    }
    if (minut > 49 and minut < 60) {
      minut1 = 5;
    }
    
    // on récupère le 2e chiffre dans $minut
    minut2 = minut;
    while (minut2 > 9) {
      minut2 = (minut2 - 10);
    }

    
    Serial.println("On appuie sur le chamignon -> Affichage de l'heure:");
    Serial.print("Il est ");
    Serial.print(heure1);
    Serial.print(heure2);
    Serial.print(minut1);
    Serial.println(minut2);
    Serial.println(" ");
  
    // si 12:00 -> on écrit "MIDI" sur le digit Sinon on affiche l'heure
    if (heure1 == 1 and heure2 == 2 and minut1 == 0 and minut2 == 0) {
      alpha4.writeDigitAscii(0, 'M');
      alpha4.writeDigitAscii(1, 'I');
      alpha4.writeDigitAscii(2, 'D');
      alpha4.writeDigitAscii(3, 'I');
      alpha4.writeDisplay();
    } else {
      // on affiche l'heure sur l'affichage
      if (heure1 != 0) { // si le premier chiffre est 0 (01 à 09h on affiche 1 à 9)
        alpha4.writeDigitAscii(0, (heure1 + 48)); // on ajoute 48 car il est affiché en hexadécimal
      } else {
        alpha4.writeDigitAscii(0, ' '); // si 0 -> on affiche rien
      }
      alpha4.writeDigitAscii(1, (heure2 + 48));
      alpha4.writeDigitAscii(2, (minut1 + 48));
      alpha4.writeDigitAscii(3, (minut2 + 48));
      alpha4.writeDisplay();
    }
    delay(2000); // on affiche l'heure durant 2 sec avant d'effacer le digit
    alpha4.writeDigitAscii(0, ' ');
    alpha4.writeDigitAscii(1, ' ');
    alpha4.writeDigitAscii(2, ' ');
    alpha4.writeDigitAscii(3, ' ');
    alpha4.writeDisplay();

    //si le champignon est tjs enfoncé ET aussi le bouton Setup -> on règle l'heure de l'alarme ###
    if (digitalRead(bouton_champi) == LOW and digitalRead(bouton_setup) == LOW) {
      Serial.println("Réglage de l'heure actuelle");
      heure1 = 0; // remise à zéro des heure et minute
      heure2 = 0;
      minut1 = 0;
      minut2 = 0;
      temps = 0; // servira à compter le temps de relache du bouton
      // on affiche les heures à 0 et on n'affiche pas les minutes
      alpha4.writeDigitAscii(0, '0');
      alpha4.writeDigitAscii(1, '0');
      alpha4.writeDigitAscii(2, ' ');
      alpha4.writeDigitAscii(3, ' ');
      alpha4.writeDisplay();
      reglage = 3; // si 3 -> réglage de l'heure de l'alarme
    } // fin de si chamignon est enfoncé ET le bouton setup
  } // fin de si chamignon est enfoncé


  while (reglage == 3) { // si 1 -> réglage des heures, de l'heure actuelle
    boolean etatBouton = digitalRead(bouton_setup);
    if (etatBouton == LOW) { // si bouton est appuyé
      //Serial.println("Pour ctrl reglage des heures de l'alarme"); // controle qu'on est bien en mode réglage de l'heure de l'alarme
      heure2 = (heure2 + 1); // on fait défiler les heures
      if (heure2 > 9) {
        heure1 = (heure1 + 1);
        heure2 = 0;
      }
      if (heure1 == 2 and heure2 > 3) { // mais évidemment pas au-delà de 23h..
        heure1 = 0;
        heure2 = 0;
      }
      temps = 0; // on initialise le temps de comptage (si pas d'appui sur le bouton durant un temps)
      alpha4.writeDigitAscii(0, (heure1 + 48)); // on affiche les heures sur le digit
      alpha4.writeDigitAscii(1, (heure2 + 48));
      alpha4.writeDisplay();
      delay(450); // délai entre les chiffres
    }
    temps = (temps + 1); // si pas d'appui sur le bouton -> on ajoute 1
    if (temps > 5000) { // si on attends trop, on passe au réglage des minutes
      temps = 0;
      // on affiche les heure à 0 et on n'affiche pas les minutes
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, '0');
      alpha4.writeDigitAscii(3, '0');
      alpha4.writeDisplay();
      delay(600); // attente avec les 00 minutes d'affichées
      reglage = 4; // on sort de la boucle et on ira dans la suivante
    }
  } // fin du réglage des heures actuelles


  while (reglage == 4) { // si 4 -> réglage des minutes de l'heure actuelle
    boolean etatBouton = digitalRead(bouton_setup);
    //Serial.print('x');
    if (etatBouton == LOW) { // si bouton est appuyé
      //Serial.println("Pour ctrl reglage des minutes de l'alarme"); // controler qu'on est bien en mode réglage des minutes de l'alarme
      minut2 = (minut2 + 1);
      if (minut2 > 9) {
        minut1 = (minut1 + 1);
        minut2 = 0;
      }
      if (minut1 >= 6) {
        minut1 = 0;
        minut2 = 0;
      }
      temps = 0;
      alpha4.writeDigitAscii(2, (minut1 + 48));
      alpha4.writeDigitAscii(3, (minut2 + 48));
      alpha4.writeDisplay();
      delay(450); // délai entre les chiffres
    }
    temps = (temps + 1);
    if (temps > 5000) { // si on attends trop, on sort du mode réglage
      temps = 0;
      heure = ((heure1 * 10) + (heure2));
      minut = ((minut1 * 10) + (minut2));
      rtc.adjust(DateTime(2018, 12, 31, heure, minut, 0)); // mise à jour des heures et minutes dans le module RTC
      reglage = 0; // on sort de la boucle et donc du mode réglage
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, ' ');
      alpha4.writeDigitAscii(3, ' ');
      alpha4.writeDisplay();
    }
  } // fin du réglage des minutes de l'heure actuelle




// Si on appuie sur le bouton alarme ->
  if (digitalRead(bouton_alarm) == LOW) { // si bouton alarme est appuyé
    Serial.println("On appuie sur le bouton de l'alarme -> Affichage de l'heure d'alarme:");
    Serial.print("L'alarme est prévue pour ");
    if (alarme == 1) { // si l'alarme est activée
      Serial.println(heure_alarme);
      Serial.println(" ");
  
      minut = (heure_alarme % 100); // on reprend les 2 premiers chiffres de heure_alarme
      heure = (heure_alarme / 100); // on reprend les 2 derniers chiffres de heure_alarme
      // J'ai eu bcp de mal pour l'affichage à cause du formatage, du coup mes variables heure1 s'afficheront en hexadécimal
  
      // on récupère le 1er chiffre dans $heure
      heure1 = 0;
      if (heure > 9 and heure < 20) {
        heure1 = 1;
      }
      if (heure > 19) {
        heure1 = 2;
      }
      
      // on récupère le 2e chiffre dans $heure
      heure2 = heure;
      while (heure2 > 9) { // boucle tant que > 9
        heure2 = (heure2 - 10);
      }
      
      
      // on récupère le 1er chiffre dans $minut
      minut1 = 0;
      if (minut > 9 and minut < 20) {
        minut1 = 1;
      }
      if (minut > 19 and minut < 30) {
        minut1 = 2;
      }
      if (minut > 29 and minut < 40) {
        minut1 = 3;
      }
      if (minut > 39 and minut < 50) {
        minut1 = 4;
      }
      if (minut > 49 and minut < 60) {
        minut1 = 5;
      }
      
      // on récupère le 2e chiffre dans $minut
      minut2 = minut;
      while (minut2 > 9) {
        minut2 = (minut2 - 10);
      }
  
      // on affiche l'heure de l'alarme sur l'affichage
      if (heure1 != 0) { // si le premier chiffre est 0 (01 à 09h on affiche 1 à 9)
        alpha4.writeDigitAscii(0, (heure1 + 48)); // on ajoute 48 car il est affiché en hexadécimal
      } else {
        alpha4.writeDigitAscii(0, ' '); // si 0 -> on affiche rien
      }
      alpha4.writeDigitAscii(1, (heure2 + 48));
      alpha4.writeDigitAscii(2, (minut1 + 48));
      alpha4.writeDigitAscii(3, (minut2 + 48));
      alpha4.writeDisplay();
    }else{ // si l'alarme est déactivée
      Serial.print("- STOP -");
      alpha4.writeDigitAscii(0, 'S'); // et on écrit "STOP" sur le digit
      alpha4.writeDigitAscii(1, 'T');
      alpha4.writeDigitAscii(2, 'O');
      alpha4.writeDigitAscii(3, 'P');
      alpha4.writeDisplay();
    }
    delay(2000); // on affiche l'heure durant 2 sec avant d'effacer le digit
    alpha4.writeDigitAscii(0, ' ');
    alpha4.writeDigitAscii(1, ' ');
    alpha4.writeDigitAscii(2, ' ');
    alpha4.writeDigitAscii(3, ' ');
    alpha4.writeDisplay();


    //si le bouton d'alarme est tjs enfoncé -> on active / désactive l'alarme
    if (digitalRead(bouton_alarm) == LOW) { // si bouton de ll'alarme est appuyé
      digitalWrite(buzzer,HIGH); // biiip
      delay(300);
      digitalWrite(buzzer,LOW);
      if (alarme == 1) { // si l'alarme est activée
        Serial.println("On desactive l'alarme");
        Serial.println(" ");
        alarme = 0; // on la désactive
        alpha4.writeDigitAscii(0, 'S'); // et on écrit "STOP" sur le digit
        alpha4.writeDigitAscii(1, 'T');
        alpha4.writeDigitAscii(2, 'O');
        alpha4.writeDigitAscii(3, 'P');
        alpha4.writeDisplay();
        delay(400);
      }else{
        Serial.println("On active l'alarme");
        Serial.println(" ");
        alarme = 1;
        alpha4.writeDigitAscii(0, 'A'); // ### MODIFIER POUR AFFICHER L'HEURE DE L'ALARME ###
        alpha4.writeDigitAscii(1, 'R');
        alpha4.writeDigitAscii(2, 'M');
        alpha4.writeDigitAscii(3, 'E');
        alpha4.writeDisplay();
        delay(400);
      }
      alpha4.writeDigitAscii(0, ' '); // ### MODIFIER POUR AFFICHER L'HEURE DE L'ALARME ###
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, ' ');
      alpha4.writeDigitAscii(3, ' ');
      alpha4.writeDisplay();
    }

  }


  
  boolean etatBouton = digitalRead(bouton_setup);
  if (etatBouton == LOW) { // si bouton est appuyé
    Serial.println("Réglage de l'alarme");
    if (alarme == 0) { // si l'alarme est déactivée
      alarme = 1; // on l'active
      Serial.println("L'alarme étant désactivée -> on l'a activée");
    }
    heure1 = 0; // remise à zéro des heure et minute
    heure2 = 0;
    minut1 = 0;
    minut2 = 0;
    temps = 0; // servira à compter le temps de relache du bouton
    // on affiche les heures à 0 et on n'affiche pas les minutes
    alpha4.writeDigitAscii(0, '0');
    alpha4.writeDigitAscii(1, '0');
    alpha4.writeDigitAscii(2, ' ');
    alpha4.writeDigitAscii(3, ' ');
    alpha4.writeDisplay();
    reglage = 1; // si 1 -> réglage des sheure de l'alarme
  }

  while (reglage == 1) { // si 1 -> réglage des heures de l'alarme
    boolean etatBouton = digitalRead(bouton_setup);
    if (etatBouton == LOW) { // si bouton est appuyé
      heure2 = (heure2 + 1); // on fait défiler les heures
      if (heure2 > 9) {
        heure1 = (heure1 + 1);
        heure2 = 0;
      }
      if (heure1 == 2 and heure2 > 3) { // mais évidemment pas au-delà de 23h..
        heure1 = 0;
        heure2 = 0;
      }
      temps = 0; // on initialise le temps de comptage (si pas d'appui sur le bouton durant un temps)
      alpha4.writeDigitAscii(0, (heure1 + 48)); // on affiche les heures sur le digit
      alpha4.writeDigitAscii(1, (heure2 + 48));
      alpha4.writeDisplay();
      delay(450); // délai entre les chiffres
    }
    temps = (temps + 1); // si pas d'appui sur le bouton -> on ajoute 1
    if (temps > 5000) { // si on attends trop, on passe au réglage des minutes
      temps = 0;
      // on affiche les heure à 0 et on n'affiche pas les minutes
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, '0');
      alpha4.writeDigitAscii(3, '0');
      alpha4.writeDisplay();
      delay(600); // attente avec les 00 minutes d'affichées
      reglage = 2; // on sort de la boucle et on ira dans la suivante
    }
  } // fin du réglage des heures de l'alarme


  while (reglage == 2) { // si 2 -> réglage des minutes de l'alarme
    boolean etatBouton = digitalRead(bouton_setup);
    //Serial.print('x');
    if (etatBouton == LOW) { // si bouton est appuyé
      //Serial.println('M'); // Affiche M dans la console pour controler qu'on est bien en mode réglage des minutes
      minut2 = (minut2 + 5);
      if (minut2 > 9) {
        minut1 = (minut1 + 1);
        minut2 = 0;
      }
      if (minut1 >= 6) {
        minut1 = 0;
        minut2 = 0;
      }
      alpha4.writeDigitAscii(2, (minut1 + 48));
      alpha4.writeDigitAscii(3, (minut2 + 48));
      alpha4.writeDisplay();
      delay(450); // délai entre les chiffres
    }
    temps = (temps + 1);
    if (temps > 5000) { // si on attends trop, on sort du mode réglage
      temps = 0;
      heure_alarme = ((heure1 * 1000)+(heure2 * 100) + (minut1 * 10) + minut2); // ex: 2359
      reglage = 0; // on sort de la boucle et donc du mode réglage
      alpha4.writeDigitAscii(0, ' ');
      alpha4.writeDigitAscii(1, ' ');
      alpha4.writeDigitAscii(2, ' ');
      alpha4.writeDigitAscii(3, ' ');
      alpha4.writeDisplay();
    }
  } // fin du réglage des minutes de l'alarme

  //si on veut maintenir un témoin si l'alarme est activée (le point)
  //if (alarme == 1) {//si l'alarme est activée -> on affiche juste le point
  //  alpha4.writeDigitAscii(3, 14); // le point pour le temoin d'alarme
  //}
  //alpha4.writeDisplay();

  delay(10); // pause
}

