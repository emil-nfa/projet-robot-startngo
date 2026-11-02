#include "print.h"
#include "rle.h"

// Programme RLE - SNGO 2024 - V2 - 2/10/2024 

void setup() {
  T_img img;
  byte v; 
  // initialize both serial ports:
  Serial.begin(115200); // interface Arduino - Moteur
  Serial1.begin(115200); // interface Arduino - PC à l'aide du câble Câble TTL-232R-RPI

  //img = getImg(t33);
  //printImg(img);
  //LPRINTLN("largeur",img.format.largeur);
  //LPRINTLN("hauteur",img.format.hauteur);
  //LPRINTLN("première couleur",COLPIX(img.pixels[0]));
  
  //v=9;
  //LPRINTLN("affichage binaire", v);
  //printByte(v); NL();
}


void loop() {
  T_img img;
  char * s; 
  static int flagPrompt = 1; 
  
   
  /* // tester avec :
  3:3:1:3:1:1:2:1:1
  10:10:0:14:2:7:4:5:6:3:2:1:2:1:2:2:8:4:1:2:1:5:1:1:2:1:1:3:1:1:1:2:1:1:1:11
  50:64:0:24:2:44:10:38:14:34:18:31:20:30:20:29:22:27:23:27:24:26:24:25:25:25:26:24:26:24:26:24:26:24:26:24:25:26:24:26:24:26:24:27:22:29:20:31:18:30:22:27:24:25:26:22:30:20:30:19:31:20:30:20:29:22:28:22:28:24:1:3:16:1:1:32:16:33:17:33:18:32:18:32:18:31:20:30:20:29:22:27:24:26:25:23:28:21:30:19:32:16:36:13:38:10:41:9:42:7:44:6:44:5:46:3:48:2:48:2:48:2:48:2:48:1:250
  */

  if (flagPrompt) {
    PRINTLN("En attente d'une chaine");
    flagPrompt = 0; 
  }
  if (AVAIL()) {
    s = lireChaine(); 
    DLPRINTLN("chaine lue",s);
    img = getImg(s);
    printImg(img);
    LPRINTLN("largeur",img.format.largeur);
    LPRINTLN("hauteur",img.format.hauteur);
    LPRINTLN("première couleur",COLPIX(img.pixels[0]));
    flagPrompt = 1;
  }


}

