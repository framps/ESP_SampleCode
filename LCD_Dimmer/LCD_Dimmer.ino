#define LED_RED 32
#define LED_GREEN 33

void DimmerTask(void *parameter) {
  for (;;) {
    Serial.printf("*** Dimmertask *** in core %i\n",xPortGetCoreID());
    yield();
    delay(1000);
  }
}

class LCD_Dimmer { 
  public:
    LCD_Dimmer(BaseType_t xCoreID, float dutyCycle, int period);
    void enable();
    void disable();
    void setDutyCycle(float dutyCycle);
    void setPeriod(int period);
    void set(float dutyCycle, int period);
    ~LCD_Dimmer();
  private:          
    BaseType_t _xCoreID; // coreid
    int _period;     // ms
    float _dutyCycle; // %
    TaskHandle_t _taskHandle;
};

// ctor
LCD_Dimmer::LCD_Dimmer(BaseType_t xCoreID, float dutyCycle, int period) {
    _period = period;
    _dutyCycle = dutyCycle;
    _xCoreID = xCoreID;
}
// dtor
LCD_Dimmer::~LCD_Dimmer() {
    disable();
}

// enable LCD dimming
void LCD_Dimmer::enable() {
    xTaskCreatePinnedToCore(DimmerTask,"LCD_Dimmer",1000,NULL,1,&_taskHandle,_xCoreID);
    Serial.printf("Enabled dimmer in core %i\n",_xCoreID); 
}

// disable LCD dimming
void LCD_Dimmer::disable() {
  vTaskDelete(_taskHandle);
  Serial.printf("Disabled dimmer in core %i\n",_xCoreID); 
}

void LCD_Dimmer::set(float dutyCycle, int period) {
    setDutyCycle(dutyCycle);
    setPeriod(period);
}

void LCD_Dimmer::setDutyCycle(float dutyCycle) {
    _dutyCycle=dutyCycle;
}

void LCD_Dimmer::setPeriod(int period) {
    _period=period;
}

LCD_Dimmer *lcdDimmer;

void setup() {

  Serial.begin(9600);  
  
  lcdDimmer= new LCD_Dimmer(0,50.0,100);
  lcdDimmer->enable();
  delay(10000);
  lcdDimmer->disable();
}

void loop() {
  Serial.println("*** Loop ***");
  if (lcdDimmer != NULL) {
    Serial.println("*** Deleted dimmer ***");
    delete lcdDimmer;
    lcdDimmer = NULL;
  } 
  else {
    Serial.println("*** No dimmer any more ***");    
  }
  delay(1000);
}
