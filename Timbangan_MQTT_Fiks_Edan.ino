 // Include library 
 
 #include "HX711.h" //import Library
 #include <PubSubClient.h> // import Library
 #include <ESP8266WiFi.h> // import Library
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Connection parameter
 * -Wifi -> internet connection foor data communication
 * -RabbitMQ/MQTT -> protoocol data communication
 */
const char* wifiSsid              = "LSKKHomeAuto"; // Deklarasi nama Wifi
const char* wifiPassword          = "1234567890"; // Deklarasi untuk Password Wifi
const char* mqttHost              = "rmq2.pptik.id"; // Deklarasi untuk link yang akan dituju
const char* mqttUserName          = "/survey:survey"; // Deklarasi untuk nama UserName DI RMQ
const char* mqttPassword          = "$surv3yy!"; // Deklarasi untuk Passwordnya di RMQ
//const char* mqttClient            = "IOT-Water-Pumpp"; 
const char* mqttQueueTimbangan       = "Timbangan"; // Deklarasi untuk nama Queue di RMQ

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*7
 * Device parameter
 * -Guid -> Id Device (unique) you can generate from here (https://www.uuidgenerator.net/version1) 
 * -status device -> save last state from the pump is on or off (1 = on , 0 = off) 
 * -pin microcontroller 
 * -mac device
 * 
 */
int deviceDout                   = 16; // Deklarasi Pin 4 (16=4)
int deviceSck                    = 14; // Deklarasi Pin 12 (14=5)
String deviceGuid                = "735a1d19-dd67-4e2e-9fff-cf7287123357"; //3828a5de-5475-42cb-b355-39cccb1d2157 (Boya)/galon
                                                                          //c35fb23b-d6be-4bb3-8b3f-6f1ec37f696a (Akew)/gas
                                                                         //735a1d19-dd67-4e2e-9fff-cf7287123357 (Hasan)/korsi
HX711 scale(deviceDout, deviceSck); 
float calibration_factor = 18650; // Memasukan Nilai "-18650" ke Varibel calibration_factor dalam tipe data Float


/*
 * Wifi setup WiFi client and mac address 
 */
WiFiClient espClient;
PubSubClient client(espClient);
byte mac[6]; //array temp mac address //mac memakai 6 byte
String MACAddress; // dari byte dirubah jadi string
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Set up WiFi connection
 */
 void setup_wifi(){
  delay(10); //Delay waktu 0,01 detik
  //We start by connecting to a WiFi network
  Serial.println(); // Tampilkan Di serial monitor
  Serial.print("Connecting to :"); //Tampilkan di serial monitor "Connecting to :"
  Serial.println(wifiSsid); // Tampilkan wifiSsid
  WiFi.begin(wifiSsid, wifiPassword); // Masukaan wifi dengan nama dan passwordnya
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // delay 0,5 detik
    Serial.print("."); // tampilkan " . "
  }
  Serial.println(""); //"tampilkan ""
  Serial.println("WiFi connected");// Tampilkan Wifi Connected
  Serial.println("IP address: "); // Tampilkan Ip Adrees :
  Serial.println(WiFi.localIP()); // Menampilkan Ip 
 }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Get Mac Address from microcontroller
 */
 
String mac2String(byte ar[]) { 
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Print Mac Address 
 */
 void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress); // Tampilkan MacAddress
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * Function for Get message payload from MQTT rabbit mq
 */
void callback(char* topic, byte* payload, unsigned int length){
  char message[5]; //variable for temp payload message
  Serial.print("Message arrived in topic: "); // Tampilkan Message arrived in topic
  Serial.println(topic); // tampilkan topic
  Serial.print("Messagge :"); // tampilkan Message :
  for(int i = 0;i < length;i++){
    Serial.print((char)payload[i]);
    message[i] = (char)payload[i]; //initiate value from payload to message variable
    
  }
  
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Function for Reconnecting to MQTT/RabbitMQ 
 */
void reconnect() {
  // Loop until we're reconnected
  printMACAddress();
  const char* CL;
  CL = MACAddress.c_str();
  Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqttUserName, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 * Function for Setup Pin Mode,wifi,mqtt,and serial
 */
void setup()
{
  pinMode(deviceDout, INPUT);
  Serial.begin(115200);
  setup_wifi();
  printMACAddress();
  client.setServer(mqttHost, 1883);
  client.setCallback(callback);
  scale.set_scale(); //229455
  // 850000  
  scale.tare(); //Reset the scale to 0
 
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



/*
 * This functioon for loop your program
 */
void loop() {
  //if you disconnected from wifi and mqtt
  if (!client.connected()) {
    reconnect();
  }
scale.set_scale(calibration_factor); //Adjust to this calibration factor 
float dataWeight;

 dataWeight = ((scale.get_units()));
 String convertDataWeight = String(dataWeight);
 String dataRMQ = String(convertDataWeight + "#" + deviceGuid );
 char dataToMQTT[50];
 dataRMQ.toCharArray(dataToMQTT, sizeof(dataToMQTT)); 
 Serial.println("Ini Data untuk ke MQTT: ");
 Serial.println(dataToMQTT);
 //Serial.println(" Kg");

 client.publish(mqttQueueTimbangan,dataToMQTT);
  client.loop();
  delay(2000); //300000
}
