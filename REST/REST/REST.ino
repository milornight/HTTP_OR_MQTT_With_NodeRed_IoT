#include <Arduino_JSON.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>

#define DHTPIN 23 //Pin pour connecter la data du DHT11
#define DHTTYPE DHT11 
#define LEDPIN 2 //pin de LED interne 

DHT dht(DHTPIN, DHTTYPE);

// spécifier le nom et le mot de passe de wifi
const char* ssid = "Mi";
const char* password =  "huihui123";

// définir un nom de device
String device_label = "ESP32_DHT11_JY";

// Entrer votre adresse IP + 1880 + nom de URL
String serverNamePost = "http://192.168.43.20:1880/post-dht11";
String serverNameGet = "http://192.168.43.20:1880/get-dht11";

String Switch;
int ValSwitch; 


// fonction pour lire la valeur de humidité
float ReadHum(){
  float hum = dht.readHumidity();
  if (isnan(hum)) {    
    Serial.println("erreur de lire");
    return NULL;
  }
  else {
    Serial.println("Humidity:");
    Serial.println(hum);
    return hum;
  }
}


// fonction pour lire la valeur de température
float ReadTemp(){
  float temp = dht.readTemperature();
  if (isnan(temp)) {    
    Serial.println("erreur de lire");
    return NULL;
  }
  else {
    Serial.println("Temperature:");
    Serial.println(temp);
    return temp;
  }   
}


void setup() {
  
  Serial.begin(115200);
  delay(4000);   //un délai nécessaire avant de la connection du wifi
  
  WiFi.begin(ssid, password); // connecter le wifi

  //verification de la connection
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  dht.begin(); // mettre à jour le DHT11
  pinMode(LEDPIN,OUTPUT); //configurer la sortie du LED
}

  
void loop() {

  // vérification de l'état du wifi
  if(WiFi.status()== WL_CONNECTED){   
  
    HTTPClient http; 
    WiFiClient client;
    http.begin(client, serverNamePost); // associer à l'adresse du POST
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Spécifer le type de tête du HTTP
    
    float hum = ReadHum(); //lire la valeur de l'humidité
    float temp = ReadTemp(); //lire la valeur de température

    String s = "api_key=" + device_label + "&sensor=DHT11"+ "&humidity=" + String(hum) + "&sensor=DHT11" + "&temperature=" + String(temp);

    int httpResponseCodePost = http.POST(s); //envoyer la demande de POST

    // vérification de la succès de l'envoie 
    if(httpResponseCodePost>0){
      String response = http.getString(); //Obtenez la réponse à la demande de POST
      Serial.println(response); //Afficher la réponse à la demande de POST
    }else{
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCodePost);
    }
  
    http.end();  //libérer la source
    
    http.begin(client, serverNameGet); //associer à l'adresse du GET
    int httpResponseCodeGet = http.GET(); //envoyer la demande de GET
    JSONVar myObject;
    JSONVar keys;

    // vérification de la succès de l'envoie 
    if (httpResponseCodeGet>0) {
      Serial.print("HTTP Response code for get: ");
      Serial.println(httpResponseCodeGet); //Afficher la réponse à la demande de GET
      Switch = http.getString(); // Stocker la valeur demandé
      Serial.println(Switch); 
      myObject = JSON.parse(Switch); 

      // vérification du type de valeur obtenu
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Echec de parser input!");
        return;
      }
    
      Serial.print("JSON object = ");
      Serial.println(myObject);
      keys = myObject.keys(); //obternir un array de tous les keys dans l'objet
     
    } else {
      Serial.print("Error Get: ");
      Serial.println(httpResponseCodeGet);
    }

    // pour obtenir la valeur switch
    for (int i = 0; i < keys.length(); i++) {
      JSONVar value = myObject[keys[i]];
      Serial.print(keys[i]);
      Serial.print(" = ");
      Serial.println(value);
      ValSwitch = (int)value;

      // mettre à jour la LED
      if(ValSwitch == 1){
        digitalWrite(LEDPIN, HIGH);
      }else{
        digitalWrite(LEDPIN, LOW); 
      }        
    }

    http.end(); 

  }else{
  
  Serial.println("Error for WiFi connection");   
  
  }

  delay(5000);  //Envoyer la demande tous les 5 seconds

}
