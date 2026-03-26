
//A4 (SDA) SDA del Si5351 
//A5 (SCL) SCL del Si5351 
#include "si5351.h"
#include <Wire.h>
Si5351 si5351;

const int pwmPin = 3; //pwm per modulare 


//enum StatoSistema {HOME, SET_RF, SET_PWM, TX_RF}; // bella invece che #define , semantica tipata, scolastica 
//StatoSistema statoAttuale = HOME ;                
// alternativa c
#define HOME '0';
#define SET_RF '1';
#define SET_PWM '2';
#define TX_RF '3';



void setup() {
  Serial.begin(9600);
  Wire.begin();
  delay(500);

  Serial.print(F("Iniz. Radio... "));

  if (si5351.init(SI5351_CRYSTAL_LOAD_10PF, 0, 0)) {
      Serial.println("OK: Si5351 inizializzato");
  } else {
      Serial.println("Err: Si5351 non trovato _STOP_");
      while (1); // Blocca se chippino non risponde
  }

  //  potenza di uscita (2mA, 4mA, 6mA o 8mA)
  // 2mA è di default ed è più pulito per esperimenti RF vicini
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
  delay(100);
  Serial.println("OK");
  
  mostraMenu();
}

void mostraMenu() {
    Serial.println("[1]SET_RF , [2]SET_PWM ");  // qui si drovrebbe usare la struttura enum dichiarata scolastimente per un motivo eheh
  
}



void loop() {
  
  // legge da serial comandi menu
    if (Serial.available() > 0 ) {
      
      eseguiAzione(Serial.read());
    
    } else { 
        // panic
      // usiamo il led col codice morse?
    }
}



void eseguiAzione(char statoAttuale) {
    switch (statoAttuale) {
        case '1': //SET_RF
            // 1. Pulizia buffer iniziale per eliminare l' '1' premuto prima
            delay(100); 
            while(Serial.available() > 0) Serial.read(); // questo cosa fa? svuota il buffer di caratteri successivi//

            Serial.println(F("\n[RADIO] Inserisci frequenza in kHz (es. 800):"));
    
            long freqTargetKHz = 0;

            // 2. Ciclo di attesa 
            while (freqTargetKHz <= 0) {
              if (Serial.available() > 0) {
                  freqTargetKHz = Serial.parseInt();
            
                  // Se l'utente preme invio a vuoto o parseInt fallisce (0)
                  // il ciclo while ricomincia e aspetta ancora.
              }
            }

            // 3. Controllo Range
            if (freqTargetKHz < 8 || freqTargetKHz > 160000) {
                Serial.println(F("Err: Frequenza fuori range (8kHz-160MHz)."));
                statoAttuale = HOME;    
                break;
            }

            // --- CONFIGURAZIONE ETHERKIT ---freq in centesimi di Hertz
            unsigned long long freqFinal = (unsigned long long)freqTargetKHz * 100000ULL;
            si5351.set_freq(freqFinal, SI5351_CLK0);
            si5351.output_enable(SI5351_CLK0, 1);
    
            Serial.print(F("TX ON: ")); Serial.print(freqTargetKHz); 
            Serial.println(F(" kHz. 'X' per stop."));

            // --- CICLO MODULAZIONE TONI ---
            bool continuaTX = true;
             
            while(continuaTX) {
                int toni[] = {440, 660, 880, 1200};
                for (int i = 0; i < 4; i++) {
                    tone(pwmPin, toni[i]);
                    unsigned long startMillis = millis();
                    while(millis() - startMillis < 300) {
                        if (Serial.available() > 0) {
                            char esc = toupper(Serial.read()); // toupper accetta sia 'x' che 'X'
                            if (esc == 'X' || esc == '6') {
                                continuaTX = false;
                                break;
                            }
                        }
                    } //end while
                    if (!continuaTX) break;
                } // end for
             } // end while TX
          
    
            noTone(pwmPin);
            si5351.output_enable(SI5351_CLK0, 0);
            Serial.println(F("[RADIO] OFF"));
            statoAttuale = HOME;
            mostraMenu(); 
            break;
   

        case '2': //SET_PWM: 
             
            break;
        case '3': //TX_RF: 
             
            break;
      } // end switch case
}
  





