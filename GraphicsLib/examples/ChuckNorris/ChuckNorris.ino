/*
  Chuck Norris Facts
 */

#include <Wire.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <digitalWriteFast.h>
#include <GraphicsLib.h>
#include <MI0283QT2.h>
#include <MI0283QT9.h>
#include <DisplaySPI.h>
#include <DisplayUART.h>
#include <DisplayI2C.h>


//Declare only one display !
// MI0283QT2 lcd;  //MI0283QT2 Adapter v1
// MI0283QT9 lcd;  //MI0283QT9 Adapter v1
// DisplaySPI lcd; //SPI (GLCD-Shield or MI0283QT Adapter v2)
// DisplayUART lcd; //UART (GLCD-Shield or MI0283QT Adapter v2)
 DisplayI2C lcd; //I2C (GLCD-Shield or MI0283QT Adapter v2)

uint8_t curr_fact=0;

#define FACTS (50)
const char cfn01[] PROGMEM = "Chuck Norris kann aus einem Word-Dokument einen Papierflieger bauen.";
const char cfn02[] PROGMEM = "Chuck Norris kann Minutenreis in 30 Sekunden kochen.";
const char cfn03[] PROGMEM = "Wenn Chuck Norris Saenger geworden waere, dann haette er 11 Top-Ten-Hits - gleichzeitig!";
const char cfn04[] PROGMEM = "Chuck Norris benutzt keine Koerperlotion - er haeutet sich mehrmals jaehrlich.";
const char cfn05[] PROGMEM = "Wenn Chuck Norris Liegestuetze macht, drueckt er nicht sich nach oben, sondern die Welt nach unten.";
const char cfn06[] PROGMEM = "Chuck Norris' Traenen heilen Krebs. Zu schade dass er noch nie geweint hat.";
const char cfn07[] PROGMEM = "Chuck Norris hat mal ein Flugzeug abgeschossen, indem er mit dem Finger draufgezeigt und PENG gerufen hat.";
const char cfn08[] PROGMEM = "Die Top3 Todesursachen in den USA: 1. Herzinfarkt - 2. Chuck Norris - 3. Krebs";
const char cfn09[] PROGMEM = "Chuck Norris hat unter seinem Bart kein Kinn, sondern eine dritte Faust.";
const char cfn10[] PROGMEM = "Brokeback Mountain ist der Name des Bergs toter Ninjas vor Chuck Norris' Haus.";
const char cfn11[] PROGMEM = "Chuck Norris schlaeft nicht. Er wartet.";
const char cfn12[] PROGMEM = "Wenn Chuck Norris ins Wasser faellt, wird Chuck Norris nicht nass. Wasser wird Chuck Norris.";
const char cfn13[] PROGMEM = "Chuck Norris hat bis zur Unendlichkeit gezaehlt. Zweimal.";
const char cfn14[] PROGMEM = "Chuck Norris kann eine Drehtuer zuschlagen dass es knallt.";
const char cfn15[] PROGMEM = "Chuck Norris traegt keine Uhr, er bestimmt wie spaet es ist.";
const char cfn16[] PROGMEM = "Wenn Chuck Norris eine 0900er-Nummer anruft muss er nichts bezahlen. Er haelt den Hoerer hoch und das Geld faellt heraus.";
const char cfn17[] PROGMEM = "Der Helikopter wurde erfunden nachdem jemand gesehen hat wie Chuck Norris 8 Roundhousekicks in einer Sekunde ausfuehrte.";
const char cfn18[] PROGMEM = "Chuck Norris hat mal eine ganze Torte gegessen bevor seine Freunde ihm sagen konnten dass da eine Striperin drin war.";
const char cfn19[] PROGMEM = "Chuck Norris kann durch Null dividieren.";
const char cfn20[] PROGMEM = "Es gibt keine Rassen, nur Laender mit Leuten die Chuck Norris in verschiedene Schwarz- und Blautoene gepruegelt hat.";
const char cfn21[] PROGMEM = "Wilt Chamberlain behauptet, mit 20.000 Frauen geschlafen zu haben. Chuck Norris nennt das einen 'lahmen Dienstag'.";
const char cfn22[] PROGMEM = "Chuck Norris hat Schwarz erfunden. Eigentlich hat er das gesamte Farbspektrum erfunden. Ausser Pink. Pink wurde von Tom Cruise erfunden.";
const char cfn23[] PROGMEM = "Am Anfang war nichts...dann hat Chuck Norris diesem Nichts einen Roundhousekick verpasst und gesagt 'Besorg dir nen Job'. Das ist die Story des Universums.";
const char cfn24[] PROGMEM = "Chuck Norris hat einen Big Mac bei Burger King bestellt. Er bekam einen.";
const char cfn25[] PROGMEM = "Chuck Norris hat mal eine ganze Flasche Schlaftabletten geschluckt. Er musste davon blinzeln.";
const char cfn26[] PROGMEM = "Chuck Norris hat Russisches Roulette mit einer voll geladenen Kanone gespielt und gewonnen.";
const char cfn27[] PROGMEM = "Manche Leute tragen Superman-Schlafanzuege. Superman traegt Chuck-Norris-Schlafanzuege.";
const char cfn28[] PROGMEM = "Indem er einfach an den Enden zieht kann Chuck Norris aus Diamanten wieder Kohle machen.";
const char cfn29[] PROGMEM = "Superman hat einmal eine Folge von 'Walker, Texas Ranger' gesehen. Dann hat er sich in den Schlaf geweint.";
const char cfn30[] PROGMEM = "Chuck Norris kann Ameisen mit einer Lupe entzuenden. Nachts.";
const char cfn31[] PROGMEM = "Chuck Norris musste damit aufhoeren seine Kleidung im Meer zu waschen, die Tsunamis haben zuviele Leute das Leben gekostet.";
const char cfn32[] PROGMEM = "Zaehle von 1 bis 10. Solange braucht Chuck Norris um dich zu toeten. 47 mal.";
const char cfn33[] PROGMEM = "Chuck Norris benutzt kein Kondom. Es gibt keinen Schutz vor Chuck Norris.";
const char cfn34[] PROGMEM = "Die meisten harten Maenner essen Naegel zum Fruehstueck. Chuck Norris kauft seine gesamten Lebensmittel im Baumarkt.";
const char cfn35[] PROGMEM = "Chuck Norris' Puls wird mit der Richterskala gemessen.";
const char cfn36[] PROGMEM = "Chuck Norris hat mal einen Flammenwerfer erschaffen indem er auf ein Feuerzeug gepisst hat.";
const char cfn37[] PROGMEM = "Chuck Norris kennt die letzte Zahl von Pi.";
const char cfn38[] PROGMEM = "Wer auch immer gesagt hat 'Nur die besten sterben jung' ging wohl gemeinsam mit Chuck Norris zum Kindergarten.";
const char cfn39[] PROGMEM = "Chuck Norris besitzt IBM. Es war eine extrem feindliche uebernahme.";
const char cfn40[] PROGMEM = "Chuck Norris kann einem Auto Starthilfe geben indem er das Kabel an seine Nippel haelt.";
const char cfn41[] PROGMEM = "Es gibt keine Evolutionstheorie. Nur eine Liste mit Lebewesen denen Chuck Norris erlaubt zu ueberleben.";
const char cfn42[] PROGMEM = "Bei seiner Geburt war der Doktor der einzige der geweint hat. Gib niemals Chuck Norris einen Klaps.";
const char cfn43[] PROGMEM = "Chuck Norris kann mit offenen Augen niesen.";
const char cfn44[] PROGMEM = "Chuck Norris braucht ein Stunt Double fuer Szenen in denen er weinen muss.";
const char cfn45[] PROGMEM = "Der Notfallplan der Erde bei einer Attacke von Ausserirdischen heisst Chuck Norris.";
const char cfn46[] PROGMEM = "Chuck Norris hat 8 Mahlzeiten am Tag. 7 mal Steak und dann den Rest der Kuh.";
const char cfn47[] PROGMEM = "Die aktive Substanz in Red Bull ist Schweiss von Chuck Norris.";
const char cfn48[] PROGMEM = "Wenn Chuck Norris bereit ist aufzuwachen, sagt er der Sonne dass sie am Horizont erscheinen soll.";
const char cfn49[] PROGMEM = "Chuck Norris kann seine Luft fuer 9 Jahre anhalten.";
const char cfn50[] PROGMEM = "Barthaare von Chuck Norris werden in China als Aphrodisiakum verkauft.";

PGM_P chucknorris[FACTS] = 
{
  cfn01,cfn02,cfn03,cfn04,cfn05,cfn06,cfn07,cfn08,cfn09,cfn10,
  cfn11,cfn12,cfn13,cfn14,cfn15,cfn16,cfn17,cfn18,cfn19,cfn20,
  cfn21,cfn22,cfn23,cfn24,cfn25,cfn26,cfn27,cfn28,cfn29,cfn30,
  cfn31,cfn32,cfn33,cfn34,cfn35,cfn36,cfn37,cfn38,cfn39,cfn40,
  cfn41,cfn42,cfn43,cfn44,cfn45,cfn46,cfn47,cfn48,cfn49,cfn50
};


uint8_t random_fact(void)
{
  int i, j;

  i = rand();
  while(i >= FACTS)
  {
    j = rand();
    while(j > i)
    {
      j /= 10;
    }
    i -= j;
  }

  return i;
}


void draw_fact(void)
{
  char tmp[256];
  
  lcd.fillScreen(RGB(255,255,255));

  lcd.drawTextPGM(5, 2, PSTR("Chuck Norris Fact:"), RGB(0,0,0), RGB(255,255,255), 2);

  lcd.setCursor(2, 25);
  lcd.setTextColor(RGB(200,0,0), RGB(255,255,255));
  lcd.setTextSize(2);
  strcpy_P(tmp, chucknorris[curr_fact]);
  lcd.print(tmp);
}


void setup()
{
  //init display
  lcd.begin();
  //SPI Displays
  // lcd.begin(); //spi-clk=Fcpu/4
  // lcd.begin(SPI_CLOCK_DIV2); //spi-clk=Fcpu/2
  // lcd.begin(SPI_CLOCK_DIV16); //spi-clk=Fcpu/16
  // lcd.begin(SPI_CLOCK_DIV4, 8); //spi-clk=Fcpu/4, rst-pin=8
  //UART Displays
  // lcd.begin(5, 6, 7); //rx-pin=5, tx-pin=6, cs-pin=7
  // lcd.begin(5, 6, 7, 8); //rx-pin=5, tx-pin=6, cs-pin=7, rst-pin=8
  //I2C Displays
  // lcd.begin(0x20); //addr=0x20
  // lcd.begin(0x20, 8); //addr=0x20, rst-pin=8

  //calibrate touchpanel
  lcd.touchStartCal();

  //random fact at start-up
  srand(666);
  curr_fact = random_fact();
  draw_fact();
}


void loop()
{
  if(lcd.touchRead()) //touch press? (lcd.touchRead() = service routine for touch panel)
  {
    if(lcd.touchX() < (lcd.getWidth()/2))
    {
      if(curr_fact > 0)
      {
        curr_fact--;
      }
      else
      {
        curr_fact = FACTS-1;
      }
    }
    else
    {
      if(curr_fact < (FACTS-1))
      {
        curr_fact++;
      }
      else
      {
        curr_fact = 0;
      }
    }

    draw_fact();
    delay(500);
  }
}
